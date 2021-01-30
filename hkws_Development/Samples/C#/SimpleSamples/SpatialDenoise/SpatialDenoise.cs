using System;
using System.Collections.Generic;
using MvCamCtrl.NET;
using System.Runtime.InteropServices;
using System.IO;

namespace SpatialDenoise
{
    class SpatialDenoise
    {
        public static MyCamera.cbOutputExdelegate ImageCallback;
        public static MyCamera device = new MyCamera();
        static bool g_IsNeedNoiseEstimate = true;
        static IntPtr g_pNoiseProfile = IntPtr.Zero;
        static IntPtr g_pDstData = IntPtr.Zero;
        static uint g_nNoiseProfileSize = 0;
        static uint g_nDstDataSize = 0;

        static void ImageCallbackFunc(IntPtr pData, ref MyCamera.MV_FRAME_OUT_INFO_EX pFrameInfo, IntPtr pUser)
        {
            int nRet = MyCamera.MV_OK;
            Console.WriteLine("Get one frame: Width[" + Convert.ToString(pFrameInfo.nWidth) + "] , Height[" + Convert.ToString(pFrameInfo.nHeight)
                                    + "] , FrameNum[" + Convert.ToString(pFrameInfo.nFrameNum) + "]");
            //判断是否需要标定
            if (true == g_IsNeedNoiseEstimate)
            {
                //噪声估计
                MyCamera.MV_CC_NOISE_ESTIMATE_PARAM stEstimateParam = new MyCamera.MV_CC_NOISE_ESTIMATE_PARAM();
                stEstimateParam.nWidth = pFrameInfo.nWidth;
                stEstimateParam.nHeight = pFrameInfo.nHeight;
                stEstimateParam.enPixelType = pFrameInfo.enPixelType;
                stEstimateParam.pSrcBuf = pData;
                stEstimateParam.nSrcBufLen = pFrameInfo.nFrameLen;

                //如果选择全图做噪声估计，nROINum可输入0，pstROIRect可置空
                MyCamera.MV_CC_RECT_I stROIRect = new MyCamera.MV_CC_RECT_I();
                stROIRect.nX = 0;
                stROIRect.nY = 0;
                stROIRect.nWidth = pFrameInfo.nWidth;
                stROIRect.nHeight = pFrameInfo.nHeight;
                stEstimateParam.pstROIRect = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(MyCamera.MV_CC_RECT_I)));
                Marshal.StructureToPtr(stROIRect, stEstimateParam.pstROIRect, false);
                stEstimateParam.nROINum = 1;

                //Bayer域噪声估计参数，Mono8/RGB域无效
                stEstimateParam.nNoiseThreshold = 1024;

                stEstimateParam.pNoiseProfile = IntPtr.Zero;
                nRet = device.MV_CC_NoiseEstimate_NET(ref stEstimateParam);
                if (MyCamera.MV_OK != nRet)
                {
                    if (g_pNoiseProfile == IntPtr.Zero || g_nNoiseProfileSize < stEstimateParam.nNoiseProfileLen)
                    {
                        if (g_pNoiseProfile != IntPtr.Zero)
                        {
                            Marshal.FreeHGlobal(g_pNoiseProfile);
                            g_pNoiseProfile = IntPtr.Zero;
                            g_nNoiseProfileSize = 0;
                        }

                        g_pNoiseProfile = Marshal.AllocHGlobal((int)stEstimateParam.nNoiseProfileLen);
                        if (g_pNoiseProfile == IntPtr.Zero)
                        {
                            Console.WriteLine("malloc pNoiseProfile failed");
                            return;
                        }
                        g_nNoiseProfileSize = stEstimateParam.nNoiseProfileLen;
                    }

                    stEstimateParam.pNoiseProfile = g_pNoiseProfile;
                    stEstimateParam.nNoiseProfileSize = g_nNoiseProfileSize;
                    nRet = device.MV_CC_NoiseEstimate_NET(ref stEstimateParam);
                    if (MyCamera.MV_OK != nRet)
                    {
                        Console.WriteLine("Noise estimate failed:{0:x8}", nRet);
                        return;
                    }
                }

