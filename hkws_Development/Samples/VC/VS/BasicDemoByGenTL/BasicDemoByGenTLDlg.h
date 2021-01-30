
// BasicDemoByGenTLDlg.h : header file
#pragma once
#include "afxwin.h" 
#include "MvCamera.h"

// CBasicDemoByGenTLDlg dialog
class CBasicDemoByGenTLDlg : public CDialog
{
// Construction
public:
	CBasicDemoByGenTLDlg(CWnd* pParent = NULL);	// Standard constructor

    // Dialog Data
	enum { IDD = IDD_BasicDemoByGenTL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

/*ch:�ؼ���Ӧ���� | en:Control corresponding variable*/
private:
    CComboBox           m_ctrlInterfaceCombo;                 // ch:ö�ٵ���Interface | en:Enumerated Interface
    CComboBox           m_ctrlDeviceCombo;                    // ch:ö�ٵ����豸 | en:Enumerated device
    int                 m_nDeviceCombo;

    BOOL                m_bSoftWareTriggerCheck;

private:
    /*ch:�ʼʱ�Ĵ��ڳ�ʼ�� | en:Window initialization*/
    void DisplayWindowInitial();

    void EnableControls(BOOL bIsCameraReady);
    void ShowErrorMsg(CString csMessage, int nErrorNum);

    int SetTriggerMode();                // ch:���ô���ģʽ | en:Set Trigger Mode
    int GetTriggerMode();
    int GetTriggerSource();              // ch:���ô���Դ | en:Set Trigger Source
    int SetTriggerSource();

    int CloseDevice();                   // ch:�ر��豸 | en:Close Device

    // ch:ȥ���Զ�������ظ�ʽ | en:Remove custom pixel formats
    bool RemoveCustomPixelFormats(enum MvGvspPixelType enPixelFormat);

private:
    BOOL                    m_bOpenDevice;                        // ch:�Ƿ���豸 | en:Whether to open device
    BOOL                    m_bStartGrabbing;                     // ch:�Ƿ�ʼץͼ | en:Whether to start grabbing
    int                     m_nTriggerMode;                       // ch:����ģʽ | en:Trigger Mode
    int                     m_nTriggerSource;                     // ch:����Դ | en:Trigger Source

    CMvCamera*              m_pcMyCamera;               // ch:CMyCamera��װ�˳��ýӿ� | en:CMyCamera packed commonly used interface
    HWND                    m_hwndDisplay;                        // ch:��ʾ��� | en:Display Handle
    MV_GENTL_DEV_INFO_LIST  m_stDevList;         // ch:�豸��Ϣ�б�ṹ������������洢�豸�б�
    MV_GENTL_IF_INFO_LIST   m_stIFList;          

    void*                   m_hGrabThread;              // ch:ȡ���߳̾�� | en:Grab thread handle
    BOOL                    m_bThreadState;

public:
    /*ch:ö�� | en:Enumerate*/
    afx_msg void OnBnClickedEnumIfButton();             // ch:ö��Interface | en:Enum Interface
    afx_msg void OnBnClickedEnumDevButton();            // ch:ö���豸 | en:Enum Device

    /*ch:��ʼ�� | en:Initialization*/
    afx_msg void OnBnClickedOpenButton();               // ch:���豸 | en:Open Devices
    afx_msg void OnBnClickedCloseButton();              // ch:�ر��豸 | en:Close Devices
   
    /*ch:ͼ��ɼ� | en:Image Acquisition*/
    afx_msg void OnBnClickedContinusModeRadio();        // ch:����ģʽ | en:Continus Mode
    afx_msg void OnBnClickedTriggerModeRadio();         // ch:����ģʽ | en:Trigger Mode
    afx_msg void OnBnClickedStartGrabbingButton();      // ch:��ʼ�ɼ� | en:Start Grabbing
    afx_msg void OnBnClickedStopGrabbingButton();       // ch:�����ɼ� | en:Stop Grabbing
    afx_msg void OnBnClickedSoftwareTriggerCheck();     // ch:���� | en:Software Trigger
    afx_msg void OnBnClickedSoftwareOnceButton();       // ch:����һ�� | en:Software Trigger Execute Once
    afx_msg void OnClose();

    virtual BOOL PreTranslateMessage(MSG* pMsg);
    int GrabThreadProcess();

};
