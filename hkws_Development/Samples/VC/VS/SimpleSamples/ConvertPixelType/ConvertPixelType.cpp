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

bool IsColor(MvGvspPixelType enType)
{
    switch(enType)
    {
    case PixelType_Gvsp_BGR8_Packed:
    case PixelType_Gvsp_YUV422_Packed:
    case PixelType_Gvsp_YUV422_YUYV_Packed:
    case PixelType_Gvsp_BayerGR8:
    case PixelType_Gvsp_BayerRG8:
    case PixelType_Gvsp_BayerGB8:
    case PixelType_Gvsp_BayerBG8:
    case PixelType_Gvsp_BayerGB10:
    case PixelType_Gvsp_BayerGB10_Packed:
    case PixelType_Gvsp_BayerBG10:
    case PixelType_Gvsp_BayerBG10_Packed:
    case PixelType_Gvsp_BayerRG10:
    case PixelType_Gvsp_BayerRG10_Packed:
    case PixelType_Gvsp_BayerGR10:
    case PixelType_Gvsp_BayerGR10_Packed:
    case PixelType_Gvsp_BayerGB12:
    case PixelType_Gvsp_BayerGB12_Packed:
    case PixelType_Gvsp_BayerBG12:
    case PixelType_Gvsp_BayerBG12_Packed:
    case PixelType_Gvsp_BayerRG12:
    case PixelType_Gvsp_BayerRG12_Packed:
    case PixelType_Gvsp_BayerGR12:
    case PixelType_Gvsp_BayerGR12_Packed:
        return true;
    default:
        return false;
    }
}

bool IsMono(MvGvspPixelType enType)
{
    switch(enType)
    {
    case PixelType_Gvsp_Mono10:
    case PixelType_Gvsp_Mono10_Packed:
    case PixelType_Gvsp_Mono12:
    case PixelType_Gvsp_Mono12_Packed:
        return true;
    default:
        return false;
    }
}

int main()
{
    int nRet = MV_OK;
    void* handle = NULL;
    unsigned char *pConvertData = NULL;
    unsigned int nConvertDataSize = 0;

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

            MvGvspPixelType enDstPixelType = PixelType_Gvsp_Undefined;
            unsigned int nChannelNum = 0;
            char chFileName[MAX_PATH] = {0};
            //����ǲ�ɫ��ת��RGB8
            if (IsColor(stImageInfo.stFrameInfo.enPixelType))
            {
                nChannelNum = 3;
                enDstPixelType = PixelType_Gvsp_RGB8_Packed;
                sprintf(chFileName, "AfterConvert.rgb");
            }
            //����Ǻڰ���ת����Mono8
            else if (IsMono(stImageInfo.stFrameInfo.enPixelType))
            {
                nChannelNum = 1;
                enDstPixelType = PixelType_Gvsp_Mono8;
                sprintf(chFileName, "AfterConvert.gray");
            }
            else
            {
                printf("Don't need to convert!\n");
            }
            
            if (enDstPixelType != PixelType_Gvsp_Undefined)
            {
                pConvertData = (unsigned char*)malloc(stImageInfo.stFrameInfo.nWidth * stImageInfo.stFrameInfo.nHeight * nChannelNum);
                if (NULL == pConvertData)
                {
                    printf("malloc pConvertData fail!\n");
                    nRet = MV_E_RESOURCE;
                    break;
                }
                nConvertDataSize = stImageInfo.stFrameInfo.nWidth * stImageInfo.stFrameInfo.nHeight * nChannelNum;

                // ch:���ظ�ʽת�� | en:Convert pixel format 
                MV_CC_PIXEL_CONVERT_PARAM stConvertParam = {0};

                stConvertParam.nWidth = stImageInfo.stFrameInfo.nWidth;                 //ch:ͼ��� | en:image width
                stConvertParam.nHeight = stImageInfo.stFrameInfo.nHeight;               //ch:ͼ��� | en:image height
                stConvertParam.pSrcData = stImageInfo.pBufAddr;                         //ch:�������ݻ��� | en:input data buffer
                stConvertParam.nSrcDataLen = stImageInfo.stFrameInfo.nFrameLen;         //ch:�������ݴ�С | en:input data size
                stConvertParam.enSrcPixelType = stImageInfo.stFrameInfo.enPixelType;    //ch:�������ظ�ʽ | en:input pixel format
                stConvertParam.enDstPixelType = enDstPixelType;                         //ch:������ظ�ʽ | en:output pixel format
                stConvertParam.pDstBuffer = pConvertData;                               //ch:������ݻ��� | en:output data buffer
                stConvertParam.nDstBufferSize = nConvertDataSize;                       //ch:��������С | en:output buffer size
                nRet = MV_CC_ConvertPixelType(handle, &stConvertParam);
                if (MV_OK != nRet)
                {
                    printf("Convert Pixel Type fail! nRet [0x%x]\n", nRet);
                    break;
                }

                FILE* fp = NULL;
                errno_t err = fopen_s(&fp, chFileName, "wb");
                if (0 != err || NULL == fp)
                {
                    printf("Open file failed\n");
                    nRet = MV_E_RESOURCE;
                    break;
                }
                fwrite(stConvertParam.pDstBuffer, 1, stConvertParam.nDstLen, fp);
                fclose(fp);
                printf("Convert pixeltype succeed\n");
            }
            MV_CC_FreeImageBuffer(handle, &stImageInfo);
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

    if (pConvertData)
    {
        free(pConvertData);
        pConvertData = NULL;
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