                Marshal.FreeHGlobal(stEstimateParam.pstROIRect);

                //保存标定表到本地
                byte[] EstimateData = new byte[stEstimateParam.nNoiseProfileLen];
                Marshal.Copy(stEstimateParam.pNoiseProfile, EstimateData, 0, (int)stEstimateParam.nNoiseProfileLen);
                FileStream pFile = null;
                try
                {
                    pFile = new FileStream("./NoiseProfile.bin", FileMode.Create);
                    pFile.Write(EstimateData, 0, EstimateData.Length);
                }
                catch
                {
                    Console.WriteLine("保存失败");
                }
                finally
                {
                    pFile.Close();
                }

                g_IsNeedNoiseEstimate = false;
            }

            //空域降噪
            if (g_pDstData == IntPtr.Zero || g_nDstDataSize < pFrameInfo.nFrameLen)
            {
                if (g_pDstData != IntPtr.Zero)
                {
                    Marshal.FreeHGlobal(g_pDstData);
                    g_pDstData = IntPtr.Zero;
                    g_nDstDataSize = 0;
                }

                g_pDstData = Marshal.AllocHGlobal((int)pFrameInfo.nFrameLen);
                if (g_pDstData == IntPtr.Zero)
                {
                    Console.WriteLine("malloc pDstData failed");
                    return;
                }
                g_nDstDataSize = pFrameInfo.nFrameLen;
            }

            MyCamera.MV_CC_SPATIAL_DENOISE_PARAM stDisnoiseParam = new MyCamera.MV_CC_SPATIAL_DENOISE_PARAM();
            stDisnoiseParam.nWidth = pFrameInfo.nWidth;
            stDisnoiseParam.nHeight = pFrameInfo.nHeight;
            stDisnoiseParam.enPixelType = pFrameInfo.enPixelType;
            stDisnoiseParam.pSrcBuf = pData;
            stDisnoiseParam.nSrcBufLen = pFrameInfo.nFrameLen;

            stDisnoiseParam.pDstBuf = g_pDstData;
            stDisnoiseParam.nDstBufSize = g_nDstDataSize;

            stDisnoiseParam.pNoiseProfile = g_pNoiseProfile;
            stDisnoiseParam.nNoiseProfileLen = g_nNoiseProfileSize;

            //Bayer域空域降噪算法参数，Mono8/RGB域无效
            stDisnoiseParam.nBayerDenoiseStrength = 50;
            stDisnoiseParam.nBayerSharpenStrength = 16;
            stDisnoiseParam.nBayerNoiseCorrect = 1024;

            //Mono8/RGB域空域降噪算法参数，Bayer域无效
            stDisnoiseParam.nNoiseCorrectLum = 1000;
            stDisnoiseParam.nNoiseCorrectChrom = 500;
            stDisnoiseParam.nStrengthLum = 50;
            stDisnoiseParam.nStrengthChrom = 100;
            stDisnoiseParam.nStrengthSharpen = 300;
            nRet = device.MV_CC_SpatialDenoise_NET(ref stDisnoiseParam);
            if (MyCamera.MV_OK != nRet)
            {
                Console.WriteLine("Spatial denoise failed:{0:x8}", nRet);
                return;
            }

            if (pFrameInfo.nFrameNum < 10)
            {
                //保存图像到文件
                MyCamera.MV_SAVE_IMG_TO_FILE_PARAM stSaveFileParam = new MyCamera.MV_SAVE_IMG_TO_FILE_PARAM();
                stSaveFileParam.enImageType = MyCamera.MV_SAVE_IAMGE_TYPE.MV_Image_Bmp;
                stSaveFileParam.enPixelType = pFrameInfo.enPixelType;
                stSaveFileParam.nWidth = pFrameInfo.nWidth;
                stSaveFileParam.nHeight = pFrameInfo.nHeight;
                stSaveFileParam.nDataLen = pFrameInfo.nFrameLen;
                stSaveFileParam.pData = pData;
                stSaveFileParam.pImagePath = "BeforeImage_w" + stSaveFileParam.nWidth.ToString() + "_" + stSaveFileParam.nHeight.ToString() + "_fn" + pFrameInfo.nFrameNum.ToString() + ".bmp";
                nRet = device.MV_CC_SaveImageToFile_NET(ref stSaveFileParam);
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("SaveImageToFile failed:{0:x8}", nRet);
                    return;
                }

