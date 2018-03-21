// HID APDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HID AP.h"
#include "HID APDlg.h"
#include "HID.h"

#ifdef _DEBUG
#define new DEBUG_NEW
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


// CHIDAPDlg dialog




CHIDAPDlg::CHIDAPDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHIDAPDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHIDAPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHIDAPDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_OPEN, &CHIDAPDlg::OnBnClickedOpen)
	ON_BN_CLICKED(IDC_LED5, &CHIDAPDlg::OnBnClickedLed5)
	ON_BN_CLICKED(IDC_LED6, &CHIDAPDlg::OnBnClickedLed6)
	ON_BN_CLICKED(IDC_LED7, &CHIDAPDlg::OnBnClickedLed7)
	ON_BN_CLICKED(IDC_LED8, &CHIDAPDlg::OnBnClickedLed8)
	ON_BN_CLICKED(IDC_Close, &CHIDAPDlg::OnBnClickedClose)
END_MESSAGE_MAP()


// CHIDAPDlg message handlers

BOOL CHIDAPDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
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

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CHIDAPDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CHIDAPDlg::OnPaint()
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
HCURSOR CHIDAPDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CHIDAPDlg::OnBnClickedOpen()
{
	// TODO: Add your control notification handler code here
	CHidCmd io;

	if(!io.OpenDevice(0x051A, 0x511B))
	{
		AfxMessageBox("USB OPEN ERROR");
	}
	else
	{
		MessageBox("USB Connect");
		SetDlgItemText(IDC_STATUS_RESULT,_T("Ready"));
		((CButton *)GetDlgItem(IDC_OPEN))->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_Close))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_GP_LED))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_LED5))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_LED6))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_LED7))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_LED8))->EnableWindow(TRUE);

		((CButton *)GetDlgItem(IDC_OPEN_STATUS))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_STATUS_RESULT))->EnableWindow(TRUE);
		
	}
}


void CHIDAPDlg::OnBnClickedClose()
{
	// TODO: Add your control notification handler code here
	CHidCmd io;
	char buffer1[62]={0};
	DWORD Length;
	BOOL bRet;

	if(!io.OpenDevice(0x051A, 0x511B))
	{
		AfxMessageBox("USB OPEN ERROR");
	}
	else
	{
		unsigned long cmd[ ] = {0xcf};
		bRet = io.WriteFile((const char *)&cmd, sizeof(cmd), &Length, 2000);					
		bRet = io.ReadFile(buffer1, sizeof(buffer1), &Length, 2000);
		//MessageBox("LED ALL OFF!!");
		SetDlgItemText(IDC_STATUS_RESULT,_T("LED ALL OFF")); 
	}

	io.CloseDevice();

	//MessageBox("USB Disconnect");
	((CButton *)GetDlgItem(IDC_OPEN))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_Close))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_GP_LED))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_LED5))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_LED6))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_LED7))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_LED8))->EnableWindow(FALSE);

	((CButton *)GetDlgItem(IDC_OPEN_STATUS))->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_STATUS_RESULT))->EnableWindow(FALSE);
	//SetDlgItemText(IDC_STATUS_RESULT,_T("Ready"));
	((CButton *)GetDlgItem(IDC_LED5))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_LED6))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_LED7))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_LED8))->SetCheck(FALSE);
}

void CHIDAPDlg::OnBnClickedLed5()
{
	// TODO: Add your control notification handler code here
	CHidCmd io;
	char buffer1[62]={0};
	DWORD Length;
	BOOL bRet;

	if(!io.OpenDevice(0x051A, 0x511B))
	{
		AfxMessageBox("USB OPEN ERROR");	
	}
	else
	{
		if(((CButton *)GetDlgItem(IDC_LED5))->GetCheck() == BST_CHECKED)
		{
			unsigned long cmd[ ] = {0xb5};
			bRet = io.WriteFile((const char *)&cmd, sizeof(cmd), &Length, 2000);					
			bRet = io.ReadFile(buffer1, sizeof(buffer1), &Length, 2000);
			//MessageBox("LED5 ON!!");
			SetDlgItemText(IDC_STATUS_RESULT,_T("LED5 ON")); 
		}
		else
		{
			unsigned long cmd[ ] = {0xc5};
			bRet = io.WriteFile((const char *)&cmd, sizeof(cmd), &Length, 2000);					
			bRet = io.ReadFile(buffer1, sizeof(buffer1), &Length, 2000);
			//MessageBox("LED5 OFF!!");
			SetDlgItemText(IDC_STATUS_RESULT,_T("LED5 OFF")); 
		}
	}
}

