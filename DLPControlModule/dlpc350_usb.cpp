/*
 * dlpc350_usb.cpp
 *
 * This module has the wrapper functions to access USB driver functions.
 *
 * Copyright (C) {2015} Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>
#include <wchar.h>

#include <qdebug.h>

#include "dlpc350_usb.h"
#ifdef Q_OS_WIN32
#include <setupapi.h>
#endif
#include "../include/hidapi.h"

///***************************************************
//*                  GLOBAL VARIABLES
//****************************************************/
//
//static hid_device *DeviceHandle;	//Handle to write
////In/Out buffers equal to HID endpoint size + 1
////First byte is for Windows internal use and it is always 0
//unsigned char g_OutputBuffer[USB_MAX_PACKET_SIZE+1];
//unsigned char g_InputBuffer[USB_MAX_PACKET_SIZE+1];
//
//
//static int USBConnected = 0;      //Boolean true when device is connected

static  int USBInit = 0;      //Boolean true when device is init

DLPUsb::DLPUsb()
{
	DeviceHandle = NULL;
	USBConnected = 0;
}

DLPUsb::~DLPUsb()
{

}

int DLPUsb::DLPC350_USB_IsConnected()
{
    return USBConnected;
}

int DLPUsb::DLPC350_USB_Init(void)
{
	if (0 == USBInit)
	{
		USBInit = 1;
		return hid_init();
	}
	return 0;    
}

int DLPUsb::DLPC350_USB_Exit(void)
{
	if (1 == USBInit)
	{
		USBInit = 0;
		return hid_exit();
	}
	return 0;
}

int DLPUsb::DLPC350_USB_Open(std::string& path)
{
	// Open the device using the VID, PID,
	// and optionally the Serial number.
	DeviceHandle = hid_open_path(path.c_str());

	if (DeviceHandle == NULL)
	{
		USBConnected = 0;
		return -1;
	}

	m_path = path;
	USBConnected = 1;

	return 0;
}

int DLPUsb::DLPC350_USB_OpenByProductStr(std::string& productStr)
{
	std::string path = "";

	int nIndex = 0;
	hid_device_info* deviceInfoList = hid_enumerate(MY_VID, MY_PID);
	hid_device_info* deviceInfo = deviceInfoList;
	while (deviceInfo != NULL)
	{
		DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, deviceInfo->product_string, -1, NULL, 0, NULL, FALSE);// WideCharToMultiByte的运用  
		char *psText; // psText为char*的临时数组，作为赋值给std::string的中间变量  
		psText = new char[dwNum];
		WideCharToMultiByte(CP_OEMCP, NULL, deviceInfo->product_string, -1, psText, dwNum, NULL, FALSE);// WideCharToMultiByte的再次运用  
		std::string szDst = psText;// std::string赋值  
		delete[]psText;// psText的清除

		if (szDst == "DLPC350")
		{
			std::string pathTmp = deviceInfo->path;
			int nIndexStart = pathTmp.find("#8&");
			int nIndexEnd = pathTmp.find_first_of('&', nIndexStart + 3);

			pathTmp = pathTmp.substr(nIndexStart + 3, nIndexEnd - nIndexStart - 3);

			//QString serialNB = QString("SN%1").arg(nIndex, 2, 10, QChar('0'));

			if (pathTmp /*serialNB.toStdString()*/ == productStr)
			{
				path = deviceInfo->path;
				break;
			}

			nIndex++;
		}

		deviceInfo = deviceInfo->next;		
	}
	hid_free_enumeration(deviceInfoList);

	if (!path.empty())
	{
		// Open the device using the VID, PID,
		// and optionally the Serial number.
		DeviceHandle = hid_open_path(path.c_str());

		if (DeviceHandle == NULL)
		{
			USBConnected = 0;
			return -1;
		}

		m_path = productStr;

		USBConnected = 1;

		return 0;
	}
	else
	{
		USBConnected = 0;
		return -1;
	}	
}