                stSaveFileParam.pData = g_pDstData;
                stSaveFileParam.pImagePath = "AfterImage_w" + stSaveFileParam.nWidth.ToString() + "_" + stSaveFileParam.nHeight.ToString() + "_fn" + pFrameInfo.nFrameNum.ToString() + ".bmp";
                nRet = device.MV_CC_SaveImageToFile_NET(ref stSaveFileParam);
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("SaveImageToFile failed:{0:x8}", nRet);
                    return;
                }
            }
        }

        static void Main(string[] args)
        {
            int nRet = MyCamera.MV_OK;
            do
            {
                // ch:枚举设备 | en:Enum device
                MyCamera.MV_CC_DEVICE_INFO_LIST stDevList = new MyCamera.MV_CC_DEVICE_INFO_LIST();
                nRet = MyCamera.MV_CC_EnumDevices_NET(MyCamera.MV_GIGE_DEVICE | MyCamera.MV_USB_DEVICE, ref stDevList);
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Enum device failed:{0:x8}", nRet);
                    break;
                }
                Console.WriteLine("Enum device count : " + Convert.ToString(stDevList.nDeviceNum));
                if (0 == stDevList.nDeviceNum)
                {
                    break;
                }

                MyCamera.MV_CC_DEVICE_INFO stDevInfo;                            // 通用设备信息

                // ch:打印设备信息 en:Print device info
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

                // ch:创建设备 | en:Create device
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

                //判断是否可以本地导入
                if (true == File.Exists("./NoiseProfile.bin"))
                {
                    FileStream fs = new FileStream("./NoiseProfile.bin", FileMode.Open);
                    byte[] data = new byte[fs.Length];

                    if (g_pNoiseProfile == IntPtr.Zero || g_nNoiseProfileSize < fs.Length)
                    {
                        if (g_pNoiseProfile != IntPtr.Zero)
                        {
                            Marshal.FreeHGlobal(g_pNoiseProfile);
                            g_pNoiseProfile = IntPtr.Zero;
                            g_nNoiseProfileSize = 0;
                        }

                        g_pNoiseProfile = Marshal.AllocHGlobal((int)fs.Length);
                        if (g_pNoiseProfile == IntPtr.Zero)
                        {
                            Console.WriteLine("malloc pNoiseProfile failed");
                            break;
                        }
                        g_nNoiseProfileSize = (uint)fs.Length;
                    }

                    fs.Read(data, 0, data.Length);
                    fs.Close();

                    Marshal.Copy(data, 0, g_pNoiseProfile, (Int32)g_nNoiseProfileSize);

                    g_IsNeedNoiseEstimate = false;
                }

                // ch:注册回调函数 | en:Register image callback
                ImageCallback = new MyCamera.cbOutputExdelegate(ImageCallbackFunc);
                nRet = device.MV_CC_RegisterImageCallBackEx_NET(ImageCallback, IntPtr.Zero);
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Register image callback failed!");
                    break;
                }

                // ch:开启抓图 || en: start grab image
                nRet = device.MV_CC_StartGrabbing_NET();
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Start grabbing failed:{0:x8}", nRet);
                    break;
                }

                Console.WriteLine("Press enter to exit");
                Console.ReadLine();

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

            if (g_pNoiseProfile != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(g_pNoiseProfile);
                g_pNoiseProfile = IntPtr.Zero;
                g_nNoiseProfileSize = 0;
            }

            if (g_pDstData != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(g_pDstData);
                g_pDstData = IntPtr.Zero;
                g_nDstDataSize = 0;
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
