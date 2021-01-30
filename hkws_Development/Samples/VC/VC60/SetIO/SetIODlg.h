// SetIODlg.h : header file
#if !defined(AFX_SETIODLG_H__F8A0419E_1AAB_4C95_A0F9_E5DF4970850D__INCLUDED_)
#define AFX_SETIODLG_H__F8A0419E_1AAB_4C95_A0F9_E5DF4970850D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MvCamera.h"

// CSetIODlg dialog
class CSetIODlg : public CDialog
{
// Construction
public:
	CSetIODlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SETIO_DIALOG };

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

private:
    CComboBox               m_ctrlDeviceCombo;
    int                     m_nDeviceCombo;
    CButton                 m_ctrlOpenButton;
    CButton                 m_ctrlCloseButton;
    CButton                 m_ctrlGetLineselButton;
    CButton                 m_ctrlSetLinesetButton;
    CButton                 m_ctrlGetLinemodeButton;
    CButton                 m_ctrlSetLinemodeButton;
    CComboBox               m_ctrlLineSelectCombo;
    CComboBox               m_ctrlLinemodeCombo;
    int                     m_nLineSelectorCombo;
    int                     m_nLinemodeCombo;

    MV_CC_DEVICE_INFO_LIST  m_stDevList;
    CMvCamera*              m_pcMyCamera;               // ch:CMyCamera��װ�˳��ýӿ� | en:CMyCamera packed commonly used interface

private:
    void EnableWindowWhenInitial();          // ch:��ʾ���ʼ��,�ʼ�ĳ�ʼ�� | en:Display window initialization
    void EnableWindowWhenClose();         // ch:�ر�ʱ�İ�ť��ɫ���൱���ʼ��ʼ��
                                                // en:Button color when Close, equivalent to the initialization
    void EnableWindowWhenOpen();  // ch:���´��豸��ťʱ�İ�ť��ɫ | en:Button color when click Open Device

    void ShowErrorMsg(CString csMessage, int nErrorNum); // ch:��ʾ������Ϣ | en:Show error message

public:
    afx_msg void OnBnClickedEnumButton();
    afx_msg void OnBnClickedOpenButton();
    afx_msg void OnBnClickedCloseButton();

    afx_msg void OnBnClickedGetLineselButton();
    afx_msg void OnBnClickedSetLineselButton();
    afx_msg void OnBnClickedGetLinemodeButton();
    afx_msg void OnBnClickedSetLinemodeButton();

    virtual BOOL PreTranslateMessage(MSG* pMsg);
    
};
#endif // !defined(AFX_SETIODLG_H__F8A0419E_1AAB_4C95_A0F9_E5DF4970850D__INCLUDED_)
