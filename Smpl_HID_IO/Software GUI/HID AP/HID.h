#ifndef INC__HID_H__
#define INC__HID_H__

/******************************************************************
          
*******************************************************************/
// MyUsbHidTestAppDlg.cpp : implementation file
//
#include "stdafx.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include "dbt.h"

extern "C" {
#include "setupapi.h"
#include "hidsdi.h"
}

#pragma comment(lib, "hid.lib")
#pragma comment(lib, "Setupapi.lib")

#define HID_MAX_PACKET_SIZE_EP 64
#define V6M_MAX_COMMAND_LENGTH (HID_MAX_PACKET_SIZE_EP - 2)

class CHidIO
{
protected:
	HANDLE m_hReadHandle;
	HANDLE m_hWriteHandle;
	HANDLE m_hReadEvent;
	HANDLE m_hWriteEvent;
	HANDLE m_hAbordEvent;
	BOOL m_bUseTwoHandle;
public:
	CHidIO()
		: m_hReadHandle(INVALID_HANDLE_VALUE)
		, m_hWriteHandle(INVALID_HANDLE_VALUE)
		, m_hAbordEvent(CreateEvent(NULL,TRUE,FALSE,NULL))
		, m_hReadEvent(CreateEvent(NULL,TRUE,FALSE,NULL))
		, m_hWriteEvent(CreateEvent(NULL,TRUE,FALSE,NULL))
		, m_bUseTwoHandle(TRUE)
	{
	}
	virtual ~CHidIO()
	{
		CloseDevice();

		CloseHandle(m_hWriteEvent);
		CloseHandle(m_hReadEvent);
		CloseHandle(m_hAbordEvent);
	}

	void CloseDevice()
	{
		if(m_bUseTwoHandle)
		{
			if(m_hReadHandle != INVALID_HANDLE_VALUE)
				CancelIo(m_hReadHandle);
		
			if(m_hWriteHandle != INVALID_HANDLE_VALUE)
				CancelIo(m_hWriteHandle);

			if(m_hReadHandle != INVALID_HANDLE_VALUE)
			{
				CloseHandle(m_hReadHandle);
				m_hReadHandle = INVALID_HANDLE_VALUE;
			}
			if(m_hWriteHandle != INVALID_HANDLE_VALUE)
			{
				CloseHandle(m_hWriteHandle);
				m_hWriteHandle = INVALID_HANDLE_VALUE;
			}
		}
		else
		{
			if(m_hReadHandle != INVALID_HANDLE_VALUE)
				CancelIo(m_hReadHandle);
		
			if(m_hReadHandle != INVALID_HANDLE_VALUE)
			{
				CloseHandle(m_hReadHandle);
				m_hReadHandle = INVALID_HANDLE_VALUE;
				m_hWriteHandle = INVALID_HANDLE_VALUE;
			}
			
		}

	}
	
