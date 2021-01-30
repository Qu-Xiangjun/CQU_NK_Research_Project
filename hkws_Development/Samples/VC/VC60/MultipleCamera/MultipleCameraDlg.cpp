// MultipleCameraDlg.cpp : implementation file
#include "stdafx.h"
#include "MultipleCamera.h"
#include "MultipleCameraDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CAboutDlg dialog used for App About
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// CMultipleCameraDlg dialog
CMultipleCameraDlg::CMultipleCameraDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMultipleCameraDlg::IDD, pParent)
    , m_nValidCamNum(0)
    , m_bOpenDevice(FALSE)
    , m_bStartGrabbing(FALSE)
    , m_nTriggerMode(MV_TRIGGER_MODE_OFF)
    , m_nCurCameraIndex(-1)
    , m_nZoomInIndex(-1)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    for (int i = 0; i < MAX_DEVICE_NUM; i++)
    {
        m_bCamCheck[i] = FALSE;
        m_hwndDisplay[i] = NULL;
        m_pcMyCamera[i] = NULL;
        m_hGrabThread[i] = NULL;
        m_pSaveImageBuf[i] = NULL;
        m_nSaveImageBufSize[i] = 0;
        memset(&(m_stImageInfo[i]), 0, sizeof(MV_FRAME_OUT_INFO_EX));
    }
}

void CMultipleCameraDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    for (int i = 0; i < MAX_DEVICE_NUM; i++)
    {
        DDX_Check(pDX, IDC_CAM1_CHECK+i, m_bCamCheck[i]);
    }
    DDX_Control(pDX, IDC_OUTPUT_INFO_LIST, m_ctrlListBoxInfo);
}

BEGIN_MESSAGE_MAP(CMultipleCameraDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	// }}AFX_MSG_MAP
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_ENUM_DEVICES_BUTTON, CMultipleCameraDlg::OnBnClickedEnumDevicesButton)
    ON_BN_CLICKED(IDC_OPEN_DEVICES_BUTTON, CMultipleCameraDlg::OnBnClickedOpenDevicesButton)
    ON_BN_CLICKED(IDC_CLOSE_DEVICES_BUTTON, CMultipleCameraDlg::OnBnClickedCloseDevicesButton)
    ON_BN_CLICKED(IDC_START_GRABBING_BUTTON, CMultipleCameraDlg::OnBnClickedStartGrabbingButton)
    ON_BN_CLICKED(IDC_STOP_GRABBING_BUTTON, CMultipleCameraDlg::OnBnClickedStopGrabbingButton)
    ON_BN_CLICKED(IDC_CONTINUS_MODE_RADIO, CMultipleCameraDlg::OnBnClickedContinusModeRadio)
    ON_BN_CLICKED(IDC_TRIGGER_MODE_RADIO, CMultipleCameraDlg::OnBnClickedTriggerModeRadio)
    ON_BN_CLICKED(IDC_SOFTWARE_MODE_BUTTON, CMultipleCameraDlg::OnBnClickedSoftwareModeButton)
    ON_BN_CLICKED(IDC_HARDWARE_MODE_BUTTON, CMultipleCameraDlg::OnBnClickedHardwareModeButton)
    ON_BN_CLICKED(IDC_SOFTWARE_ONCE_BUTTON, CMultipleCameraDlg::OnBnClickedSoftwareOnceButton)
    ON_BN_CLICKED(IDC_SAVE_IMAGE_BUTTON, CMultipleCameraDlg::OnBnClickedSaveImageButton)
    ON_WM_CLOSE()
    ON_LBN_DBLCLK(IDC_OUTPUT_INFO_LIST, CMultipleCameraDlg::OnLbnDblclkOutputInfoList)
    ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

// ch:�����߳� | en:Working thread
unsigned int   __stdcall   WorkThread(void* pUser)
{
    if (pUser)
    {
        CMultipleCameraDlg* pCam = (CMultipleCameraDlg*)pUser;
        if (NULL == pCam)
        {
            return -1;
        }
        int nCurCameraIndex = pCam->m_nCurCameraIndex;
        pCam->m_nCurCameraIndex = -1;       
        pCam->ThreadFun(nCurCameraIndex);

        return 0;
    }

    return -1;
}

