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

int main()
{
    int nRet = MV_OK;
    void* handle = NULL;
    unsigned char *pDstData = NULL;
    unsigned int nDstDataSize = 0;

    unsigned char *pCLUTData = NULL;
    unsigned int nCLUTDataSize = 0;

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

        nRet = MV_CC_SetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_OFF);
        if (MV_OK != nRet)
        {
            printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet);
            break;
        }

        // ch:开始取流 | en:Start grab image
        nRet = MV_CC_StartGrabbing(handle);
        if (MV_OK != nRet)
        {
            printf("Start Grabbing fail! nRet [0x%x]\n", nRet);
            break;
        }

        MV_FRAME_OUT stImageInfo = {0};

        nRet = MV_CC_GetImageBuffer(handle, &stImageInfo, 1000);
        if (nRet == MV_OK)
        {
            printf("Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n", 
                stImageInfo.stFrameInfo.nWidth, stImageInfo.stFrameInfo.nHeight, stImageInfo.stFrameInfo.nFrameNum);

            if (pDstData == NULL || nDstDataSize < stImageInfo.stFrameInfo.nFrameLen)
            {
                if (pDstData)
                {
                    free(pDstData);
                    pDstData = NULL;
                    nDstDataSize = 0;
                }

                pDstData = (unsigned char *)malloc(stImageInfo.stFrameInfo.nFrameLen);
                if (pDstData == NULL)
                {
                    printf("malloc pDstData fail!\n");
                    nRet = MV_E_RESOURCE;
                    break;
                }
                nDstDataSize = stImageInfo.stFrameInfo.nFrameLen;
            }

            MV_CC_COLOR_CORRECT_PARAM stColorCorrectParam = {0};

            //图像相关
            stColorCorrectParam.nWidth = stImageInfo.stFrameInfo.nWidth;
            stColorCorrectParam.nHeight = stImageInfo.stFrameInfo.nHeight;
            stColorCorrectParam.pSrcBuf = stImageInfo.pBufAddr;
            stColorCorrectParam.nSrcBufLen = stImageInfo.stFrameInfo.nFrameLen;
            stColorCorrectParam.enPixelType = stImageInfo.stFrameInfo.enPixelType;

            stColorCorrectParam.pDstBuf = pDstData;
            stColorCorrectParam.nDstBufSize = nDstDataSize;
            stColorCorrectParam.nImageBit = 8;

            //Gamma参数，可根据实际情况选择类型
            stColorCorrectParam.stGammaParam.enGammaType = MV_CC_GAMMA_TYPE_VALUE;
            stColorCorrectParam.stGammaParam.fGammaValue = 0.6;

            //CCM参数和CLUT参数至少输入一种，也可以两种一起处理
            stColorCorrectParam.stCCMParam.bCCMEnable = false;

            stColorCorrectParam.stCLUTParam.bCLUTEnable = true;
            stColorCorrectParam.stCLUTParam.nCLUTScale = 1024;
            stColorCorrectParam.stCLUTParam.nCLUTSize = 17;

            FILE* fp = fopen("./CLUTCalib.bin", "rb+");
            if (fp == NULL)
            {
                printf("Failed to open 'CLUTCalib.bin', This file is required to do CLUT!\n");
                nRet = MV_E_RESOURCE;
                break;
            }

            unsigned int nNeedCLUTBufLen = stColorCorrectParam.stCLUTParam.nCLUTSize * stColorCorrectParam.stCLUTParam.nCLUTSize * stColorCorrectParam.stCLUTParam.nCLUTSize * sizeof(int) * 3;
            int nFileLen = filelength(fileno(fp));
            if (nFileLen < nNeedCLUTBufLen)
            {
                printf("The file (CLUTCalib.bin) is too small!\n");
                nRet = MV_E_BUFOVER;
                break;
            }

            if (pCLUTData == NULL || nCLUTDataSize < nNeedCLUTBufLen)
            {
                if (pCLUTData)
                {
                    free(pCLUTData);
                    pCLUTData = NULL;
                    nCLUTDataSize = 0;
                }

                pCLUTData = (unsigned char *)malloc(nNeedCLUTBufLen);
                if (pCLUTData == NULL)
                {
                    printf("malloc pCLUTData fail !\n");
                    nRet = MV_E_RESOURCE;
                    break;
                }
                nCLUTDataSize = nNeedCLUTBufLen;
            }
            fread(pCLUTData, 1, nCLUTDataSize, fp);
            fclose(fp);

            stColorCorrectParam.stCLUTParam.pCLUTBuf = pCLUTData;
            stColorCorrectParam.stCLUTParam.nCLUTBufLen = nCLUTDataSize;

            nRet = MV_CC_ColorCorrect(handle, &stColorCorrectParam);
            if (MV_OK != nRet)
            {
                printf("Color Correct fail! nRet [0x%x]\n", nRet);
                break;
            }

            //保存图像到文件
            MV_SAVE_IMG_TO_FILE_PARAM stSaveFileParam;
            memset(&stSaveFileParam, 0, sizeof(MV_SAVE_IMG_TO_FILE_PARAM));

            stSaveFileParam.enImageType = MV_Image_Bmp;
            stSaveFileParam.enPixelType = stImageInfo.stFrameInfo.enPixelType;
            stSaveFileParam.nWidth      = stImageInfo.stFrameInfo.nWidth;
            stSaveFileParam.nHeight     = stImageInfo.stFrameInfo.nHeight;
            stSaveFileParam.nDataLen    = stImageInfo.stFrameInfo.nFrameLen;
            stSaveFileParam.pData       = stImageInfo.pBufAddr;
            sprintf_s(stSaveFileParam.pImagePath, 256, "BeforeImage_w%d_h%d_fn%03d.bmp", stSaveFileParam.nWidth, stSaveFileParam.nHeight, stImageInfo.stFrameInfo.nFrameNum);
            nRet = MV_CC_SaveImageToFile(handle, &stSaveFileParam);
            if (MV_OK != nRet)
            {
                printf("SaveImageToFile failed[%x]!\n", nRet);
                break;
            }

            stSaveFileParam.pData       = pDstData;
            sprintf_s(stSaveFileParam.pImagePath, 256, "AfterImage_w%d_h%d_fn%03d.bmp", stSaveFileParam.nWidth, stSaveFileParam.nHeight, stImageInfo.stFrameInfo.nFrameNum);
            nRet = MV_CC_SaveImageToFile(handle, &stSaveFileParam);
            if (MV_OK != nRet)
            {
                printf("SaveImageToFile failed[%x]!\n", nRet);
                break;
            }

            MV_CC_FreeImageBuffer(handle, &stImageInfo);
        }
        else
        {
            printf("Get Image fail! nRet [0x%x]\n", nRet);
        }

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

    if (pDstData)
    {
        free(pDstData);
        pDstData = NULL;
    }

    if (pCLUTData)
    {
        free(pCLUTData);
        pCLUTData = NULL;
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
