// HID APDlg.h : header file
//

#pragma once


// CHIDAPDlg dialog
class CHIDAPDlg : public CDialog
{
// Construction
public:
	CHIDAPDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_HIDAP_DIALOG };

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
public:
	afx_msg void OnBnClickedOpen();
	afx_msg void OnBnClickedLed1();
	afx_msg void OnBnClickedLed5();
	afx_msg void OnBnClickedLed6();
	afx_msg void OnBnClickedLed7();
	afx_msg void OnBnClickedLed8();
	afx_msg void OnBnClickedClose();
};
