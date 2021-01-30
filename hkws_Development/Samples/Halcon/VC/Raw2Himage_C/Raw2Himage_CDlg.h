
// Raw2Himage_CDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h" 
#include <stdio.h>
#include "MvCamera.h"
#include "ConvertPixel.h"

// CRaw2Himage_CDlg �Ի���
class CRaw2Himage_CDlg : public CDialog
{
// ����
public:
	CRaw2Himage_CDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_RAW2HIMAGE_C_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

    /*�ؼ���Ӧ����*/
private:
    /*��ʼ��*/
    CButton m_ctrlOpenButton;                   //���豸
    CButton m_ctrlCloseButton;                  //�ر��豸

    /*ͼ��ɼ�*/
    CButton m_ctrlContinusModeRadio;            //����ģʽ             
    CButton m_ctrlTriggerModeRadio;             //����ģʽ
    CButton m_ctrlStartGrabbingButton;          //��ʼץͼ          
    CButton m_ctrlStopGrabbingButton;           //����ץͼ
    CButton m_ctrlSoftwareTriggerCheck;         //����
    BOOL    m_bSoftWareTriggerCheck;
    CButton m_ctrlSoftwareOnceButton;           //����һ��

    /*�豸��ʾ������*/
    CComboBox m_ctrlDeviceCombo;                //ö�ٵ����豸
    int     m_nDeviceCombo;

    /*�ڲ�����*/
private:
    /*�ʼʱ�Ĵ��ڳ�ʼ��*/
    void DisplayWindowInitial(void);          //��ʾ���ʼ��,�ʼ�ĳ�ʼ�� 

    /*��ť������ʹ��*/
    void EnableWindowWhenClose(void);         //�ر�ʱ�İ�ť��ɫ���൱���ʼ��ʼ��
    void EnableWindowWhenOpenNotStart(void);  //���´��豸��ťʱ�İ�ť��ɫ  
    void EnableWindowWhenStart(void);         //���¿�ʼ�ɼ�ʱ�İ�ť��ɫ 
    void EnableWindowWhenStop(void);          //���½����ɼ�ʱ�İ�ť��ɫ

    /*��ʼ���������*/
    int OpenDevice(void);                    //�����

    /*���á���ȡ��������*/
    int SetTriggerMode(void);                //���ô���ģʽ
    int GetTriggerMode(void);
    int GetTriggerSource(void);              //���ô���Դ
    int SetTriggerSource(void);

    /*�ر����ٲ���*/
    int CloseDevice(void);                   //�ر��豸
    int DestroyHandle(void);                 //�����豸

    /*�Լ��������*/
private:
    BOOL  m_bOpenDevice;                        //�Ƿ���豸
    BOOL  m_bStartGrabbing;                     //�Ƿ�ʼץͼ

    BOOL  m_bThreadState;
    void* m_hGrabThread;                        //ȡ���߳̾��

    int   m_nTriggerMode;                       //����ģʽ
    int   m_nTriggerSource;                     //����Դ

    /*�豸���*/
    CMvCamera*      m_pcMyCamera;               //CMyCamera��װ�˳��ýӿ�
    HTuple  m_hWindowID;                        //Halcon��ʾ���
    MV_CC_DEVICE_INFO_LIST m_stDevList;         //�豸��Ϣ�б�ṹ������������洢�豸�б�
public:
    /*��ʼ��*/
    afx_msg void OnBnClickedEnumButton();               //�����豸
    afx_msg void OnBnClickedOpenButton();               //���豸
    afx_msg void OnBnClickedCloseButton();              //�ر��豸

    /*ͼ��ɼ�*/
    afx_msg void OnBnClickedContinusModeRadio();        //����ģʽ
    afx_msg void OnBnClickedTriggerModeRadio();         //����ģʽ
    afx_msg void OnBnClickedStartGrabbingButton();      //��ʼ�ɼ�
    afx_msg void OnBnClickedStopGrabbingButton();       //�����ɼ�
    afx_msg void OnBnClickedSoftwareTriggerCheck();     //����
    afx_msg void OnBnClickedSoftwareOnceButton();       //����һ��

    /*�������û�ȡ*/
    afx_msg void OnBnClickedGetParameterButton();       //��ȡ����

public:    
    afx_msg void OnClose();                             //���Ͻ��˳�
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    static unsigned int __stdcall WorkThread(void* pUser);
};
