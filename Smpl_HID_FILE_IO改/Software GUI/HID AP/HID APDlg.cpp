// HID APDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HID AP.h"
#include "HID APDlg.h"
#include "HID.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern "C" {
#include "hidsdi.h"
#include "setupapi.h"
}

static UINT g_packno = 1;
CHIDAPDlg *pMainDlg;
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
	memset(&m_sMyFileInfo,0,sizeof(MY_FILE_INFO_TYPE));
}

void CHIDAPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_OPEN_DEVICE, m_button_open);
	DDX_Control(pDX, IDC_EDIT_CODE_FILE, m_ctlEditCodeFile);
	DDX_Control(pDX, IDC_STATUS_RESULT, m_text_message);
}

BEGIN_MESSAGE_MAP(CHIDAPDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
//	ON_WM_QUERYDRAGICON()
	ON_WM_SHOWWINDOW()
	//ON_WM_SETCURSOR()
	/*ON_WM_CTLCOLOR()*/
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_OPEN_DEVICE, &CHIDAPDlg::OnBnClickedOpen)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE_DEVICE, &CHIDAPDlg::OnBnClickedClose)
	ON_BN_CLICKED(IDC_BUTTON_CODE_FILE, &CHIDAPDlg::OnBnClickedButtonCodeFile)
	ON_BN_CLICKED(IDC_BUTTON_BURN_APROM, &CHIDAPDlg::OnBnClickedButtonBurnAprom)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CHIDAPDlg message handlers
void CHIDAPDlg::AddToInfOut(CString InStr, BOOL AddTime, BOOL NewLine)
{
	CString str,str1;
	CHAR SysTime[10];
	str = InStr; 

	if(AddTime==TRUE)
	{
		_strtime_s(SysTime);
		str1=SysTime;
		str1=_T(" (")+str1+_T(")");
		str+=str1;
	}

	if(NewLine==TRUE)
	{
		str+=_T("\r\n");
		
	}

	((CEdit*)GetDlgItem(IDC_STATUS_RESULT))->SetSel(-1,-1);
	((CEdit*)GetDlgItem(IDC_STATUS_RESULT))->ReplaceSel(str);

}

void CHIDAPDlg::AddToInfOut(char *p, BOOL AddTime, BOOL NewLine)
{
	CString str;
	str.Format(_T("%s"),p);
	AddToInfOut(str,AddTime,NewLine);
}


DWORD CHIDAPDlg::Checksum (unsigned char *buf, int len)
{
    int i;
    unsigned short c;

    for (c=0, i=0; i < len; i++) {
        c += buf[i];
    }
    return (c);
}


unsigned long CHIDAPDlg::HexStringToDec(TCHAR *buf, UINT len)
{
	/*TCHAR hexString[16]={0};*/
	wchar_t hexString[16]={0};
	memcpy(hexString,buf,len*sizeof(hexString[0]));

	return wcstoul(hexString, NULL, 16);

}

