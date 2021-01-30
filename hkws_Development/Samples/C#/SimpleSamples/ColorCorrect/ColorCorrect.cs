using System;
using System.Collections.Generic;
using MvCamCtrl.NET;
using System.Runtime.InteropServices;
using System.IO;

namespace ColorCorrect
{
    class ColorCorrect
    {
        static void Main(string[] args)
        {
            int nRet = MyCamera.MV_OK;
            MyCamera device = new MyCamera();
            IntPtr pDstData = IntPtr.Zero;
            UInt32 nDstDataSize = 0;

            IntPtr pCLUTData = IntPtr.Zero;
            UInt32 nCLUTDataSize = 0;

            do
            {
                // ch:枚举设备 | en:Enum deivce
                MyCamera.MV_CC_DEVICE_INFO_LIST stDevList = new MyCamera.MV_CC_DEVICE_INFO_LIST();
                nRet = MyCamera.MV_CC_EnumDevices_NET(MyCamera.MV_GIGE_DEVICE | MyCamera.MV_USB_DEVICE, ref stDevList);
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Enum device failed:{0:x8}", nRet);
                    break;
                }
                Console.WriteLine("Enum device count :{0} \n", stDevList.nDeviceNum);
                if (0 == stDevList.nDeviceNum)
                {
                    break;
                }

                MyCamera.MV_CC_DEVICE_INFO stDevInfo;

                // ch:打印设备信息 | en:Print device info
                for (Int32 i = 0; i < stDevList.nDeviceNum; i++)
                {
                    stDevInfo = (MyCamera.MV_CC_DEVICE_INFO)Marshal.PtrToStructure(stDevList.pDeviceInfo[i], typeof(MyCamera.MV_CC_DEVICE_INFO));

                    if (MyCamera.MV_GIGE_DEVICE == stDevInfo.nTLayerType)
                    {
                        MyCamera.MV_GIGE_DEVICE_INFO stGigEDeviceInfo = (MyCamera.MV_GIGE_DEVICE_INFO)MyCamera.ByteToStruct(stDevInfo.SpecialInfo.stGigEInfo, typeof(MyCamera.MV_GIGE_DEVICE_INFO));
                        uint nIp1 = ((stGigEDeviceInfo.nCurrentIp & 0xff000000) >> 24);
                        uint nIp2 = ((stGigEDeviceInfo.nCurrentIp & 0x00ff0000) >> 16);
                        uint nIp3 = ((stGigEDeviceInfo.nCurrentIp & 0x0000ff00) >> 8);
                        uint nIp4 = (stGigEDeviceInfo.nCurrentIp & 0x000000ff);
                        Console.WriteLine("[device " + i.ToString() + "]:");
                        Console.WriteLine("DevIP:" + nIp1 + "." + nIp2 + "." + nIp3 + "." + nIp4);
                        Console.WriteLine("UserDefineName:" + stGigEDeviceInfo.chUserDefinedName + "\n");
                    }
                    else if (MyCamera.MV_USB_DEVICE == stDevInfo.nTLayerType)
                    {
                        MyCamera.MV_USB3_DEVICE_INFO stUsb3DeviceInfo = (MyCamera.MV_USB3_DEVICE_INFO)MyCamera.ByteToStruct(stDevInfo.SpecialInfo.stUsb3VInfo, typeof(MyCamera.MV_USB3_DEVICE_INFO));
                        Console.WriteLine("[device " + i.ToString() + "]:");
                        Console.WriteLine("SerialNumber:" + stUsb3DeviceInfo.chSerialNumber);
                        Console.WriteLine("UserDefineName:" + stUsb3DeviceInfo.chUserDefinedName + "\n");
                    }
                }

                Int32 nDevIndex = 0;
                Console.Write("Please input index(0-{0:d}):", stDevList.nDeviceNum - 1);
                try
                {
                    nDevIndex = Convert.ToInt32(Console.ReadLine());
                }
                catch
                {
                    Console.Write("Invalid Input!\n");
                    break;
                }

                if (nDevIndex > stDevList.nDeviceNum - 1 || nDevIndex < 0)
                {
                    Console.Write("Input Error!\n");
                    break;
                }
                stDevInfo = (MyCamera.MV_CC_DEVICE_INFO)Marshal.PtrToStructure(stDevList.pDeviceInfo[nDevIndex], typeof(MyCamera.MV_CC_DEVICE_INFO));

                // ch:创建设备 | en: Create device
                nRet = device.MV_CC_CreateDevice_NET(ref stDevInfo);
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Create device failed:{0:x8}", nRet);
                    break;
                }

