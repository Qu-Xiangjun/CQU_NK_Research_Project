using System;
using System.Collections.Generic;
using MvCamCtrl.NET;
using System.Runtime.InteropServices;
using System.IO;

namespace ImageEnhance
{
    class ImageEnhance
    {
        static void Main(string[] args)
        {
            int nRet = MyCamera.MV_OK;
            MyCamera device = new MyCamera();
            IntPtr pDstData = IntPtr.Zero;

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

                Console.Write("*********************************************************************\n");
                Console.Write("*  0.Contrast;  1.Sharpen;                                          *\n");
                Console.Write("*********************************************************************\n");
                Console.Write("Select enhance type:");
                Int32 nType = Convert.ToInt32(Console.ReadLine());
                if (nType != 0 && nType != 1)
                {
                    Console.WriteLine("Input error", nRet);
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

                    if (pDstData == IntPtr.Zero)
                    {
                        pDstData = Marshal.AllocHGlobal((int)(stFrameOut.stFrameInfo.nFrameLen));
                    }

                    if (0 == nType)//对比度
                    {
                        MyCamera.MV_CC_CONTRAST_PARAM stContrastParam = new MyCamera.MV_CC_CONTRAST_PARAM();
                        stContrastParam.nWidth = stFrameOut.stFrameInfo.nWidth;
                        stContrastParam.nHeight = stFrameOut.stFrameInfo.nHeight;
                        stContrastParam.enPixelType = stFrameOut.stFrameInfo.enPixelType;
                        stContrastParam.pSrcBuf = stFrameOut.pBufAddr;
                        stContrastParam.nSrcBufLen = stFrameOut.stFrameInfo.nFrameLen;

                        stContrastParam.pDstBuf = pDstData;
                        stContrastParam.nDstBufSize = stFrameOut.stFrameInfo.nFrameLen;
                        stContrastParam.nContrastFactor = 1000;
                        nRet = device.MV_CC_ImageContrast_NET(ref stContrastParam);
                        if (MyCamera.MV_OK != nRet)
                        {
                            Console.WriteLine("Adjust image contrast Failed:{0:x8}", nRet);
                            break;
                        }
                    }
                    else if (1 == nType)//锐化
                    {
                        MyCamera.MV_CC_SHARPEN_PARAM stSharpenParam = new MyCamera.MV_CC_SHARPEN_PARAM();
                        stSharpenParam.nWidth = stFrameOut.stFrameInfo.nWidth;
                        stSharpenParam.nHeight = stFrameOut.stFrameInfo.nHeight;
                        stSharpenParam.enPixelType = stFrameOut.stFrameInfo.enPixelType;
                        stSharpenParam.pSrcBuf = stFrameOut.pBufAddr;
                        stSharpenParam.nSrcBufLen = stFrameOut.stFrameInfo.nFrameLen;

                        stSharpenParam.pDstBuf = pDstData;
                        stSharpenParam.nDstBufSize = stFrameOut.stFrameInfo.nFrameLen;

                        stSharpenParam.nSharpenAmount = 350;
                        stSharpenParam.nSharpenRadius = 10;
                        stSharpenParam.nSharpenThreshold = 50;
                        nRet = device.MV_CC_ImageSharpen_NET(ref stSharpenParam);
                        if (MyCamera.MV_OK != nRet)
                        {
                            Console.WriteLine("Image Sharpen Failed:{0:x8}", nRet);
                            break;
                        }
                    }

                    // ch:将图像数据保存到本地文件 | en:Save image data to local file
                    byte[] data = new byte[stFrameOut.stFrameInfo.nFrameLen];
                    Marshal.Copy(stFrameOut.pBufAddr, data, 0, (int)stFrameOut.stFrameInfo.nFrameLen);
                    FileStream pFile = null;
                    try
                    {
                        pFile = new FileStream("BeforeEnhance.raw", FileMode.Create);
                        pFile.Write(data, 0, data.Length);
                    }
                    catch
                    {
                        Console.WriteLine("Image enhance fail");
                    }
                    finally
                    {
                        pFile.Close();
                    }

                    device.MV_CC_FreeImageBuffer_NET(ref stFrameOut);

                    Marshal.Copy(pDstData, data, 0, (int)stFrameOut.stFrameInfo.nFrameLen);
                    try
                    {
                        pFile = new FileStream("AfterEnhance.raw", FileMode.Create);
                        pFile.Write(data, 0, data.Length);
                    }
                    catch
                    {
                        Console.WriteLine("Image enhance fail");
                    }
                    finally
                    {
                        pFile.Close();
                    }

                    Console.WriteLine("Image enhance succeed");
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