BOOL CHIDAPDlg::HexToBin(LPCTSTR filename,UINT nMaxBufSize,MY_FILE_INFO_TYPE *fileInfo)
{
	int nRecordType, nRecordLen;
	UINT nRecordAddr;
	bool bEndOfField;
	CString strBuffer;
	TCHAR *Buffer;
	TCHAR *pRecordData;
	BYTE cCalCheckSum, cRecordCheckSum;
	//CHAR cFillByte = 0xFF;
	UCHAR *TargetBuf;

	UINT curMode = 0;
	UINT highOffset = 0;
	BOOL bInitAddr = TRUE;

	
	UINT startAddr = 0;
	UINT maxAddr = 0;
	UINT lastlen = 0;

	CStdioFile file;
	if( file.Open(filename,CFile::modeRead) == NULL )
		return FALSE;


	bEndOfField = false;
	UINT nReadSize = 0;

	while( bEndOfField == false)
	{
		if( file.ReadString(strBuffer) == NULL )
			break;

		Buffer = strBuffer.GetBuffer();

		if ( Buffer[0] != _T(':') )
		{
			// An field format error.
			goto out;
		}
		// Get record's data length.
		nRecordLen = HexStringToDec( Buffer + 1, 2 );
		// Get record's start address.
		nRecordAddr = HexStringToDec( Buffer + 3, 4 );
		// Get Record's type.
		nRecordType = HexStringToDec( Buffer + 7, 2 );
		// Get the first data's address within record.
		pRecordData = Buffer + 9;
		switch( nRecordType )
		{
		case 00:
			if(curMode == 0x2)
			{
				nRecordAddr = (highOffset<<4) + nRecordAddr;
			}
			else if(curMode == 0x4)
			{
				nRecordAddr = (highOffset<<16) + nRecordAddr;
			}

			if(bInitAddr)
			{
				startAddr = nRecordAddr;
				maxAddr = nRecordAddr;
				lastlen = nRecordLen;
				bInitAddr = FALSE;

			}


			if( nRecordAddr<startAddr)
				startAddr = nRecordAddr;
			if( nRecordAddr>maxAddr )
			{
				maxAddr = nRecordAddr;
				lastlen = nRecordLen;
			}


			break;
		case 01:
			bEndOfField = true;

			break;
		case 02:
			curMode = 0x02;
			highOffset = HexStringToDec( pRecordData, 4 );

			break;
		case 04:
			curMode = 0x04;
			highOffset = HexStringToDec( pRecordData, 4 );

			break;
		default: 
			break;
		}

		strBuffer.ReleaseBuffer();

	}

	  fileInfo->uCodeFileType = 1;//hex
	  fileInfo->uCodeFileStartAddr = startAddr;
      fileInfo->uCodeFileSize = maxAddr-startAddr+lastlen;
	  TargetBuf = CodeFileBuffer;


	  if(!bEndOfField)
		  return FALSE;

	bEndOfField = false;
	curMode = 0;
	highOffset = 0;
	::memset( TargetBuf, 0xFF, nMaxBufSize );
	file.SeekToBegin();
	while( bEndOfField == false)
	{
		if( file.ReadString(strBuffer) == NULL )
			break;

		Buffer = strBuffer.GetBuffer();

		if ( Buffer[0] != _T(':') )
		{
			// An field format error.
			goto out;
		}
		// Get record's data length.
		nRecordLen = HexStringToDec( Buffer + 1, 2 );
		// Get record's start address.
		nRecordAddr = HexStringToDec( Buffer + 3, 4 );
		// Get Record's type.
		nRecordType = HexStringToDec( Buffer + 7, 2 );
		// Get the first data's address within record.
		pRecordData = Buffer + 9;
		switch( nRecordType )
		{
		case 00:
			cCalCheckSum = (BYTE)nRecordLen + 
			((BYTE)(nRecordAddr>>8) + (BYTE)nRecordAddr) +
			(BYTE)nRecordType;

			if(curMode == 0x2) //扩展模式时加上高位偏移量
			{
				nRecordAddr = (highOffset<<4) + nRecordAddr;
			}
			else if(curMode == 0x4)
			{
				nRecordAddr = (highOffset<<16) + nRecordAddr;
			}

			break;
		case 01:
			bEndOfField = true;
			strBuffer.ReleaseBuffer();

			goto out;
		case 02: //扩展段地址
			curMode = 0x02;
			highOffset = HexStringToDec( pRecordData, 4 );
			strBuffer.ReleaseBuffer();

			continue;
		case 04: //扩展线性地址
			curMode = 0x04;
			highOffset = HexStringToDec( pRecordData, 4 );
			strBuffer.ReleaseBuffer();

			continue;
		default: 

			strBuffer.ReleaseBuffer();
			continue;
		}

		

		if ( (nRecordAddr-startAddr) >= nMaxBufSize ) 
			break;
		else if( (nRecordAddr-startAddr+nRecordLen) > nMaxBufSize ) 
			break;

		BYTE cData;
		int nValidDataLen, i;


		nValidDataLen = nRecordLen;

		// Read data from record.
		for( i = 0; i < nValidDataLen; i++ )
		{
			cData = (BYTE)HexStringToDec( pRecordData, 2 );
			TargetBuf[nRecordAddr-startAddr] = cData;
			cCalCheckSum += cData;
			nRecordAddr ++;
			pRecordData += 2;
		}

		cCalCheckSum = -cCalCheckSum; // 2'complement
		// Get Check sum from record.
		cRecordCheckSum = (BYTE)HexStringToDec( pRecordData, 2 );
		if ( cRecordCheckSum != cCalCheckSum )
		{
			MessageBox(_T("Check sum error!"),_T("Error"),MB_OK);
			break;
		}

		strBuffer.ReleaseBuffer();

	}



out:
	file.Close();

	return bEndOfField;


}



