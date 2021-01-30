#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include "MvCameraControl.h"

// ch:�ȴ��������� | en:Wait for key press
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

        // ch:��ӡ��ǰ���ip���û��Զ������� | en:print current ip and user defined name
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
    unsigned char * pDstData = NULL;

    do 
    {
        // ch:ö���豸 | en:Enum device
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

        // ch:ѡ���豸��������� | en:Select device and create handle
        nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]);
        if (MV_OK != nRet)
        {
            printf("Create Handle fail! nRet [0x%x]\n", nRet);
            break;
        }

        // ch:���豸 | en:Open device
        nRet = MV_CC_OpenDevice(handle);
        if (MV_OK != nRet)
        {
            printf("Open Device fail! nRet [0x%x]\n", nRet);
            break;
        }

        // ch:̽��������Ѱ���С(ֻ��GigE�����Ч) | en:Detection network optimal package size(It only works for the GigE camera)
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

        printf("*********************************************************************\n");
        printf("*  0.Contrast;  1.Sharpen;                                          *\n");
        printf("*********************************************************************\n");
        printf("Select enhance type:");
        unsigned int nType = 0;
        scanf_s("%d", &nType);

        if (nType != 0 && nType != 1)
        {
            printf("Input error!\n");
            break;
        }

        // ch:��ʼȡ�� | en:Start grab image
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

            pDstData = (unsigned char *)malloc(sizeof(unsigned char) * (stImageInfo.stFrameInfo.nFrameLen));
            if (pDstData == NULL)
            {
                printf("malloc pData fail !\n");
                nRet = MV_E_RESOURCE;
                break;
            }

            if (0 == nType)//�Աȶ�
            {
                MV_CC_CONTRAST_PARAM stContrastParam = {0};

                stContrastParam.nWidth = stImageInfo.stFrameInfo.nWidth;
                stContrastParam.nHeight = stImageInfo.stFrameInfo.nHeight;
                stContrastParam.enPixelType = stImageInfo.stFrameInfo.enPixelType;
                stContrastParam.pSrcBuf = stImageInfo.pBufAddr;
                stContrastParam.nSrcBufLen = stImageInfo.stFrameInfo.nFrameLen;

                stContrastParam.pDstBuf = pDstData;
                stContrastParam.nDstBufSize = stImageInfo.stFrameInfo.nFrameLen;
                stContrastParam.nContrastFactor = 1000;
                nRet = MV_CC_ImageContrast(handle, &stContrastParam);
                if (MV_OK != nRet)
                {
                    printf("Adjust image contrast fail! nRet [0x%x]\n", nRet);
                    break;
                }
            }
            else if (1 == nType)//��
            {
                MV_CC_SHARPEN_PARAM stSharpenParam = {0};

                stSharpenParam.nWidth = stImageInfo.stFrameInfo.nWidth;
                stSharpenParam.nHeight = stImageInfo.stFrameInfo.nHeight;
                stSharpenParam.enPixelType = stImageInfo.stFrameInfo.enPixelType;
                stSharpenParam.pSrcBuf = stImageInfo.pBufAddr;
                stSharpenParam.nSrcBufLen = stImageInfo.stFrameInfo.nFrameLen;

                stSharpenParam.pDstBuf = pDstData;
                stSharpenParam.nDstBufSize = stImageInfo.stFrameInfo.nFrameLen;

                stSharpenParam.nSharpenAmount = 350;
                stSharpenParam.nSharpenRadius = 10;
                stSharpenParam.nSharpenThreshold = 50;
                nRet = MV_CC_ImageSharpen(handle, &stSharpenParam);
                if (MV_OK != nRet)
                {
                    printf("Image Sharpen fail! nRet [0x%x]\n", nRet);
                    break;
                }
            }

            FILE* fp = NULL;
            errno_t err = fopen_s(&fp, "BeforeEnhance.raw", "wb");
            if (0 != err || NULL == fp)
            {
                printf("Open file failed\n");
                nRet = MV_E_RESOURCE;
                break;
            }
            fwrite(stImageInfo.pBufAddr, 1, stImageInfo.stFrameInfo.nFrameLen, fp);
            fclose(fp);

            MV_CC_FreeImageBuffer(handle, &stImageInfo);

            err = fopen_s(&fp, "AfterEnhance.raw", "wb");
            if (0 != err || NULL == fp)
            {
                printf("Open file failed\n");
                nRet = MV_E_RESOURCE;
                break;
            }
            fwrite(pDstData, 1, stImageInfo.stFrameInfo.nFrameLen, fp);
            fclose(fp);
            printf("Image enhance succeed\n");
        }
        else
        {
            printf("Get Image fail! nRet [0x%x]\n", nRet);
        }

        // ch:ֹͣȡ�� | en:Stop grab image
        nRet = MV_CC_StopGrabbing(handle);
        if (MV_OK != nRet)
        {
            printf("Stop Grabbing fail! nRet [0x%x]\n", nRet);
            break;
        }

        // ch:�ر��豸 | en:Close device
        nRet = MV_CC_CloseDevice(handle);
        if (MV_OK != nRet)
        {
            printf("Close Device fail! nRet [0x%x]\n", nRet);
            break;
        }

        // ch:���پ�� | en:Destroy handle
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