// CMultipleCameraDlg message handlers
BOOL CMultipleCameraDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

    // IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	OnBnClickedEnumDevicesButton();

    for (unsigned int i = 0; i < MAX_DEVICE_NUM; i++)
    {
        m_hwndDisplay[i] = GetDlgItem(IDC_DISPLAY1_STATIC+i)->GetSafeHwnd();
        GetDlgItem(IDC_DISPLAY1_STATIC+i)->GetWindowRect(&m_hwndRect[i]);
        ScreenToClient(&m_hwndRect[i]);
        InitializeCriticalSection(&m_hSaveImageMux[i]);
    }

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMultipleCameraDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CMultipleCameraDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMultipleCameraDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// ch:��ťʹ�� | en:Enable control
void CMultipleCameraDlg::EnableControls()
{
    // ch:�ɼ��������� | en:Acquisition control area
    GetDlgItem(IDC_ENUM_DEVICES_BUTTON)->EnableWindow(m_bOpenDevice ? FALSE : TRUE);
    GetDlgItem(IDC_OPEN_DEVICES_BUTTON)->EnableWindow(m_bOpenDevice ? FALSE : TRUE);
    GetDlgItem(IDC_CLOSE_DEVICES_BUTTON)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);

    GetDlgItem(IDC_START_GRABBING_BUTTON)->EnableWindow(m_bStartGrabbing ? FALSE : m_bOpenDevice);
    GetDlgItem(IDC_STOP_GRABBING_BUTTON)->EnableWindow(m_bStartGrabbing ? m_bOpenDevice : FALSE);
    GetDlgItem(IDC_SAVE_IMAGE_BUTTON)->EnableWindow(m_bStartGrabbing ? m_bOpenDevice : FALSE);
    GetDlgItem(IDC_SOFTWARE_MODE_BUTTON)->EnableWindow(m_nTriggerMode ? m_bOpenDevice : FALSE);
    GetDlgItem(IDC_HARDWARE_MODE_BUTTON)->EnableWindow(m_nTriggerMode ? m_bOpenDevice : FALSE);
    GetDlgItem(IDC_SOFTWARE_ONCE_BUTTON)->EnableWindow(m_nTriggerSource == MV_TRIGGER_SOURCE_SOFTWARE && m_nTriggerMode == MV_TRIGGER_MODE_ON ? m_bOpenDevice : FALSE);
    GetDlgItem(IDC_CONTINUS_MODE_RADIO)->EnableWindow(m_bOpenDevice ? m_bOpenDevice : FALSE);
    GetDlgItem(IDC_TRIGGER_MODE_RADIO)->EnableWindow(m_bOpenDevice ? m_bOpenDevice : FALSE);
}

void CMultipleCameraDlg::PrintMessage(const char* pszFormat, ... )
{
    va_list args;
    va_start(args, pszFormat);
    char   szInfo[512] = {0};
    vsprintf(szInfo, pszFormat, args);
    va_end(args);
    m_ctrlListBoxInfo.AddString(szInfo);
    m_ctrlListBoxInfo.SetTopIndex(m_ctrlListBoxInfo.GetCount() - 1);
}

// ch:���ô���ģʽ | en:Set trigger mode
void CMultipleCameraDlg::SetTriggerMode(void)
{
    int nRet = MV_OK;
    for (unsigned int i = 0; i < MAX_DEVICE_NUM; i++)
    {
        if (m_pcMyCamera[i])
        {
            nRet = m_pcMyCamera[i]->SetEnumValue("TriggerMode", m_nTriggerMode);
            if (MV_OK != nRet)
            {
                PrintMessage("Set Trigger mode fail! DevIndex[%d], TriggerMode[%d], nRet[%#x]\r\n", i+1, m_nTriggerMode, nRet);
            }
        }
    }
}