BOOL CHIDAPDlg::GetFileInfo(LPCTSTR filename,MY_FILE_INFO_TYPE *fileInfo)
{
	HANDLE hFileHandle = INVALID_HANDLE_VALUE;
	BOOL Result=FALSE;
	unsigned long totallen=0,readcn=0;
	DWORD lcksum; //32 bit checksum
	CString postfix = filename;
	UCHAR * buf;

	CString tmpStr;

	postfix = postfix.Right(postfix.GetLength()-postfix.ReverseFind('.')-1);
	if(!postfix.CompareNoCase(_T("hex")))
	{
		Result = HexToBin(filename,MAX_BIN_FILE_SIZE,fileInfo);
		//tmpStr.Format(_T("Result:%d; size:%d; Start:%X"),Result,fileInfo->uCodeFileSize,fileInfo->uCodeFileStartAddr);
		//AfxMessageBox(tmpStr);
		if( (fileInfo->uCodeFileSize > MAX_BIN_FILE_SIZE))
	    {
		    MessageBox(_T("File size is too big"),_T("Error"),MB_OK);
		    return FALSE;
	    }
		if(Result == FALSE)
			return FALSE;

		fileInfo->uCodeFileCheckSum = Checksum(CodeFileBuffer, fileInfo->uCodeFileSize);

	}
	else
	{
	  
	  hFileHandle=CreateFile(filename, 
				GENERIC_READ,
				FILE_SHARE_READ, 
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
	  if(hFileHandle==INVALID_HANDLE_VALUE)
		  return FALSE;

	  totallen = GetFileSize(hFileHandle, NULL); 

	  fileInfo->uCodeFileType = 0;//bin
	  fileInfo->uCodeFileStartAddr = 0;
      fileInfo->uCodeFileSize = totallen;
	  buf = CodeFileBuffer;

	  if(totallen > MAX_BIN_FILE_SIZE)
	  {
		  MessageBox(_T("File size is too big"),_T("Error"),MB_OK);
		  return FALSE;
	  }


	  Result = ReadFile(hFileHandle,
							buf,
							MAX_BIN_FILE_SIZE,
							&readcn,
							NULL);
				
	 if( (Result == TRUE) && (readcn !=totallen) )
			  return FALSE;
	 else if(Result == FALSE)
			  return FALSE;
	 lcksum = Checksum(buf, readcn);
	 fileInfo->uCodeFileCheckSum = lcksum;
	}
       
    return TRUE;
}

void CHIDAPDlg::CheckFileSize(UINT8 index)
{
	CString tmpStr;

	if(index == 1)
	{
		if(m_sMyFileInfo.uCodeFileSize > MAX_BIN_FILE_SIZE)
		{
			bCodeFlagErrorColorEnable = TRUE;
	        SetDlgItemText(IDC_EDIT_CODE_CHECKSUM,_T(""));
		}
		else
		{
			bCodeFlagErrorColorEnable = FALSE;
			if(m_sMyFileInfo.uCodeFileCheckSum)
			{
			  tmpStr.Format(_T("0x%04X"),m_sMyFileInfo.uCodeFileCheckSum);
	          SetDlgItemText(IDC_EDIT_CODE_CHECKSUM,tmpStr);
			}
			else
			  SetDlgItemText(IDC_EDIT_CODE_CHECKSUM,_T(""));
		}

		if(m_sMyFileInfo.uCodeFileSize==0)
			SetDlgItemText(IDC_EDIT_CODE_SIZE,_T(""));
		else
		{
			tmpStr.Format(_T("%d Bytes"),m_sMyFileInfo.uCodeFileSize);
	        SetDlgItemText(IDC_EDIT_CODE_SIZE,tmpStr);
		}
	}
}

BOOL CHIDAPDlg::CmdToDo(DWORD cmd)
{
    SetDlgItemText(IDC_STATUS_RESULT,_T(""));	
	return true;
}
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
	TCHAR chPath[MAX_PATH];	
    GetModuleFileName(NULL,(LPCH)chPath,MAX_PATH);
	m_strCurExePath.Format((const char *)_T("%s"),chPath);
    int nPos=m_strCurExePath.ReverseFind('\\');
    m_strCurExePath=m_strCurExePath.Left(nPos+1);

	if( m_strCurExePath.IsEmpty() )
	{
		AddToInfOut(_T("Failed to get current path!"),1,1);
	}

	if(!m_strCurExePath.IsEmpty())
		m_strCodeFilePath = m_strCurExePath + CString("code.bin");
	else
		m_strCodeFilePath.Empty();

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

void CHIDAPDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
	m_ctlEditCodeFile.SetWindowText(m_strCodeFilePath.Left(m_strCodeFilePath.ReverseFind('\\') + 1));

	SetDlgItemText(IDC_EDIT_CODE_SIZE,_T(""));
	SetDlgItemText(IDC_EDIT_CODE_CHECKSUM,_T(""));
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
	CHidLDCmd io;

	if(!io.OpenDevice(0x051A, 0x511B))
	{
		AfxMessageBox("USB OPEN ERROR");
	}
	else
	{
		//MessageBox("USB Connect");
		SetDlgItemText(IDC_STATUS_RESULT,_T("USB Connect"));
//		GetDlgItem(IDC_BUTTON_OPEN_DEVICE)->SetWindowTextW(_T("Connect"));

		((CButton *)GetDlgItem(IDC_BUTTON_OPEN_DEVICE))->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_BUTTON_CLOSE_DEVICE))->EnableWindow(TRUE);

		((CButton *)GetDlgItem(IDC_OPEN_STATUS))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_STATUS_RESULT))->EnableWindow(TRUE);

		((CButton *)GetDlgItem(IDC_BUTTON_CODE_FILE))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_STATIC_FILE_NAME))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_EDIT_CODE_FILE))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_STATIC_FILE_SIZE))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_EDIT_CODE_SIZE))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_STATIC_CHECKSUM))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_EDIT_CODE_CHECKSUM))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_BUTTON_BURN_APROM))->EnableWindow(TRUE);
		
	}
}