	BOOL OpenDevice(BOOL bUseTwoHandle, USHORT usVID, USHORT usPID)
	{
		m_bUseTwoHandle = bUseTwoHandle;
		//CString MyDevPathName="";
		TCHAR MyDevPathName[MAX_PATH];
		GUID HidGuid;
		HDEVINFO hDevInfoSet;
		DWORD MemberIndex;
		SP_DEVICE_INTERFACE_DATA DevInterfaceData;
		BOOL Result;
		DWORD RequiredSize;
		PSP_DEVICE_INTERFACE_DETAIL_DATA	pDevDetailData;
		HANDLE hDevHandle;
		HIDD_ATTRIBUTES DevAttributes;
		
		BOOL MyDevFound=FALSE;
		
		m_hReadHandle=INVALID_HANDLE_VALUE;
		m_hWriteHandle=INVALID_HANDLE_VALUE;
		
		DevInterfaceData.cbSize=sizeof(DevInterfaceData);
		DevAttributes.Size=sizeof(DevAttributes);

		HidD_GetHidGuid(&HidGuid);
		
		hDevInfoSet=SetupDiGetClassDevs(&HidGuid,
			NULL,
			NULL,
			DIGCF_DEVICEINTERFACE|DIGCF_PRESENT);
		
		MemberIndex=0;
		while(1)
		{
			Result=SetupDiEnumDeviceInterfaces(hDevInfoSet,
				NULL,
				&HidGuid,
				MemberIndex,
				&DevInterfaceData);

			if(Result==FALSE) break;
			
			MemberIndex++;
			
			Result=SetupDiGetDeviceInterfaceDetail(hDevInfoSet,
				&DevInterfaceData,
				NULL,
				NULL,
				&RequiredSize,
				NULL);
			
			pDevDetailData=(PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(RequiredSize);
			if(pDevDetailData==NULL)
			{
				//MessageBox("ÄÚ´æ²»×ã!");
				SetupDiDestroyDeviceInfoList(hDevInfoSet);
				return FALSE;
			}
			
			pDevDetailData->cbSize=sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			
			Result=SetupDiGetDeviceInterfaceDetail(hDevInfoSet,
				&DevInterfaceData,
				pDevDetailData,
				RequiredSize,
				NULL,
				NULL);
			
			//_tcscpy(MyDevPathName, pDevDetailData->DevicePath);
			strcpy_s(MyDevPathName, pDevDetailData->DevicePath);
			free(pDevDetailData);

			if(Result==FALSE) continue;
			
			hDevHandle=CreateFile(MyDevPathName, 
				NULL,
				FILE_SHARE_READ|FILE_SHARE_WRITE, 
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL);

			if(hDevHandle!=INVALID_HANDLE_VALUE)
			{
				Result=HidD_GetAttributes(hDevHandle,
					&DevAttributes);
				
				CloseHandle(hDevHandle);
				
				if(Result==FALSE) continue;
				
				if(DevAttributes.VendorID == usVID
					&& DevAttributes.ProductID == usPID)
				{
					MyDevFound=TRUE;
							
							if(bUseTwoHandle)
							{
								m_hReadHandle=CreateFile(MyDevPathName, 
									GENERIC_READ,
									FILE_SHARE_READ|FILE_SHARE_WRITE, 
									NULL,
									OPEN_EXISTING,
									//FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
									FILE_ATTRIBUTE_NORMAL,
									NULL);
								
								m_hWriteHandle=CreateFile(MyDevPathName, 
									GENERIC_WRITE,
									FILE_SHARE_READ|FILE_SHARE_WRITE, 
									NULL,
									OPEN_EXISTING,
									//FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
									FILE_ATTRIBUTE_NORMAL,
									NULL);
							}
							else
							{
								m_hWriteHandle =
								m_hReadHandle = CreateFile(MyDevPathName, 
									GENERIC_READ | GENERIC_WRITE,
									FILE_SHARE_READ|FILE_SHARE_WRITE, 
									NULL,
									OPEN_EXISTING,
									//FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
									FILE_ATTRIBUTE_NORMAL,
									NULL);
							}
							break;
						}
			}
			else continue;
		}
		SetupDiDestroyDeviceInfoList(hDevInfoSet);
		return MyDevFound;
	}


	BOOL ReadFile(char *pcBuffer, size_t szMaxLen, DWORD *pdwLength, DWORD dwMilliseconds)
	{
		HANDLE events[2] = {m_hAbordEvent, m_hReadEvent};

		OVERLAPPED overlapped;
		memset(&overlapped, 0, sizeof(overlapped));
		overlapped.hEvent = m_hReadEvent;

		if(pdwLength != NULL)
			*pdwLength = 0;

		if(!::ReadFile(m_hReadHandle, pcBuffer, szMaxLen, NULL, &overlapped))
			return FALSE;

		DWORD dwIndex = WaitForMultipleObjects(2, events, FALSE, dwMilliseconds);
		if(dwIndex == WAIT_OBJECT_0
			|| dwIndex == WAIT_OBJECT_0 + 1)
		{
			ResetEvent(events[dwIndex - WAIT_OBJECT_0]);

			if(dwIndex == WAIT_OBJECT_0)
				return FALSE;	//Abort event
			else
			{
				DWORD dwLength = 0;
				//Read OK
				GetOverlappedResult(m_hReadHandle, &overlapped, &dwLength, TRUE);
				if(pdwLength != NULL)
					*pdwLength = dwLength;

				return TRUE;
			}				
		}
		else
			return FALSE;
	}

	BOOL WriteFile(const char *pcBuffer, size_t szLen, DWORD *pdwLength, DWORD dwMilliseconds)
	{
		HANDLE events[2] = {m_hAbordEvent, m_hWriteEvent};

		OVERLAPPED overlapped;
		memset(&overlapped, 0, sizeof(overlapped));
		overlapped.hEvent = m_hWriteEvent;

		if(pdwLength != NULL)
			*pdwLength = 0;

		if(!::WriteFile(m_hWriteHandle, pcBuffer, szLen, NULL, &overlapped))
			return FALSE;

		DWORD dwIndex = WaitForMultipleObjects(2, events, FALSE, dwMilliseconds);
		if(dwIndex == WAIT_OBJECT_0
			|| dwIndex == WAIT_OBJECT_0 + 1)
		{
			ResetEvent(events[dwIndex - WAIT_OBJECT_0]);

			if(dwIndex == WAIT_OBJECT_0)
				return FALSE;	//Abort event
			else
			{
				DWORD dwLength = 0;
				//Write OK
				GetOverlappedResult(m_hWriteHandle, &overlapped, &dwLength, TRUE);
				if(pdwLength != NULL)
					*pdwLength = dwLength;


				return TRUE;
			}				
		}
		else
			return FALSE;
	}
};



class CHidCmd
{
protected:
	CHAR	m_acBuffer[HID_MAX_PACKET_SIZE_EP + 1];
	UCHAR	m_ucCmdIndex;
	BOOL	m_bCmdError;
	CHidIO	m_hidIO;
public:
	CHidCmd()
		: m_ucCmdIndex(18)	//Do not use 0 to avoid firmware already has index 0 occasionally.
		, m_hidIO()
	{
	}
	virtual ~CHidCmd()
	{
	}