// ch:���ô���Դ | en:Set trigger source
void CMultipleCameraDlg::SetTriggerSource(void)
{
    int nRet = MV_OK;
    for (unsigned int i = 0; i < MAX_DEVICE_NUM; i++)
    {
        if (m_pcMyCamera[i])
        {
            nRet = m_pcMyCamera[i]->SetEnumValue("TriggerSource", m_nTriggerSource);
            if (MV_OK != nRet)
            {
                PrintMessage("Set Trigger source fail! DevIndex[%d], TriggerSource[%d], nRet[%#x]\r\n", i+1, m_nTriggerSource, nRet);
            }
        }
    }
}

// ch:����һ�� | en:Software trigger once
void CMultipleCameraDlg::DoSoftwareOnce(void)
{
    int nRet = MV_OK;
    for (unsigned int i = 0; i < MAX_DEVICE_NUM; i++)
    {
        if (m_pcMyCamera[i])
        {
            nRet = m_pcMyCamera[i]->CommandExecute("TriggerSoftware");
            if (MV_OK != nRet)
            {
                PrintMessage("Soft trigger fail! DevIndex[%d], nRet[%#x]\r\n", i+1, nRet);
            }
        }
    }
}

void CMultipleCameraDlg::OnTimer(UINT_PTR nIDEvent)
{
    if(nIDEvent == WM_TIMER_GRAB_INFO)
    {
        int nRet = 0;
        unsigned int nLostFrame = 0;
        unsigned int nFrameCount = 0;
        MV_MATCH_INFO_NET_DETECT stMatchInfoNetDetect = {0};
        MV_MATCH_INFO_USB_DETECT stMatchInfoUSBDetect = {0};

        for (int i = 0; i < MAX_DEVICE_NUM; i++)
        {
            if (m_pcMyCamera[i])
            {
                MV_CC_DEVICE_INFO stDevInfo = {0};
                m_pcMyCamera[i]->GetDeviceInfo(&stDevInfo);

                if (stDevInfo.nTLayerType == MV_GIGE_DEVICE)
                {
                    nRet = m_pcMyCamera[i]->GetGevAllMatchInfo(&stMatchInfoNetDetect);
                    nLostFrame = stMatchInfoNetDetect.nLostFrameCount;
                    nFrameCount = stMatchInfoNetDetect.nNetRecvFrameCount;
                }
                else if (stDevInfo.nTLayerType == MV_USB_DEVICE)
                {
                    nRet = m_pcMyCamera[i]->GetU3VAllMatchInfo(&stMatchInfoUSBDetect);
                    nLostFrame = stMatchInfoUSBDetect.nErrorFrameCount;
                    nFrameCount = stMatchInfoUSBDetect.nReceivedFrameCount;
                }
                else
                {
                    return;
                }

                if (MV_OK == nRet)
                {
                    char chFrameCount[128] = {0};

                    sprintf(chFrameCount, "FrameCount:%d,LostFrame:%d", nFrameCount, nLostFrame);
                    GetDlgItem(IDC_FRAME_COUNT1_STATIC + i)->SetWindowText(chFrameCount);
                }
            }
        }
    }

    CDialog::OnTimer(nIDEvent);
}