void CHIDAPDlg::OnBnClickedClose()
{
	// TODO: Add your control notification handler code here
	CHidLDCmd io;

	if(!io.OpenDevice(0x051A, 0x511B))
	{
		AfxMessageBox("USB OPEN ERROR");
	}
	else
	{
		//MessageBox("USB Disconnect");
		SetDlgItemText(IDC_STATUS_RESULT,_T("USB Disconnect"));

		io.CloseDevice();

		((CButton *)GetDlgItem(IDC_BUTTON_OPEN_DEVICE))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_BUTTON_CLOSE_DEVICE))->EnableWindow(FALSE);

		((CButton *)GetDlgItem(IDC_OPEN_STATUS))->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_STATUS_RESULT))->EnableWindow(FALSE);
		//SetDlgItemText(IDC_STATUS_RESULT,_T("Ready"));
		
		//Load File group
		((CButton *)GetDlgItem(IDC_BUTTON_CODE_FILE))->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_STATIC_FILE_NAME))->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_EDIT_CODE_FILE))->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_STATIC_FILE_SIZE))->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_EDIT_CODE_SIZE))->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_STATIC_CHECKSUM))->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_EDIT_CODE_CHECKSUM))->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_BUTTON_BURN_APROM))->EnableWindow(FALSE);
	}


}

void CHIDAPDlg::OnBnClickedButtonCodeFile()
{
	// TODO: Add your control notification handler code here
	BOOL Result=FALSE;
	TCHAR szFilters[] = TEXT("Bin Files (*.bin)|*.bin|Intel Hex Files (*.hex)|*.hex|All Files (*.*)|*.*||");

	CFileDialog dlg (TRUE, TEXT("bin"), TEXT("code.bin"), OFN_FILEMUSTEXIST| OFN_READONLY, szFilters, this);

	CString strCurPath = m_strCodeFilePath;
	strCurPath = strCurPath.Left(strCurPath.ReverseFind('\\') + 1);
	dlg.m_ofn.lpstrInitialDir = strCurPath;

	if(IDOK == dlg.DoModal())
	{
		m_strCodeFilePath = dlg.GetPathName();
		m_ctlEditCodeFile.SetWindowText(m_strCodeFilePath);;
		m_sMyFileInfo.uCodeFileCheckSum = 0;
		m_sMyFileInfo.uCodeFileSize = 0;
        Result = GetFileInfo(m_strCodeFilePath,&m_sMyFileInfo);
		CheckFileSize(1);
	}
}