bool DLPUsb::DLPC350_USB_Check_Connected()
{
	if (0 == USBConnected)
		return false;

	if (NULL == DeviceHandle)
		return false;

	bool bConnected = false;

	hid_device_info* deviceInfoList = hid_enumerate(MY_VID, MY_PID);
	hid_device_info* deviceInfo = deviceInfoList;
	while (deviceInfo != NULL)
	{
		DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, deviceInfo->product_string, -1, NULL, 0, NULL, FALSE);// WideCharToMultiByte的运用  
		char *psText; // psText为char*的临时数组，作为赋值给std::string的中间变量  
		psText = new char[dwNum];
		WideCharToMultiByte(CP_OEMCP, NULL, deviceInfo->product_string, -1, psText, dwNum, NULL, FALSE);// WideCharToMultiByte的再次运用  
		std::string szDst = psText;// std::string赋值  
		delete[]psText;// psText的清除

		if (szDst == "DLPC350")
		{
			std::string pathTmp = deviceInfo->path;
			int nIndexStart = pathTmp.find("#8&");
			int nIndexEnd = pathTmp.find_first_of('&', nIndexStart + 3);

			pathTmp = pathTmp.substr(nIndexStart + 3, nIndexEnd - nIndexStart - 3);

			if (pathTmp == m_path)
			{
				bConnected = true;
				break;
			}
		}

		deviceInfo = deviceInfo->next;
	}
	hid_free_enumeration(deviceInfoList);

	if (bConnected)
	{		
		return true;
	}
	else
	{
		USBConnected = 0;
		return false;
	}
}

int DLPUsb::DLPC350_USB_Open(wchar_t* serial_number)
{
    // Open the device using the VID, PID,
    // and optionally the Serial number.
	DeviceHandle = hid_open(MY_VID, MY_PID, serial_number);

    if(DeviceHandle == NULL)
    {
        USBConnected = 0;
        return -1;
    }

    USBConnected = 1;

    return 0;
}

int DLPUsb::DLPC350_USB_Write()
{
    int bytesWritten;

    if(DeviceHandle == NULL)
        return -1;
	
	try
	{
		/*unsigned char inputBuffer[USB_MIN_PACKET_SIZE + 1];
		int bytesRead = 0;
		if ((bytesRead = hid_read(DeviceHandle, inputBuffer, USB_MIN_PACKET_SIZE + 1)) == -1)
		{
			hid_close(DeviceHandle);
			USBConnected = 0;
			return -1;
		}*/

		if ((bytesWritten = hid_write(DeviceHandle, g_OutputBuffer, USB_MIN_PACKET_SIZE + 1)) == -1)
		{
			hid_close(DeviceHandle);
			USBConnected = 0;
			return -1;
		}
	}
	catch (...)
	{
		std::string szTmp = m_path;
		qDebug() << "Write Exception, DLP path as " << szTmp.c_str();
	}   

    return bytesWritten;
}

int DLPUsb::DLPC350_USB_Read()
{
    int bytesRead;

    if(DeviceHandle == NULL)
        return -1;

	try
	{
		//clear out the input buffer
		memset((void*)&g_InputBuffer[0], 0x00, USB_MIN_PACKET_SIZE + 1);

		if ((bytesRead = hid_read_timeout(DeviceHandle, g_InputBuffer, USB_MIN_PACKET_SIZE + 1, 2000)) == -1)
		{
			hid_close(DeviceHandle);
			USBConnected = 0;
			return -1;
		}

	}
	catch (...)
	{
		std::string szTmp = m_path;
		qDebug() << "Write Exception, DLP path as " << szTmp.c_str();
	}

    return bytesRead;
}

int DLPUsb::DLPC350_USB_Close()
{
	try
	{ 
		if (USBConnected)
		{
			hid_close(DeviceHandle);
			DeviceHandle = NULL;
		}		
		USBConnected = 0;
	}
	catch (_exception)
	{

	}

    return 0;
}