                // ch:打开设备 | en:Open device
                nRet = device.MV_CC_OpenDevice_NET();
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Open device failed:{0:x8}", nRet);
                    break;
                }

                // ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
                if (stDevInfo.nTLayerType == MyCamera.MV_GIGE_DEVICE)
                {
                    int nPacketSize = device.MV_CC_GetOptimalPacketSize_NET();
                    if (nPacketSize > 0)
                    {
                        nRet = device.MV_CC_SetIntValue_NET("GevSCPSPacketSize", (uint)nPacketSize);
                        if (nRet != MyCamera.MV_OK)
                        {
                            Console.WriteLine("Warning: Set Packet Size failed {0:x8}", nRet);
                        }
                    }
                    else
                    {
                        Console.WriteLine("Warning: Get Packet Size failed {0:x8}", nPacketSize);
                    }
                }

                // ch:设置触发模式为off || en:set trigger mode as off
                nRet = device.MV_CC_SetEnumValue_NET("TriggerMode", 0);
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Set TriggerMode failed:{0:x8}", nRet);
                    break;
                }

                // ch:开启抓图 || en: start grab image
                nRet = device.MV_CC_StartGrabbing_NET();
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Start grabbing failed:{0:x8}", nRet);
                    break;
                }

                MyCamera.MV_FRAME_OUT stFrameOut = new MyCamera.MV_FRAME_OUT();

                nRet = device.MV_CC_GetImageBuffer_NET(ref stFrameOut, 1000);
                // 获取一帧图像
                if (MyCamera.MV_OK == nRet)
                {
                    Console.WriteLine("Get One Frame:" + "Width[" + Convert.ToString(stFrameOut.stFrameInfo.nWidth) + "] , Height["
                        + Convert.ToString(stFrameOut.stFrameInfo.nHeight) + "] , FrameNum[" + Convert.ToString(stFrameOut.stFrameInfo.nFrameNum) + "]");

                    if (pDstData == IntPtr.Zero || nDstDataSize < stFrameOut.stFrameInfo.nFrameLen)
                    {
                        if (pDstData != IntPtr.Zero)
                        {
                            Marshal.FreeHGlobal(pDstData);
                            pDstData = IntPtr.Zero;
                            nDstDataSize = 0;
                        }

                        pDstData = Marshal.AllocHGlobal((int)stFrameOut.stFrameInfo.nFrameLen);
                        if (pDstData == IntPtr.Zero)
                        {
                            Console.WriteLine("malloc pDstData failed");
                            nRet = MyCamera.MV_E_RESOURCE;
                            break;
                        }
                        nDstDataSize = stFrameOut.stFrameInfo.nFrameLen;
                    }

                    MyCamera.MV_CC_COLOR_CORRECT_PARAM stColorCorrectParam = new MyCamera.MV_CC_COLOR_CORRECT_PARAM();

                    //图像相关
                    stColorCorrectParam.nWidth = stFrameOut.stFrameInfo.nWidth;
                    stColorCorrectParam.nHeight = stFrameOut.stFrameInfo.nHeight;
                    stColorCorrectParam.pSrcBuf = stFrameOut.pBufAddr;
                    stColorCorrectParam.nSrcBufLen = stFrameOut.stFrameInfo.nFrameLen;
                    stColorCorrectParam.enPixelType = stFrameOut.stFrameInfo.enPixelType;

                    stColorCorrectParam.pDstBuf = pDstData;
                    stColorCorrectParam.nDstBufSize = nDstDataSize;
                    stColorCorrectParam.nImageBit = 8;

                    //Gamma参数，可根据实际情况选择类型
                    stColorCorrectParam.stGammaParam.enGammaType = MyCamera.MV_CC_GAMMA_TYPE.MV_CC_GAMMA_TYPE_VALUE;
                    stColorCorrectParam.stGammaParam.fGammaValue = (float)0.6;

                    //CCM参数和CLUT参数至少输入一种，也可以两种一起处理
                    stColorCorrectParam.stCCMParam.bCCMEnable = false;

                    stColorCorrectParam.stCLUTParam.bCLUTEnable = true;
                    stColorCorrectParam.stCLUTParam.nCLUTScale = 1024;
                    stColorCorrectParam.stCLUTParam.nCLUTSize = 17;

                    if (false == File.Exists("./CLUTCalib.bin"))
                    {
                        Console.WriteLine("Open file failed");
                        nRet = MyCamera.MV_E_RESOURCE;
                        break;
                    }

                    FileStream fs = new FileStream("./CLUTCalib.bin", FileMode.Open);
                    byte[] data = new byte[fs.Length];

                    if (pCLUTData == IntPtr.Zero || nCLUTDataSize < fs.Length)
                    {
                        if (pCLUTData != IntPtr.Zero)
                        {
                            Marshal.FreeHGlobal(pCLUTData);
                            pCLUTData = IntPtr.Zero;
                            nCLUTDataSize = 0;
                        }

                        pCLUTData = Marshal.AllocHGlobal((int)fs.Length);
                        if (pCLUTData == IntPtr.Zero)
                        {
                            Console.WriteLine("malloc pCLUTData failed");
                            break;
                        }
                        nCLUTDataSize = (uint)fs.Length;
                    }

                    fs.Read(data, 0, data.Length);
                    fs.Close();

                    Marshal.Copy(data, 0, pCLUTData, (Int32)nCLUTDataSize);

                    stColorCorrectParam.stCLUTParam.pCLUTBuf = pCLUTData;
                    stColorCorrectParam.stCLUTParam.nCLUTBufLen = nCLUTDataSize;

                    nRet = device.MV_CC_ColorCorrect_NET(ref stColorCorrectParam);
                    if (MyCamera.MV_OK != nRet)
                    {
                        Console.WriteLine("Color Correct Failed:{0:x8}", nRet);
                        break;
                    }

                    //保存图像到文件
                    MyCamera.MV_SAVE_IMG_TO_FILE_PARAM stSaveFileParam = new MyCamera.MV_SAVE_IMG_TO_FILE_PARAM();
                    stSaveFileParam.enImageType = MyCamera.MV_SAVE_IAMGE_TYPE.MV_Image_Bmp;
                    stSaveFileParam.enPixelType = stFrameOut.stFrameInfo.enPixelType;
                    stSaveFileParam.nWidth = stFrameOut.stFrameInfo.nWidth;
                    stSaveFileParam.nHeight = stFrameOut.stFrameInfo.nHeight;
                    stSaveFileParam.nDataLen = stFrameOut.stFrameInfo.nFrameLen;
                    stSaveFileParam.pData = stFrameOut.pBufAddr;
                    stSaveFileParam.pImagePath = "BeforeImage_w" + stSaveFileParam.nWidth.ToString() + "_" + stSaveFileParam.nHeight.ToString() + "_fn" + stFrameOut.stFrameInfo.nFrameNum.ToString() + ".bmp";
                    nRet = device.MV_CC_SaveImageToFile_NET(ref stSaveFileParam);
                    if (MyCamera.MV_OK != nRet)
                    {
                        Console.WriteLine("SaveImageToFile failed:{0:x8}", nRet);
                        break;
                    }

                    stSaveFileParam.pData = pDstData;
                    stSaveFileParam.pImagePath = "AfterImage_w" + stSaveFileParam.nWidth.ToString() + "_" + stSaveFileParam.nHeight.ToString() + "_fn" + stFrameOut.stFrameInfo.nFrameNum.ToString() + ".bmp";
                    nRet = device.MV_CC_SaveImageToFile_NET(ref stSaveFileParam);
                    if (MyCamera.MV_OK != nRet)
                    {
                        Console.WriteLine("SaveImageToFile failed:{0:x8}", nRet);
                        break;
                    }

                    device.MV_CC_FreeImageBuffer_NET(ref stFrameOut);
                }
                else
                {
                    Console.WriteLine("Get Image failed:{0:x8}", nRet);
                }

                // ch:停止抓图 | en:Stop grabbing
                nRet = device.MV_CC_StopGrabbing_NET();
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Stop grabbing failed:{0:x8}", nRet);
                    break;
                }

                // ch:关闭设备 | en:Close device
                nRet = device.MV_CC_CloseDevice_NET();
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Close device failed:{0:x8}", nRet);
                    break;
                }

                // ch:销毁设备 | en:Destroy device
                nRet = device.MV_CC_DestroyDevice_NET();
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Destroy device failed:{0:x8}", nRet);
                    break;
                }
            } while (false);

            if (pDstData != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(pDstData);
                pDstData = IntPtr.Zero;
            }

            if (pCLUTData != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(pCLUTData);
                pCLUTData = IntPtr.Zero;
            }

            if (MyCamera.MV_OK != nRet)
            {
                // ch:销毁设备 | en:Destroy device
                nRet = device.MV_CC_DestroyDevice_NET();
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Destroy device failed:{0:x8}", nRet);
                }
            }

            Console.WriteLine("Press enter to exit");
            Console.ReadKey();
        }
    }
}