	void CloseDevice()
	{
		m_hidIO.CloseDevice();
	}

	BOOL OpenDevice(USHORT usVID, USHORT usPID)
	{
		return m_hidIO.OpenDevice(TRUE, usVID, usPID);
	}

	BOOL ReadFile1(char *pcBuffer, size_t szMaxLen, DWORD *pdwLength, DWORD dwMilliseconds)
	{
		DWORD dwStart = GetTickCount();
		while(1)
		{
			DWORD dwLength;
			if(!m_hidIO.ReadFile(m_acBuffer, sizeof(m_acBuffer), &dwLength, dwMilliseconds))
				return FALSE;

			/* Check if correct package index was read */
			//m_acBuffer[0];	//For HID internal usage, ignored.
			UCHAR ucCmdIndex = ((UCHAR)m_acBuffer[1] & (UCHAR)0x7F);
			m_bCmdError = (((UCHAR)m_acBuffer[1] & (UCHAR)0x80) == (UCHAR)0x80 ? TRUE : FALSE);
			DWORD dwCmdLength = (DWORD)(UCHAR)m_acBuffer[2];
			if(dwLength >= 3 && ucCmdIndex == m_ucCmdIndex)
			{
				if(dwCmdLength > dwLength - 3)
					dwCmdLength = dwLength - 3;
				if(dwCmdLength > szMaxLen)
					dwCmdLength = szMaxLen;

				memcpy(pcBuffer, m_acBuffer + 3, dwCmdLength);
				if(pdwLength != NULL)
					*pdwLength = dwCmdLength;
#if 0
				for(int i = 0; i < dwCmdLength + 3; ++i)
				{
					printf("%02x ", (int)(unsigned char)m_acBuffer[i]);
					if((i+1)%8==0)
						printf("\n");
				}
				printf("\n");
#endif
				return TRUE;
			}

			/* Check timeout */
			if(GetTickCount() - dwStart > dwMilliseconds)
			{
#if 0
				int i;
				printf("GetTickCount = %d %d %d - %d %d %d\n", GetTickCount(), dwStart, dwMilliseconds,
					dwLength, (int)ucCmdIndex, (int)m_ucCmdIndex);

				for(i = 0; i < dwLength; ++i)
				{
					printf("%02x ", (int)(unsigned char)m_acBuffer[i]);
					if((i+1)%8==0)
						printf("\n");
				}
				printf("\n");

				unsigned long cmd[1] = {0xFC};
				WriteFile((char *)&cmd, sizeof(cmd), NULL, dwMilliseconds);
				m_hidIO.ReadFile(m_acBuffer, sizeof(m_acBuffer), &dwLength, dwMilliseconds);

				for(i = 0; i < dwLength; ++i)
				{
					printf("%02x ", (int)(unsigned char)m_acBuffer[i]);
					if((i+1)%8==0)
						printf("\n");
				}
				printf("\n");
#endif
				return FALSE;
			}
		}
		return TRUE;
	}

	BOOL ReadFile(char *pcBuffer, size_t szMaxLen, DWORD *pdwLength, DWORD dwMilliseconds)
	{
		BOOL bRet = ReadFile1(pcBuffer, szMaxLen, pdwLength, dwMilliseconds);
		return bRet;
	}

	BOOL WriteFile(const char *pcBuffer, DWORD dwLen, DWORD *pdwLength, DWORD dwMilliseconds)
	{
		/* Set new package index value */
		++m_ucCmdIndex;
		m_ucCmdIndex = (m_ucCmdIndex & (UCHAR)0x7F);

		DWORD dwCmdLength = dwLen;
		if(dwCmdLength > sizeof(m_acBuffer) - 3)
			dwCmdLength = sizeof(m_acBuffer) - 3;
		m_acBuffer[0] = 0x00;	//Always 0x00
		m_acBuffer[1] = (CHAR)m_ucCmdIndex;
		m_acBuffer[2] = (UCHAR)dwCmdLength;
		memcpy(m_acBuffer + 3, pcBuffer, dwCmdLength);
		BOOL bRet = m_hidIO.WriteFile(m_acBuffer, sizeof(m_acBuffer), pdwLength, dwMilliseconds);

		return bRet;
	}