void CHIDAPDlg::OnBnClickedLed6()
{
	// TODO: Add your control notification handler code here
	CHidCmd io;
	char buffer1[62]={0};
	DWORD Length;
	BOOL bRet;
	
	if(!io.OpenDevice(0x051A, 0x511B))
	{
		AfxMessageBox("USB OPEN ERROR");
		
	}
	else
	{
		if(((CButton *)GetDlgItem(IDC_LED6))->GetCheck() == BST_CHECKED)
		{
			unsigned long cmd[ ] = {0xb6};
			bRet = io.WriteFile((const char *)&cmd, sizeof(cmd), &Length, 2000);					
			bRet = io.ReadFile(buffer1, sizeof(buffer1), &Length, 2000);
			//MessageBox("LED6 ON!!");
			SetDlgItemText(IDC_STATUS_RESULT,_T("LED6 ON")); 
		}
		else
		{
			unsigned long cmd[ ] = {0xc6};
			bRet = io.WriteFile((const char *)&cmd, sizeof(cmd), &Length, 2000);					
			bRet = io.ReadFile(buffer1, sizeof(buffer1), &Length, 2000);
			//MessageBox("LED6 OFF!!");
			SetDlgItemText(IDC_STATUS_RESULT,_T("LED6 OFF")); 
		}
	}
}

void CHIDAPDlg::OnBnClickedLed7()
{
	// TODO: Add your control notification handler code here
	CHidCmd io;
	char buffer1[62]={0};
	DWORD Length;
	BOOL bRet;

	if(!io.OpenDevice(0x051A, 0x511B))
	{
		AfxMessageBox("USB OPEN ERROR");
		
	}
	else
	{
		if(((CButton *)GetDlgItem(IDC_LED7))->GetCheck() == BST_CHECKED)
		{
			unsigned long cmd[ ] = {0xb7};
			bRet = io.WriteFile((const char *)&cmd, sizeof(cmd), &Length, 2000);					
			bRet = io.ReadFile(buffer1, sizeof(buffer1), &Length, 2000);
			//MessageBox("LED7 ON!!");
			SetDlgItemText(IDC_STATUS_RESULT,_T("LED7 ON"));
		}
		else
		{
			unsigned long cmd[ ] = {0xc7};
			bRet = io.WriteFile((const char *)&cmd, sizeof(cmd), &Length, 2000);					
			bRet = io.ReadFile(buffer1, sizeof(buffer1), &Length, 2000);
			//MessageBox("LED7 OFF!!");
			SetDlgItemText(IDC_STATUS_RESULT,_T("LED7 OFF")); 
		}
	}
}

void CHIDAPDlg::OnBnClickedLed8()
{
	// TODO: Add your control notification handler code here
	CHidCmd io;
	char buffer1[62]={0};
	DWORD Length;
	BOOL bRet;

	if(!io.OpenDevice(0x051A, 0x511B))
	{
		AfxMessageBox("USB OPEN ERROR");
		
	}
	else
	{
		if(((CButton *)GetDlgItem(IDC_LED8))->GetCheck() == BST_CHECKED)
		{
			unsigned long cmd[ ] = {0xb8};
			bRet = io.WriteFile((const char *)&cmd, sizeof(cmd), &Length, 2000);					
			bRet = io.ReadFile(buffer1, sizeof(buffer1), &Length, 2000);
			//MessageBox("LED8 ON!!");
			SetDlgItemText(IDC_STATUS_RESULT,_T("LED8 ON"));
		}
		else
		{
			unsigned long cmd[ ] = {0xc8};
			bRet = io.WriteFile((const char *)&cmd, sizeof(cmd), &Length, 2000);					
			bRet = io.ReadFile(buffer1, sizeof(buffer1), &Length, 2000);
			//MessageBox("LED8 OFF!!");
			SetDlgItemText(IDC_STATUS_RESULT,_T("LED8 OFF")); 
		}
	}
}