void CHIDAPDlg::OnBnClickedButtonBurnAprom()
{
	// TODO: Add your control notification handler code here
	DWORD ret;

	m_ctlEditCodeFile.GetWindowText(m_strCodeFilePath);

	ret = GetFileAttributes(m_strCodeFilePath);
    if( (ret == INVALID_FILE_ATTRIBUTES) || (ret & FILE_ATTRIBUTE_DIRECTORY) || (m_sMyFileInfo.uCodeFileSize == 0) )
    {
		MessageBox(_T("No APROM file!"),_T("Error"),MB_OK);
	    return;
    }

	if(m_sMyFileInfo.uCodeFileSize > MAX_BIN_FILE_SIZE)
	{
		MessageBox(_T("APROM file size is too big!"),_T("Error"),MB_OK);
	    return;
	}

	CHidLDCmd io;
	BOOL Result;
	pMainDlg = this;

	UINT packetCount, remainByte;

	if(pMainDlg->CodeFileBuffer != NULL)
	{
		//Calculate packetCount
		if (m_sMyFileInfo.uCodeFileSize > 48)
		{
			if (((m_sMyFileInfo.uCodeFileSize - 48) % 56) == 0)
			{
				packetCount = (m_sMyFileInfo.uCodeFileSize - 48) / 56 + 1;
				remainByte = 0;
			}
			else
			{
				packetCount = (m_sMyFileInfo.uCodeFileSize - 48) / 56 + 2;
				remainByte = (m_sMyFileInfo.uCodeFileSize - 48) % 56;
			}
		}
		else
		{
			packetCount = 1;
			remainByte = m_sMyFileInfo.uCodeFileSize;
		}

		//Transfer Data
		if (packetCount > 0)
		{
			for (packetNum = 0; packetNum < packetCount; packetNum++)
			{
                if (packetNum != 0) //Data Packet
                {
					pMainDlg->m_curCmd = CMD_NONE;
                    if (remainByte != 0)
                    {
                        if (packetNum != (packetCount - 1))
                        {
							if(io.OpenDevice(0x051A, 0x511B))
								Result = io.WriteDataFile(CMD_NONE, (const char *)pMainDlg->CodeFileBuffer+((packetNum - 1) * 56 + 48), m_sMyFileInfo.uCodeFileSize-48-(packetNum - 1) * 56, 2000);
                        }
                        else
                        {
							if(io.OpenDevice(0x051A, 0x511B))
							{
								Result = io.WriteDataFile(CMD_NONE, (const char *)pMainDlg->CodeFileBuffer+((packetNum - 1) * 56 + 48), m_sMyFileInfo.uCodeFileSize-48-(packetNum - 1) * 56, 2000);
								if(Result && (packetNum == (packetCount - 1)))
								{
									Result = io.ReadFile((char *)pMainDlg->ReadReportBuffer, remainByte, 2000, true);
									if(!Result)
										MessageBox(_T("Read Fail!"),_T("Error"),MB_OK);
									else
										SetDlgItemText(IDC_STATUS_RESULT,_T("OK"));
								}
							}
                        }
                    }
                    else
                    {
						if(io.OpenDevice(0x051A, 0x511B))
						{
							Result = io.WriteDataFile(CMD_NONE, (const char *)pMainDlg->CodeFileBuffer+((packetNum - 1) * 56 + 48), m_sMyFileInfo.uCodeFileSize-48-(packetNum - 1) * 56, 2000);
							if(Result && (packetNum == (packetCount - 1)))
							{
								Result = io.ReadFile((char *)pMainDlg->ReadReportBuffer, 56, 2000, true);
								if(!Result)
									MessageBox(_T("Read Fail!"),_T("Error"),MB_OK);
								else
									SetDlgItemText(IDC_STATUS_RESULT,_T("OK"));
							}
						}
                    }
                }
                else
                {
                    //CMD,Download Program to APROM
					pMainDlg->m_curCmd = CMD_UPDATE_APROM;

                    if (m_sMyFileInfo.uCodeFileSize > 48)
                    {
						if(io.OpenDevice(0x051A, 0x511B))
						{
							Result = io.WriteFile(CMD_UPDATE_APROM, (const char *)pMainDlg->CodeFileBuffer, 0, m_sMyFileInfo.uCodeFileSize,2000);
							SetDlgItemText(IDC_STATUS_RESULT,_T("Waiting..."));
						}
                    }
                    else
                    {
						if(io.OpenDevice(0x051A, 0x511B))
						{
							Result = io.WriteFile(CMD_UPDATE_APROM, (const char *)pMainDlg->CodeFileBuffer, 0, m_sMyFileInfo.uCodeFileSize,2000);
							if(Result)
							{
								Result = io.ReadFile((char *)pMainDlg->ReadReportBuffer, remainByte, 2000, true);
								if(!Result)
									MessageBox(_T("Read Fail!"),_T("Error"),MB_OK);
								else
									SetDlgItemText(IDC_STATUS_RESULT,_T("OK"));
							}
						}
                    }
                }
			}
		}
	}
}