int CMultipleCameraDlg::ThreadFun(int nCurCameraIndex)
{
    if (m_pcMyCamera[nCurCameraIndex])
    {
        MV_FRAME_OUT stImageOut = {0};
        MV_DISPLAY_FRAME_INFO stDisplayInfo = {0};
        while(m_bStartGrabbing)
        {
            int nRet = m_pcMyCamera[nCurCameraIndex]->GetImageBuffer(&stImageOut, 1000);
            if (nRet == MV_OK)
            {
                //���ڱ���ͼƬ
                EnterCriticalSection(&m_hSaveImageMux[nCurCameraIndex]);
                if (NULL == m_pSaveImageBuf[nCurCameraIndex] || stImageOut.stFrameInfo.nFrameLen > m_nSaveImageBufSize[nCurCameraIndex])
                {
                    if (m_pSaveImageBuf[nCurCameraIndex])
                    {
                        free(m_pSaveImageBuf[nCurCameraIndex]);
                        m_pSaveImageBuf[nCurCameraIndex] = NULL;
                    }

                    m_pSaveImageBuf[nCurCameraIndex] = (unsigned char *)malloc(sizeof(unsigned char) * stImageOut.stFrameInfo.nFrameLen);
                    if (m_pSaveImageBuf[nCurCameraIndex] == NULL)
                    {
                        EnterCriticalSection(&m_hSaveImageMux[nCurCameraIndex]);
                        return 0;
                    }
                    m_nSaveImageBufSize[nCurCameraIndex] = stImageOut.stFrameInfo.nFrameLen;
                }
                memcpy(m_pSaveImageBuf[nCurCameraIndex], stImageOut.pBufAddr, stImageOut.stFrameInfo.nFrameLen);
                memcpy(&(m_stImageInfo[nCurCameraIndex]), &(stImageOut.stFrameInfo), sizeof(MV_FRAME_OUT_INFO_EX));
                LeaveCriticalSection(&m_hSaveImageMux[nCurCameraIndex]);

                stDisplayInfo.hWnd = m_hwndDisplay[nCurCameraIndex];
                stDisplayInfo.pData = stImageOut.pBufAddr;
                stDisplayInfo.nDataLen = stImageOut.stFrameInfo.nFrameLen;
                stDisplayInfo.nWidth = stImageOut.stFrameInfo.nWidth;
                stDisplayInfo.nHeight = stImageOut.stFrameInfo.nHeight;
                stDisplayInfo.enPixelType = stImageOut.stFrameInfo.enPixelType;

                nRet = m_pcMyCamera[nCurCameraIndex]->DisplayOneFrame(&stDisplayInfo);
                if (MV_OK != nRet)
                {
                    PrintMessage("Display one frame fail! DevIndex[%d], nRet[%#x]\r\n", nCurCameraIndex+1, nRet);
                }

                nRet = m_pcMyCamera[nCurCameraIndex]->FreeImageBuffer(&stImageOut);
                if (MV_OK != nRet)
                {
                    PrintMessage("Free image buffer fail! DevIndex[%d], nRet[%#x]\r\n", nCurCameraIndex+1, nRet);
                }
            }
            else
            {
                if (MV_TRIGGER_MODE_ON ==  m_nTriggerMode)
                {
                    Sleep(5);
                }
            }
        }
    }

    return MV_OK;
}

