#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include <io.h>
#include "MvCameraControl.h"

// ch:等待按键输入 | en:Wait for key press
void WaitForKeyPress(void)
{
    while(!_kbhit())
    {
        Sleep(10);
    }
    _getch();
}

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo)
{
    if (NULL == pstMVDevInfo)
    {
        printf("The Pointer of pstMVDevInfo is NULL!\n");
        return false;
    }
    if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE)
    {
        int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
        int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
        int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
        int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

        // ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name
        printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4);
        printf("UserDefinedName: %s\n\n" , pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
    }
    else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE)
    {
        printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
        printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
        printf("Device Number: %d\n\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.nDeviceNumber);
    }
    else
    {
        printf("Not support.\n");
    }

    return true;
}

unsigned char * g_pDstData = NULL;
unsigned int g_nDstDataSize = 0;

unsigned char * g_pNoiseProfile = NULL;
unsigned int g_nNoiseProfileSize = 0;

bool g_IsNeedNoiseEstimate = true;

void __stdcall ImageCallBackEx(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{
    printf("Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n", pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo->nFrameNum);

    int nRet = MV_OK;
    //判断是否需要噪声估计
    if (true == g_IsNeedNoiseEstimate)
    {
        // ch:噪声估计 | en:Noise estimate
        MV_CC_NOISE_ESTIMATE_PARAM stEstimateParam = {0};
        stEstimateParam.nWidth = pFrameInfo->nWidth;
        stEstimateParam.nHeight = pFrameInfo->nHeight;
        stEstimateParam.enPixelType = pFrameInfo->enPixelType;
        stEstimateParam.pSrcBuf = pData;
        stEstimateParam.nSrcBufLen = pFrameInfo->nFrameLen;

        //如果选择全图做噪声估计，nROINum可输入0，pstROIRect可置空
        MV_CC_RECT_I stROIRect = {0};
        stROIRect.nX = 0;
        stROIRect.nY = 0;
        stROIRect.nWidth = pFrameInfo->nWidth;
        stROIRect.nHeight = pFrameInfo->nHeight;
        stEstimateParam.pstROIRect = &stROIRect;
        stEstimateParam.nROINum = 1;

        //Bayer域噪声估计参数，Mono8/RGB域无效
        stEstimateParam.nNoiseThreshold = 1024;

        stEstimateParam.pNoiseProfile = NULL;
        nRet = MV_CC_NoiseEstimate(pUser, &stEstimateParam);
        if (MV_OK != nRet)
        {
            if (g_pNoiseProfile == NULL || g_nNoiseProfileSize < stEstimateParam.nNoiseProfileLen)
            {
                if (g_pNoiseProfile)
                {
                    free(g_pNoiseProfile);
                    g_pNoiseProfile = NULL;
                    g_nNoiseProfileSize = 0;
                }

                g_pNoiseProfile = (unsigned char *)malloc(stEstimateParam.nNoiseProfileLen);
                if (g_pNoiseProfile == NULL)
                {
                    printf("malloc pNoiseProfile fail !\n");
                    return;
                }
                g_nNoiseProfileSize = stEstimateParam.nNoiseProfileLen;
            }

            stEstimateParam.pNoiseProfile = g_pNoiseProfile;
            stEstimateParam.nNoiseProfileSize = g_nNoiseProfileSize;
            nRet = MV_CC_NoiseEstimate(pUser, &stEstimateParam);
            if (MV_OK != nRet)
            {
                printf("Noise estimate fail! nRet [0x%x]\n", nRet);
                return;
            }
        }

        //保存噪声特性到本地
        FILE* fp_out = fopen("./NoiseProfile.bin", "wb+");
        if (NULL == fp_out)
        {
            return ;
        }
        fwrite(stEstimateParam.pNoiseProfile, 1, stEstimateParam.nNoiseProfileLen, fp_out);
        fclose(fp_out);

        g_IsNeedNoiseEstimate = false;
    }

    // ch:空域降噪 | en:Spatial denoise
    if (g_pDstData == NULL || g_nDstDataSize < pFrameInfo->nFrameLen)
    {
        if (g_pDstData)
        {
            free(g_pDstData);
            g_pDstData = NULL;
            g_nDstDataSize = 0;
        }

        g_pDstData = (unsigned char *)malloc(pFrameInfo->nFrameLen);
        if (g_pDstData == NULL)
        {
            printf("malloc pDstData fail !\n");
            return;
        }
        g_nDstDataSize = pFrameInfo->nFrameLen;
    }

    MV_CC_SPATIAL_DENOISE_PARAM stDisnoiseParam = {0};

    stDisnoiseParam.nWidth = pFrameInfo->nWidth;
    stDisnoiseParam.nHeight = pFrameInfo->nHeight;
    stDisnoiseParam.enPixelType = pFrameInfo->enPixelType;
    stDisnoiseParam.pSrcBuf = pData;
    stDisnoiseParam.nSrcBufLen = pFrameInfo->nFrameLen;

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
    nRet = MV_CC_SpatialDenoise(pUser, &stDisnoiseParam);
    if (MV_OK != nRet)
    {
        printf("Spatial denoise fail! nRet [0x%x]\n", nRet);
        return;
    }

    if (pFrameInfo->nFrameNum < 10)
    {
        //保存图像到文件
        MV_SAVE_IMG_TO_FILE_PARAM stSaveFileParam;
        memset(&stSaveFileParam, 0, sizeof(MV_SAVE_IMG_TO_FILE_PARAM));

        stSaveFileParam.enImageType = MV_Image_Bmp;
        stSaveFileParam.enPixelType = pFrameInfo->enPixelType;
        stSaveFileParam.nWidth      = pFrameInfo->nWidth;
        stSaveFileParam.nHeight     = pFrameInfo->nHeight;
        stSaveFileParam.nDataLen    = pFrameInfo->nFrameLen;
        stSaveFileParam.pData       = pData;
        sprintf_s(stSaveFileParam.pImagePath, 256, "BeforeImage_w%d_h%d_fn%03d.bmp", stSaveFileParam.nWidth, stSaveFileParam.nHeight, pFrameInfo->nFrameNum);
        nRet = MV_CC_SaveImageToFile(pUser, &stSaveFileParam);
        if (MV_OK != nRet)
        {
            printf("SaveImageToFile failed[%x]!\n", nRet);
            return;
        }

        stSaveFileParam.pData       = g_pDstData;
        sprintf_s(stSaveFileParam.pImagePath, 256, "AfterImage_w%d_h%d_fn%03d.bmp", stSaveFileParam.nWidth, stSaveFileParam.nHeight, pFrameInfo->nFrameNum);
        nRet = MV_CC_SaveImageToFile(pUser, &stSaveFileParam);
        if (MV_OK != nRet)
        {
            printf("SaveImageToFile failed[%x]!\n", nRet);
            return;
        }
    }
}

int main()
{
    int nRet = MV_OK;
    void* handle = NULL;

    do 
    {
        // ch:枚举设备 | en:Enum device
        MV_CC_DEVICE_INFO_LIST stDeviceList;
        memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
        nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
        if (MV_OK != nRet)
        {
            printf("Enum Devices fail! nRet [0x%x]\n", nRet);
            break;
        }

        if (stDeviceList.nDeviceNum > 0)
        {
            for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++)
            {
                printf("[device %d]:\n", i);
                MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
                if (NULL == pDeviceInfo)
                {
                    break;
                } 
                PrintDeviceInfo(pDeviceInfo);            
            }  
        } 
        else
        {
            printf("Find No Devices!\n");
            break;
        }

        printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1);
        unsigned int nIndex = 0;
        scanf_s("%d", &nIndex);

        if (nIndex >= stDeviceList.nDeviceNum)
        {
            printf("Input error!\n");
            break;
        }

        // ch:选择设备并创建句柄 | en:Select device and create handle
        nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]);
        if (MV_OK != nRet)
        {
            printf("Create Handle fail! nRet [0x%x]\n", nRet);
            break;
        }

        // ch:打开设备 | en:Open device
        nRet = MV_CC_OpenDevice(handle);
        if (MV_OK != nRet)
        {
            printf("Open Device fail! nRet [0x%x]\n", nRet);
            break;
        }

        // ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
        if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
        {
            int nPacketSize = MV_CC_GetOptimalPacketSize(handle);
            if (nPacketSize > 0)
            {
                nRet = MV_CC_SetIntValue(handle,"GevSCPSPacketSize",nPacketSize);
                if(nRet != MV_OK)
                {
                    printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet);
                }
            }
            else
            {
                printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize);
            }
        }

        // ch:设置触发模式为off | eb:Set trigger mode as off
        nRet = MV_CC_SetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_OFF);
        if (MV_OK != nRet)
        {
            printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet);
            break;
        }

        //判断是否可以本地导入
        FILE* fp = fopen("./NoiseProfile.bin", "rb+");
        if (fp)
        {
            int nFileLen = filelength(fileno(fp));
            if (g_pNoiseProfile == NULL || g_nNoiseProfileSize < nFileLen)
            {
                if (g_pNoiseProfile)
                {
                    free(g_pNoiseProfile);
                    g_pNoiseProfile = NULL;
                    g_nNoiseProfileSize = 0;
                }

                g_pNoiseProfile = (unsigned char *)malloc(nFileLen);
                if (g_pNoiseProfile == NULL)
                {
                    printf("malloc pNoiseProfile fail !\n");
                    break;
                }
                g_nNoiseProfileSize = nFileLen;
            }
            fread(g_pNoiseProfile, 1, g_nNoiseProfileSize, fp);
            fclose(fp);

            g_IsNeedNoiseEstimate = false;
        }

        // ch:注册抓图回调 | en:Register image callback
        nRet = MV_CC_RegisterImageCallBackEx(handle, ImageCallBackEx, handle);
        if (MV_OK != nRet)
        {
            printf("Register Image CallBack fail! nRet [0x%x]\n", nRet);
            break;
        }

        // ch:开始取流 | en:Start grab image
        nRet = MV_CC_StartGrabbing(handle);
        if (MV_OK != nRet)
        {
            printf("Start Grabbing fail! nRet [0x%x]\n", nRet);
            break;
        }

        printf("Press a key to stop grabbing.\n");
        WaitForKeyPress();

        Sleep(1000);

        // ch:停止取流 | en:Stop grab image
        nRet = MV_CC_StopGrabbing(handle);
        if (MV_OK != nRet)
        {
            printf("Stop Grabbing fail! nRet [0x%x]\n", nRet);
            break;
        }

        // ch:关闭设备 | en:Close device
        nRet = MV_CC_CloseDevice(handle);
        if (MV_OK != nRet)
        {
            printf("Close Device fail! nRet [0x%x]\n", nRet);
            break;
        }

        // ch:销毁句柄 | en:Destroy handle
        nRet = MV_CC_DestroyHandle(handle);
        if (MV_OK != nRet)
        {
            printf("Destroy Handle fail! nRet [0x%x]\n", nRet);
            break;
        }
    } while (0);

    if (g_pNoiseProfile)
    {
        free(g_pNoiseProfile);
        g_pNoiseProfile = NULL;
        g_nNoiseProfileSize = 0;
    }

    if (g_pDstData)
    {
        free(g_pDstData);
        g_pDstData = NULL;
        g_nDstDataSize = 0;
    }

    if (nRet != MV_OK)
    {
        if (handle != NULL)
        {
            MV_CC_DestroyHandle(handle);
            handle = NULL;
        }
    }

    printf("Press a key to exit.\n");
    WaitForKeyPress();

    return 0;
}
