#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include <io.h>
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

unsigned char * g_pDstData = NULL;
unsigned int g_nDstDataSize = 0;

unsigned char * g_pCalibBuf = NULL;
unsigned int g_nCalibBufSize = 0;

bool g_IsNeedCalib = true;

void __stdcall ImageCallBackEx(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{
    printf("Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n", pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo->nFrameNum);

    int nRet = MV_OK;
    //�ж��Ƿ���Ҫ�궨
    if (true == g_IsNeedCalib)
    {
        // LSC�궨
        MV_CC_LSC_CALIB_PARAM stLSCCalib = {0};
        stLSCCalib.nWidth = pFrameInfo->nWidth;
        stLSCCalib.nHeight = pFrameInfo->nHeight;
        stLSCCalib.enPixelType = pFrameInfo->enPixelType;
        stLSCCalib.pSrcBuf = pData;
        stLSCCalib.nSrcBufLen = pFrameInfo->nFrameLen;

        if (g_pCalibBuf == NULL || g_nCalibBufSize < (pFrameInfo->nWidth*pFrameInfo->nHeight*sizeof(unsigned short)))
        {
            if (g_pCalibBuf)
            {
                free(g_pCalibBuf);
                g_pCalibBuf = NULL;
                g_nCalibBufSize = 0;
            }

            g_pCalibBuf = (unsigned char *)malloc(pFrameInfo->nWidth*pFrameInfo->nHeight*sizeof(unsigned short));
            if (g_pCalibBuf == NULL)
            {
                printf("malloc pCalibBuf fail !\n");
                return;
            }
            g_nCalibBufSize = pFrameInfo->nWidth*pFrameInfo->nHeight*sizeof(unsigned short);
        }

        stLSCCalib.pCalibBuf = g_pCalibBuf;
        stLSCCalib.nCalibBufSize = g_nCalibBufSize;

        stLSCCalib.nSecNumW = 689;
        stLSCCalib.nSecNumH = 249;
        stLSCCalib.nPadCoef = 2;
        stLSCCalib.nCalibMethod = 2;
        stLSCCalib.nTargetGray = 100;

        //ͬһ��������ڳ������㷨������ͼ���������������£�������ֻ��Ҫ����һ�α궨���ɽ��궨����������
        //��ͬ���ͼƬ�궨�����ı궨���ɸ��ã��������ı���㷨�����ı��ͼ������ı�ʱ����Ҫ���±궨��
        nRet = MV_CC_LSCCalib(pUser, &stLSCCalib);
        if (MV_OK != nRet)
        {
            printf("LSC Calib fail! nRet [0x%x]\n", nRet);
            return;
        }

        //����궨������
        FILE* fp_out = fopen("./LSCCalib.bin", "wb+");
        if (NULL == fp_out)
        {
            return ;
        }
        fwrite(stLSCCalib.pCalibBuf, 1, stLSCCalib.nCalibBufLen, fp_out);
        fclose(fp_out);

        g_IsNeedCalib = false;
    }

    // LSCУ��
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

    MV_CC_LSC_CORRECT_PARAM stLSCCorrectParam = {0};

    stLSCCorrectParam.nWidth = pFrameInfo->nWidth;
    stLSCCorrectParam.nHeight = pFrameInfo->nHeight;
    stLSCCorrectParam.enPixelType = pFrameInfo->enPixelType;
    stLSCCorrectParam.pSrcBuf = pData;
    stLSCCorrectParam.nSrcBufLen = pFrameInfo->nFrameLen;

    stLSCCorrectParam.pDstBuf = g_pDstData;
    stLSCCorrectParam.nDstBufSize = g_nDstDataSize;

    stLSCCorrectParam.pCalibBuf = g_pCalibBuf;
    stLSCCorrectParam.nCalibBufLen = g_nCalibBufSize;
    nRet = MV_CC_LSCCorrect(pUser, &stLSCCorrectParam);
    if (MV_OK != nRet)
    {
        printf("LSC Correct fail! nRet [0x%x]\n", nRet);
        return;
    }

    if (pFrameInfo->nFrameNum < 10)
    {
        //����ͼ���ļ�
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

        // ch:���ô���ģʽΪoff | eb:Set trigger mode as off
        nRet = MV_CC_SetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_OFF);
        if (MV_OK != nRet)
        {
            printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet);
            break;
        }

        //�ж��Ƿ���Ա��ص���
        FILE* fp = fopen("./LSCCalib.bin", "rb+");
        if (fp)
        {
            int nFileLen = filelength(fileno(fp));
            if (g_pCalibBuf == NULL || g_nCalibBufSize < nFileLen)
            {
                if (g_pCalibBuf)
                {
                    free(g_pCalibBuf);
                    g_pCalibBuf = NULL;
                    g_nCalibBufSize = 0;
                }

                g_pCalibBuf = (unsigned char *)malloc(nFileLen);
                if (g_pCalibBuf == NULL)
                {
                    printf("malloc pCalibBuf fail !\n");
                    break;
                }
                g_nCalibBufSize = nFileLen;
            }
            fread(g_pCalibBuf, 1, g_nCalibBufSize, fp);
            fclose(fp);

            g_IsNeedCalib = false;
        }

        // ch:ע��ץͼ�ص� | en:Register image callback
        nRet = MV_CC_RegisterImageCallBackEx(handle, ImageCallBackEx, handle);
        if (MV_OK != nRet)
        {
            printf("Register Image CallBack fail! nRet [0x%x]\n", nRet);
            break;
        }

        // ch:��ʼȡ�� | en:Start grab image
        nRet = MV_CC_StartGrabbing(handle);
        if (MV_OK != nRet)
        {
            printf("Start Grabbing fail! nRet [0x%x]\n", nRet);
            break;
        }

        printf("Press a key to stop grabbing.\n");
        WaitForKeyPress();

        Sleep(1000);

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

    if (g_pCalibBuf)
    {
        free(g_pCalibBuf);
        g_pCalibBuf = NULL;
        g_nCalibBufSize = 0;
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
