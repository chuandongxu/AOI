/*
 * dlpc350_usb.h
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
#include <string>


#ifndef DLPC350_USB_H
#define DLPC350_USB_H

#define USB_MIN_PACKET_SIZE 64
#define USB_MAX_PACKET_SIZE 64

#define MY_VID 0x0451
#define MY_PID 0x6401

#include "../include/hidapi.h"

class DLPUsb
{
public:
	DLPUsb();
	~DLPUsb();

public:
	int DLPC350_USB_Open(wchar_t* serial_number);
	int DLPC350_USB_Open(std::string& path);
	int DLPC350_USB_OpenByProductStr(std::string& productStr);
	bool DLPC350_USB_Check_Connected();
	int DLPC350_USB_IsConnected();
	int DLPC350_USB_Write();
	int DLPC350_USB_Read();
	int DLPC350_USB_Close();
	int DLPC350_USB_Init();
	int DLPC350_USB_Exit();

public:
	unsigned char * getOutputBuffer() { return g_OutputBuffer; }
	unsigned char * getInputBuffer() { return g_InputBuffer; }

private:
	hid_device *DeviceHandle;	//Handle to write
	//In/Out buffers equal to HID endpoint size + 1
	//First byte is for Windows internal use and it is always 0
	unsigned char g_OutputBuffer[USB_MAX_PACKET_SIZE + 1];
	unsigned char g_InputBuffer[USB_MAX_PACKET_SIZE + 1];


	int USBConnected;      //Boolean true when device is connected	
	std::string m_path;
};

#endif //USB_H
