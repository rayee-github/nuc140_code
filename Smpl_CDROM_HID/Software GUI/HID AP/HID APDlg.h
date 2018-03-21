// HID APDlg.h : header file
//

#pragma once

#define MAX_PACKET 64
#define FILE_BUFFER	2048
#define MAX_BIN_FILE_SIZE (128*1024)

#define CMD_UPDATE_APROM	0x000000A0
#define CMD_NONE			0x00000000

typedef struct
{
	UINT uIndex;
	UINT uChipID;
	UINT uRamSize;
	TCHAR cChipName[128];
	UINT uFlashSize;
	UINT uCodeFlashSize;
	UINT uDataFlashSize;
	UINT uDataFlashStartAddr;
	//Any more...

}MY_CHIP_TYPE;

typedef struct
{
	UINT uCodeFileType; // 0:bin 1:hex
	UINT uCodeFileStartAddr; //hex only
    UINT uCodeFileSize;
	DWORD uCodeFileCheckSum;
}MY_FILE_INFO_TYPE;

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
private:
	DWORD Checksum (unsigned char *buf, int len);
	BOOL HexToBin(LPCTSTR filename,UINT nMaxBufSize,MY_FILE_INFO_TYPE *fileInfo);
	BOOL GetFileInfo(LPCTSTR filename,MY_FILE_INFO_TYPE *fileInfo);
	unsigned long HexStringToDec(TCHAR *buf, UINT len);
	void CheckFileSize(UINT8 index);

	UCHAR WriteReportBuffer[256];
	UCHAR ReadReportBuffer[256];
	UCHAR CodeFileBuffer[MAX_BIN_FILE_SIZE];	

	CString m_strCodeFilePath;
	CString m_strCurExePath;

	BOOL bCodeFlagErrorColorEnable;
	CWinThread * pWriteReportThread;

	MY_FILE_INFO_TYPE m_sMyFileInfo;

	DWORD m_curCmd;
	DWORD gcksum;
	DWORD packetNum;
	HCURSOR hcursor;
public:
	afx_msg void OnBnClickedOpen();
	afx_msg void OnBnClickedClose();
	afx_msg void OnBnClickedButtonCodeFile();
	afx_msg void OnBnClickedButtonBurnAprom();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

	void AddToInfOut(CString InStr, BOOL AddTime, BOOL NewLine);
	void AddToInfOut(char *p, BOOL AddTime, BOOL NewLine);
	BOOL CmdToDo(DWORD cmd);

	CButton m_button_open;
	CEdit m_ctlEditCodeFile;
	CEdit m_text_message;
};