void CMultipleCameraDlg::OnBnClickedEnumDevicesButton()
{
    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    int nRet = CMvCamera::EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &m_stDevList);
    if (MV_OK != nRet || m_stDevList.nDeviceNum == 0)
    {
        PrintMessage("Find no device!\r\n");
        return;
    }
    PrintMessage("Find %d devices!\r\n", m_stDevList.nDeviceNum);

    m_nValidCamNum = 0;

    for (unsigned int i = 0; i < MAX_DEVICE_NUM; i++)
    {
        char chDeviceName[128] = {0};
        if (i < m_stDevList.nDeviceNum)
        {
            MV_CC_DEVICE_INFO* pDeviceInfo = m_stDevList.pDeviceInfo[i];
            if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
            {
                if (strcmp("", (LPCSTR)(pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName)) != 0)
                {
                    sprintf(chDeviceName, "%s [%s]", pDeviceInfo->SpecialInfo.stGigEInfo.chModelName, pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
                }
                else
                {
                    sprintf(chDeviceName, "%s (%s)", pDeviceInfo->SpecialInfo.stGigEInfo.chModelName, pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber);
                }
            }
            else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE)
            {
                if (strcmp("", (LPCSTR)(pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName)) != 0)
                {
                    sprintf(chDeviceName, "%s [%s]", pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName, pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
                }
                else
                {
                    sprintf(chDeviceName, "%s (%s)", pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName, pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
                }
            }
            GetDlgItem(IDC_CAM1_CHECK + i)->EnableWindow(TRUE);
            m_nValidCamNum++;
        }
        else
        {
            sprintf(chDeviceName, "Cam%d", i+1);
            GetDlgItem(IDC_CAM1_CHECK + i)->EnableWindow(FALSE);
        }

        GetDlgItem(IDC_CAM1_CHECK + i)->SetWindowText(chDeviceName);
        m_bCamCheck[i] = FALSE;
    }
    UpdateData(FALSE);
}

// ch:��ʼ��������д�������� | en:Initialzation, include opening device
void CMultipleCameraDlg::OnBnClickedOpenDevicesButton()
{
    if (TRUE == m_bOpenDevice)
    {
        return;
    }

    UpdateData(TRUE);
    BOOL bHaveCheck = FALSE;

    for (unsigned int i = 0; i < MAX_DEVICE_NUM; i++)
    {
        if (m_bCamCheck[i])//�ѹ�ѡ�����
        {
            bHaveCheck = TRUE;
            if (NULL == m_pcMyCamera[i])
            {
                m_pcMyCamera[i] = new CMvCamera;
            }

            int nRet = m_pcMyCamera[i]->Open(m_stDevList.pDeviceInfo[i]);
            if (MV_OK != nRet)
            {
                delete(m_pcMyCamera[i]);
                m_pcMyCamera[i] = NULL;

                PrintMessage("Open device failed! DevIndex[%d], nRet[%#x]\r\n", i+1, nRet);
                continue;
            }
            else
            {
                m_bOpenDevice = TRUE;
                // ch:̽��������Ѱ���С(ֻ��GigE�����Ч) | en:Detection network optimal package size(It only works for the GigE camera)
                if (m_stDevList.pDeviceInfo[i]->nTLayerType == MV_GIGE_DEVICE)
                {
                    unsigned int nPacketSize = 0;
                    nRet = m_pcMyCamera[i]->GetOptimalPacketSize(&nPacketSize);
                    if (nPacketSize > 0)
                    {
                        nRet = m_pcMyCamera[i]->SetIntValue("GevSCPSPacketSize",nPacketSize);
                        if(nRet != MV_OK)
                        {
                            PrintMessage("Set Packet Size fail! DevIndex[%d], nRet[%#x]\r\n", i+1, nRet);
                        }
                    }
                    else
                    {
                        PrintMessage("Get Packet Size fail! DevIndex[%d], nRet[%#x]\r\n", i+1, nRet);
                    }
                }
            }
        }
    }

    if (TRUE == m_bOpenDevice)
    {
        OnBnClickedContinusModeRadio();
        EnableControls();
    }
    else
    {
        if (FALSE == bHaveCheck)
        {
            PrintMessage("Unchecked device!\r\n");
        }
        else
        {
            PrintMessage("No device opened successfully!\r\n");
        }
    }

    for (unsigned int j = 0; j < MAX_DEVICE_NUM; j++)
    {
        if (j < m_nValidCamNum)
        {
            GetDlgItem(IDC_CAM1_CHECK + j)->EnableWindow(!m_bOpenDevice);
        }
    }
}

// ch:�رգ��������پ�� | en:Close, include destroy handle
void CMultipleCameraDlg::OnBnClickedCloseDevicesButton()
{
    KillTimer(WM_TIMER_GRAB_INFO);
    m_bStartGrabbing = FALSE;
    for (int m = 0; m < MAX_DEVICE_NUM; m++)
    {
        if (m_hGrabThread[m])
        {
            WaitForSingleObject(m_hGrabThread[m], INFINITE);
            CloseHandle(m_hGrabThread[m]);
            m_hGrabThread[m] = NULL;
        }
    }

    int nRet = MV_OK;
    for (unsigned int j = 0; j < MAX_DEVICE_NUM; j++)
    {
        if (m_pcMyCamera[j])
        {
            nRet = m_pcMyCamera[j]->Close();
            if (MV_OK != nRet)
            {
                PrintMessage("Close device fail! DevIndex[%d], nRet[%#x]\r\n", j+1, nRet);
            }

            delete(m_pcMyCamera[j]);
            m_pcMyCamera[j] = NULL;
        }

        if (j < m_nValidCamNum)
        {
            GetDlgItem(IDC_CAM1_CHECK + j)->EnableWindow(TRUE);
            GetDlgItem(IDC_FRAME_COUNT1_STATIC + j)->SetWindowText("FrameCount:0,LostFrame:0");
        }

        if (m_pSaveImageBuf[j])
        {
            free(m_pSaveImageBuf[j]);
            m_pSaveImageBuf[j] = NULL;
        }
        m_nSaveImageBufSize[j] = 0;
    }

    m_bOpenDevice = FALSE;
    //�ر����ʱ�ָ����ڴ�С
    if (m_nZoomInIndex != -1)
    {
        CPoint point;
        OnLButtonDblClk(0, point);
    }

    EnableControls();
    Invalidate();//ˢ��������
}

// ch:��ʼץͼ | en:Start grabbing
void CMultipleCameraDlg::OnBnClickedStartGrabbingButton()
{
    if (FALSE == m_bOpenDevice || TRUE == m_bStartGrabbing)
    {        
        return;
    }

    int nRet = MV_OK;

    SetTimer(WM_TIMER_GRAB_INFO,300,NULL);
    for (int i = 0; i < MAX_DEVICE_NUM; i++)
    {
        if (m_pcMyCamera[i])
        {
            memset(&(m_stImageInfo[i]), 0, sizeof(MV_FRAME_OUT_INFO_EX));

            nRet = m_pcMyCamera[i]->StartGrabbing();
            if (MV_OK != nRet)
            {
                PrintMessage("Start grabbing fail! DevIndex[%d], nRet[%#x]\r\n", i+1, nRet);
            }
            m_bStartGrabbing = TRUE;

            // ch:��ʼ�ɼ�֮��Ŵ���workthread�߳�
            unsigned int nCount = 0;
            while(-1 != m_nCurCameraIndex)
            {
                nCount++;
                if (nCount > 50)
                {
                    return;
                }

                Sleep(2);
            }
            unsigned int nThreadID = 0;
            m_nCurCameraIndex = i;
            m_hGrabThread[i] = (void*) _beginthreadex( NULL , 0 , WorkThread , this, 0 , &nThreadID );
            if (NULL == m_hGrabThread[i])
            {
                PrintMessage("Create grab thread fail! DevIndex[%d]\r\n", i+1);
            }
        }
    }

    EnableControls();
}

// ch:����ץͼ | en:Stop grabbing
void CMultipleCameraDlg::OnBnClickedStopGrabbingButton()
{
    if (FALSE == m_bOpenDevice || FALSE == m_bStartGrabbing)
    {        
        return;
    }

    KillTimer(WM_TIMER_GRAB_INFO);

    int nRet = MV_OK;
    m_bStartGrabbing = FALSE;
    for (unsigned int i = 0; i < MAX_DEVICE_NUM; i++)
    {
        if (m_pcMyCamera[i])
        {
            nRet = m_pcMyCamera[i]->StopGrabbing();
            if (MV_OK != nRet)
            {
                PrintMessage("Stop grabbing fail! DevIndex[%d], nRet[%#x]\r\n", i+1, nRet);
            }
        }

        if (m_hGrabThread[i])
        {
            WaitForSingleObject(m_hGrabThread[i], INFINITE);
            CloseHandle(m_hGrabThread[i]);
            m_hGrabThread[i] = NULL;
        }
    }

    EnableControls();
}

// ch:��������ģʽ | en:Set continuous mode
void CMultipleCameraDlg::OnBnClickedContinusModeRadio()
{
    ((CButton *)GetDlgItem(IDC_CONTINUS_MODE_RADIO))->SetCheck(TRUE);
    ((CButton *)GetDlgItem(IDC_TRIGGER_MODE_RADIO))->SetCheck(FALSE);
    m_nTriggerMode = MV_TRIGGER_MODE_OFF;
    SetTriggerMode();

    GetDlgItem(IDC_SOFTWARE_MODE_BUTTON)->EnableWindow(FALSE);
    GetDlgItem(IDC_HARDWARE_MODE_BUTTON)->EnableWindow(FALSE);
    GetDlgItem(IDC_SOFTWARE_ONCE_BUTTON)->EnableWindow(FALSE);
}

// ch:���ô���ģʽ | en:Set trigger mode
void CMultipleCameraDlg::OnBnClickedTriggerModeRadio()
{
    ((CButton *)GetDlgItem(IDC_CONTINUS_MODE_RADIO))->SetCheck(FALSE);
    ((CButton *)GetDlgItem(IDC_TRIGGER_MODE_RADIO))->SetCheck(TRUE);
    m_nTriggerMode = MV_TRIGGER_MODE_ON;
    SetTriggerMode();

    GetDlgItem(IDC_SOFTWARE_MODE_BUTTON)->EnableWindow(TRUE);
    GetDlgItem(IDC_HARDWARE_MODE_BUTTON)->EnableWindow(TRUE);
}

// ch:����ģʽΪ������� | en:Software trigger
void CMultipleCameraDlg::OnBnClickedSoftwareModeButton()
{
    m_nTriggerSource = MV_TRIGGER_SOURCE_SOFTWARE;
    // ch:����Ϊ����ģʽ | en:Set trigger mode as software trigger
    SetTriggerSource();

    EnableControls();
}

// ch:����ģʽΪӲ������ | en:Hardware trigger
void CMultipleCameraDlg::OnBnClickedHardwareModeButton()
{
    m_nTriggerSource = MV_TRIGGER_SOURCE_LINE0;
    // ch:����ΪӲ����ģʽ | en:Set trigger mode as hardware trigger
    SetTriggerSource();

    EnableControls();
}

// ch:����һ�� | en:Software trigger
void CMultipleCameraDlg::OnBnClickedSoftwareOnceButton()
{
    if (FALSE == m_bStartGrabbing)
    {
        PrintMessage("Please start grabbing first!\r\n");
        return;
    }

    DoSoftwareOnce();
}

void CMultipleCameraDlg::OnBnClickedSaveImageButton()
{
    if (FALSE == m_bStartGrabbing)
    {        
        return;
    }

    MV_SAVE_IMG_TO_FILE_PARAM stSaveFileParam;
    memset(&stSaveFileParam, 0, sizeof(MV_SAVE_IMG_TO_FILE_PARAM));

    for (int i = 0; i < MAX_DEVICE_NUM; i++)
    {
        if (m_pcMyCamera[i])
        {
            EnterCriticalSection(&m_hSaveImageMux[i]);
            if (m_pSaveImageBuf[i] == NULL || m_stImageInfo[i].enPixelType == 0)
            {
                PrintMessage("Save Image fail! No data! DevIndex[%d]\r\n", i+1);
                LeaveCriticalSection(&m_hSaveImageMux[i]);
                continue;
            }

            stSaveFileParam.enImageType = MV_Image_Bmp; // ch:��Ҫ�����ͼ������ | en:Image format to save
            stSaveFileParam.enPixelType = m_stImageInfo[i].enPixelType;  // ch:�����Ӧ�����ظ�ʽ | en:Camera pixel type
            stSaveFileParam.nWidth      = m_stImageInfo[i].nWidth;         // ch:�����Ӧ�Ŀ� | en:Width
            stSaveFileParam.nHeight     = m_stImageInfo[i].nHeight;          // ch:�����Ӧ�ĸ� | en:Height
            stSaveFileParam.nDataLen    = m_stImageInfo[i].nFrameLen;
            stSaveFileParam.pData       = m_pSaveImageBuf[i];
            stSaveFileParam.iMethodValue = 0;
            sprintf(stSaveFileParam.pImagePath, "Image%d_w%d_h%d_fn%03d.bmp", i, stSaveFileParam.nWidth, stSaveFileParam.nHeight, m_stImageInfo[i].nFrameNum);

            int nRet = m_pcMyCamera[i]->SaveImageToFile(&stSaveFileParam);
            LeaveCriticalSection(&m_hSaveImageMux[i]);
            if (MV_OK != nRet)
            {
                PrintMessage("Save Image fail! DevIndex[%d], nRet[%#x]\r\n", i+1, nRet);
            }
        }
    }
}

void CMultipleCameraDlg::OnClose()
{
    KillTimer(WM_TIMER_GRAB_INFO);
    m_bStartGrabbing = FALSE;
    for (int i = 0; i < MAX_DEVICE_NUM; i++)
    {
        if (m_hGrabThread[i])
        {
            WaitForSingleObject(m_hGrabThread[i], INFINITE);
            CloseHandle(m_hGrabThread[i]);
            m_hGrabThread[i] = NULL;
        }

        if (m_pcMyCamera[i])
        {
            delete(m_pcMyCamera[i]);
            m_pcMyCamera[i] = NULL;
        }
        DeleteCriticalSection(&m_hSaveImageMux[i]);
    }
    m_bOpenDevice = FALSE;

    CDialog::OnClose();
}

BOOL CMultipleCameraDlg::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN &&pMsg->wParam == VK_ESCAPE)
    {
        return TRUE;
    }
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
    {
        return TRUE;
    }

    return CDialog::PreTranslateMessage(pMsg);
}