	BOOL IsCmdError()
	{
		return m_bCmdError;
	}
};



/* CHidLDCmd is revised version of 
	Cnuc100WriterDlg::ReadData(BOOL bUSB)
	Cnuc100WriterDlg::WriteData(BOOL bUSB)
   from ISP writer tool for easy integration.
 */
class CHidLDCmd
{
protected:
	CHAR	m_acBuffer[HID_MAX_PACKET_SIZE_EP + 1];
	ULONG	m_uCmdIndex;
	USHORT	m_usCheckSum;
	CHidIO	m_hidIO;

public:
	CHidLDCmd()
		: m_uCmdIndex(18)	//Do not use 0 to avoid firmware already has index 0 occasionally.
		, m_hidIO()
	{
	}
	virtual ~CHidLDCmd()
	{
	}


	void CloseDevice()
	{
		m_hidIO.CloseDevice();
	}

	BOOL OpenDevice(USHORT usVID, USHORT usPID)
	{
		return m_hidIO.OpenDevice(FALSE, usVID, usPID);
	}

	BOOL ReadFile(char *pcBuffer, size_t szMaxLen, DWORD dwMilliseconds, BOOL bCheckIndex)
	{
		DWORD dwStart = GetTickCount();
		while(1)
		{
			DWORD dwLength;
			if(!m_hidIO.ReadFile(m_acBuffer, sizeof(m_acBuffer), &dwLength, dwMilliseconds))
				return FALSE;

#if 0
			printf("Read %d\n", dwLength);
			for(int i = 0; i < dwLength; ++i)
			{
				printf("%02x ", (int)(UCHAR)m_acBuffer[i]);
				if(i % 8 == 0)
					printf("\n");
			}
#endif

			/* Check if correct package index was read */
			//m_acBuffer[0];	//For HID internal usage, ignored.
			USHORT usCheckSum = *((USHORT *)&m_acBuffer[1]);
			ULONG uCmdIndex = *((ULONG *)&m_acBuffer[5]);
			if(dwLength >= 9
				&& (!bCheckIndex || uCmdIndex == m_uCmdIndex - 1)
				&& usCheckSum == m_usCheckSum)
			{
				if(szMaxLen > dwLength - 9)
					szMaxLen = dwLength - 9;

				if(pcBuffer != NULL && szMaxLen > 0)
					memcpy(pcBuffer, m_acBuffer + 9, szMaxLen);
				return TRUE;
			}

			/* Check timeout */
			//printf("GetTickCount() - dwStart = %d\n", GetTickCount() - dwStart);
			if(GetTickCount() - dwStart > dwMilliseconds)
				return FALSE;
		}
		return TRUE;
	}

	BOOL WriteFile(unsigned long uCmd, const char *pcBuffer, DWORD dwLen, DWORD dwMilliseconds)
	{
		/* Set new package index value */
		DWORD dwCmdLength = dwLen;
		if(dwCmdLength > sizeof(m_acBuffer) - 9)
			dwCmdLength = sizeof(m_acBuffer) - 9;

		memset(m_acBuffer, 0, sizeof(m_acBuffer));
		//m_acBuffer[0] = 0x00;	//Always 0x00
		*((ULONG *)&m_acBuffer[1]) = uCmd;
		*((ULONG *)&m_acBuffer[5]) = m_uCmdIndex;
		if(pcBuffer != NULL && dwCmdLength > 0)
			memcpy(m_acBuffer + 9, pcBuffer, dwCmdLength);

		m_usCheckSum = Checksum((unsigned char *)&m_acBuffer[1], sizeof(m_acBuffer) - 1);

		DWORD dwLength;
		BOOL bRet = m_hidIO.WriteFile(m_acBuffer, sizeof(m_acBuffer), &dwLength, dwMilliseconds);

		if(bRet != FALSE)
			m_uCmdIndex += 2;

#if 0
		printf("Write %d\n", dwLength);
		for(int i = 0; i < dwLength; ++i)
		{
			printf("%02x ", (int)(UCHAR)m_acBuffer[i]);
			if(i % 8 == 0)
				printf("\n");
		}
#endif

		return bRet;
	}

	ULONG GetCmdIndex() const
	{
		return m_uCmdIndex;
	}

	unsigned short Checksum(const unsigned char *buf, int len)
	{
		int i;
		unsigned short c;

		for (c=0, i=0; i < len; i++) {
			c += buf[i];
		}
		return (c);
	}
};


#endif
