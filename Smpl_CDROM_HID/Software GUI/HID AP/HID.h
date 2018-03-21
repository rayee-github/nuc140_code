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

class CHidLDCmd
{
protected:
	CHAR	m_acBuffer[HID_MAX_PACKET_SIZE_EP + 1];
	DWORD	m_uCmdIndex;
	DWORD	m_usCheckSum;
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
			DWORD usCheckSum = *((DWORD *)&m_acBuffer[1]);
			DWORD uCmdIndex = *((DWORD *)&m_acBuffer[5]);

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

    // CMD (4 bytes) + Packet Number (4 bytes) + Start Address (4 bytes) + Total Length (4 bytes)
    // Packet Number and Start Address are 0x00000000.
	BOOL WriteFile(unsigned long uCmd, const char *pcBuffer, DWORD dwAddr, DWORD dwLen, DWORD dwMilliseconds)
	{
		/* Set new package index value */
		m_uCmdIndex = 0;
		m_usCheckSum = 0;

		memset(m_acBuffer, 0, sizeof(m_acBuffer));
		m_acBuffer[0] = 0x00;	//Always 0x00
		m_acBuffer[1] = (BYTE)uCmd;
		m_acBuffer[2] = (BYTE)(uCmd >> 8);
		m_acBuffer[3] = (BYTE)(uCmd >> 16);
		m_acBuffer[4] = (BYTE)(uCmd >> 24);

		m_acBuffer[5] = (BYTE)m_uCmdIndex;
		m_acBuffer[6] = (BYTE)(m_uCmdIndex >> 8);
		m_acBuffer[7] = (BYTE)(m_uCmdIndex >> 16);
		m_acBuffer[8] = (BYTE)(m_uCmdIndex >> 24);

		m_acBuffer[9] = (BYTE)dwAddr;
		m_acBuffer[10] = (BYTE)(dwAddr >> 8);
		m_acBuffer[11] = (BYTE)(dwAddr >> 16);
		m_acBuffer[12] = (BYTE)(dwAddr >> 24);

		m_acBuffer[13] = (BYTE)dwLen;
		m_acBuffer[14] = (BYTE)(dwLen >> 8);
		m_acBuffer[15] = (BYTE)(dwLen >> 16);
		m_acBuffer[16] = (BYTE)(dwLen >> 24);

		DWORD dwLength;
		BOOL bRet = m_hidIO.WriteFile(m_acBuffer, sizeof(m_acBuffer), &dwLength, dwMilliseconds);
		m_uCmdIndex++;

		return bRet;
	}

	BOOL WriteDataFile(unsigned long uCmd, const char *pcBuffer, DWORD dwLen, DWORD dwMilliseconds)
	{
		DWORD dwDataLength = dwLen;
		DWORD dwDataCheckSum = m_usCheckSum;

		if(dwDataLength > sizeof(m_acBuffer) - 1)
			dwDataLength = sizeof(m_acBuffer) - 1;

		memset(m_acBuffer, 0, sizeof(m_acBuffer));
		m_acBuffer[0] = 0x00;	//Always 0x00

		if(pcBuffer != NULL && dwDataLength > 0)
			memcpy(m_acBuffer + 1, pcBuffer, dwDataLength);

		dwDataCheckSum = Checksum((unsigned char *)&m_acBuffer[1], dwDataLength);
		m_usCheckSum += dwDataCheckSum;

		DWORD dwLength;
		BOOL bRet = m_hidIO.WriteFile(m_acBuffer, sizeof(m_acBuffer), &dwLength, dwMilliseconds);
		m_uCmdIndex++;

		return bRet;
	}

	DWORD GetCmdIndex() const
	{
		return m_uCmdIndex;
	}

	DWORD Checksum(const unsigned char *buf, int len)
	{
		int i;
		unsigned long c;

		for (c=0, i=0; i < len; i++) {
			c += buf[i];
		}
		return (c);
	}
};


#endif