void CMultipleCameraDlg::OnLbnDblclkOutputInfoList()
{
    m_ctrlListBoxInfo.ResetContent();
}

void CMultipleCameraDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    if (m_nZoomInIndex == -1)
    {
        CRect Rect;
        GetDlgItem(IDC_IMAGE_SHOW_STATIC)->GetWindowRect(&Rect);
        ScreenToClient(Rect);
        Rect.top += 10;
        Rect.bottom -= 5;
        Rect.left += 5;
        Rect.right -= 5;

        for (unsigned int i = 0; i < MAX_DEVICE_NUM; i++)
        {
            //�ж��Ƿ�������������ѡ
            if (m_bOpenDevice == TRUE && m_bCamCheck[i] == TRUE)
            {
                //�ж�������Ƿ��ڿؼ���
                if(point.x> m_hwndRect[i].left&&point.x <m_hwndRect[i].right&&point.y <m_hwndRect[i].bottom&&point.y> m_hwndRect[i].top)
                {
                    for (unsigned int j = 0; j < MAX_DEVICE_NUM; j++)
                    {
                        if(i != j)
                        {
                            GetDlgItem(IDC_DISPLAY1_STATIC+j)->ShowWindow(FALSE);
                        }
                        GetDlgItem(IDC_CAM1_CHECK+j)->ShowWindow(FALSE);
                        GetDlgItem(IDC_FRAME_COUNT1_STATIC+j)->ShowWindow(FALSE);
                    }
                    GetDlgItem(IDC_DISPLAY1_STATIC+i)->MoveWindow(Rect, TRUE);
                    m_nZoomInIndex = i;
                    Invalidate();//ˢ��������
                    break;
                }
            }
        }
    }
    else
    {
        //��ʾ���ڻָ�ԭ�ȴ�С
        GetDlgItem(IDC_DISPLAY1_STATIC+m_nZoomInIndex)->MoveWindow(m_hwndRect[m_nZoomInIndex], TRUE);
        for (unsigned int i = 0; i < MAX_DEVICE_NUM; i++)
        {
            GetDlgItem(IDC_DISPLAY1_STATIC+i)->ShowWindow(TRUE);
            GetDlgItem(IDC_CAM1_CHECK+i)->ShowWindow(TRUE);
            GetDlgItem(IDC_FRAME_COUNT1_STATIC+i)->ShowWindow(TRUE);
        }
        m_nZoomInIndex = -1;
        Invalidate();//ˢ��������
    }

    CDialog::OnLButtonDblClk(nFlags, point);
}