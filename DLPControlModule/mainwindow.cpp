/*
 * mainwindow.cpp
 *
 * This is the headerfile for the functions implemented in mainwindow.cpp.
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

#include "../include/hidapi.h"

#include "dlpc350_common.h"
#include "dlpc350_error.h"
#include "dlpc350_usb.h"
#include "dlpc350_api.h"
#include "dlpc350_flashDevice.h"
#include "dlpc350_BMPParser.h"
#include "dlpc350_firmware.h"
#include "dlpc350_version.h"

#include <QMainWindow>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QtGlobal>
#include <QThread>
#include <QTextStream>
#include <QListWidgetItem>
#include <QItemSelectionModel>
#include <QMenu>
#include <QPainter>
#include <QTimer>
#include <QTime>

#include "../Common/SystemData.h"

#ifndef DEBUG_LOG_EN
//Enable MACRO to see debug logs
//#define DEBUG_LOG_EN
#include <QDebug>
#endif

#define MAX_NUM_RETRIES 5

#include "Helper.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"


//FlashDevice g_FlashDevice;
//static int g_FrameIdx=0;
//static int g_VarExpFrameIdx=0;
static unsigned char *g_pImageBuffer;
//QString g_displayStr_splashImageCount = "# Original Images: ";
//QString g_displayStr_splashImageAddedCount = "Images Added: ";
//QString g_displayStr_splashImageRemovedCount = "Images Removed: ";
//QString g_displayStr_splashImageTotalCount = "Total Images: ";
//QStringList g_iniGUITokens;

//extern INIPARAM_INFO g_iniParam_Info[];

typedef enum _qListDataRoles
{
    TrigType,
    PatNum,
    BitDepth,
    LEDSelect,
    InvertPat,
    InsertBlack,
    BufSwap,
    trigOutPrev,
    frameIndex,
    PatExposure,
    PatPeriod,
}qListDataRoles;

typedef enum _trigInType
{
    IntTrig,
    ExtPos,
    ExtNeg,
    NoTrig,
}trigInType;

quint32 BitPlanes[24][8] =
{
    { 0x01, 0x03, 0x07, 0x0F, 0x3E, 0x3F, 0xFE, 0xFF },  //G0 Bit-depth 1-8
    { 0x02, 0x03, 0x07, 0x0F, 0x3E, 0x3F, 0xFE, 0xFF },  //G1 Bit-depth 1-8
    { 0x04, 0x0C, 0x07, 0x0F, 0x3E, 0x3F, 0xFE, 0xFF },  //G2 Bit-depth 1-8
    { 0x08, 0x0C, 0x38, 0x0F, 0x3E, 0x3F, 0xFE, 0xFF },  //G3 Bit-depth 1-8
    { 0x10, 0x30, 0x38, 0xF0, 0x3E, 0x3F, 0xFE, 0xFF },  //G4 Bit-depth 1-8
    { 0x20, 0x30, 0x38, 0xF0, 0x3E, 0x3F, 0xFE, 0xFF },  //G5 Bit-depth 1-8
    { 0x40, 0xC0, 0x1C0, 0xF0, 0xF80, 0xFC0,0xFE, 0xFF },  //G6 Bit-depth 1-8
    { 0x80, 0xC0, 0x1C0, 0xF0, 0xF80, 0xFC0,0xFE, 0xFF }, //G7 Bit-depth 1-8


    { 0x0100, 0x0300, 0x1C0, 0xF00, 0xF80,     0xFC0,   0xFE00, 0xFF00 },  //R0 Bit-depth 1-8
    { 0x0200, 0x0300, 0xE00, 0xF00, 0xF80,     0xFC0,   0xFE00, 0xFF00 },  //R1 Bit-depth 1-8
    { 0x0400, 0x0C00, 0xE00, 0xF00, 0xF80,     0xFC0,   0xFE00, 0xFF00 },  //R2 Bit-depth 1-8
    { 0x0800, 0x0C00, 0xE00, 0xF00, 0xF80,     0xFC0,   0xFE00, 0xFF00 },  //R3 Bit-depth 1-8
    { 0x1000, 0x3000, 0x7000, 0xF000, 0x3E000,        0x3F000, 0xFE00, 0xFF00 },  //R4 Bit-depth 1-8
    { 0x2000, 0x3000, 0x7000, 0xF000, 0x3E000,  0x3F000, 0xFE00, 0xFF00 },  //R5 Bit-depth 1-8
    { 0x4000, 0xC000, 0x7000, 0xF000, 0x3E000, 0x3F000, 0xFE00, 0xFF00 },  //R6 Bit-depth 1-8
    { 0x8000, 0xC000, 0x38000, 0xF000, 0x3E000, 0x3F000, 0xFE00, 0xFF00 }, //R7 Bit-depth 1-8

    { 0x010000, 0x030000, 0x38000, 0xF0000,   0x3E000,  0x3F000,  0xFE0000, 0xFF0000 },  //B0 Bit-depth 1-8
    { 0x020000, 0x030000, 0x38000, 0xF0000,   0x3E000,  0x3F000,  0xFE0000, 0xFF0000 },  //B1 Bit-depth 1-8
    { 0x040000, 0x0C0000, 0x1C0000, 0xF0000,  0xF80000,   0xFC0000, 0xFE0000, 0xFF0000 },  //B2 Bit-depth 1-8
    { 0x080000, 0x0C0000, 0x1C0000, 0xF0000,  0xF80000, 0xFC0000, 0xFE0000, 0xFF0000 },  //B3 Bit-depth 1-8
    { 0x100000, 0x300000, 0x1C0000, 0xF00000, 0xF80000, 0xFC0000, 0xFE0000, 0xFF0000 },  //B4 Bit-depth 1-8
    { 0x200000, 0x300000, 0xE00000, 0xF00000, 0xF80000, 0xFC0000, 0xFE0000, 0xFF0000 },  //B5 Bit-depth 1-8
    { 0x400000, 0xC00000, 0xE00000, 0xF00000, 0xF80000, 0xFC0000, 0xFE0000, 0xFF0000 },  //B6 Bit-depth 1-8
    { 0x800000, 0xC00000, 0xE00000, 0xF00000, 0xF80000, 0xFC0000, 0xFE0000, 0xFF0000 },  //B7 Bit-depth 1-8
};

static int My_FileRead(void *Param, unsigned char *Data, unsigned int Size)
{
    QDataStream *pIn = (QDataStream *)Param;

    if(Data == NULL)
    {
        return 0;
    }
    else if(Size > 0)
    {
        if(pIn->readRawData((char *)Data, Size) <= 0)
            return -1;
    }

    return 0;
}

static int My_ImgeDraw(void *Param, unsigned int X, unsigned int Y, \
                       unsigned char *Pix, unsigned int Count)
{
    // LSB 5 bits represent where to place the pixel in stitched output image
    unsigned char bitPos = *(unsigned char *)Param & 0x1F;

    //MSB 3-bits represent bit depth of input image (zero based)
    unsigned char bitDepth = (*(unsigned char *)Param >> 5) + 1;

#ifdef DEBUG_LOG_EN
    qDebug() << "BitDepth = " << bitDepth << "BitPos = " << bitPos;
#endif

    if(bitPos > 23)
        return 0;

    if(X >= PTN_WIDTH || Y >= PTN_HEIGHT)
        return 0;

    if(X + Count > PTN_WIDTH)
    {
        Count = PTN_WIDTH - X;
    }

    unsigned mask = ((1 << bitDepth) - 1);
    unsigned char *pixPtr = (g_pImageBuffer + \
                             (X + Y * PTN_WIDTH)*BYTES_PER_PIXEL);
    unsigned inPix;

    for(unsigned i=0; i<Count; i++)
    {

        //GRB format as used in the splash - pixPtr[0] B[7:0], pixPtr[1] G[7:0], pixPtr[2] R[7:0]
        //inPix = pixPtr[0] | (pixPtr[1] << 8) | (pixPtr[2] << 16);

        //pixPtr[0] = B[7:0] = 7:0
        //pixPtr[1] = G[7:0] = 15:8
        //pixPtr[2] = R[7:0] = 23:16

        //Default is in RGB format, convert => to GRB format as used in DLPC350
        //G[7:0] - 7:0
        //R[7:0] - 15:8
        //B[7:0] - 23:16

        inPix = (pixPtr[0]<<16) | (pixPtr[1]) | (pixPtr[2] << 8);

        //Modify pixel value as passed in Pix[] 8-bit variable as given position
        inPix = ((inPix & ~(mask << bitPos)) | ((Pix[i]) << bitPos));

        //Revert back to RGB format before storing
        pixPtr[0] = (inPix >> 16) & 0xFF;
        pixPtr[1] = (inPix) & 0xFF;
        pixPtr[2] = (inPix >> 8) & 0xFF;
        pixPtr += 3;
    }
    return 0;
}

static int My_FileWrite(void *Param, unsigned char *Data, unsigned int Size)
{
    QFile *pOutFile = (QFile *)Param;

    if(Data == NULL)
    {
        if(pOutFile->seek(Size) != true)
            return -1;
    }
    else if(Size > 0)
    {
        if(pOutFile->write((char *)Data, Size) <= 0)
            return -1;
    }

    return 0;
}

static int My_ImgeGet(void *Param, unsigned int X, unsigned int Y, \
                      unsigned char *Pix, unsigned int Count)
{
    if(Param == NULL)
        Param = NULL;

    if(X >= PTN_WIDTH || Y >= PTN_HEIGHT)
        return 0;
    if(X + Count > PTN_WIDTH)
    {
        Count = PTN_WIDTH - X;
    }
    memcpy(Pix, g_pImageBuffer + (X + Y * PTN_WIDTH) * BYTES_PER_PIXEL,\
           Count * BYTES_PER_PIXEL);
    return 0;
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_settings("Texas Instruments", "LightCrafter GUI")
{
    ui->setupUi(this);
	
	g_FrameIdx = 0;
	g_VarExpFrameIdx = 0;
	g_displayStr_splashImageCount = "# Original Images: ";
	g_displayStr_splashImageAddedCount = "Images Added: ";
	g_displayStr_splashImageRemovedCount = "Images Removed: ";
	g_displayStr_splashImageTotalCount = "Total Images: ";	

	m_dlpUSB = new DLPUsb();
	m_dlpAPI = new DLPApi(m_dlpUSB);
	m_dlpFrm = new DLPFrmw();
	m_dlpBMPParser = new DLPBMPParser();

	//m_dlpFrm->g_iniParam_Info[0] = { "APPCONFIG.VERSION.SUBMINOR", { 0x00 }, { 0x00 }, 1, 1, false, 0, 1 };//SK: Remove
	//m_dlpFrm->g_iniParam_Info[1] = { "APPCONFIG.VERSION.MINOR", { 0x00 }, { 0x00 }, 1, 1, false, 1, 1 }; //SK: Remove
	//m_dlpFrm->g_iniParam_Info[2] = { "APPCONFIG.VERSION.MAJOR", { 0x03 }, { 0x00 }, 1, 1, false, 2, 1 }; //SK:Remove
	////{"APPCONFIG.VERSION.RSERVED", {0x00}, {0x00}, 1, 1, true, 3, 1},
	//m_dlpFrm->g_iniParam_Info[3] = { "DEFAULT.FIRMWARE_TAG", { 0x44, 0x4C, 0x50 }, { 0x00 }, 3, 1, true, 4, 32 };
	//m_dlpFrm->g_iniParam_Info[4] = { "DEFAULT.AUTOSTART", { 0x00 }, { 0x00 }, 1, 1, false, 36, 1 };
	//m_dlpFrm->g_iniParam_Info[5] = { "DEFAULT.DISPMODE", { 0x00 }, { 0x00 }, 1, 1, true, 37, 1 };
	//m_dlpFrm->g_iniParam_Info[6] = { "DEFAULT.SHORT_FLIP", { 0x00 }, { 0x00 }, 1, 1, true, 38, 1 };
	//m_dlpFrm->g_iniParam_Info[7] = { "DEFAULT.LONG_FLIP", { 0x00 }, { 0x00 }, 1, 1, true, 39, 1 };
	//m_dlpFrm->g_iniParam_Info[8] = { "DEFAULT.TRIG_OUT_1.POL", { 0x00 }, { 0x00 }, 1, 1, true, 104, 1 };
	//m_dlpFrm->g_iniParam_Info[9] = { "DEFAULT.TRIG_OUT_1.RDELAY", { 0xBB }, { 0xBB }, 1, 1, true, 105, 1 };
	//m_dlpFrm->g_iniParam_Info[10] = { "DEFAULT.TRIG_OUT_1.FDELAY", { 0xBB }, { 0xBB }, 1, 1, true, 106, 1 };
	//m_dlpFrm->g_iniParam_Info[11] = { "DEFAULT.TRIG_OUT_2.POL", { 0x00 }, { 0x00 }, 1, 1, true, 108, 1 };
	//m_dlpFrm->g_iniParam_Info[12] = { "DEFAULT.TRIG_OUT_2.WIDTH", { 0xBB }, { 0xBB }, 1, 1, true, 109, 1 };
	//m_dlpFrm->g_iniParam_Info[13] = { "DEFAULT.TRIG_IN_1.DELAY", { 0x00 }, { 0x00 }, 1, 1, true, 112, 4 };
	////{"DEFAULT.TRIG_IN_1.POL", {0x00}, {0x00}, 1, 1, false, 116, 1},
	////{"DEFAULT.TRIG_IN_2.DELAY", {0x00}, {0x00}, 1, 1, false, 120, 4},
	//m_dlpFrm->g_iniParam_Info[14] = { "DEFAULT.TRIG_IN_2.POL", { 0x00 }, { 0x00 }, 1, 1, true, 124, 1 };
	//m_dlpFrm->g_iniParam_Info[15] = { "DEFAULT.RED_STROBE.RDELAY", { 0xBB }, { 0xBB }, 1, 1, true, 128, 1 };
	//m_dlpFrm->g_iniParam_Info[16] = { "DEFAULT.RED_STROBE.FDELAY", { 0xBB }, { 0xBB }, 1, 1, true, 129, 1 };
	//m_dlpFrm->g_iniParam_Info[17] = { "DEFAULT.GRN_STROBE.RDELAY", { 0xBB }, { 0xBB }, 1, 1, true, 132, 1 };
	//m_dlpFrm->g_iniParam_Info[18] = { "DEFAULT.GRN_STROBE.FDELAY", { 0xBB }, { 0xBB }, 1, 1, true, 133, 1 };
	//m_dlpFrm->g_iniParam_Info[19] = { "DEFAULT.BLU_STROBE.RDELAY", { 0xBB }, { 0xBB }, 1, 1, true, 136, 1 };
	//m_dlpFrm->g_iniParam_Info[20] = { "DEFAULT.BLU_STROBE.FDELAY", { 0xBB }, { 0xBB }, 1, 1, true, 137, 1 };
	//m_dlpFrm->g_iniParam_Info[21] = { "DEFAULT.INVERTDATA", { 0x00 }, { 0x00 }, 1, 1, true, 140, 1 };
	//m_dlpFrm->g_iniParam_Info[22] = { "DEFAULT.LEDCURRENT_RED", { 0x97 }, { 0x97 }, 1, 1, true, 149, 1 };
	//m_dlpFrm->g_iniParam_Info[23] = { "DEFAULT.LEDCURRENT_GRN", { 0x78 }, { 0x78 }, 1, 1, true, 150, 1 };
	//m_dlpFrm->g_iniParam_Info[24] = { "DEFAULT.LEDCURRENT_BLU", { 0x7D }, { 0x7D }, 1, 1, true, 151, 1 };
	//m_dlpFrm->g_iniParam_Info[25] = { "DEFAULT.PATTERNCONFIG.PAT_EXPOSURE", { 0x7A120 }, { 0x7A120 }, 1, 1, true, 156, 4 };
	//m_dlpFrm->g_iniParam_Info[26] = { "DEFAULT.PATTERNCONFIG.PAT_PERIOD", { 0x7A120 }, { 0x7A120 }, 1, 1, true, 160, 4 };
	//m_dlpFrm->g_iniParam_Info[27] = { "DEFAULT.PATTERNCONFIG.PAT_MODE", { 0x03 }, { 0x03 }, 1, 1, true, 164, 1 };
	//m_dlpFrm->g_iniParam_Info[28] = { "DEFAULT.PATTERNCONFIG.TRIG_MODE", { 0x1 }, { 0x1 }, 1, 1, true, 165, 1 };
	//m_dlpFrm->g_iniParam_Info[29] = { "DEFAULT.PATTERNCONFIG.PAT_REPEAT", { 0x1 }, { 0x1 }, 1, 1, true, 166, 1 };
	//m_dlpFrm->g_iniParam_Info[30] = { "DEFAULT.PATTERNCONFIG.NUM_LUT_ENTRIES", { 0x02 }, { 0x1 }, 1, 1, true, 168, 2 };
	//m_dlpFrm->g_iniParam_Info[31] = { "DEFAULT.PATTERNCONFIG.NUM_PATTERNS", { 0x02 }, { 0x1 }, 1, 1, true, 170, 2 };
	//m_dlpFrm->g_iniParam_Info[32] = { "DEFAULT.PATTERNCONFIG.NUM_SPLASH", { 0x00 }, { 0x00 }, 1, 1, true, 172, 2 };
	//m_dlpFrm->g_iniParam_Info[33] = { "DEFAULT.SPLASHLUT", { 0x01 }, { 0x0 }, 1, 1, true, 176, 256 };
	//m_dlpFrm->g_iniParam_Info[34] = { "DEFAULT.SEQPATLUT", { 0x00061800, 0x00022804, 0x00024808 }, { 0x0 }, 3, 1, true, 432, 29184 };
	//m_dlpFrm->g_iniParam_Info[35] = { "DEFAULT.LED_ENABLE_MAN_MODE", { 0x0 }, { 0x0 }, 1, 1, true, 29616, 1 };
	//m_dlpFrm->g_iniParam_Info[36] = { "DEFAULT.MAN_ENABLE_RED_LED", { 0x0 }, { 0x0 }, 1, 1, true, 29617, 1 };
	//m_dlpFrm->g_iniParam_Info[37] = { "DEFAULT.MAN_ENABLE_GRN_LED", { 0x0 }, { 0x0 }, 1, 1, true, 29618, 1 };
	//m_dlpFrm->g_iniParam_Info[38] = { "DEFAULT.MAN_ENABLE_BLU_LED", { 0x0 }, { 0x0 }, 1, 1, true, 29619, 1 };
	//m_dlpFrm->g_iniParam_Info[39] = { "DEFAULT.PORTCONFIG.PORT", { 0x0 }, { 0x0 }, 1, 1, true, 40, 1 };
	//m_dlpFrm->g_iniParam_Info[40] = { "DEFAULT.PORTCONFIG.BPP", { 0x1 }, { 0x1 }, 1, 1, true, 41, 1 };
	//m_dlpFrm->g_iniParam_Info[41] = { "DEFAULT.PORTCONFIG.PIX_FMT", { 0x0 }, { 0x0 }, 1, 1, true, 42, 1 };
	//m_dlpFrm->g_iniParam_Info[42] = { "DEFAULT.PORTCONFIG.PORT_CLK", { 0x0 }, { 0x0 }, 1, 1, true, 43, 1 };
	////{"DEFAULT.PORTCONFIG.CSC[0]", {0x0400, 0x0000, 0x0000, 0x0000, 0x0400, 0x0000, 0x0000, 0x0000, 0x0400}, {0}, 9, 1, false, 44, 18},
	////{"DEFAULT.PORTCONFIG.CSC[1]", {0x04A8, 0xFDC7, 0xFF26, 0x04A8, 0x0715, 0x0000, 0x04A8, 0x0000, 0x0875}, {0}, 9, 1, false, 62, 18},
	////{"DEFAULT.PORTCONFIG.CSC[2]", {0x04A8, 0xFCC0, 0xFE6F, 0x04A8, 0x0662, 0x0000, 0x04A8, 0x0000, 0x0812}, {0}, 9, 1, false, 80, 18},
	//m_dlpFrm->g_iniParam_Info[43] = { "DEFAULT.PORTCONFIG.ABC_MUX", { 0x4 }, { 0x4 }, 1, 1, true, 100, 1 };
	//m_dlpFrm->g_iniParam_Info[44] = { "DEFAULT.PORTCONFIG.PIX_MODE", { 0x1 }, { 0x1 }, 1, 1, true, 101, 1 };
	//m_dlpFrm->g_iniParam_Info[45] = { "DEFAULT.PORTCONFIG.SWAP_POL", { 0x1 }, { 0x1 }, 1, 1, true, 102, 1 };
	//m_dlpFrm->g_iniParam_Info[46] = { "DEFAULT.PORTCONFIG.FLD_SEL", { 0x0 }, { 0x0 }, 1, 1, true, 103, 1 };
	//m_dlpFrm->g_iniParam_Info[47] = { "PERIPHERALS.I2CADDRESS[0]", { 0x34 }, { 0x34 }, 1, 1, false, 29649, 1 };
	//m_dlpFrm->g_iniParam_Info[48] = { "PERIPHERALS.I2CADDRESS[1]", { 0x3A }, { 0x3A }, 1, 1, false, 29650, 1 };
	////{"PERIPHERALS.USB_SRL[0]", {0x004C, 0x0043, 0x0052, 0x0032}, {0x0}, 4, 1, false, 29656, 8},
	////{"PERIPHERALS.USB_SRL[1]", {0x004C, 0x0043, 0x0052, 0x0033}, {0x0}, 4, 1, false, 29664, 8},
	//m_dlpFrm->g_iniParam_Info[49] = { "DATAPATH.SPLASHSTARTUPTIMEOUT", { 0x1388 }, { 0x1388 }, 1, 1, false, 29676, 2 };
	//m_dlpFrm->g_iniParam_Info[50] = { "DATAPATH.SPLASHATSTARTUPENABLE", { 0x01 }, { 0x1 }, 1, 1, true, 29680, 1 };
	//m_dlpFrm->g_iniParam_Info[51] = { "MACHINE_DATA.COLORPROFILE_0_BRILLIANTCOLORLOOK", { 0x0 }, { 0x0 }, 1, 1, true, 29750, 1 };

    // Hide the Connect button
    ui->pushButton_Connect_2->hide();

    // Hide the invert LED pwn checkbox
    ui->checkBox_LedPwmInvert->hide();

    // Hide Download Pattern button
    ui->pushButton_FWSplashImageUpload->hide();


    // Hide Bitplane selection combobox
    ui->comboBox_PatSeqBitPlaneNum->hide();
    ui->label_PatSeqBitPlaneNum->hide();

	//Disable auto-scroll as it is direct selection only
    ui->listWidget_PatSeqBitPlanes->setAutoScroll(false);
    ui->listWidget_VarExpPatSeqBitPlanes->setAutoScroll(false);

    //initialize variables
    m_isPrevModeStandBy = false;
    m_splashImageAddIndex = 0;
    m_splashImageCount = 0;
    m_splashImageAdded = 0;
    m_splashImageRemoved = 0;
    m_numExtraSplashLutEntries = 0;
    m_numImgInFlash = 0;

    m_firmwarePath = m_settings.value("FirmwarePath", "").toString();
    m_ptnImagePath = m_settings.value("PtnImagePath", "").toString();
    m_outFileName = m_settings.value("outFileName", "").toString();
    m_csvFileName = m_settings.value("csvFileName", "").toString();

    g_pImageBuffer = (unsigned char *)malloc(PTN_WIDTH*PTN_HEIGHT*BYTES_PER_PIXEL);
    if(g_pImageBuffer == NULL)
        exit(-1);
    memset(g_pImageBuffer, 0, PTN_WIDTH*PTN_HEIGHT*BYTES_PER_PIXEL);
    ui->lineEdit_CreaImgOutputBmpFileName->setText(m_outFileName);

    //Setup get video information UI
    ui->lineEdit_VidHRes->setValidator(new QIntValidator(ui->lineEdit_VidHRes));
    ui->lineEdit_VidVRes->setValidator(new QIntValidator(ui->lineEdit_VidVRes));
    ui->lineEdit_VidRsvd->setValidator(new QIntValidator(ui->lineEdit_VidRsvd));
    ui->lineEdit_VidPixClk->setValidator(new QIntValidator(ui->lineEdit_VidPixClk));
    ui->lineEdit_VidHFreq->setValidator(new QIntValidator(ui->lineEdit_VidHFreq));
    ui->lineEdit_VidVFreq->setValidator(new QIntValidator(ui->lineEdit_VidVFreq));
    ui->lineEdit_VidPixPerLine->setValidator(new QIntValidator(ui->lineEdit_VidPixPerLine));
    ui->lineEdit_VidLinesPerFrame->setValidator(new QIntValidator(ui->lineEdit_VidLinesPerFrame));
    ui->lineEdit_VidActvPixPerLine->setValidator(new QIntValidator(ui->lineEdit_VidActvPixPerLine));
    ui->lineEdit_VidActvLinesPerFrame->setValidator(new QIntValidator(ui->lineEdit_VidActvLinesPerFrame));
    ui->lineEdit_VidActvFirstPix->setValidator(new QIntValidator(ui->lineEdit_VidActvFirstPix));
    ui->lineEdit_VidActvFirstLine->setValidator(new QIntValidator(ui->lineEdit_VidActvFirstLine));

    //Set defaults to zero
    ui->lineEdit_VidHRes->setText(numToStr(0));
    ui->lineEdit_VidVRes->setText(numToStr(0));
    ui->lineEdit_VidRsvd->setText(numToStr(0));
    ui->lineEdit_VidPixClk->setText(numToStr(0));
    ui->lineEdit_VidHFreq->setText(numToStr(0));
    ui->lineEdit_VidVFreq->setText(numToStr(0));
    ui->lineEdit_VidPixPerLine->setText(numToStr(0));
    ui->lineEdit_VidLinesPerFrame->setText(numToStr(0));
    ui->lineEdit_VidActvPixPerLine->setText(numToStr(0));
    ui->lineEdit_VidActvLinesPerFrame->setText(numToStr(0));
    ui->lineEdit_VidActvFirstPix->setText(numToStr(0));
    ui->lineEdit_VidActvFirstLine->setText(numToStr(0));


    /* Set up some of the line edit boxes to receive only numerical value inputs */
    ui->lineEdit_PatSeqPatPeriod->setValidator(new QIntValidator(ui->lineEdit_PatSeqPatPeriod));
    ui->lineEdit_PatSeqPatExpTime->setValidator(new QIntValidator(ui->lineEdit_PatSeqPatExpTime));
    ui->lineEdit_VarExpPatSeqPatPeriod->setValidator(new QIntValidator(ui->lineEdit_VarExpPatSeqPatPeriod));
    ui->lineEdit_VarExpPatSeqPatExpTime->setValidator(new QIntValidator(ui->lineEdit_VarExpPatSeqPatExpTime));
    //ui->framePer_lineEdit->setValidator(new QIntValidator(ui->framePer_lineEdit));
    QIntValidator *inputRange = new QIntValidator(0, 255, this);
    ui->lineEdit_RedLEDCurrent->setValidator(inputRange);
    ui->lineEdit_GreenLEDCurrent->setValidator(inputRange);
    ui->lineEdit_BlueLEDCurrent->setValidator(inputRange);
    ui->lineEdit_PWMPeriod->setValidator(new QIntValidator(ui->lineEdit_PWMPeriod));
    ui->lineEdit_PWMCapSampleRate->setValidator(new QIntValidator(ui->lineEdit_PWMCapSampleRate));
    ui->lineEdit_PatSeqPatPeriod->setText(numToStr(100000));
    ui->lineEdit_PatSeqPatExpTime->setText(numToStr(100000));
    ui->lineEdit_VarExpPatSeqPatPeriod->setText(numToStr(10000));
    ui->lineEdit_VarExpPatSeqPatExpTime->setText(numToStr(10000));

    //Pattern Trigger Page update
    ui->spinBox_PatSeqFrameImgIndex->setValue(0);
    ui->spinBox_VarExpPatSeqFrameImgIndex->setValue(0);
    ui->pushButton_PatSeqCtrlStart->setEnabled(false);
    ui->pushButton_PatSeqCtrlPause->setEnabled(false);
    ui->pushButton_PatSeqCtrlStop->setEnabled(false);

    g_iniGUITokens << "DEFAULT.DISPMODE" \
                   << "DEFAULT.SHORT_FLIP" \
                   << "DEFAULT.LONG_FLIP" \
                   << "DEFAULT.TRIG_OUT_1.POL" \
                   << "DEFAULT.TRIG_OUT_1.RDELAY" \
                   << "DEFAULT.TRIG_OUT_1.FDELAY" \
                   << "DEFAULT.TRIG_OUT_2.POL" \
                   << "DEFAULT.TRIG_OUT_2.WIDTH" \
                   << "DEFAULT.TRIG_IN_1.DELAY" \
                   << "DEFAULT.TRIG_IN_2.POL" \
                   << "DEFAULT.RED_STROBE.RDELAY" \
                   << "DEFAULT.RED_STROBE.FDELAY" \
                   << "DEFAULT.GRN_STROBE.RDELAY" \
                   << "DEFAULT.GRN_STROBE.FDELAY" \
                   << "DEFAULT.BLU_STROBE.RDELAY" \
                   << "DEFAULT.BLU_STROBE.FDELAY" \
                   << "DEFAULT.INVERTDATA" \
                   << "DEFAULT.LEDCURRENT_RED" \
                   << "DEFAULT.LEDCURRENT_GRN" \
                   << "DEFAULT.LEDCURRENT_BLU" \
                   << "DEFAULT.PATTERNCONFIG.PAT_EXPOSURE" \
                   << "DEFAULT.PATTERNCONFIG.PAT_PERIOD" \
                   << "DEFAULT.PATTERNCONFIG.PAT_MODE" \
                   << "DEFAULT.PATTERNCONFIG.TRIG_MODE" \
                   << "DEFAULT.PATTERNCONFIG.PAT_REPEAT" \
                   << "DEFAULT.PATTERNCONFIG.NUM_LUT_ENTRIES" \
                   << "DEFAULT.PATTERNCONFIG.NUM_PATTERNS" \
                   << "DEFAULT.PATTERNCONFIG.NUM_SPLASH" \
                   << "DEFAULT.SPLASHLUT" \
                   << "DEFAULT.SEQPATLUT" \
                   << "DEFAULT.LED_ENABLE_MAN_MODE" \
                   << "DEFAULT.MAN_ENABLE_RED_LED" \
                   << "DEFAULT.MAN_ENABLE_GRN_LED" \
                   << "DEFAULT.MAN_ENABLE_BLU_LED" \
                   << "DEFAULT.PORTCONFIG.PORT" \
                   << "DEFAULT.PORTCONFIG.BPP" \
                   << "DEFAULT.PORTCONFIG.PIX_FMT" \
                   << "DEFAULT.PORTCONFIG.PORT_CLK" \
                   << "DEFAULT.PORTCONFIG.ABC_MUX" \
                   << "DEFAULT.PORTCONFIG.PIX_MODE" \
                   << "DEFAULT.PORTCONFIG.SWAP_POL" \
                   << "DEFAULT.PORTCONFIG.FLD_SEL" ;

	m_dlpUSB->DLPC350_USB_Init();
    ui->pushButton_Connect->setEnabled(m_dlpUSB->DLPC350_USB_IsConnected());

    m_usbPollTimer = new QTimer(this);
    m_usbPollTimer->setInterval(2000);
    connect(m_usbPollTimer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
    m_usbPollTimer->start();

    ui->comboBox_PatSeqPatBitDepthSel->setCurrentIndex(7);
    ui->comboBox_PatSeqSelPatColor->setCurrentIndex(7);

	//UI initial for hiding sensitive data
	ui->groupBox_LEDDriverCtrl->setVisible(true);
	ui->groupBox_ImageSetting->setVisible(false);
	ui->tabWidget->removeTab(4);
	ui->tabWidget->removeTab(3);
	ui->tabWidget->removeTab(2);
	ui->tabWidget->removeTab(0);
	ui->radioButton_VariableExpSLMode->setEnabled(false);
	ui->radioButton_VideoMode->setEnabled(false);
	//ui->radioButton_PatSeqDispRunOnce->setChecked(true);

	ui->tabWidget_2->removeTab(5);
	ui->tabWidget_2->removeTab(3);
	ui->tabWidget_2->setTabEnabled(1, false);	
	ui->tabWidget_2->setCurrentIndex(0);


	QPalette Pal(palette());
	// set black background
	Pal.setColor(QPalette::Background, QColor(220, 220, 220));
	setAutoFillBackground(true);
	setPalette(Pal);

	m_bApplyDefaultSolution = false;
	m_bConnected = false;
	m_bPatSeqValidated = false;
	m_nDLPIndex = 0;
}

MainWindow::~MainWindow()
{
    int i;

	m_dlpUSB->DLPC350_USB_Close();
	m_dlpUSB->DLPC350_USB_Exit();
    m_settings.setValue("FirmwarePath", m_firmwarePath);
    m_settings.setValue("PtnImagePath", m_ptnImagePath);
    m_settings.setValue("outFileName", m_outFileName);
    m_settings.setValue("csvFileName", m_csvFileName);
	if (g_pImageBuffer)
	{
		free(g_pImageBuffer);
		g_pImageBuffer = NULL;
	}		
    for(i = 0; i < MAX_SPLASH_IMAGES; i++)
    {
        char file_name[11];

        sprintf(file_name, "temp_%d.bmp", i);
        QFile outFile(file_name);

        outFile.remove();
    }
    
    delete ui;
}

void MainWindow::ShowError(const char *str)
{
    QString title("LightCrafter Error Message");
    QString text(str);
    QMessageBox msgBox(QMessageBox::Warning, title, text, QMessageBox::NoButton, this);
    msgBox.exec();
}

void MainWindow::RefreshGUISettingsFromDLPC350()
{
    //Update all setting under the page
    emit on_pushButton_GetPortSource_clicked();
    emit on_pushButton_GetPortSwap_clicked();
    emit on_pushButton_GetPortClock_clicked();
    emit on_pushButton_GetPortPixelFormat_clicked();
    emit on_pushButton_GetDisplayConfiguration_clicked();
    emit on_pushButton_GetFPDMode_clicked();
    emit on_pushButton_GetTPGColor_clicked();
    emit on_pushButton_GetVideoSingalInfo_clicked();
    emit on_pushButton_GetFlip_clicked();
    emit on_pushButton_GetLEDConfiguration_clicked();
    emit on_pushButton_GetLEDDlyCtrlConfig_clicked();
    emit on_pushButton_GetTrigConfig_clicked();
}

void MainWindow::ApplyGUISettingToDLPC350()
{
    //If it is in structured light mode
    //Stop pattern sequence mode if it is running
    bool mode;
    unsigned int patMode;
	m_dlpAPI->DLPC350_GetMode(&mode);
    if(mode == true)
    {
		m_dlpAPI->DLPC350_GetPatternDisplay(&patMode);
        if(patMode == 2)
            emit on_pushButton_PatSeqCtrlStop_clicked();
    }

    emit on_pushButton_SetLedDlyCtrlConfig_clicked();
    emit on_pushButton_SetLEDConfiguration_clicked();
    emit on_pushButton_SetFlip_clicked();
    emit on_pushButton_SetPortSource_clicked();
    emit on_pushButton_SetPortClock_clicked();
    emit on_pushButton_SetPortPixelFormat_clicked();
    emit on_pushButton_SetPortSwap_clicked();
    emit on_pushButton_SetFPDMode_clicked();

    if(ui->radioButton_VideoMode->isChecked())
    {
        emit on_radioButton_VideoMode_clicked();
    }
    else
    {
        SetDLPC350InPatternMode();

        //LED trigger configuration applicable in Pattern Mode **ONLY**
        emit on_pushButton_SetTrigConfig_clicked();

        if(ui->radioButton_SLMode->isChecked())
        {
            // Select the video mode tab
            ui->tabWidget->setCurrentIndex(1);
            ui->tabWidget_2->setCurrentIndex(0);

            if(ui->radioButton_PatSeqSrcFrmVideoPort->isChecked())
            {
				m_dlpAPI->DLPC350_SetPatternTriggerMode(0x00); //Trigger Mode - 0
				m_dlpAPI->DLPC350_SetPatternDisplayMode(true);
            }
            else
            {
				m_dlpAPI->DLPC350_SetPatternTriggerMode(0x01); //Trigger Mode - 1
				m_dlpAPI->DLPC350_SetPatternDisplayMode(false);
            }
        }

        if(ui->radioButton_VariableExpSLMode->isChecked())
        {
            // Select the video mode tab
            ui->tabWidget->setCurrentIndex(1);
            ui->tabWidget_2->setCurrentIndex(1);

            if(ui->radioButton_VarExpPatSeqSrcFrmVideoPort->isChecked())
            {
				m_dlpAPI->DLPC350_SetPatternTriggerMode(0x04); //Trigger Mode - 4
				m_dlpAPI->DLPC350_SetPatternDisplayMode(true);
            }
            else
            {
				m_dlpAPI->DLPC350_SetPatternTriggerMode(0x03); //Trigger Mode - 3
				m_dlpAPI->DLPC350_SetPatternDisplayMode(false);
            }
        }
    }
}

int MainWindow::GetDLPC350Status()
{
    unsigned char HWStatus, SysStatus, MainStatus;

	if (m_dlpAPI->DLPC350_GetStatus(&HWStatus, &SysStatus, &MainStatus) == 0)
    {
        ui->indicatorButton_statusInitDone->setEnabled((HWStatus & BIT0) == BIT0); //Init Done
        ui->indicatorButton_statusForcedSwap->setEnabled((HWStatus & BIT3) == BIT3); //Forced Swap
        ui->indicatorButton_statusSeqAbort->setEnabled((HWStatus & BIT6) == BIT6); //Sequence Abort
        ui->indicatorButton_statusDrcErr->setEnabled((HWStatus & BIT2) == BIT2); // DRC Error
        ui->indicatorButton_statusSeqErr->setEnabled((HWStatus & BIT7) == BIT7); // Sequence Error


        ui->indicatorButton_statusDmdParked->setEnabled((MainStatus & BIT0) == BIT0); //DMD Parked
        ui->indicatorButton_statusSeqRun->setEnabled((MainStatus & BIT1) == BIT1); // Sequence Running
        ui->indicatorButton_statusBufFreeze->setEnabled((MainStatus & BIT2) == BIT2); //Buffer frozen
        return 0;
    }
    else
    {
        ui->indicatorButton_statusInitDone->setEnabled(false);
        ui->indicatorButton_statusForcedSwap->setEnabled(false);
        ui->indicatorButton_statusSeqAbort->setEnabled(false);
        ui->indicatorButton_statusDrcErr->setEnabled(false);
        ui->indicatorButton_statusSeqErr->setEnabled(false);
        ui->indicatorButton_statusDmdParked->setEnabled(false);
        ui->indicatorButton_statusSeqRun->setEnabled(false);
        ui->indicatorButton_statusBufFreeze->setEnabled(false);
        return -1;
    }
}

void MainWindow::SetDLPC350InVideoMode()
{
    int i = 0;
    bool mode;
    unsigned int patMode;

    //Check if it is in Pattern Mode
	m_dlpAPI->DLPC350_GetMode(&mode);
    if(mode == true)
    {
       //First stop pattern sequence
		m_dlpAPI->DLPC350_GetPatternDisplay(&patMode);
        //if it is in PAUSE or RUN mode
        if(patMode != 0)
        {
            emit on_pushButton_PatSeqCtrlStop_clicked();
        }

        //Switch to Video Mode
		m_dlpAPI->DLPC350_SetMode(false);
        SleeperThread::msleep(100);
        while(1)
        {
			m_dlpAPI->DLPC350_GetMode(&mode);
            if(!mode)
                break;
            SleeperThread::msleep(100);
            if(i++ > MAX_NUM_RETRIES)
                break;
        }
    }

    return;
}

void MainWindow::SetDLPC350InPatternMode()
{
    int i = 0;
    bool mode;
    unsigned int patMode;

    //Check if it is in Pattern Mode
	m_dlpAPI->DLPC350_GetMode(&mode);
    if(mode == false)
    {
        //Switch to Pattern Mode
		m_dlpAPI->DLPC350_SetMode(true);
        SleeperThread::msleep(100);
        while(1)
        {
			m_dlpAPI->DLPC350_GetMode(&mode);
            if(mode)
                break;
            SleeperThread::msleep(100);
            if(i++ > MAX_NUM_RETRIES)
                break;
        }
    }
    else
    {
        //First stop pattern sequence
		m_dlpAPI->DLPC350_GetPatternDisplay(&patMode);
         //if it is in PAUSE or RUN mode
         if(patMode != 0)
         {
             emit on_pushButton_PatSeqCtrlStop_clicked();
         }
    }

    return;
}

void MainWindow::timerTimeout(void)
{
	if (m_dlpUSB->DLPC350_USB_IsConnected())
    {
        if(ui->checkBox_updateStatus->isChecked())
        {
            if(GetDLPC350Status()!= 0)
            {
                ui->pushButton_Connect->setEnabled(m_dlpUSB->DLPC350_USB_IsConnected());
            }
        }

		if (!m_bApplyDefaultSolution)
		{
			ApplyDefaultSolution();
			m_bApplyDefaultSolution = true;
		}

		m_bConnected = true;
    }
    else
    {
		m_bConnected = false;		
	
		if (m_usbSerialNumber.empty())
		{
			if (m_dlpUSB->DLPC350_USB_Open(NULL) == 0)
			{
				System->setTrackInfo("USB connect!");

				ui->pushButton_Connect->setEnabled(m_dlpUSB->DLPC350_USB_IsConnected());

				emit on_pushButton_Connect_clicked();
			}
		}
		else
		{
			//int mystringSize = (int)(m_usbSerialNumber.length() + 1);
			//wchar_t* mywstring = new wchar_t[mystringSize];
			//MultiByteToWideChar(CP_ACP, 0, m_usbSerialNumber.c_str(), -1, mywstring, mystringSize);
			//mywstring[mystringSize - 1] = '\0';
			//use mywstring to do:
			if (m_dlpUSB->DLPC350_USB_OpenByProductStr(m_usbSerialNumber) == 0)
			{
				m_bApplyDefaultSolution = false; 

				System->setTrackInfo("USB connect!");

				ui->pushButton_Connect->setEnabled(m_dlpUSB->DLPC350_USB_IsConnected());

				emit on_pushButton_Connect_clicked();
			}
			//delete[] mywstring;
		}
    }
}

void MainWindow::on_checkBox_updateStatus_toggled(bool checked)
{
    if(!checked){
        ui->indicatorButton_statusInitDone->setEnabled(false);
        ui->indicatorButton_statusForcedSwap->setEnabled(false);
        ui->indicatorButton_statusSeqAbort->setEnabled(false);
        ui->indicatorButton_statusDrcErr->setEnabled(false);
        ui->indicatorButton_statusSeqErr->setEnabled(false);
        ui->indicatorButton_statusDmdParked->setEnabled(false);
        ui->indicatorButton_statusSeqRun->setEnabled(false);
        ui->indicatorButton_statusBufFreeze->setEnabled(false);
    }
    else{
        GetDLPC350Status();
    }
}

void MainWindow::on_pushButton_Connect_clicked()
{
    char versionStr[255];
    unsigned int API_ver, App_ver, SWConfig_ver, SeqConfig_ver;
    int trigMode=0;
    bool SLmode=0;
    bool isExtPatDisplayMode=false;
    unsigned int numImgInFlash = 0;

    ui->pushButton_Connect->setEnabled(m_dlpUSB->DLPC350_USB_IsConnected());

    // Display GUI Version #
    sprintf(versionStr, "DLP LightCrafter 4500 Control Software - %d.%d.%d", \
            GUI_VERSION_MAJOR, GUI_VERSION_MINOR, GUI_VERSION_BUILD);

    setWindowTitle(versionStr);

    if(m_dlpUSB->DLPC350_USB_IsConnected())
    {
        //Read System Status
        if(GetDLPC350Status() <0 )
            return ;

		if (m_dlpAPI->DLPC350_GetVersion(&App_ver, &API_ver, &SWConfig_ver, &SeqConfig_ver) == 0)
        {
            sprintf(versionStr, "%d.%d.%d", (App_ver >> 24), ((App_ver << 8) >> 24), ((App_ver << 16) >> 16));
            ui->label_ApiVersion->setText(versionStr);
        }
        else
            return ;

//        m_numImgInFlash = 60;
//        ui->spinBox_PatSeqFrameImgIndex->setRange(0,(m_numImgInFlash-1));
        //Read firmware tag information
        unsigned char firmwareTag[33];
		if (m_dlpAPI->DLPC350_GetFirmwareTagInfo(&firmwareTag[0]) == 0)
        {
            QString str((char *)firmwareTag);
            ui->label_firmwareTagInfo->setText(str);
        }
        else
            return;

        //Retrieve the total number of Images in the firmware info
		if (m_dlpAPI->DLPC350_GetNumImagesInFlash(&numImgInFlash) == 0)
        {            
            m_numImgInFlash = numImgInFlash;
            //m_numImgInFlash = 64;
            ui->spinBox_ImgLdTmImgIndex->setRange(0,(numImgInFlash-1));
			ui->spinBox_PatSeqFrameImgIndex->setRange(0,(m_numImgInFlash-1));
			ui->spinBox_VarExpPatSeqFrameImgIndex->setRange(0,(m_numImgInFlash-1));
        }
        else
            return ;

        // qDebug() << "Number of splash images in the flash = " << numImgInFlash;

        //Update UI content based on the mode
        bool standby;
		if (m_dlpAPI->DLPC350_GetPowerMode(&standby) != 0)
        {
            ;//ShowError("PowerMode Read failed!!!");
        }

        //Read Display Mode & update UI elements
		if ((m_dlpAPI->DLPC350_GetMode(&SLmode) == 0) && (standby == false))
        {
            if(SLmode)
            {
                ui->radioButton_VideoMode->setChecked(false);

				if (m_dlpAPI->DLPC350_GetPatternTriggerMode(&trigMode) == 0)
                {
                    if(trigMode <= 2)
                    {
                        ui->radioButton_SLMode->setChecked(true);
                        ui->radioButton_VariableExpSLMode->setChecked(false);
						if (m_dlpAPI->DLPC350_GetPatternDisplayMode(&isExtPatDisplayMode) == 0)
                        {
                            if(isExtPatDisplayMode) //if set to external DVI/FPD port
                            {
                                ui->radioButton_PatSeqSrcFrmVideoPort->setChecked(true);
                                emit on_radioButton_PatSeqSrcFrmVideoPort_clicked();
                            }
                            else
                            {
                                ui->radioButton_PatSeqSrcFrmFlash->setChecked(true);
                                emit on_radioButton_PatSeqSrcFrmFlash_clicked();
                            }
                        }
                        else
                            return ;

                        emit on_radioButton_SLMode_clicked();
                    }
                    else
                    {
                        ui->radioButton_SLMode->setChecked(false);
                        ui->radioButton_VariableExpSLMode->setChecked(true);
						if (m_dlpAPI->DLPC350_GetPatternDisplayMode(&isExtPatDisplayMode) == 0)
                        {
                            if(isExtPatDisplayMode) //if set to external DVI/FPD port
                            {
                                ui->radioButton_VarExpPatSeqSrcFrmVideoPort->setChecked(true);
                                emit on_radioButton_VarExpPatSeqSrcFrmVideoPort_clicked();
                            }
                            else
                            {
                                ui->radioButton_VarExpPatSeqSrcFrmFlash->setChecked(true);
                                emit on_radioButton_VarExpPatSeqSrcFrmFlash_clicked();
                            }
                        }
                        else
                            return ;

                        emit on_radioButton_VariableExpSLMode_clicked();
                    }
                }
            }
            else
            {
                ui->radioButton_SLMode->setChecked(false);
                ui->radioButton_VariableExpSLMode->setChecked(false);
                ui->radioButton_VideoMode->setChecked(true);
                emit on_radioButton_VideoMode_clicked();
            }
        }
        else
        {
            ui->radioButton_SLMode->setChecked(false);
            ui->radioButton_VariableExpSLMode->setChecked(false);
            ui->radioButton_SLMode->setEnabled(false);
            ui->radioButton_VariableExpSLMode->setEnabled(false);
            ui->radioButton_VideoMode->setChecked(false);
            ui->radioButton_StandbyMode->setChecked(true);
            m_isPrevModeStandBy = true;
        }

//        //Update LED status
//        emit on_pushButton_GetLEDConfiguration_clicked();
//        emit on_pushButton_GetLEDDlyCtrlConfig_clicked();
//        emit on_pushButton_GetTrigConfig_clicked();
    }
}

void MainWindow::on_pushButton_Reset_clicked()
{
    m_usbPollTimer->stop();

	m_dlpAPI->DLPC350_SoftwareReset();
    m_dlpUSB->DLPC350_USB_Close();
    ui->pushButton_Connect->setEnabled(false);

    QEventLoop loop;
    QTimer::singleShot(5000, &loop, SLOT(quit()));
    loop.exec();

    m_usbPollTimer->start();

}

void MainWindow::on_radioButton_VideoMode_clicked()
{
    int i = 0;
    bool mode;

    if(m_isPrevModeStandBy)
    {
        m_isPrevModeStandBy = false;

        ui->radioButton_SLMode->setEnabled(true);
        ui->radioButton_VariableExpSLMode->setEnabled(true);

        //Wakeup from standby
        //Bug? - Response fails for DLPC350_SetPowerMode command
		m_dlpAPI->DLPC350_SetPowerMode(0);
        //Assuming it takes around 2.0 seconds
        SleeperThread::msleep(2000);
        //        if(DLPC350_SetPowerMode(0) != 0)
        //        {
        //            qDebug() << "DLPC350_SetPowerMode(0) FAILED";
        //            return;
        //        }

        while(1)
        {
			m_dlpAPI->DLPC350_GetPowerMode(&mode);
            if(!mode)
                break;
            SleeperThread::msleep(100);
            if(i++ > (MAX_NUM_RETRIES*2))
                break;
        }
    }

    //Enter in Video Mode
    SetDLPC350InVideoMode();

    //Clear Validate Button
    ui->pushButton_PatSeqValIndExpOOR->setEnabled(false);
    ui->pushButton_PatSeqValIndPatNumOOR->setEnabled(false);
    ui->pushButton_PatSeqValIndTrigOutOverlap->setEnabled(false);
    ui->pushButton_PatSeqValIndBlkVecMiss->setEnabled(false);
    ui->pushButton_PatSeqValPatPeriodShort->setEnabled(false);

    ui->pushButton_PatSeqValStatus->setEnabled(false);
    ui->pushButton_PatSeqCtrlStart->setEnabled(false);
    ui->pushButton_PatSeqCtrlPause->setEnabled(false);
    ui->pushButton_PatSeqCtrlStop->setEnabled(false);

    // Select the video mode tab
    ui->tabWidget->setCurrentIndex(0);

    //Refresh the GUI settings
    RefreshGUISettingsFromDLPC350();
}

void MainWindow::on_radioButton_SLMode_clicked()
{
    int trigMode = 0;
    bool isExtPatDisplayMode = false;


    SetDLPC350InPatternMode();

    // Select the video mode tab
    ui->tabWidget->setCurrentIndex(1);
    ui->tabWidget_2->setCurrentIndex(0);

    //Update all the settings under the page
	if (m_dlpAPI->DLPC350_GetPatternTriggerMode(&trigMode) == 0)
    {
        if(trigMode <= 2)
        {
			if (m_dlpAPI->DLPC350_GetPatternDisplayMode(&isExtPatDisplayMode) == 0)
            {
                if(isExtPatDisplayMode) //if set to external DVI/FPD port
                {
                    ui->radioButton_PatSeqSrcFrmVideoPort->setChecked(true);
                    emit on_radioButton_PatSeqSrcFrmVideoPort_clicked();
                }
                else
                {
                    ui->radioButton_PatSeqSrcFrmFlash->setChecked(true);
                    emit on_radioButton_PatSeqSrcFrmFlash_clicked();
                }
            }
        }
    }

    //Refresh the GUI settings
    RefreshGUISettingsFromDLPC350();
}

void MainWindow::on_radioButton_VariableExpSLMode_clicked()
{
    int trigMode = 0;
    bool isExtPatDisplayMode = false;

    //if pattern sequence is already running it must be stopped first
    emit on_pushButton_PatSeqCtrlStop_clicked();

    SetDLPC350InPatternMode();

    // Select the variable exposure tab
    ui->tabWidget->setCurrentIndex(1);
    ui->tabWidget_2->setCurrentIndex(1);

    //Update all the settings under the page
	if (m_dlpAPI->DLPC350_GetPatternTriggerMode(&trigMode) == 0)
    {
        if((trigMode == 3) || (trigMode == 4))
        {
			if (m_dlpAPI->DLPC350_GetPatternDisplayMode(&isExtPatDisplayMode) == 0)
            {
                if(isExtPatDisplayMode) //if set to external DVI/FPD port
                {
                    ui->radioButton_PatSeqSrcFrmVideoPort->setChecked(true);
                    emit on_radioButton_PatSeqSrcFrmVideoPort_clicked();
                }
                else
                {
                    ui->radioButton_PatSeqSrcFrmFlash->setChecked(true);
                    emit on_radioButton_PatSeqSrcFrmFlash_clicked();
                }
            }
        }
    }

    //Refresh the GUI settings
    RefreshGUISettingsFromDLPC350();
}

void MainWindow::on_radioButton_StandbyMode_clicked()
{
    SetDLPC350InVideoMode();

    /* Set power mode standby */
	m_dlpAPI->DLPC350_SetPowerMode(1);
    SleeperThread::msleep(2000);

    ui->radioButton_SLMode->setEnabled(false);
    ui->radioButton_VariableExpSLMode->setEnabled(false);

    m_isPrevModeStandBy = true;
}

void MainWindow::on_pushButton_GetLEDConfiguration_clicked()
{
    bool SeqCtrl, Red, Green, Blue, inverted;
    unsigned char RedCurrent, GreenCurrent, BlueCurrent;
    char currentStr[8];

	if (m_dlpAPI->DLPC350_GetLedEnables(&SeqCtrl, &Red, &Green, &Blue) == 0)
    {
        ui->radioButton_ColorDisplayAuto->setChecked(SeqCtrl);
        ui->radioButton_ColorDisplayManual->setChecked(!SeqCtrl);

        if(SeqCtrl)
        {
            ui->checkBox_RedEnable->setDisabled(true);
            ui->checkBox_GreenEnable->setDisabled(true);
            ui->checkBox_BlueEnable->setDisabled(true);
        }
        else
        {
            ui->checkBox_RedEnable->setChecked(Red);
            ui->checkBox_GreenEnable->setChecked(Green);
            ui->checkBox_BlueEnable->setChecked(Blue);
        }
    }


	if (m_dlpAPI->DLPC350_GetLedCurrents(&RedCurrent, &GreenCurrent, &BlueCurrent) == 0)
    {
        sprintf(currentStr,"%d", 255-RedCurrent);
        ui->lineEdit_RedLEDCurrent->setText(currentStr);
        sprintf(currentStr,"%d", 255-GreenCurrent);
        ui->lineEdit_GreenLEDCurrent->setText(currentStr);
        sprintf(currentStr,"%d", 255-BlueCurrent);
        ui->lineEdit_BlueLEDCurrent->setText(currentStr);
    }

	if (m_dlpAPI->DLPC350_GetLEDPWMInvert(&inverted) == 0)
    {
        ui->checkBox_LedPwmInvert->setChecked(inverted);
    }
}

void MainWindow::on_pushButton_SetLEDConfiguration_clicked()
{
    bool SeqCtrl, Red, Green, Blue;
    unsigned char RedCurrent, GreenCurrent, BlueCurrent;

    // Should the LEDs be controlled by the sequence?
    SeqCtrl = ui->radioButton_ColorDisplayAuto->isChecked();
    Red = ui->checkBox_RedEnable->isChecked();
    Green = ui->checkBox_GreenEnable->isChecked();
    Blue = ui->checkBox_BlueEnable->isChecked();

	m_dlpAPI->DLPC350_SetLedEnables(SeqCtrl, Red, Green, Blue);

	m_dlpAPI->DLPC350_SetLEDPWMInvert(ui->checkBox_LedPwmInvert->isChecked());

    RedCurrent      = 255-strToNum(ui->lineEdit_RedLEDCurrent->text());
    GreenCurrent    = 255-strToNum(ui->lineEdit_GreenLEDCurrent->text());
    BlueCurrent     = 255-strToNum(ui->lineEdit_BlueLEDCurrent->text());

	m_dlpAPI->DLPC350_SetLedCurrents(RedCurrent, GreenCurrent, BlueCurrent);
}

void MainWindow::on_pushButton_GetFlip_clicked()
{
	ui->checkBox_longAxisFlip->setChecked(m_dlpAPI->DLPC350_GetLongAxisImageFlip());
	ui->checkBox_shortAxisFlip->setChecked(m_dlpAPI->DLPC350_GetShortAxisImageFlip());
}

void MainWindow::on_pushButton_SetFlip_clicked()
{
	m_dlpAPI->DLPC350_SetLongAxisImageFlip(ui->checkBox_longAxisFlip->isChecked());
	m_dlpAPI->DLPC350_SetShortAxisImageFlip(ui->checkBox_shortAxisFlip->isChecked());
}

void MainWindow::on_pushButton_InitPatternSeq_clicked()
{
    //DLPC350_SetFreeze(false);

    uint numImgInFlash = 0;
	if (m_dlpAPI->DLPC350_GetNumImagesInFlash(&numImgInFlash) == 0)
    {
        QString str = QString("%0").arg(numImgInFlash);
        qDebug() << "Flash Num:" << str;
    }

    //if (DLPC350_SetBufferWriteEnables(false) == 0)
    //{
    //    qDebug() << "Buffer write:" << "set disable";
    //}
   // else
    //{
    //    qDebug() << "set buffer write disable error";
    //}

    /*
    if (DLPC350_SetBufferWriteEnables(false) != 0)
    {
         qDebug() << "DLPC350_SetBufferWriteEnables error";
    }

    ::Sleep(100);

    if (DLPC350_SetBufferWriteEnables(true) != 0)
    {
         qDebug() << "DLPC350_SetBufferWriteEnables error";
    }
    */


    //if (DLPC350_LoadImageIndex(0) != 0)
    {
    //     qDebug() << "DLPC350_LoadImageIndex error";
    }


    //if (DLPC350_LoadImageIndex(1) != 0)
    {
    //     qDebug() << "DLPC350_LoadImageIndex error";
    }


    //DLPC350_SetFreeze(false);

	if (m_dlpAPI->DLPC350_SetBufferWriteEnables(false) < 0)
    {
         qDebug() << "DLPC350_SetBufferWriteEnables error";
    }

//    if (DLPC350_LoadImageIndex(0) < 0)
//    {
//         qDebug() << "DLPC350_LoadImageIndex error";
//    }

//    if (DLPC350_LoadImageIndex(1) < 0)
//    {
//         qDebug() << "DLPC350_LoadImageIndex error";
//    }

//    if (DLPC350_SetBufferWriteEnables(true) < 0)
//    {
//         qDebug() << "DLPC350_SetBufferWriteEnables error";
//    }

    bool bDisable = false;
	if (m_dlpAPI->DLPC350_GetBufferWriteEnables(&bDisable) == 0)
    {
        QString str = bDisable ? "disable" : "enable";
        qDebug() << "Buffer write:" << str;
    }
    else
    {
        qDebug() << "get buffer write enable error";
    }
}

void MainWindow::on_radioButton_ColorDisplayAuto_clicked()
{
    //Disable the manual color display checkboxes
    ui->checkBox_RedEnable->setDisabled(true);
    ui->checkBox_GreenEnable->setDisabled(true);
    ui->checkBox_BlueEnable->setDisabled(true);
}

void MainWindow::on_radioButton_ColorDisplayManual_clicked()
{
    //Enable the manual color display checkboxes
    ui->checkBox_RedEnable->setDisabled(false);
    ui->checkBox_GreenEnable->setDisabled(false);
    ui->checkBox_BlueEnable->setDisabled(false);
}


/* Solution - Apply/Save/Default */

void MainWindow::ApplyIniParam(QString token, uint32 *params, int numParams)
{
    static unsigned char splashLut[256];
    static int numSplashLutEntries = 0;
    static int numPatLutEntries = 0;
    static int numPatToDispForTrigOut2 = 0;
    static bool isVarExpTrigMode = false;
    int splashIndex = 0, frameIndex;
    char dispStr[255];
    bool tempBoolVar;

    switch(g_iniGUITokens.indexOf(token))
    {
    case 0:				//DEFAULT.DISPMODE
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.DISPMODE");

        if (params[0])
            //ui->Radiobutton_SLmode->click();
            ui->radioButton_SLMode->setChecked(true);
        else
            ui->radioButton_VideoMode->setChecked(true);

        break;

    case 1:				//DEFAULT.SHORT_FLIP
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.SHORT_FLIP");

        if (params[0])
            ui->checkBox_shortAxisFlip->setChecked(true);
        else
            ui->checkBox_shortAxisFlip->setChecked(false);

        break;

    case 2:				//DEFAULT.LONG_FLIP
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.LONG_FLIP");

        if (params[0])
            ui->checkBox_longAxisFlip->setChecked(true);
        else
            ui->checkBox_longAxisFlip->setChecked(false);
        break;


    case 3:				//DEFAULT.TRIG_OUT_1.POL
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.TRIG_OUT_1.POL");

        if (params[0])
            ui->checkBox_InvertTrig1Out->setChecked(true);
        else
            ui->checkBox_InvertTrig1Out->setChecked(false);
        break;

    case 4:				//DEFAULT.TRIG_OUT_1.RDELAY
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.TRIG_OUT_1.RDELAY");

        ui->spinBox_Trig1OutRDly->setValue(params[0]);
        break;

    case 5:				//DEFAULT.TRIG_OUT_1.FDELAY
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.DISPMODE\n");

        ui->spinBox_Trig1OutFDly->setValue(params[0]);
        break;

    case 6:				//DEFAULT.TRIG_OUT_2.POL
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.TRIG_OUT_1.FDELAY");

        if (params[0])
            ui->checkBox_InvertTrig2Out->setChecked(true);
        else
            ui->checkBox_InvertTrig2Out->setChecked(false);
        break;

    case 7:				//DEFAULT.TRIG_OUT_2.WIDTH
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.TRIG_OUT_2.WIDTH");

        ui->spinBox_Trig2OutRDly->setValue(params[0]);
        break;

    case 8:				//DEFAULT.TRIG_IN_1.DELAY
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.TRIG_IN_1.DELAY");

        ui->spinBox_TrigIn1->setValue(params[0]);
        break;

    case 9:				//DEFAULT.TRIG_IN_2.POL
        if((numParams > 1) && (params[0] <= 1))
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.TRIG_IN_2.POL");

        ui->comboBox_TrigIn2Pol->setCurrentIndex(params[0]);
        break;


    case 10:				//DEFAULT.RED_STROBE.RDELAY
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.RED_STROBE.RDELAY");

        ui->spinBox_LedDlyCtrlRedREdgeDly->setValue(params[0]);
        break;

    case 11:				//DEFAULT.RED_STROBE.FDELAY
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.RED_STROBE.FDELAY");

        ui->spinBox_LedDlyCtrlRedFEdgeDly->setValue(params[0]);
        break;

    case 12:				//DEFAULT.GRN_STROBE.RDELAY
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.GRN_STROBE.RDELAY");

        ui->spinBox_LedDlyCtrlGreenREdgeDly->setValue(params[0]);
        break;

    case 13:				//DEFAULT.GRN_STROBE.FDELAY
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.GRN_STROBE.FDELAY");

        ui->spinBox_LedDlyCtrlGreenFEdgeDly->setValue(params[0]);
        break;

    case 14:				//DEFAULT.BLU_STROBE.RDELAY
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.BLU_STROBE.RDELAY");

        ui->spinBox_LedDlyCtrlBlueREdgeDly->setValue(params[0]);
        break;

    case 15:				//DEFAULT.BLU_STROBE.FDELAY
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.BLU_STROBE.FDELAY");

        ui->spinBox_LedDlyCtrlBlueFEdgeDly->setValue(params[0]);
        break;

    case 16:				//DEFAULT.INVERTDATA
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.INVERTDATA");
        if (params[0])
            ui->checkBox_PatSeqCtrlGlobalDataInvert->setChecked(true);
        else
            ui->checkBox_PatSeqCtrlGlobalDataInvert->setChecked(false);
        break;

    case 17:				//DEFAULT.LEDCURRENT_RED
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.LEDCURRENT_RED");

        ui->lineEdit_RedLEDCurrent->setText(QString::number(255 - params[0]));
        break;

    case 18:				//DEFAULT.LEDCURRENT_GRN
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.LEDCURRENT_GRN");

        ui->lineEdit_GreenLEDCurrent->setText(QString::number(255 - params[0]));

        break;

    case 19:				//DEFAULT.LEDCURRENT_BLU
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.LEDCURRENT_BLU");

        ui->lineEdit_BlueLEDCurrent->setText(QString::number(255 - params[0]));
        break;

    case 20:				//DEFAULT.PATTERNCONFIG.PAT_EXPOSURE
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PATTERNCONFIG.PAT_EXPOSURE");
        ui->lineEdit_PatSeqPatExpTime->setText(QString::number(params[0]));

        break;

    case 21:				//DEFAULT.PATTERNCONFIG.PAT_PERIOD
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PATTERNCONFIG.PAT_PERIOD");
        ui->lineEdit_PatSeqPatPeriod->setText(QString::number(params[0]));
        break;

    case 22:				//DEFAULT.PATTERNCONFIG.PAT_MODE
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PATTERNCONFIG.PAT_MODE");

        if (params[0] == 0x3)
        {
            ui->radioButton_PatSeqSrcFrmFlash->setChecked(true);
            emit on_radioButton_PatSeqSrcFrmFlash_clicked();

            ui->radioButton_VarExpPatSeqSrcFrmFlash->setChecked(true);
            emit on_radioButton_VarExpPatSeqSrcFrmFlash_clicked();
        }
        else if (params[0] == 0x0)
        {
            ui->radioButton_PatSeqSrcFrmVideoPort->setChecked(true);
            emit on_radioButton_PatSeqSrcFrmVideoPort_clicked();

            ui->radioButton_VarExpPatSeqSrcFrmVideoPort->setChecked(true);
            emit on_radioButton_VarExpPatSeqSrcFrmVideoPort_clicked();
        }
        else
            ShowError("Wrong value as argument for DEFAULT.PATTERNCONFIG.PAT_MODE");
        break;

    case 23:				//DEFAULT.PATTERNCONFIG.TRIG_MODE

        isVarExpTrigMode = false;

        if(numParams > 1 && params[0] <= 4)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PATTERNCONFIG.TRIG_MODE");

        if (params[0])
            ui->radioButton_PatSeqTrigTypeIntExt->setChecked(true);
        else
            ui->radioButton_PatSeqTrigTypeVSync->setChecked(true);

        //If trigger mode pointing to variable exposure pattern sequence update the GUI elements under variable exposure
        if(params[0] > 2 && params[0] <= 4 )
        {
            isVarExpTrigMode = true;

            //1.Update Variable exposure trigger mode
            if(params[0] == 3)
                ui->radioButton_VarExpPatSeqTrigTypeIntExt->setChecked(true);
            else
                ui->radioButton_VarExpPatSeqTrigTypeVSync->setChecked(true);

            //2.Update the display mode element
            if(ui->radioButton_SLMode->isChecked()) {
                ui->radioButton_SLMode->setChecked(false);
                ui->radioButton_VariableExpSLMode->click();
            }
        }

        break;

    case 24:				//DEFAULT.PATTERNCONFIG.PAT_REPEAT
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PATTERNCONFIG.PAT_REPEAT");
        if (params[0])
            ui->radioButton_PatSeqDispRunContinuous->setChecked(true);
        else
            ui->radioButton_PatSeqDispRunOnce->setChecked(false);

        if(isVarExpTrigMode)
        {
            if (params[0])
                ui->radioButton_VarExpPatSeqDispRunContinuous->setChecked(true);
            else
                ui->radioButton_VarExpPatSeqDispRunOnce->setChecked(false);
        }
        break;

    case 25:                //DEFAULT.PATTERNCONFIG.NUM_LUT_ENTRIES
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PATTERNCONFIG.NUM_LUT_ENTRIES");
        numPatLutEntries = params[0];
        numPatLutEntries++; //+1 since 0 index based
        break;

    case 26:                //DEFAULT.PATTERNCONFIG.NUM_PATTERNS
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PATTERNCONFIG.PAT_REPEAT");
        numPatToDispForTrigOut2 = params[0];
        numPatToDispForTrigOut2++; //+1 since 0 index based
        break;

    case 27:                //DEFAULT.PATTERNCONFIG.NUM_SPLASH
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PATTERNCONFIG.NUM_SPLASH");
        numSplashLutEntries = params[0];
        numSplashLutEntries++; //+1 since 0 index based
        break;

    case 28:				//DEFAULT.SPLASHLUT
        if(numSplashLutEntries != numParams)
            ShowError("Number of Splash Lut entries not matching with actual Lut size");

        for(int i = 0; i < numParams; i++)
            splashLut[i] = params[i];

        break;

    case 29:				//DEFAULT.SEQPATLUT

        if(isVarExpTrigMode == false)
        {
            if(numPatLutEntries != numParams)
                ShowError("Number of Splash Lut entries not matching with actual Lut size");

            emit on_pushButton_PatSeqClearLUTFrmGUI_clicked();

            unsigned char patNum, bitDepth, trigger_type, maxPatNum, ledSelect;
            bool invertPat, insertBlack, bufSwap, trigOutPrev;

            for(int i = 0; i < numParams; i++)
            {
                trigger_type = params[i] & 0x3;
                InsertTriggerItem(trigger_type);

                patNum = (params[i] >> 2) & 0x3F;
                bitDepth = (params[i] >> 8) & 0xF;

                if(bitDepth < 1 || bitDepth > 8)
                {
                    sprintf(dispStr, "Invalid bit-depth in PAT LUT entry%d ", i);
                    ShowError(dispStr);
                    continue;
                }

                if(bitDepth == 1)
                    maxPatNum =24;
                else if(bitDepth == 5)
                    maxPatNum = 3;
                else if(bitDepth == 7)
                    maxPatNum = 2;
                else
                    maxPatNum = 24/bitDepth - 1;
                if(patNum > maxPatNum)
                {
                    sprintf(dispStr, "Invalid pattern-number in PAT LUT entry%d ", i);
                    ShowError(dispStr);
                    continue;
                }

                ledSelect = (params[i] >> 12) & 0x7;

                if ((params[i] & 0x00010000))
                    invertPat = true;
                else
                    invertPat = false;

                if((params[i] & 0x00020000))
                    insertBlack = true;
                else
                    insertBlack = false;

                if((params[i] & 0x00040000) || (!i))
                {
                    if (splashIndex >= numSplashLutEntries)
                    {
                        ShowError("Bad .ini! SplashLUT entries do not match the number of buffer swaps in PAT LUT");
                        continue;
                    }
                    else
                    {
                        bufSwap = true;
                        frameIndex = splashLut[splashIndex++];
                    }
                }
                else
                    bufSwap = false;

                if((params[i] & 0x00080000))
                    trigOutPrev = true;
                else
                    trigOutPrev = false;

                if(patNum == 24)
                    ui->listWidget_PatSeqLUT->addItem(GenerateItemText(frameIndex, bitDepth, 0, 0, 0));
                else
                    ui->listWidget_PatSeqLUT->addItem(GenerateItemText(frameIndex, bitDepth, patNum*bitDepth, (patNum+1)*bitDepth-1, invertPat));

                int seqListLength = ui->listWidget_PatSeqLUT->count();

                QColor bgColor = GetColorFromIndex(ledSelect);
                if(patNum == 24)
                    bgColor = Qt::black;
                ui->listWidget_PatSeqLUT->item(seqListLength-1)->setBackgroundColor(bgColor);
                UpdateSeqItemData(trigger_type, patNum, bitDepth, ledSelect, frameIndex, invertPat, insertBlack, bufSwap, trigOutPrev);
            }
        }
        else
        {
            //In variable exposure the number of LUT size is 3x 32bit word per pattern therefore +2 for
            //each pattern exposure and period
            if((numPatLutEntries*3) != numParams)
                ShowError("Number of Splash Lut entries not matching with actual Lut size");

            emit on_pushButton_VarExpPatSeqClearLUTFrmGUI_clicked();

            unsigned char patNum, bitDepth, trigger_type, maxPatNum, ledSelect;
            bool invertPat, insertBlack, bufSwap, trigOutPrev;
            unsigned int patExposure, patPeriod;

            for(int i = 0; i < numParams; i += 3)
            {
                trigger_type = params[i] & 0x3;

                VarExpInsertTriggerItem(trigger_type);

                patNum = (params[i] >> 2) & 0x3F;
                bitDepth = (params[i] >> 8) & 0xF;

                if(bitDepth < 1 || bitDepth > 8)
                {
                    sprintf(dispStr, "Invalid bit-depth in Var Exp PAT LUT entry%d ", i);
                    ShowError(dispStr);
                    continue;
                }

                if(bitDepth == 1)
                    maxPatNum =24;
                else if(bitDepth == 5)
                    maxPatNum = 3;
                else if(bitDepth == 7)
                    maxPatNum = 2;
                else
                    maxPatNum = 24/bitDepth - 1;

                if(patNum > maxPatNum)
                {
                    sprintf(dispStr, "Invalid pattern-number in Var Exp PAT LUT entry%d ", i);
                    ShowError(dispStr);
                    continue;
                }

                ledSelect = (params[i] >> 12) & 0x7;

                if ((params[i] & 0x00010000))
                    invertPat = true;
                else
                    invertPat = false;

                if((params[i] & 0x00020000))
                    insertBlack = true;
                else
                    insertBlack = false;

                if((params[i] & 0x00040000) || (!i))
                {
                    if (splashIndex >= numSplashLutEntries)
                    {
                        ShowError("Bad .ini! SplashLUT entries do not match the number of buffer swaps in PAT LUT");
                        continue;
                    }
                    else
                    {
                        bufSwap = true;
                        frameIndex = splashLut[splashIndex++];
                    }
                }
                else
                {
                    bufSwap = false;
                }

                if((params[i] & 0x00080000))
                    trigOutPrev = true;
                else
                    trigOutPrev = false;

                patExposure = params[i+1];
                patPeriod = params[i+2];

                if(patNum == 24)
                    ui->listWidget_VarExpPatSeqLUT->addItem(VarExpGenerateItemText(frameIndex, bitDepth, 0, 0, 0,0,0));
                else
                    ui->listWidget_VarExpPatSeqLUT->addItem(VarExpGenerateItemText(frameIndex, bitDepth, patNum*bitDepth, (patNum+1)*bitDepth-1, invertPat,patExposure,patPeriod));

                int seqListLength = ui->listWidget_VarExpPatSeqLUT->count();

                QColor bgColor = GetColorFromIndex(ledSelect);
                if(patNum == 24)
                    bgColor = Qt::black;

                ui->listWidget_VarExpPatSeqLUT->item(seqListLength-1)->setBackgroundColor(bgColor);
                VarExpUpdateSeqItemData(trigger_type, patNum, bitDepth, ledSelect, frameIndex, invertPat, insertBlack, bufSwap, trigOutPrev,patExposure,patPeriod);
            }
        }

        if (numSplashLutEntries > splashIndex)
        {
            m_numExtraSplashLutEntries = numSplashLutEntries - splashIndex;
            for (int i = 0; i < m_numExtraSplashLutEntries; i++)
                m_extraSplashLutEntries[i] = splashLut[splashIndex++];
        }
        break;

    case 30:                //DEFAULT.LED_ENABLE_MAN_MODE
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.LED_ENABLE_MAN_MODE");
        tempBoolVar = (params[0]) ? true : false;
        ui->radioButton_ColorDisplayAuto->setChecked(!tempBoolVar);
        ui->radioButton_ColorDisplayManual->setChecked(tempBoolVar);
        break;

    case 31:                //DEFAULT.MAN_ENABLE_RED_LED
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.MAN_ENABLE_RED_LED");
        tempBoolVar = params[0] ? true : false;
        if(ui->radioButton_ColorDisplayManual->isChecked())
            ui->checkBox_RedEnable->setChecked(tempBoolVar);
        else
            ui->checkBox_RedEnable->setChecked(false);
        break;

    case 32:                //DEFAULT.MAN_ENABLE_GRN_LED
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.MAN_ENABLE_GRN_LED");
        tempBoolVar = params[0] ? true : false;
        if(ui->radioButton_ColorDisplayManual->isChecked())
            ui->checkBox_GreenEnable->setChecked(tempBoolVar);
        else
            ui->checkBox_GreenEnable->setChecked(false);
        break;

    case 33:                //DEFAULT.MAN_ENABLE_BLU_LED
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.MAN_ENABLE_BLU_LED");
        tempBoolVar = params[0] ? true : false;
        if(ui->radioButton_ColorDisplayManual->isChecked())
            ui->checkBox_BlueEnable->setChecked(tempBoolVar);
        else
            ui->checkBox_BlueEnable->setChecked(false);
        break;

    case 34:				//DEFAULT.PORTCONFIG.PORT
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PORTCONFIG.PORT");
        ui->comboBox_InputSourceList->setCurrentIndex(params[0]);
        break;

    case 35:				//DEFAULT.PORTCONFIG.BPP
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PORTCONFIG.BPP");
        ui->comboBox_InputSourceOptionList->setCurrentIndex(params[0]);
        break;

    case 36:				//DEFAULT.PORTCONFIG.PIX_FMT
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PORTCONFIG.PIX_FMT");
        if((params[0] > 0) && ((ui->comboBox_InputSourceList->currentIndex() == 1) || (ui->comboBox_InputSourceList->currentIndex() == 3)))
        {
            ShowError("Wrong pixel format in the .ini file for the chosen port\n");
            break;
        }
        if(ui->comboBox_InputSourceList->currentIndex() == 2)
        {
            if ((params[0] == 1) || (params[0] > 2))
            {
                ShowError("Wrong pixel format in the .ini file for the chosen port\n");
                break;
            }
            if (params[0] == 2)
                params[0] = 1;
        }
        if((ui->comboBox_InputSourceList->currentIndex() == 0) && (params[0] > 2))
        {
            ShowError("Wrong pixel format in the .ini file for the chosen port\n");
            break;
        }
        ui->comboBox_PixelFormatList->setCurrentIndex(params[0]);
        break;

    case 37:				//DEFAULT.PORTCONFIG.PORT_CLK
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PORTCONFIG.PORT_CLK");
        ui->comboBox_PortClockList->setCurrentIndex(params[0]);
        break;

    case 38:				//DEFAULT.PORTCONFIG.ABC_MUX
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PORTCONFIG.ABC_MUX");
        ui->comboBox_SwapSelectList->setCurrentIndex(params[0]);
        break;

    case 39:				//DEFAULT.PORTCONFIG.PIX_MODE
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PORTCONFIG.PIX_MODE");
        ui->spinBox_FPDPixMode->setValue(params[0]);
        break;

    case 40:				//DEFAULT.PORTCONFIG.SWAP_POL
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PORTCONFIG.SWAP_POL");
        if(params[0])
            ui->checkBox_FPDInvPol->setChecked(true);
        else
            ui->checkBox_FPDInvPol->setChecked(false);
        break;

    case 41:				//DEFAULT.PORTCONFIG.FLD_SEL
        if(numParams > 1)
            ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PORTCONFIG.FLD_SEL");
        ui->comboBox_FPDFieldSelectList->setCurrentIndex(params[0]);
        break;

    default:
        break;
    }
}

void MainWindow::ApplyDefaultSolution()
{
	QString path = QApplication::applicationDirPath();	
    bool bMotionCardTrigger = System->isHardwareTrigger();
	QString fileName = path + System->getParam((bMotionCardTrigger ? QString("dlp_default_solution_file_path_motion") : QString("dlp_default_solution_file_path")) + QString("_%1").arg(m_nDLPIndex + 1)).toString();

	if (fileName.isEmpty() || !fileName.contains(".ini"))
		return;

	QFileInfo firmwareFileInfo;
	firmwareFileInfo.setFile(fileName);
	m_firmwarePath = firmwareFileInfo.absolutePath();

	QFile iniFile(fileName);
	if (!iniFile.open(QIODevice::ReadWrite | QIODevice::Text))
	{
		ShowError("Unable to open .ini file");
		return;
	}

	QTextStream in(&iniFile);

	QString firstIniToken;
	QString line;
	QByteArray byteArray;
	char cFirstIniToken[128];
	char *pCh;
	uint32 iniParams[MAX_VAR_EXP_PAT_LUT_ENTRIES * 3];//Change for variable exposure 1824x3
	int numIniParams;

#ifdef DEBUG_LOG_EN
	char tempString[256];
#endif

	while (!in.atEnd())
	{
		line = in.readLine();
		byteArray = line.toLocal8Bit();
		pCh = byteArray.data();

		if (m_dlpFrm->DLPC350_Frmw_ParseIniLines(pCh))
			continue;

		m_dlpFrm->DLPC350_Frmw_GetCurrentIniLineParam(&cFirstIniToken[0], &iniParams[0], &numIniParams);
		firstIniToken = QString(&cFirstIniToken[0]);

#ifdef DEBUG_LOG_EN
		qDebug() << "firstIniToken = " << firstIniToken << " numIniParams = " << numIniParams;
		for (int i = 0; i < numIniParams; i++) {
			sprintf(&tempString[0], "0x%X", iniParams[i]);
			qDebug() << tempString;
		}
#endif
		ApplyIniParam(firstIniToken, iniParams, numIniParams);
	}

	iniFile.close();

	//Apply GUI settings to DLPC350
	ApplyGUISettingToDLPC350();
}

bool MainWindow::isConnected()
{
	return m_bConnected;
}

bool MainWindow::isDataValiated()
{
	return m_bPatSeqValidated;
}

bool MainWindow::startUpCapture()
{
    bool bMotionCardTrigger = System->isHardwareTrigger();

	if (bMotionCardTrigger)
		ui->radioButton_PatSeqDispRunContinuous->setChecked(true);
	else
		ui->radioButton_PatSeqDispRunOnce->setChecked(true);

	emit on_pushButton_PatSeqSendLUT_clicked();

	emit on_pushButton_ValidatePatSeq_clicked();

	if (bMotionCardTrigger)
	{
		int nRet = m_dlpAPI->DLPC350_PatternDisplay(2);
		if (nRet < 0)
		{
			return false;
		}
	}	

	return isDataValiated();
}

bool MainWindow::endUpCapture()
{
	emit on_pushButton_PatSeqCtrlStop_clicked();
	return true;
}

void MainWindow::setDLP(int nIndex)
{
	m_nDLPIndex = nIndex;
}

void MainWindow::setUSBSerialNB(std::string& serialNumber)
{
	m_usbSerialNumber = serialNumber;
}

std::string MainWindow::getUSBSerialNB()
{
	return m_usbSerialNumber;
}

bool MainWindow::trigger()
{
	if (!m_dlpAPI->DLPC350_Check_Connected())
	{
		return false;
	}

	int nRet = m_dlpAPI->DLPC350_PatternDisplay(2);
	if (nRet >= 0)
	{
		return true;
	}
	return false;
}

void MainWindow::on_pushButton_ApplySolution_clicked()
{
    QString fileName;

    fileName = QFileDialog::getOpenFileName(this,
                                            QString("Select .ini file"),
                                            m_firmwarePath,
                                            tr("ini files(*.ini)"));
    if(fileName.isEmpty())
        return;

    QFileInfo firmwareFileInfo;
    firmwareFileInfo.setFile(fileName);
    m_firmwarePath = firmwareFileInfo.absolutePath();	

    QFile iniFile(fileName);
    if(!iniFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        ShowError("Unable to open .ini file");
        return;
    }

    QTextStream in(&iniFile);

    QString firstIniToken;
    QString line;
    QByteArray byteArray;
    char cFirstIniToken[128];
    char *pCh;
    uint32 iniParams[MAX_VAR_EXP_PAT_LUT_ENTRIES*3];//Change for variable exposure 1824x3
    int numIniParams;

#ifdef DEBUG_LOG_EN
    char tempString[256];
#endif

    while(!in.atEnd())
    {
        line = in.readLine();
        byteArray = line.toLocal8Bit();
        pCh = byteArray.data();

		if (m_dlpFrm->DLPC350_Frmw_ParseIniLines(pCh))
            continue;

		m_dlpFrm->DLPC350_Frmw_GetCurrentIniLineParam(&cFirstIniToken[0], &iniParams[0], &numIniParams);
        firstIniToken = QString(&cFirstIniToken[0]);

#ifdef DEBUG_LOG_EN
        qDebug() << "firstIniToken = " << firstIniToken << " numIniParams = " << numIniParams;
        for(int i=0;i<numIniParams;i++) {
            sprintf(&tempString[0],"0x%X",iniParams[i]);
            qDebug() << tempString;
        }
#endif
        ApplyIniParam(firstIniToken, iniParams, numIniParams);
    }

    iniFile.close();

    //Apply GUI settings to DLPC350
    ApplyGUISettingToDLPC350();

}

void MainWindow::on_pushButton_SaveSolution_clicked()
{
    unsigned int patLutEntry = 0;
    unsigned int num_splashLut = 0;
    unsigned int num_patLut = 0;
    int i = 0;
    bool skipPatSeqSetSaveFromGui = false;
    bool isLegacyPatSeqConfig = false;

    /*Top Level Configuration*/
	m_dlpFrm->g_iniParam_Info[DEFAULT_AUTOSTART].gui_defined_param[0] = (ui->radioButton_StandbyMode->isChecked()) ? 0x01 : 0x00;
	m_dlpFrm->g_iniParam_Info[DEFAULT_DISPMODE].gui_defined_param[0] = (ui->radioButton_SLMode->isChecked() || ui->radioButton_VariableExpSLMode->isChecked());
    if(ui->radioButton_SLMode->isChecked() || ui->radioButton_VariableExpSLMode->isChecked())
    {
		m_dlpFrm->g_iniParam_Info[DATAPATH_SPLASHATSTARTUPENABLE].gui_defined_param[0] = 0x00;
    }
    else
    {
		m_dlpFrm->g_iniParam_Info[DATAPATH_SPLASHATSTARTUPENABLE].gui_defined_param[0] = 0x01;
    }
	m_dlpFrm->g_iniParam_Info[DEFAULT_SHORT_FLIP].gui_defined_param[0] = (ui->checkBox_shortAxisFlip->isChecked()) ? 0x01 : 0x00;
	m_dlpFrm->g_iniParam_Info[DEFAULT_LONG_FLIP].gui_defined_param[0] = (ui->checkBox_longAxisFlip->isChecked()) ? 0x01 : 0x00;

    /*Trigger I/O Configuration*/
	m_dlpFrm->g_iniParam_Info[DEFAULT_TRIG_OUT_1_POL].gui_defined_param[0] = (ui->checkBox_InvertTrig1Out->isChecked()) ? 0x01 : 0x00;
	m_dlpFrm->g_iniParam_Info[DEFAULT_TRIG_OUT_1_RDELAY].gui_defined_param[0] = ui->spinBox_Trig1OutRDly->value();
	m_dlpFrm->g_iniParam_Info[DEFAULT_TRIG_OUT_1_FDELAY].gui_defined_param[0] = ui->spinBox_Trig1OutFDly->value();
	m_dlpFrm->g_iniParam_Info[DEFAULT_TRIG_OUT_2_POL].gui_defined_param[0] = (ui->checkBox_InvertTrig2Out->isChecked()) ? 0x01 : 0x00;
	m_dlpFrm->g_iniParam_Info[DEFAULT_TRIG_OUT_2_WIDTH].gui_defined_param[0] = ui->spinBox_Trig2OutRDly->value();
	m_dlpFrm->g_iniParam_Info[DEFAULT_TRIG_IN_1_DELAY].gui_defined_param[0] = ui->spinBox_TrigIn1->value();
	m_dlpFrm->g_iniParam_Info[DEFAULT_TRIG_IN_2_POL].gui_defined_param[0] = ui->comboBox_TrigIn2Pol->currentIndex();

    /*LED Configuration*/
	m_dlpFrm->g_iniParam_Info[DEFAULT_RED_STROBE_RDELAY].gui_defined_param[0] = ui->spinBox_LedDlyCtrlRedREdgeDly->value();
	m_dlpFrm->g_iniParam_Info[DEFAULT_RED_STROBE_FDELAY].gui_defined_param[0] = ui->spinBox_LedDlyCtrlRedFEdgeDly->value();
	m_dlpFrm->g_iniParam_Info[DEFAULT_GRN_STROBE_RDELAY].gui_defined_param[0] = ui->spinBox_LedDlyCtrlGreenREdgeDly->value();
	m_dlpFrm->g_iniParam_Info[DEFAULT_GRN_STROBE_FDELAY].gui_defined_param[0] = ui->spinBox_LedDlyCtrlGreenFEdgeDly->value();
	m_dlpFrm->g_iniParam_Info[DEFAULT_BLU_STROBE_RDELAY].gui_defined_param[0] = ui->spinBox_LedDlyCtrlBlueREdgeDly->value();
	m_dlpFrm->g_iniParam_Info[DEFAULT_BLU_STROBE_FDELAY].gui_defined_param[0] = ui->spinBox_LedDlyCtrlBlueFEdgeDly->value();
	m_dlpFrm->g_iniParam_Info[DEFAULT_INVERTDATA].gui_defined_param[0] = ui->checkBox_PatSeqCtrlGlobalDataInvert->isChecked();
	m_dlpFrm->g_iniParam_Info[DEFAULT_LEDCURRENT_RED].gui_defined_param[0] = 255 - strToNum(ui->lineEdit_RedLEDCurrent->text());
	m_dlpFrm->g_iniParam_Info[DEFAULT_LEDCURRENT_GRN].gui_defined_param[0] = 255 - strToNum(ui->lineEdit_GreenLEDCurrent->text());
	m_dlpFrm->g_iniParam_Info[DEFAULT_LEDCURRENT_BLU].gui_defined_param[0] = 255 - strToNum(ui->lineEdit_BlueLEDCurrent->text());

    /*Pattern Sequence LUT Configuration*/

    //Now explicitly perform a validation pattern sequence
    //to ensure that the we are saving valid pattern display
    //mode configuration from the GUI
    if(ui->radioButton_SLMode->isChecked() || ui->radioButton_VariableExpSLMode->isChecked())
        emit on_pushButton_ValidatePatSeq_clicked();

    //Okay now check which all flags are setting
    if(ui->pushButton_PatSeqValIndExpOOR->isEnabled() || \
            ui->pushButton_PatSeqValIndPatNumOOR->isEnabled())
    {
        skipPatSeqSetSaveFromGui = true;
        ShowError("The Pattern Sequence settings in the GUI has errors; Default settings will be saved in the .ini file");
    }
    else
    {
        if(ui->pushButton_PatSeqValIndTrigOutOverlap->isEnabled() || \
           ui->pushButton_PatSeqValIndBlkVecMiss->isEnabled() || \
           ui->pushButton_PatSeqValPatPeriodShort->isEnabled())
        {
            ShowError("The Pattern Sequence settings in the GUI producing warnings; continuing to save setting in the .ini file");
        }
    }
    if((skipPatSeqSetSaveFromGui == false) && ui->listWidget_PatSeqLUT->count())
    {
        isLegacyPatSeqConfig = true;

		m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_PAT_EXPOSURE].gui_defined_param[0] = ui->lineEdit_PatSeqPatExpTime->text().toUInt();
        m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_PAT_PERIOD].gui_defined_param[0] = ui->lineEdit_PatSeqPatPeriod->text().toUInt();

        if (ui->radioButton_PatSeqSrcFrmFlash->isChecked())
        {
            m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_PAT_MODE].gui_defined_param[0] = 0x03;//From Flash
        }
        else
        {
            m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_PAT_MODE].gui_defined_param[0] = 0x00;//From Video
        }

        m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_TRIG_MODE].gui_defined_param[0] = (ui->radioButton_PatSeqTrigTypeIntExt->isChecked())?0x01:0x00;
        m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_PAT_REPEAT].gui_defined_param[0] = (ui->radioButton_PatSeqDispRunContinuous->isChecked())?0x01:0x00;

        for (i = 0; i < ui->listWidget_PatSeqLUT->count(); i++)
        {
            if(!ui->listWidget_PatSeqLUT->item(i)->icon().isNull()) //Selected item is a trigger item
                continue;

            patLutEntry = 0;

            patLutEntry = ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+TrigType).toInt() & 3;
            patLutEntry |= ((ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+PatNum).toInt() & 0x3F) << 2);
            patLutEntry |= ((ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+BitDepth).toInt() & 0xF) << 8);
            patLutEntry |= ((ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+LEDSelect).toInt() & 0x7) << 12);
            if(ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+InvertPat).toBool())
            {
                patLutEntry |= BIT16;
            }

            if(ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+InsertBlack).toBool())
            {
                patLutEntry |= BIT17;
            }

            if(ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+BufSwap).toBool())
            {
                patLutEntry |= BIT18;
            }

            if(ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+trigOutPrev).toBool())
            {
                patLutEntry |= BIT19;
            }

            m_dlpFrm->g_iniParam_Info[DEFAULT_SEQPATLUT].gui_defined_param[num_patLut++] = patLutEntry;

            if((ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+BufSwap).toBool()) || (i == 1))
            {
                m_dlpFrm->g_iniParam_Info[DEFAULT_SPLASHLUT].gui_defined_param[num_splashLut++] = ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+frameIndex).toInt();
            }
        }

        m_dlpFrm->g_iniParam_Info[DEFAULT_SEQPATLUT].nr_user_defined_params = num_patLut;
        m_dlpFrm->g_iniParam_Info[DEFAULT_SPLASHLUT].nr_user_defined_params = num_splashLut;
        m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_LUT_ENTRIES].gui_defined_param[0] = (num_patLut-1);
        m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_PATTERNS].gui_defined_param[0] = (num_patLut-1);
        m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_SPLASH].gui_defined_param[0] = (num_splashLut-1);
    }
    else
    {
        m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_PAT_EXPOSURE].gui_defined_param[0] = 0;
        m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_PAT_PERIOD].gui_defined_param[0] = 0;
        m_dlpFrm->g_iniParam_Info[DEFAULT_SEQPATLUT].nr_user_defined_params = 0;
        m_dlpFrm->g_iniParam_Info[DEFAULT_SPLASHLUT].nr_user_defined_params = 0;
        m_dlpFrm->g_iniParam_Info[DEFAULT_SEQPATLUT].gui_defined_param[0] = 0;
        m_dlpFrm->g_iniParam_Info[DEFAULT_SPLASHLUT].gui_defined_param[0] = 0;
        m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_LUT_ENTRIES].gui_defined_param[0] = 0;
        m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_PATTERNS].gui_defined_param[0] = 0;
        m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_SPLASH].gui_defined_param[0] = 0;
    }

    /*Variable Exposure Pattern Sequence LUT Configuration*/
    if((skipPatSeqSetSaveFromGui == false) && (isLegacyPatSeqConfig == false))
    {
        if(ui->listWidget_VarExpPatSeqLUT->count())
        {
            num_splashLut = 0;
            num_patLut = 0;
            m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_PAT_EXPOSURE].gui_defined_param[0] = 0;
            m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_PAT_PERIOD].gui_defined_param[0] = 0;

            if (ui->radioButton_VarExpPatSeqSrcFrmFlash->isChecked())
            {
                m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_PAT_MODE].gui_defined_param[0] = 0x3;
            }
            else
            {
                m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_PAT_MODE].gui_defined_param[0] = 0x0;
            }

            if(ui->radioButton_VarExpPatSeqTrigTypeIntExt->isChecked())
            {
                m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_TRIG_MODE].gui_defined_param[0] = 0x03;//Variable Exposure Int/Ext Trigger Mode
            }
            else
            {
                m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_TRIG_MODE].gui_defined_param[0] = 0x04; //Variable Exposure VSYNC Trigger Mode
            }

            m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_PAT_REPEAT].gui_defined_param[0] = ui->radioButton_VarExpPatSeqDispRunContinuous->isChecked();

            for (i = 0; i < ui->listWidget_VarExpPatSeqLUT->count(); i++)
            {
                if(!ui->listWidget_VarExpPatSeqLUT->item(i)->icon().isNull()) //Selected item is a trigger item
                    continue;

                patLutEntry = 0;

                patLutEntry = ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+TrigType).toInt() & 3;
                patLutEntry |= ((ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+PatNum).toInt() & 0x3F) << 2);
                patLutEntry |= ((ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+BitDepth).toInt() & 0xF) << 8);
                patLutEntry |= ((ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+LEDSelect).toInt() & 0x7) << 12);
                if(ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+InvertPat).toBool())
                {
                    patLutEntry |= BIT16;
                }

                if(ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+InsertBlack).toBool())
                {
                    patLutEntry |= BIT17;
                }

                if(ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+BufSwap).toBool())
                {
                    patLutEntry |= BIT18;
                }

                if(ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+trigOutPrev).toBool())
                {
                    patLutEntry |= BIT19;
                }

                m_dlpFrm->g_iniParam_Info[DEFAULT_SEQPATLUT].gui_defined_param[num_patLut++] = patLutEntry;
                m_dlpFrm->g_iniParam_Info[DEFAULT_SEQPATLUT].gui_defined_param[num_patLut++] = ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+PatExposure).toInt();
                m_dlpFrm->g_iniParam_Info[DEFAULT_SEQPATLUT].gui_defined_param[num_patLut++] = ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+PatPeriod).toInt();

                if((ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+BufSwap).toBool()) || (i == 1))
                {
                    m_dlpFrm->g_iniParam_Info[DEFAULT_SPLASHLUT].gui_defined_param[num_splashLut++] = ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+frameIndex).toInt();
                }

            }

            m_dlpFrm->g_iniParam_Info[DEFAULT_SEQPATLUT].nr_user_defined_params = num_patLut;
            m_dlpFrm->g_iniParam_Info[DEFAULT_SPLASHLUT].nr_user_defined_params = num_splashLut;
            m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_LUT_ENTRIES].gui_defined_param[0] = (num_patLut-1);
            m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_PATTERNS].gui_defined_param[0] = (num_patLut-1);
            m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_SPLASH].gui_defined_param[0] = (num_splashLut-1);
        }
        else
        {
            m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_PAT_EXPOSURE].gui_defined_param[0] = 0;
            m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_PAT_PERIOD].gui_defined_param[0] = 0;
            m_dlpFrm->g_iniParam_Info[DEFAULT_SEQPATLUT].nr_user_defined_params = 0;
            m_dlpFrm->g_iniParam_Info[DEFAULT_SPLASHLUT].nr_user_defined_params = 0;
            m_dlpFrm->g_iniParam_Info[DEFAULT_SEQPATLUT].gui_defined_param[0] = 0;
            m_dlpFrm->g_iniParam_Info[DEFAULT_SPLASHLUT].gui_defined_param[0] = 0;
            m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_LUT_ENTRIES].gui_defined_param[0] = 0;
            m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_PATTERNS].gui_defined_param[0] = 0;
            m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_SPLASH].gui_defined_param[0] = 0;
        }
    }

    /*Single Illumination System Configuration*/
    m_dlpFrm->g_iniParam_Info[DEFAULT_LED_ENABLE_MAN_MODE].gui_defined_param[0] = ui->radioButton_ColorDisplayManual->isChecked();
    m_dlpFrm->g_iniParam_Info[DEFAULT_MAN_ENABLE_RED_LED].gui_defined_param[0] = ui->checkBox_RedEnable->isChecked();
    m_dlpFrm->g_iniParam_Info[DEFAULT_MAN_ENABLE_GRN_LED].gui_defined_param[0] = ui->checkBox_GreenEnable->isChecked();
    m_dlpFrm->g_iniParam_Info[DEFAULT_MAN_ENABLE_BLU_LED].gui_defined_param[0] = ui->checkBox_BlueEnable->isChecked();

    /*Video Input Port Configuration*/
    m_dlpFrm->g_iniParam_Info[DEFAULT_PORTCONFIG_PORT].gui_defined_param[0] = ui->comboBox_InputSourceList->currentIndex();
    m_dlpFrm->g_iniParam_Info[DEFAULT_PORTCONFIG_BPP].gui_defined_param[0] = ui->comboBox_InputSourceOptionList->currentIndex();
    if((ui->comboBox_InputSourceList->currentIndex() == 2) && (ui->comboBox_PixelFormatList->currentIndex() == 1))
    {
        m_dlpFrm->g_iniParam_Info[DEFAULT_PORTCONFIG_PIX_FMT].gui_defined_param[0] = 2;
    }
    else
    {
        m_dlpFrm->g_iniParam_Info[DEFAULT_PORTCONFIG_PIX_FMT].gui_defined_param[0] = ui->comboBox_PixelFormatList->currentIndex();
    }
    m_dlpFrm->g_iniParam_Info[DEFAULT_PORTCONFIG_PORT_CLK].gui_defined_param[0] = ui->comboBox_PortClockList->currentIndex();
    m_dlpFrm->g_iniParam_Info[DEFAULT_PORTCONFIG_ABC_MUX].gui_defined_param[0] = ui->comboBox_SwapSelectList->currentIndex();
    m_dlpFrm->g_iniParam_Info[DEFAULT_PORTCONFIG_PIX_MODE].gui_defined_param[0] =  ui->spinBox_FPDPixMode->value();
    m_dlpFrm->g_iniParam_Info[DEFAULT_PORTCONFIG_SWAP_POL].gui_defined_param[0] = ui->checkBox_FPDInvPol->isChecked();
    m_dlpFrm->g_iniParam_Info[DEFAULT_PORTCONFIG_FLD_SEL].gui_defined_param[0] = ui->comboBox_FPDFieldSelectList->currentIndex();


    /*Debug - Print GUI editable parameters*/
#ifdef DEBUG_LOG_EN
    int j = 0;
    char tempBuf[256];

    for(i=0;i<INITOKENS_MAX;i++)
    {
        if(m_dlpFrm->g_iniParam_Info[i].is_gui_editable)
        {

            if((i == DEFAULT_SEQPATLUT) || (i == DEFAULT_SPLASHLUT))
            {
                qDebug() << m_dlpFrm->g_iniParam_Info[i].token;
                for(j = 0; j< m_dlpFrm->g_iniParam_Info[i].nr_user_defined_params;j++)
                {
                    sprintf(tempBuf,"0x%X",m_dlpFrm->g_iniParam_Info[i].gui_defined_param[j]);
                    qDebug() << tempBuf;
                }
            }
            else
            {
                sprintf(tempBuf,"0x%X",m_dlpFrm->g_iniParam_Info[i].gui_defined_param[0]);
                qDebug() << m_dlpFrm->g_iniParam_Info[i].token << "\t" <<  tempBuf;
            }
        }
    }
#endif

    /*Save settings into the user provided .ini file*/
	QString path = QApplication::applicationDirPath();
	path += "/config/";

    QString fileName;

    fileName = QFileDialog::getSaveFileName(this,
                                            QString("Enter name of the .ini file"),
											path/*m_firmwarePath*/,
                                            tr("ini files(*.ini)"));

    //if(fileName.isEmpty())
    //    return;
    
    fileName = fileName.toLower();
    QDir dir(path);
    QString dirPath = dir.absolutePath().toLower();	

    if (fileName.isEmpty() || fileName.indexOf(dirPath) < 0)
	{
		return;
	}

    QFileInfo firmwareFileInfo;
    firmwareFileInfo.setFile(fileName);
    m_firmwarePath = firmwareFileInfo.absolutePath();

	QString str = fileName;
	if (!str.isEmpty())
	{
		path = QApplication::applicationDirPath();
		QDir dir(path);
        QString dirPath = dir.absolutePath().toLower();
        QString szSavePath = str.remove(dirPath);

        bool bMotionCardTrigger = System->isHardwareTrigger();
		System->setParam((bMotionCardTrigger ? QString("dlp_default_solution_file_path_motion") : QString("dlp_default_solution_file_path")) + QString("_%1").arg(m_nDLPIndex + 1), szSavePath);
	}

    QFile iniFile(fileName);
    if(!iniFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        ShowError("Unable to open .ini file");
        return;
    }

    QTextStream out(&iniFile);

    for (int i = 0; i < NR_INI_TOKENS; i++)
    {
        if ((i == DEFAULT_PATTERNCONFIG_NUM_LUT_ENTRIES) || (i == DEFAULT_PATTERNCONFIG_NUM_PATTERNS) || (i == DEFAULT_PATTERNCONFIG_NUM_SPLASH))
            continue;

        if(m_dlpFrm->g_iniParam_Info[i].is_gui_editable)
        {
            if (i == DEFAULT_SPLASHLUT)
            {
                //if the Image LUT is in the GUI then better to retain the default
                if(m_dlpFrm->g_iniParam_Info[i].nr_user_defined_params != 0)
                {
                    out << m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_SPLASH].token << " " << "0x" <<
                           m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_SPLASH].gui_defined_param[0] << " ;\n";

                    out << m_dlpFrm->g_iniParam_Info[i].token << "        ";
                    for (int j = 0; j < m_dlpFrm->g_iniParam_Info[i].nr_user_defined_params; j++)
                        out << "0x" << m_dlpFrm->g_iniParam_Info[i].gui_defined_param[j] << " ";
                }
                else
                {
                    out << m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_SPLASH].token << " " << "0x" <<
                           m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_SPLASH].default_param[0] << " ;\n";

                    out << m_dlpFrm->g_iniParam_Info[i].token << " ";
                    for (int j = 0; j < m_dlpFrm->g_iniParam_Info[i].nr_default_params; j++)
                        out << "0x" << m_dlpFrm->g_iniParam_Info[i].default_param[j] << " ";

                }
            }
            else if (i == DEFAULT_SEQPATLUT)
            {
                //If the Pat LUT in the GUI is empty better to retain the default Pat LUT
                if(m_dlpFrm->g_iniParam_Info[i].nr_user_defined_params != 0)
                {

                    if(isLegacyPatSeqConfig)
                    {
                        out << m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_LUT_ENTRIES].token << " " << "0x" <<
                               m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_LUT_ENTRIES].gui_defined_param[0] << " ;\n";

                        out << m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_PATTERNS].token << " " << "0x" <<
                               m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_PATTERNS].gui_defined_param[0] << " ;\n";
                    }
                    else
                    {
                        //For variable exposure pattern seq setting count must be /3 because it
                        out << m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_LUT_ENTRIES].token << " " << "0x" <<
                               (((m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_LUT_ENTRIES].gui_defined_param[0]+1)/3)-1)  << " ;\n";

                        out << m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_PATTERNS].token << " " << "0x" <<
                               (((m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_PATTERNS].gui_defined_param[0]+1)/3)-1) << " ;\n";
                    }

                    out << m_dlpFrm->g_iniParam_Info[i].token << " ";
                    for (int j = 0; j < m_dlpFrm->g_iniParam_Info[i].nr_user_defined_params; j++)
                        out << "0x" << m_dlpFrm->g_iniParam_Info[i].gui_defined_param[j] << " ";
                }
                else
                {
                    out << m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_LUT_ENTRIES].token << " " << "0x" <<
                           m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_LUT_ENTRIES].default_param[0] << " ;\n";

                    out << m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_PATTERNS].token << " " << "0x" <<
                           m_dlpFrm->g_iniParam_Info[DEFAULT_PATTERNCONFIG_NUM_PATTERNS].default_param[0] << " ;\n";

                    out << m_dlpFrm->g_iniParam_Info[i].token << " ";
                    for (int j = 0; j < m_dlpFrm->g_iniParam_Info[i].nr_default_params; j++)
                        out << "0x" << m_dlpFrm->g_iniParam_Info[i].default_param[j] << " ";
                }

            }
            else
            {
                //For Pattern Exposure and Pattern Period select Default or Gui define
                //based on the Pattern Lut and Image Lut settings in the GUI
                if((i == DEFAULT_PATTERNCONFIG_PAT_EXPOSURE) || \
                   (i == DEFAULT_PATTERNCONFIG_PAT_PERIOD) || \
                   (i == DEFAULT_PATTERNCONFIG_PAT_MODE) || \
                   (i == DEFAULT_PATTERNCONFIG_TRIG_MODE) || \
                   (i == DEFAULT_PATTERNCONFIG_PAT_REPEAT))
                {
                    if( (m_dlpFrm->g_iniParam_Info[DEFAULT_SEQPATLUT].nr_user_defined_params == 0) \
                            && (m_dlpFrm->g_iniParam_Info[DEFAULT_SEQPATLUT].nr_user_defined_params == 0) )
                    {
                        out << m_dlpFrm->g_iniParam_Info[i].token << " ";
                        for (int j = 0; j < m_dlpFrm->g_iniParam_Info[i].nr_default_params; j++)
                            out << "0x" << m_dlpFrm->g_iniParam_Info[i].default_param[j] << " ";
                    }
                    else
                    {
                        out << m_dlpFrm->g_iniParam_Info[i].token << " ";
                        for (int j = 0; j < m_dlpFrm->g_iniParam_Info[i].nr_user_defined_params; j++)
                            out << "0x" << m_dlpFrm->g_iniParam_Info[i].gui_defined_param[j] << " ";
                    }
                }
                else
                {
                    out << m_dlpFrm->g_iniParam_Info[i].token << " ";
                    for (int j = 0; j < m_dlpFrm->g_iniParam_Info[i].nr_user_defined_params; j++)
                        out << "0x" << m_dlpFrm->g_iniParam_Info[i].gui_defined_param[j] << " ";
                }
            }
        }
        else
        {
            out << m_dlpFrm->g_iniParam_Info[i].token << " ";
            for (int j = 0; j < m_dlpFrm->g_iniParam_Info[i].nr_default_params; j++)
                out << "0x" << hex << m_dlpFrm->g_iniParam_Info[i].default_param[j] << " ";
        }

        out << ";\n";
    }

    iniFile.close();

    return;
}

void MainWindow::on_pushButton_ApplyDefaultSolution_clicked()
{
    for (int i = 0; i < NR_INI_TOKENS; i++)
        ApplyIniParam(m_dlpFrm->g_iniParam_Info[i].token, m_dlpFrm->g_iniParam_Info[i].default_param, m_dlpFrm->g_iniParam_Info[i].nr_default_params);

    //Apply GUI settings
    ApplyGUISettingToDLPC350();
}


/* Video Port Settings */

void MainWindow::on_pushButton_SetPortSource_clicked()
{
    unsigned int index = ui->comboBox_InputSourceList->currentIndex();
    if(index == 0 || index == 3)
    {
		m_dlpAPI->DLPC350_SetInputSource(ui->comboBox_InputSourceList->currentIndex(), ui->comboBox_InputSourceOptionList->currentIndex());
    }
    else if (index == 1)
    {
		m_dlpAPI->DLPC350_SetInputSource(ui->comboBox_InputSourceList->currentIndex(), 0);
		m_dlpAPI->DLPC350_SetTPGSelect(ui->comboBox_InputSourceOptionList->currentIndex());
    }
    else
    {
        unsigned int source, portWidth;

		m_dlpAPI->DLPC350_GetInputSource(&source, &portWidth);
        if (source != 2)
			m_dlpAPI->DLPC350_SetInputSource(ui->comboBox_InputSourceList->currentIndex(), 0);
		m_dlpAPI->DLPC350_LoadImageIndex(ui->comboBox_InputSourceOptionList->currentIndex());
    }

}

void MainWindow::on_pushButton_GetPortSource_clicked()
{
    unsigned int source, portWidth, pattern, splashindex;

	if (m_dlpAPI->DLPC350_GetInputSource(&source, &portWidth) >= 0)
    {
        ui->comboBox_InputSourceList->setCurrentIndex(source);

        if(source == 0 || source == 3)
        {
            ui->comboBox_InputSourceOptionList->setCurrentIndex(portWidth);
        }
        else if(source == 1)
        {
			m_dlpAPI->DLPC350_GetTPGSelect(&pattern);
            ui->comboBox_InputSourceOptionList->setCurrentIndex(pattern);
        }
        else
        {

			m_dlpAPI->DLPC350_GetImageIndex(&splashindex);
            ui->comboBox_InputSourceOptionList->setCurrentIndex(splashindex);
        }
    }
}

void MainWindow::on_comboBox_InputSourceList_currentIndexChanged(int index)
{
    QStringList PixelFormats;
    QStringList PortWidth  = QStringList() << "30 BITS" << "24 BITS" << "20 BITS" << "16 BITS" << "10 BITS" << "8 BITS";
    QStringList TestPatterns = QStringList() << "Solid Field" << "Horizontal Ramp" << "Vertical Ramp" << "Horizontal Lines" << \
                                                "Diagonal Lines" << "Vertical Lines" << "Grid" << "Checkerboard" << "RGB Ramp" <<\
                                                "Color Bar" << "Step Bar";
    QStringList SplashIndices;
    char tmpBuf[16];

    //Dynamically create the number of image list
    SplashIndices.clear();
    for(int i = 0; i < m_numImgInFlash; i++)
    {
        sprintf(tmpBuf,"%d",i);
        SplashIndices.append(QString(tmpBuf));
    }

    ui->pushButton_SetTPGColor->setEnabled(false);
    ui->pushButton_GetTPGColor->setEnabled(false);

    ui->spinBox_TPGBackgroundColorRed->setEnabled(false);
    ui->spinBox_TPGForegroundColorRed->setEnabled(false);
    ui->spinBox_TPGBackgroundColorGreen->setEnabled(false);
    ui->spinBox_TPGForegroundColorGreen->setEnabled(false);
    ui->spinBox_TPGBackgroundColorBlue->setEnabled(false);
    ui->spinBox_TPGForegroundColorBlue->setEnabled(false);

    ui->comboBox_FPDFieldSelectList->setEnabled(false);
    ui->checkBox_FPDInvPol->setEnabled(false);
    ui->spinBox_FPDPixMode->setEnabled(false);
    ui->pushButton_GetFPDMode->setEnabled(false);
    ui->pushButton_SetFPDMode->setEnabled(false);

    ui->comboBox_SwapSelectList->setEnabled(false);
    ui->pushButton_GetPortSwap->setEnabled(false);
    ui->pushButton_SetPortSwap->setEnabled(false);

    ui->comboBox_PortClockList->setEnabled(false);
    ui->pushButton_GetPortClock->setEnabled(false);
    ui->pushButton_SetPortClock->setEnabled(false);

    ui->comboBox_InputSourceOptionList->clear();
    switch(index)
    {
    case 0:
        PixelFormats = QStringList() << "RGB 444" << "YUV 444" << "YUV 422";
        ui->comboBox_InputSourceOptionList->addItems(PortWidth);
        ui->comboBox_SwapSelectList->setEnabled(true);
        ui->pushButton_GetPortSwap->setEnabled(true);
        ui->pushButton_SetPortSwap->setEnabled(true);
        ui->comboBox_PortClockList->setEnabled(true);
        ui->pushButton_GetPortClock->setEnabled(true);
        ui->pushButton_SetPortClock->setEnabled(true);
        break;
    case 1:
        PixelFormats = QStringList() << "RGB 444" ;
        ui->comboBox_InputSourceOptionList->addItems(TestPatterns);
        ui->pushButton_SetTPGColor->setEnabled(true);
        ui->pushButton_GetTPGColor->setEnabled(true);
        ui->spinBox_TPGBackgroundColorRed->setEnabled(true);
        ui->spinBox_TPGForegroundColorRed->setEnabled(true);
        ui->spinBox_TPGBackgroundColorGreen->setEnabled(true);
        ui->spinBox_TPGForegroundColorGreen->setEnabled(true);
        ui->spinBox_TPGBackgroundColorGreen->setEnabled(true);
        ui->spinBox_TPGForegroundColorBlue->setEnabled(true);
        break;
    case 2:
        PixelFormats = QStringList() << "RGB 444" << "YUV 422";
        ui->comboBox_InputSourceOptionList->addItems(SplashIndices);
        break;
    case 3:
        PixelFormats = QStringList() << "RGB 444" ;
        ui->comboBox_InputSourceOptionList->addItems(PortWidth);
        ui->comboBox_FPDFieldSelectList->setEnabled(true);
        ui->checkBox_FPDInvPol->setEnabled(true);
        ui->spinBox_FPDPixMode->setEnabled(true);
        ui->pushButton_GetFPDMode->setEnabled(true);
        ui->pushButton_SetFPDMode->setEnabled(true);

        ui->comboBox_SwapSelectList->setEnabled(true);
        ui->pushButton_GetPortSwap->setEnabled(true);
        ui->pushButton_SetPortSwap->setEnabled(true);
        break;
    }
    ui->comboBox_PixelFormatList->clear();
    ui->comboBox_PixelFormatList->addItems(PixelFormats);
}

void MainWindow::on_pushButton_SetPortSwap_clicked()
{
    unsigned int portSelect;
    if(ui->comboBox_InputSourceList->currentIndex() == 0)
        portSelect = 0;
    else if (ui->comboBox_InputSourceList->currentIndex() == 3)
        portSelect = 1;
    else
        return;
	m_dlpAPI->DLPC350_SetDataChannelSwap(portSelect, ui->comboBox_SwapSelectList->currentIndex());
}

void MainWindow::on_pushButton_GetPortSwap_clicked()
{
    unsigned int port, swap;
	if (m_dlpAPI->DLPC350_GetDataChannelSwap(&port, &swap) == 0)
    {
        ui->comboBox_SwapSelectList->setCurrentIndex(swap);
    }
}

void MainWindow::on_pushButton_SetPortPixelFormat_clicked()
{
    unsigned int PixelFormat = ui->comboBox_PixelFormatList->currentIndex();

    //If input source selected is splash
    if(ui->comboBox_InputSourceList->currentIndex() == 2)
    {
        //If second item in the pixel format box is selected, the value to be passed is 2 for YUV444.
        if(ui->comboBox_PixelFormatList->currentIndex() == 1)
            PixelFormat = 2;
    }
	m_dlpAPI->DLPC350_SetPixelFormat(PixelFormat);
}

void MainWindow::on_pushButton_GetPortPixelFormat_clicked()
{
    unsigned int pixelFormat;

    //Get input source first so that the options for formatlist is correctly popoulated.
    emit on_pushButton_GetPortSource_clicked();

	if (m_dlpAPI->DLPC350_GetPixelFormat(&pixelFormat) == 0)
    {
        //If input source selected is splash
        if(ui->comboBox_InputSourceList->currentIndex() == 2)
        {
            //YUV422 is in index 1
            if(pixelFormat == 2)
                pixelFormat = 1;
        }
        ui->comboBox_PixelFormatList->setCurrentIndex(pixelFormat);
    }
}

void MainWindow::on_pushButton_SetPortClock_clicked()
{
	m_dlpAPI->DLPC350_SetPortClock(ui->comboBox_PortClockList->currentIndex());
}

void MainWindow::on_pushButton_GetPortClock_clicked()
{
    unsigned int portClock;
	if (m_dlpAPI->DLPC350_GetPortClock(&portClock) == 0)
        ui->comboBox_PortClockList->setCurrentIndex(portClock);
}

void MainWindow::on_pushButton_SetFPDMode_clicked()
{
	m_dlpAPI->DLPC350_SetFPD_Mode_Field(ui->spinBox_FPDPixMode->value(), \
                              ui->checkBox_FPDInvPol->isChecked(), \
                              ui->comboBox_FPDFieldSelectList->currentIndex());
}

void MainWindow::on_pushButton_GetFPDMode_clicked()
{
    unsigned int pixelMappingMode, signalSelect;
    bool pol;

	if (m_dlpAPI->DLPC350_GetFPD_Mode_Field(&pixelMappingMode, &pol, &signalSelect) == 0)
    {
        ui->spinBox_FPDPixMode->setValue(pixelMappingMode);
        ui->checkBox_FPDInvPol->setChecked(pol);
        ui->comboBox_FPDFieldSelectList->setCurrentIndex(signalSelect);
    }
}

/* Internal TPG Color */

void MainWindow::on_pushButton_SetTPGColor_clicked()
{
	m_dlpAPI->DLPC350_SetTPGColor(ui->spinBox_TPGForegroundColorRed->value(), ui->spinBox_TPGForegroundColorGreen->value(), ui->spinBox_TPGForegroundColorBlue->value(), ui->spinBox_TPGBackgroundColorRed->value(), ui->spinBox_TPGBackgroundColorGreen->value(), ui->spinBox_TPGBackgroundColorBlue->value());
}

void MainWindow::on_pushButton_GetTPGColor_clicked()
{
    unsigned short redFG, greenFG, blueFG;
    unsigned short redBG, greenBG, blueBG;

	if (m_dlpAPI->DLPC350_GetTPGColor(&redFG, &greenFG, &blueFG, &redBG, &greenBG, &blueBG) == 0)
    {
        ui->spinBox_TPGForegroundColorRed->setValue(redFG);
        ui->spinBox_TPGForegroundColorGreen->setValue(greenFG);
        ui->spinBox_TPGForegroundColorBlue->setValue(blueFG);
        ui->spinBox_TPGBackgroundColorRed->setValue(redBG);
        ui->spinBox_TPGBackgroundColorGreen->setValue(greenBG);
        ui->spinBox_TPGBackgroundColorBlue->setValue(blueBG);
    }
}

/* Display Function */

void MainWindow::on_pushButton_SetDisplayConfiguration_clicked()
{
    rectangle croppedArea, displayArea;

    croppedArea.firstPixel = strToNum(ui->lineEdit_CropAreaFirstPix->text());
    croppedArea.firstLine = strToNum(ui->lineEdit_CropAreaFirstLine->text());
    croppedArea.pixelsPerLine = strToNum(ui->lineEdit_CropAreaPixelsPerLine->text());
    croppedArea.linesPerFrame = strToNum(ui->lineEdit_CropAreaLinesPerFrame->text());

    displayArea.firstPixel = strToNum(ui->lineEdit_DispAreaFirstPixel->text());
    displayArea.firstLine = strToNum(ui->lineEdit_DispAreaFirstLine->text());
    displayArea.pixelsPerLine = strToNum(ui->lineEdit_DispAreaPixPerFrame->text());
    displayArea.linesPerFrame = strToNum(ui->lineEdit_DispAreaLinesPerFrame->text());

	m_dlpAPI->DLPC350_SetDisplay(croppedArea, displayArea);
}

void MainWindow::on_pushButton_GetDisplayConfiguration_clicked()
{
    rectangle croppedArea, displayArea;

	if (m_dlpAPI->DLPC350_GetDisplay(&croppedArea, &displayArea) == 0)
    {
        ui->lineEdit_CropAreaFirstPix->setText(numToStr(croppedArea.firstPixel));
        ui->lineEdit_CropAreaFirstLine->setText(numToStr(croppedArea.firstLine));
        ui->lineEdit_CropAreaPixelsPerLine->setText(numToStr(croppedArea.pixelsPerLine));
        ui->lineEdit_CropAreaLinesPerFrame->setText(numToStr(croppedArea.linesPerFrame));

        ui->lineEdit_DispAreaFirstPixel->setText(numToStr(displayArea.firstPixel));
        ui->lineEdit_DispAreaFirstLine->setText(numToStr(displayArea.firstLine));
        ui->lineEdit_DispAreaPixPerFrame->setText(numToStr(displayArea.pixelsPerLine));
        ui->lineEdit_DispAreaLinesPerFrame->setText(numToStr(displayArea.linesPerFrame));
    }
}

/* Read input Video Signal Information */

void MainWindow::on_pushButton_GetVideoSingalInfo_clicked()
{
    char tempStrBuf[128];
    bool isSLMode;
    VideoSigStatus vidoSignInfo;

    //Clear all the fields before updating
    ui->lineEdit_VidHRes->setText(numToStr(0));
    ui->lineEdit_VidVRes->setText(numToStr(0));
    ui->lineEdit_VidRsvd->setText(numToStr(0));
    ui->lineEdit_VidPixClk->setText(numToStr(0));
    ui->lineEdit_VidHFreq->setText(numToStr(0));
    ui->lineEdit_VidVFreq->setText(numToStr(0));
    ui->lineEdit_VidPixPerLine->setText(numToStr(0));
    ui->lineEdit_VidLinesPerFrame->setText(numToStr(0));
    ui->lineEdit_VidActvPixPerLine->setText(numToStr(0));
    ui->lineEdit_VidActvLinesPerFrame->setText(numToStr(0));
    ui->lineEdit_VidActvFirstPix->setText(numToStr(0));
    ui->lineEdit_VidActvFirstLine->setText(numToStr(0));

	if (m_dlpAPI->DLPC350_GetMode(&isSLMode) == 0)
    {
		if (m_dlpAPI->DLPC350_GetVideoSignalStatus(&vidoSignInfo) == 0)
        {
            ui->comboBox_VidLockSatus->setCurrentIndex((vidoSignInfo.Status&0x03));

            if(isSLMode == false)
            {
                ui->lineEdit_VidHRes->setText(numToStr(vidoSignInfo.HRes));
                ui->lineEdit_VidVRes->setText(numToStr(vidoSignInfo.VRes));
                ui->lineEdit_VidRsvd->setText(numToStr(vidoSignInfo.RSVD));
                ui->comboBox_VidHPol->setCurrentIndex((vidoSignInfo.HSyncPol&0x01));
                ui->comboBox_VidVPol->setCurrentIndex((vidoSignInfo.VSyncPol&0x01));

                sprintf(tempStrBuf,"%f",(float)(vidoSignInfo.PixClock/1000.0)); //Convert into MHz
                ui->lineEdit_VidPixClk->setText(QString(tempStrBuf));

                sprintf(tempStrBuf,"%f",(float)(vidoSignInfo.HFreq/100.0)); //Convert into KHz
                ui->lineEdit_VidHFreq->setText(QString(tempStrBuf));

                sprintf(tempStrBuf,"%f",(float)(vidoSignInfo.VFreq/100.0)); //Convert into Hz
                ui->lineEdit_VidVFreq->setText(QString(tempStrBuf));

                ui->lineEdit_VidPixPerLine->setText(numToStr(vidoSignInfo.TotPixPerLine));
                ui->lineEdit_VidLinesPerFrame->setText(numToStr(vidoSignInfo.TotLinPerFrame));
                ui->lineEdit_VidActvPixPerLine->setText(numToStr(vidoSignInfo.ActvPixPerLine));
                ui->lineEdit_VidActvLinesPerFrame->setText(numToStr(vidoSignInfo.ActvLinePerFrame));
                ui->lineEdit_VidActvFirstPix->setText(numToStr(vidoSignInfo.FirstActvPix));
                ui->lineEdit_VidActvFirstLine->setText(numToStr(vidoSignInfo.FirstActvLine));
            }
        }
    }
}

/* Common - Sequence Settings */

QColor MainWindow::GetColorFromIndex(int index)
{
    QColor bgColor;

    switch(index)
    {
    case 0:
        return -1;
    case 1:
        bgColor = Qt::red;
        break;
    case 2:
        bgColor = Qt::green;
        break;
    case 3:
        bgColor = Qt::yellow;
        break;
    case 4:
        bgColor = Qt::blue;
        break;
    case 5:
        bgColor = Qt::magenta;
        break;
    case 6:
        bgColor = Qt::cyan;
        break;
    case 7:
        bgColor = Qt::white;
        break;
    }
    return bgColor;
}

/* Sequence Settings */

void MainWindow::on_radioButton_PatSeqSrcFrmFlash_clicked()
{
    ui->spinBox_PatSeqFrameImgIndex->setValue(0);
    ui->comboBox_PatSeqSelTrigType->setDisabled(false);
    ui->comboBox_PatSeqSelTrigType->setCurrentIndex(IntTrig);
    ui->checkBox_PatSeqInsBlackPat->setChecked(true);

    ui->spinBox_PatSeqFrameImgIndex->setRange(0,(m_numImgInFlash-1));

    ui->radioButton_PatSeqTrigTypeIntExt->setChecked(true);
    ui->radioButton_PatSeqTrigTypeVSync->setChecked(false);
    ui->radioButton_PatSeqSrcFrmVideoPort->setChecked(false);
    this->UpdateFrameTitle(false);
    //if there are entries in the SeqLut it must be cleared
    if(ui->listWidget_PatSeqLUT->count() != 0)
        emit on_pushButton_PatSeqClearLUTFrmGUI_clicked();
}

void MainWindow::on_radioButton_PatSeqSrcFrmVideoPort_clicked()
{
    ui->spinBox_PatSeqFrameImgIndex->setValue(0);
    ui->comboBox_PatSeqSelTrigType->setDisabled(true);
    ui->comboBox_PatSeqSelTrigType->setCurrentIndex(ExtPos);
    ui->checkBox_PatSeqInsBlackPat->setChecked(false);

    ui->spinBox_PatSeqFrameImgIndex->setRange(0,63);

    ui->radioButton_PatSeqTrigTypeIntExt->setChecked(false);
    ui->radioButton_PatSeqTrigTypeVSync->setChecked(true);
    ui->radioButton_PatSeqSrcFrmFlash->setChecked(false);
    this->UpdateFrameTitle(true);
    //if there are entries in the SeqLut it must be cleared
    if(ui->listWidget_PatSeqLUT->count() != 0)
        emit on_pushButton_PatSeqClearLUTFrmGUI_clicked();
}

void MainWindow::on_radioButton_PatSeqTrigTypeIntExt_clicked()
{

    ui->spinBox_PatSeqFrameImgIndex->setValue(0);
    ui->comboBox_PatSeqSelTrigType->setDisabled(false);
    ui->comboBox_PatSeqSelTrigType->setCurrentIndex(IntTrig);
    ui->checkBox_PatSeqInsBlackPat->setChecked(true);

     ui->spinBox_PatSeqFrameImgIndex->setRange(0,(m_numImgInFlash-1));

    ui->radioButton_PatSeqSrcFrmVideoPort->setChecked(false);
    ui->radioButton_PatSeqSrcFrmFlash->setChecked(true);
    ui->radioButton_PatSeqTrigTypeVSync->setChecked(false);
    this->UpdateFrameTitle(false);
    //if there are entries in the SeqLut it must be cleared
    if(ui->listWidget_PatSeqLUT->count() != 0)
        emit on_pushButton_PatSeqClearLUTFrmGUI_clicked();
}

void MainWindow::on_radioButton_PatSeqTrigTypeVSync_clicked()
{

    ui->spinBox_PatSeqFrameImgIndex->setValue(0);
    ui->comboBox_PatSeqSelTrigType->setDisabled(true);
    ui->comboBox_PatSeqSelTrigType->setCurrentIndex(ExtPos);
    ui->checkBox_PatSeqInsBlackPat->setChecked(false);

    ui->spinBox_PatSeqFrameImgIndex->setRange(0,63);

    ui->radioButton_PatSeqSrcFrmVideoPort->setChecked(true);
    ui->radioButton_PatSeqSrcFrmFlash->setChecked(false);
    ui->radioButton_PatSeqTrigTypeIntExt->setChecked(false);
    this->UpdateFrameTitle(true);
    //if there are entries in the SeqLut it must be cleared
    if(ui->listWidget_PatSeqLUT->count() != 0)
        emit on_pushButton_PatSeqClearLUTFrmGUI_clicked();
}

void MainWindow::on_spinBox_PatSeqFrameImgIndex_valueChanged(int index)
{
    g_FrameIdx = index;
}

void MainWindow::on_comboBox_PatSeqPatBitDepthSel_currentIndexChanged(int index)
{

    int i, j;
    QColor bgColor = Qt::gray;
    int index_orig = index;

    // Adjust label_PatSeqBitPlaneNum
    if (index == 0)
    {
        ui->label_PatSeqBitPlaneNum->setText("Bit Plane");
    }
    else
    {
        ui->label_PatSeqBitPlaneNum->setText("Bit Planes");
    }

    // Adjust comboBox_PatSeqBitPlaneNum
    ui->comboBox_PatSeqBitPlaneNum->clear();
    switch(index)
    {
    case 0:     // Bit Depth = 1
        ui->comboBox_PatSeqBitPlaneNum->addItem("G0");
        ui->comboBox_PatSeqBitPlaneNum->addItem("G1");
        ui->comboBox_PatSeqBitPlaneNum->addItem("G2");
        ui->comboBox_PatSeqBitPlaneNum->addItem("G3");
        ui->comboBox_PatSeqBitPlaneNum->addItem("G4");
        ui->comboBox_PatSeqBitPlaneNum->addItem("G5");
        ui->comboBox_PatSeqBitPlaneNum->addItem("G6");
        ui->comboBox_PatSeqBitPlaneNum->addItem("G7");
        ui->comboBox_PatSeqBitPlaneNum->addItem("R0");
        ui->comboBox_PatSeqBitPlaneNum->addItem("R1");
        ui->comboBox_PatSeqBitPlaneNum->addItem("R2");
        ui->comboBox_PatSeqBitPlaneNum->addItem("R3");
        ui->comboBox_PatSeqBitPlaneNum->addItem("R4");
        ui->comboBox_PatSeqBitPlaneNum->addItem("R5");
        ui->comboBox_PatSeqBitPlaneNum->addItem("R6");
        ui->comboBox_PatSeqBitPlaneNum->addItem("R7");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B0");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B1");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B2");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B3");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B4");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B5");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B6");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B7");
        break;
    case 1:     // Bit Depth = 2
        ui->comboBox_PatSeqBitPlaneNum->addItem("G1,G0");
        ui->comboBox_PatSeqBitPlaneNum->addItem("G3,G2");
        ui->comboBox_PatSeqBitPlaneNum->addItem("G5,G4");
        ui->comboBox_PatSeqBitPlaneNum->addItem("G7,G6");
        ui->comboBox_PatSeqBitPlaneNum->addItem("R1,R0");
        ui->comboBox_PatSeqBitPlaneNum->addItem("R3,R2");
        ui->comboBox_PatSeqBitPlaneNum->addItem("R5,R4");
        ui->comboBox_PatSeqBitPlaneNum->addItem("R7,R6");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B1,B0");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B3,B2");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B5,B4");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B7,B6");
        break;
    case 2:     // Bit Depth = 3
        ui->comboBox_PatSeqBitPlaneNum->addItem("G2,G1,G0");
        ui->comboBox_PatSeqBitPlaneNum->addItem("G5,G4,G3");
        ui->comboBox_PatSeqBitPlaneNum->addItem("R0,G7,G6");
        ui->comboBox_PatSeqBitPlaneNum->addItem("R3,R2,R1");
        ui->comboBox_PatSeqBitPlaneNum->addItem("R6,R5,R4");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B1,B0,R7");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B4,B3,B2");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B7,B6,B5");
        break;
    case 3:     // Bit Depth = 4
        ui->comboBox_PatSeqBitPlaneNum->addItem("G3,G2,G1,G0");
        ui->comboBox_PatSeqBitPlaneNum->addItem("G7,G6,G5,G4");
        ui->comboBox_PatSeqBitPlaneNum->addItem("R3,R2,R1,R0");
        ui->comboBox_PatSeqBitPlaneNum->addItem("R7,R6,R5,R4");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B3,B2,B1,B0");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B7,B6,B5,B4");
        break;
    case 4:     // Bit Depth = 5
        ui->comboBox_PatSeqBitPlaneNum->addItem("G5,G4,G3,G2,G1");
        ui->comboBox_PatSeqBitPlaneNum->addItem("R3,R2,R1,R0,G7");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B1,B0,R7,R6,R5");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B7,B6,B5,B4,B3");
        break;
    case 5:     // Bit Depth = 6
        ui->comboBox_PatSeqBitPlaneNum->addItem("G5,G4,G3,G2,G1,G0");
        ui->comboBox_PatSeqBitPlaneNum->addItem("R3,R2,R1,R0,G7,G6");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B1,B0,R7,R6,R5,R4");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B7,B6,B5,B4,B3,B2");
        break;
    case 6:     // Bit Depth = 7
        ui->comboBox_PatSeqBitPlaneNum->addItem("G7,G6,G5,G4,G3,G2,G1");
        ui->comboBox_PatSeqBitPlaneNum->addItem("R7,R6,R5,R4,R3,R2,R1");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B7,B6,B5,B4,B3,B2,B1");
        break;
    case 7:     // Bit Depth = 8
        ui->comboBox_PatSeqBitPlaneNum->addItem("G7,G6,G5,G4,G3,G2,G1,G0");
        ui->comboBox_PatSeqBitPlaneNum->addItem("R7,R6,R5,R4,R3,R2,R1,R0");
        ui->comboBox_PatSeqBitPlaneNum->addItem("B7,B6,B5,B4,B3,B2,B1,B0");
        break;
    }

    if(index == 4 || index == 6)
        index +=1;

    ui->listWidget_PatSeqBitPlanes->selectionModel()->clearSelection();

    for(i=0; i<24;)
    {
        for(j=0; j<=index; j++,i++)
        {
            ui->listWidget_PatSeqBitPlanes->item(i)->setBackgroundColor(bgColor);
        }
        if(bgColor == Qt::gray)
            bgColor = Qt::lightGray;
        else
            bgColor = Qt::gray;
    }

    if(index_orig == 4)  //Special case of bit-depth 5
    {
        ui->listWidget_PatSeqBitPlanes->item(0)->setBackgroundColor(Qt::black);
        ui->listWidget_PatSeqBitPlanes->item(6)->setBackgroundColor(Qt::black);
        ui->listWidget_PatSeqBitPlanes->item(12)->setBackgroundColor(Qt::black);
        ui->listWidget_PatSeqBitPlanes->item(18)->setBackgroundColor(Qt::black);
    }

    if(index_orig == 6)  //Special case of bit-depth 7
    {
        ui->listWidget_PatSeqBitPlanes->item(0)->setBackgroundColor(Qt::black);
        ui->listWidget_PatSeqBitPlanes->item(8)->setBackgroundColor(Qt::black);
        ui->listWidget_PatSeqBitPlanes->item(16)->setBackgroundColor(Qt::black);
    }

}

void MainWindow::on_listWidget_PatSeqBitPlanes_itemClicked(QListWidgetItem *item)
{
    int selectedRow = item->listWidget()->row(item);
    int i;
    unsigned int selectionMask;

    item->listWidget()->selectionModel()->clearSelection();
    selectionMask = BitPlanes[selectedRow][ui->comboBox_PatSeqPatBitDepthSel->currentIndex()];
    for(i=0; i < 24; i++)
    {
        if(selectionMask & 1)
        {
            item->listWidget()->item(i)->setSelected(true);
        }
        selectionMask >>= 1;
    }
}

void MainWindow::UpdateFrameTitle(bool isFrame)
{
    if(isFrame)
        ui->label_PatSeqFrmOrImgIndex->setText("Frame Index");
    else
        ui->label_PatSeqFrmOrImgIndex->setText("Flash Index");
}

int MainWindow::InsertTriggerItem(int trig_type)
{
    QIcon trigIcon;

    //If not "No Trigger"
    if(trig_type != NoTrig)
    {
        ui->listWidget_PatSeqLUT->addItem("");
        if(trig_type != ExtNeg)
        {
            trigIcon.addFile(":/new/prefix1/Icons/trigger.png");
        }
        else
        {
            trigIcon.addFile(":/new/prefix1/Icons/trigger_inv.png");
        }
        ui->listWidget_PatSeqLUT->item(ui->listWidget_PatSeqLUT->count()-1)->setIcon(trigIcon);
        return 0;
    }
    return -1;
}

QString MainWindow::GenerateItemText(int frame_Index, int bitDepth, int firstItem, int lastItem, bool invert)
{
    QString itemText;

    if(ui->radioButton_PatSeqSrcFrmFlash->isChecked())
        itemText = "S";
    else
        itemText = "F";

    itemText.append(numToStr(frame_Index));
    itemText.append("::");
    itemText.append(ui->listWidget_PatSeqBitPlanes->item(firstItem)->text());
    if(lastItem != firstItem)
    {
        itemText.append("-");
        itemText.append(ui->listWidget_PatSeqBitPlanes->item(lastItem)->text());
    }
    else
        itemText.prepend("   ");

    itemText.append("\n");
    itemText.append("  (");
    itemText.append(numToStr(bitDepth));
    itemText.append(")");

    if(invert)
        itemText.append("\n    ~");

    return itemText;

}

void MainWindow::UpdateSeqItemData(int trigType, int pat_num,int bitDepth,int Led_Select,int frame_index, bool invertPat, bool insertBlack,bool bufSwap, bool TrigOutPrev)
{
    //Update the Data of the listItem with all relevant information for retrieval later.
    ui->listWidget_PatSeqLUT->item(ui->listWidget_PatSeqLUT->count()-1)->setData(Qt::UserRole+TrigType, trigType);
    ui->listWidget_PatSeqLUT->item(ui->listWidget_PatSeqLUT->count()-1)->setData(Qt::UserRole+PatNum, pat_num);
    ui->listWidget_PatSeqLUT->item(ui->listWidget_PatSeqLUT->count()-1)->setData(Qt::UserRole+BitDepth, bitDepth);
    ui->listWidget_PatSeqLUT->item(ui->listWidget_PatSeqLUT->count()-1)->setData(Qt::UserRole+LEDSelect, Led_Select);
    ui->listWidget_PatSeqLUT->item(ui->listWidget_PatSeqLUT->count()-1)->setData(Qt::UserRole+InvertPat, invertPat);
    ui->listWidget_PatSeqLUT->item(ui->listWidget_PatSeqLUT->count()-1)->setData(Qt::UserRole+InsertBlack, insertBlack);
    ui->listWidget_PatSeqLUT->item(ui->listWidget_PatSeqLUT->count()-1)->setData(Qt::UserRole+frameIndex, frame_index);
    ui->listWidget_PatSeqLUT->item(ui->listWidget_PatSeqLUT->count()-1)->setData(Qt::UserRole+BufSwap, bufSwap);
    ui->listWidget_PatSeqLUT->item(ui->listWidget_PatSeqLUT->count()-1)->setData(Qt::UserRole+trigOutPrev, TrigOutPrev);
}

void MainWindow::RemoveSelectedItemFromSeqList()
{
    int i;
    unsigned int buffer_swap=0;
    bool foundSelection = false;
    bool done = true;   //Single selection only

    for(i=0; i<ui->listWidget_PatSeqLUT->count(); i++)
    {
        while(ui->listWidget_PatSeqLUT->item(i)->isSelected())
        {
            if(ui->listWidget_PatSeqLUT->item(i)->icon().isNull()) //Selected item is not a trigger Icon
            {
                if( i > 0 && ui->listWidget_PatSeqLUT->item(i-1)->icon().isNull() == false) //if the previous item is a trigger icon; remove that first
                {
                    i--;
                    done = false;
                }
            }

            buffer_swap=0;
            //Item to be removed now is a pattern
            if(ui->listWidget_PatSeqLUT->item(i)->icon().isNull())
            {
                if(ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+BufSwap) == 1)
                    buffer_swap=1;

                //if the next item is also a pattern and if it shares the exposure time with the item being removed; then remove that also
                while((i+1) < ui->listWidget_PatSeqLUT->count()) //make sure we don't read past the array
                {
                    if(ui->listWidget_PatSeqLUT->item(i+1)->icon().isNull())
                    {
                        if(ui->listWidget_PatSeqLUT->item(i+1)->data(Qt::UserRole+trigOutPrev) == 1)
                        {
                            ui->listWidget_PatSeqLUT->takeItem(i+1);
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else //Item to be removed now is a trigger
            {
                //Set trigger type to "No Trigger" when trigger is removed.
                ui->listWidget_PatSeqLUT->item(i+1)->setData(Qt::UserRole+TrigType, NoTrig);
            }

            ui->listWidget_PatSeqLUT->takeItem(i);
            foundSelection = true;
            if(i>=ui->listWidget_PatSeqLUT->count())
            {
                ui->listWidget_PatSeqLUT->selectionModel()->clearSelection();
                return;
            }
            if(buffer_swap) //If BufSwap item was removed, the bufSwap needs to be inserted in the next item
                ui->listWidget_PatSeqLUT->item(i)->setData(Qt::UserRole+BufSwap, buffer_swap);

            if(done)
                break;

            done = true;
        }
    }
    if(!foundSelection)
        ShowError("None Selected");
    ui->listWidget_PatSeqLUT->selectionModel()->clearSelection();
}

void MainWindow::CombineItemWithPrev(unsigned int itemRow)
{
    unsigned int lastVisibleItem;

    if(itemRow == 0)
        return;

    //Return if prev item is trigger item
    if(ui->listWidget_PatSeqLUT->item(itemRow-1)->icon().isNull() == false)
        return;

    lastVisibleItem = itemRow-1;

    while(ui->listWidget_PatSeqLUT->item(lastVisibleItem)->isHidden())
        lastVisibleItem--;

    //Append color name to the end of prev item text
    if (ui->listWidget_PatSeqLUT->item(lastVisibleItem)->backgroundColor() != Qt::gray)
    {
        switch (ui->listWidget_PatSeqLUT->item(lastVisibleItem)->data(Qt::UserRole+LEDSelect).toUInt())
        {
        case 1:
            ui->listWidget_PatSeqLUT->item(lastVisibleItem)->setText(ui->listWidget_PatSeqLUT->item(itemRow-1)->text().append("Red"));;
            break;
        case 2:
            ui->listWidget_PatSeqLUT->item(lastVisibleItem)->setText(ui->listWidget_PatSeqLUT->item(itemRow-1)->text().append("Green"));;
            break;
        case 3:
            ui->listWidget_PatSeqLUT->item(lastVisibleItem)->setText(ui->listWidget_PatSeqLUT->item(itemRow-1)->text().append("Yellow"));;
            break;
        case 4:
            ui->listWidget_PatSeqLUT->item(lastVisibleItem)->setText(ui->listWidget_PatSeqLUT->item(itemRow-1)->text().append("Blue"));;
            break;
        case 5:
            ui->listWidget_PatSeqLUT->item(lastVisibleItem)->setText(ui->listWidget_PatSeqLUT->item(itemRow-1)->text().append("Magenta"));;
            break;
        case 6:
            ui->listWidget_PatSeqLUT->item(lastVisibleItem)->setText(ui->listWidget_PatSeqLUT->item(itemRow-1)->text().append("Cyan"));;
            break;
        case 7:
            ui->listWidget_PatSeqLUT->item(lastVisibleItem)->setText(ui->listWidget_PatSeqLUT->item(itemRow-1)->text().append("White"));;
            break;
        }
    }

    //Append color name to the end of selected item text
    if (ui->listWidget_PatSeqLUT->item(itemRow)->backgroundColor() != Qt::gray)
    {
        switch (ui->listWidget_PatSeqLUT->item(itemRow)->data(Qt::UserRole+LEDSelect).toUInt())
        {
        case 1:
            ui->listWidget_PatSeqLUT->item(itemRow)->setText(ui->listWidget_PatSeqLUT->item(itemRow)->text().append("Red"));;
            break;
        case 2:
            ui->listWidget_PatSeqLUT->item(itemRow)->setText(ui->listWidget_PatSeqLUT->item(itemRow)->text().append("Green"));;
            break;
        case 3:
            ui->listWidget_PatSeqLUT->item(itemRow)->setText(ui->listWidget_PatSeqLUT->item(itemRow)->text().append("Yellow"));;
            break;
        case 4:
            ui->listWidget_PatSeqLUT->item(itemRow)->setText(ui->listWidget_PatSeqLUT->item(itemRow)->text().append("Blue"));;
            break;
        case 5:
            ui->listWidget_PatSeqLUT->item(itemRow)->setText(ui->listWidget_PatSeqLUT->item(itemRow)->text().append("Magenta"));;
            break;
        case 6:
            ui->listWidget_PatSeqLUT->item(itemRow)->setText(ui->listWidget_PatSeqLUT->item(itemRow)->text().append("Cyan"));;
            break;
        case 7:
            ui->listWidget_PatSeqLUT->item(itemRow)->setText(ui->listWidget_PatSeqLUT->item(itemRow)->text().append("White"));;
            break;
        }
    }

    ui->listWidget_PatSeqLUT->item(lastVisibleItem)->setText(ui->listWidget_PatSeqLUT->item(lastVisibleItem)->text().append("\n"));
    ui->listWidget_PatSeqLUT->item(lastVisibleItem)->setText(ui->listWidget_PatSeqLUT->item(lastVisibleItem)->text().append(ui->listWidget_PatSeqLUT->item(itemRow)->text()));
    ui->listWidget_PatSeqLUT->item(itemRow)->setData(Qt::UserRole+trigOutPrev, 1);
    ui->listWidget_PatSeqLUT->item(lastVisibleItem)->setBackgroundColor(Qt::gray);
    ui->listWidget_PatSeqLUT->setRowHidden(itemRow, true);
    ui->listWidget_PatSeqLUT->selectionModel()->clearSelection();

    return;
}

void MainWindow::on_pushButton_PatSeqAddPatToLut_clicked()
{
    int i;
    int firstItem=25, lastItem;
    unsigned int pat_num=0;
    unsigned int prevFrameIdx=0;
    QColor bgColor;
    QPoint triggerPos;
    bool bufSwap;
    QListWidgetItem *curItem;
    unsigned int seqListLength;
    int trigger_type;

    if(ui->comboBox_PatSeqSelPatColor->currentIndex()==0)
    {
        ShowError("Select a Color");
        return;
    }

    if(ui->listWidget_PatSeqBitPlanes->selectedItems().count() != (ui->comboBox_PatSeqPatBitDepthSel->currentIndex()+1))
    {
        ShowError("Number of bits selected must equal Bit Depth");
        return;
    }

    for(i=0; i<24; i++)
    {
        if(ui->listWidget_PatSeqBitPlanes->item(i)->isSelected())
        {
            if(firstItem == 25)
                firstItem = i;
            lastItem = i;
        }
    }

    //If first item
    if(ui->listWidget_PatSeqLUT->count() == 0)
    {
        if(ui->radioButton_PatSeqSrcFrmFlash->isChecked())
        {
            //Trigger type can't be "No Trigger"
            if(ui->comboBox_PatSeqSelTrigType->currentIndex() == NoTrig)
            {
                ShowError("First Item must be triggered. Please select a Trigger_In_Type other than No Trigger");
                return;
            }
            bufSwap = true;//false;
        }
        else
        {
            //In streaming mode, first item has to be triggered by vsync
            bufSwap = true;
        }
    }
    else
    {
        prevFrameIdx = ui->listWidget_PatSeqLUT->item(ui->listWidget_PatSeqLUT->count()-1)->data(Qt::UserRole+frameIndex).toUInt();
        if(prevFrameIdx != g_FrameIdx)
        {
            bufSwap = true;
        }
        else
        {
            bufSwap = false;
        }
    }

    if(ui->radioButton_PatSeqSrcFrmFlash->isChecked())
    {
        trigger_type = ui->comboBox_PatSeqSelTrigType->currentIndex();
    }
    else
    {
        if(bufSwap == true)
            trigger_type = ExtPos;
        else
            trigger_type = NoTrig;
    }

    InsertTriggerItem(trigger_type);

    ui->listWidget_PatSeqLUT->addItem(GenerateItemText(g_FrameIdx, ui->comboBox_PatSeqPatBitDepthSel->currentIndex()+1, firstItem, lastItem, ui->checkBox_PatSeqInvPatData->isChecked()));

    seqListLength = ui->listWidget_PatSeqLUT->count();
    curItem = ui->listWidget_PatSeqLUT->item(seqListLength-1);

    bgColor = GetColorFromIndex(ui->comboBox_PatSeqSelPatColor->currentIndex());
    curItem->setBackgroundColor(bgColor);

    //Update the Data of the listItem with all relevant information for retrieval later.
    pat_num = firstItem/(ui->comboBox_PatSeqPatBitDepthSel->currentIndex()+1);
    UpdateSeqItemData(trigger_type, pat_num, ui->comboBox_PatSeqPatBitDepthSel->currentIndex()+1, ui->comboBox_PatSeqSelPatColor->currentIndex(),g_FrameIdx, ui->checkBox_PatSeqInvPatData->isChecked(), ui->checkBox_PatSeqInsBlackPat->isChecked(), bufSwap, 0);
    ui->listWidget_PatSeqLUT->scrollToItem(ui->listWidget_PatSeqLUT->item(seqListLength-1));
}

void MainWindow::on_pushButton_PatSeqSendLUT_clicked()
{
    int i, numLutEntries=0;
    //unsigned int status;
    char errorStr[256];
    unsigned char splashLut[64];
    int numSplashLutEntries = 0;
    int num_pats_in_exposure=1;
    unsigned int min_pat_exposure[8] = {235, 700, 1570, 1700, 2000, 2500, 4500, 8333};
    unsigned int worstCaseBitDepth = 0;
    unsigned int numPatterns;
    int trigMode = 0;

    if(ui->listWidget_PatSeqLUT->count() == 0)
        return;

    //Make sure the Pattern Exposure and Pattern Period timings are within the spec

    //Pattern Exposure > Pattern Period not a valid settings
    if((ui->lineEdit_PatSeqPatExpTime->text().toInt() > ui->lineEdit_PatSeqPatPeriod->text().toInt()))
    {
        ShowError("Pattern exposure setting voilation, it should be, Pattern Exposure = Pattern Period or (Pattern Period - Pattern Exposure) > 230us");
        return;
    }

    //If Pattern Exposure != Pattern Period then (Pattern Period - Pattern Exposure) > 230us
    if( ((ui->lineEdit_PatSeqPatExpTime->text().toInt() != ui->lineEdit_PatSeqPatPeriod->text().toInt())) &&
        ((ui->lineEdit_PatSeqPatPeriod->text().toInt() - (ui->lineEdit_PatSeqPatExpTime->text().toInt())) <= 230))
    {
        ShowError("Pattern exposure setting voilation, it should be, Pattern Exposure = Pattern Period or (Pattern Period - Pattern Exposure) > 230us");
        return;
    }

	m_dlpAPI->DLPC350_ClearPatLut();
    for(i=0; i<ui->listWidget_PatSeqLUT->count(); i++)
    {
        if(ui->listWidget_PatSeqLUT->item(i)->icon().isNull()) //only if pattern item (skip trigger items)
        {
            if(i==0)
            {
                ShowError("First Item must be triggered. Please select a Trigger_In_Type other than No Trigger");
                return;
            }

            //If trigOut = NEW
            if(ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+trigOutPrev).toBool() == false)
            {
                if(num_pats_in_exposure != 1)
                {
                    //Check if expsoure time is above the minimum requirement
                    if(ui->lineEdit_PatSeqPatExpTime->text().toUInt()/num_pats_in_exposure < min_pat_exposure[worstCaseBitDepth])
                    {
                        sprintf(errorStr, "Exposure time %d < Minimum Exposure time %d for bit depth %d", ui->lineEdit_PatSeqPatExpTime->text().toUInt()/num_pats_in_exposure, min_pat_exposure[worstCaseBitDepth], worstCaseBitDepth+1);
                        ShowError(errorStr);
                        return;
                    }
                }
                if(ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+BitDepth).toUInt()-1 > worstCaseBitDepth)
                {
                    worstCaseBitDepth = ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+BitDepth).toInt()-1;
                }
                num_pats_in_exposure=1;
                worstCaseBitDepth = 0;
            }
            else //if trigOut = PREV
            {
                num_pats_in_exposure++;
                if(ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+BitDepth).toUInt()-1 > worstCaseBitDepth)
                    worstCaseBitDepth = ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+BitDepth).toInt()-1;
            }

#ifdef DEBUG_LOG_EN
            qDebug() << "[" << i << "] " << "TrigType = " << ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+TrigType).toInt() << "," \
                     << "PatNum = " << ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+PatNum).toInt() << "," \
                     << "BitDepth = " << ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+BitDepth).toInt() << "," \
                     << "LEDSelect = " << ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+LEDSelect).toInt() << "," \
                     << "InvertPattern = " << ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+InvertPat).toBool() << "," \
                     << "Insert Black = " << ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+InsertBlack).toBool() << "," \
                     << "BufSwap = " << ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+BufSwap).toBool() << "," \
                     << "TrigOutPrev = " << ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+trigOutPrev).toBool();
#endif
			if (m_dlpAPI->DLPC350_AddToPatLut(ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole + TrigType).toInt(), \
                                   ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+PatNum).toInt(), \
                                   ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+BitDepth).toInt(), \
                                   ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+LEDSelect).toInt(), \
                                   ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+InvertPat).toBool(), \
                                   ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+InsertBlack).toBool(), \
                                   ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+BufSwap).toBool(), \
                                   ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+trigOutPrev).toBool()) < 0)
            {
                ShowError("Error Updating LUT");
                return;
            }
            //If there is a buffer swap or if this is the first pattern
            if((ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+BufSwap).toBool()) || (numSplashLutEntries == 0))
            {
                if (numSplashLutEntries >= 64)
                    ShowError("Image LUT entries(64) reached maximum. Will not add anymore entries\n");
                else
                    splashLut[numSplashLutEntries++] = ui->listWidget_PatSeqLUT->item(i)->data(Qt::UserRole+frameIndex).toInt();
            }
            numLutEntries++;
        }
    }

    if (m_numExtraSplashLutEntries)
    {
        for (i = 0; i < m_numExtraSplashLutEntries; i++)
        {
            if (numSplashLutEntries >= 64)
            {
                ShowError("Image LUT entries(64) reached maximum. Will not add anymore entries\n");
                break;
            }
            splashLut[numSplashLutEntries++] = m_extraSplashLutEntries[i];
        }
    }

    if(num_pats_in_exposure != 1)
    {
        //Check if expsoure time is above the minimum requirement
        if(ui->lineEdit_PatSeqPatExpTime->text().toUInt()/num_pats_in_exposure < min_pat_exposure[worstCaseBitDepth])
        {
            sprintf(errorStr, "Exposure time %d < Minimum Exposure time %d for bit depth %d", ui->lineEdit_PatSeqPatExpTime->text().toUInt()/num_pats_in_exposure, min_pat_exposure[worstCaseBitDepth], worstCaseBitDepth+1);
            ShowError(errorStr);
            return;
        }
    }

	m_dlpAPI->DLPC350_SetPatternDisplayMode(ui->radioButton_PatSeqSrcFrmVideoPort->isChecked());

    //if play once is selected
    if(ui->radioButton_PatSeqDispRunOnce->isChecked())
    {
        numPatterns = numLutEntries;
    }
    else
    {
        numPatterns = ui->spinBox_TrigOut2PulsePerNumPat->value();
    }

	if (m_dlpAPI->DLPC350_SetPatternConfig(numLutEntries, ui->radioButton_PatSeqDispRunContinuous->isChecked(), numPatterns, numSplashLutEntries) < 0)
    {
        ShowError("Error Sending Pattern Config");
        return;
    }
	if (m_dlpAPI->DLPC350_SetExposure_FramePeriod(ui->lineEdit_PatSeqPatExpTime->text().toInt(), ui->lineEdit_PatSeqPatPeriod->text().toInt()) < 0)
    {
        ShowError("Error Sending Exposure period");
        return;
    }

    if(ui->radioButton_PatSeqTrigTypeIntExt->isChecked() == true)
    {
        trigMode = 1; //Internal trigger
    }
    else
    {
        trigMode = 0; //VSync trigger
    }

    //Configure Trigger Mode - 0 or 1
	if (m_dlpAPI->DLPC350_SetPatternTriggerMode(trigMode) < 0)
    {
        ShowError("Error Sending trigger Mode");
        return;
    }

    //Send Pattern LUT
	if (m_dlpAPI->DLPC350_SendPatLut() < 0)
    {
        ShowError("Error Sending Pattern LUT");
        return;
    }

	if (m_dlpAPI->DLPC350_SendImageLut(&splashLut[0], numSplashLutEntries) < 0)
    {
        ShowError("Error Sending Image LUT");
        return;
    }

    ui->pushButton_PatSeqValIndExpOOR->setEnabled(false);
    ui->pushButton_PatSeqValIndPatNumOOR->setEnabled(false);
    ui->pushButton_PatSeqValIndTrigOutOverlap->setEnabled(false);
    ui->pushButton_PatSeqValIndBlkVecMiss->setEnabled(false);
    ui->pushButton_PatSeqValPatPeriodShort->setEnabled(false);

    ui->pushButton_PatSeqValStatus->setEnabled(false);
    ui->pushButton_PatSeqCtrlStart->setEnabled(false);
    ui->pushButton_PatSeqCtrlPause->setEnabled(false);
    ui->pushButton_PatSeqCtrlStop->setEnabled(false);

    // Select the pattern stop start pause tab
    ui->tabWidget_2->setCurrentIndex(2);
}

void MainWindow::on_pushButton_PatSeqReadLUTFrmHW_clicked()
{
    unsigned int exposure, framePeriod, i, j;
    int trigMode;
    unsigned int numLutEntries, numPatsForTrigOut2, numSplash;
    bool repeat;
    int trig_type, Pat_Num, Bit_Depth, LED_Select, Frame_Index;
    bool Invert_Pat, Insert_Black, Buf_Swap, TrigOutPrev;
    //QString itemText = "F";
    unsigned int firstItem=32, lastItem, index;
    QColor bgColor;
    unsigned char splashLut[64];
    bool isExtPatDisplayMode=false;
    int patLutBytesRead, numLUTEntriesRead;
    char dbgMsg[256];

	if (m_dlpAPI->DLPC350_GetPatternTriggerMode(&trigMode) == 0)
    {
        //Only if the trigger mode is 0 or 1 or 2 refresh GUI
        if(trigMode <= 2)
        {
			if (m_dlpAPI->DLPC350_GetExposure_FramePeriod(&exposure, &framePeriod) == 0)
            {
                ui->lineEdit_PatSeqPatExpTime->setText(numToStr(exposure));
                ui->lineEdit_PatSeqPatPeriod->setText(numToStr(framePeriod));
            }
            else
            {
                ShowError("Could not read from target. Please check the connection");
                return;
            }

#ifdef DEBUG_LOG_EN
            qDebug() << "Pattern exposure = " << exposure << " Pattern Period = " << framePeriod;
#endif
			if (m_dlpAPI->DLPC350_GetPatternDisplayMode(&isExtPatDisplayMode) == 0)
            {
                if (isExtPatDisplayMode)
                {
                    ui->radioButton_PatSeqSrcFrmVideoPort->setChecked(true);
                    emit on_radioButton_PatSeqSrcFrmVideoPort_clicked();
                }
                else
                {
                    ui->radioButton_PatSeqSrcFrmFlash->setChecked(true);
                    emit on_radioButton_PatSeqSrcFrmFlash_clicked();
                }
            }

			if (m_dlpAPI->DLPC350_GetPatternConfig(&numLutEntries, &repeat, &numPatsForTrigOut2, &numSplash) == 0)
            {
                if(repeat == true)
                {
                    ui->radioButton_PatSeqDispRunContinuous->setChecked(true);
                    ui->spinBox_TrigOut2PulsePerNumPat->setValue(numPatsForTrigOut2);
                }
                else
                {
                    ui->radioButton_PatSeqDispRunOnce->setChecked(true);
                }
            }

#ifdef DEBUG_LOG_EN
            qDebug() << "numLutEntries = " << numLutEntries << "repeat = " << repeat << "numPatsForTrigOut2 = " << numPatsForTrigOut2 << "numSplash = " << numSplash;
#endif
			if ((patLutBytesRead = m_dlpAPI->DLPC350_GetPatLut(numLutEntries)) < 0)
            {
                ShowError("Could not read pattern LUT from target");
                return;
            }

            if(patLutBytesRead % 3 == 0)
                numLUTEntriesRead = patLutBytesRead/3+1;
            else
                numLUTEntriesRead = patLutBytesRead/3;

            if (numLUTEntriesRead != numLutEntries)
            {
                sprintf(dbgMsg, "Only %d pattern LUT entries read back correctly. This issue will be fixed in the next release of firmware", numLUTEntriesRead);
                numLutEntries = numLUTEntriesRead;
                ShowError(dbgMsg);
            }

			if (m_dlpAPI->DLPC350_GetImageLut(&splashLut[0], numSplash) < 0)
            {
                ShowError("Could not read image LUT from target");
                return;
            }
            else
            {
                // Values read correctly so check for special 2 numSplash case and adjust so GUI displays correctly
                if(numSplash == 2)
                {
                    unsigned char temp_val = splashLut[0];
                    splashLut[0] = splashLut[1];
                    splashLut[1] = temp_val;
                }
#ifdef DEBUG_LOG_EN
                //Readout the ImageLut table
                for(unsigned int i = 0; i< numSplash; i++)
                    qDebug() << "ImageLut[" << i << "] " << splashLut[i] ;
#endif
            }

            emit on_pushButton_PatSeqClearLUTFrmGUI_clicked();

            Frame_Index = -1;

            for(i=0; i<numLutEntries; i++)
            {
				m_dlpAPI->DLPC350_GetPatLutItem(i, &trig_type, &Pat_Num, &Bit_Depth, &LED_Select, &Invert_Pat, &Insert_Black, &Buf_Swap, &TrigOutPrev);
#ifdef DEBUG_LOG_EN
                qDebug() << "Pat_Num[" << i << "] " << "TrigType = " << trig_type << "," << "PatIndex = " << Pat_Num << "," \
                         << "Bit-Depth = " << Bit_Depth << "," << "LED_Color = " << LED_Select << "Invert = " << Invert_Pat << "," \
                         << "Insert_Black = " << Insert_Black << "," << "Buffer-Swap = " << Buf_Swap << "," << "TrigOutPrev = " << TrigOutPrev;
#endif
                if(Bit_Depth <= 0 || Bit_Depth > 8)
                {
                    ShowError("Received unexpected value for Bit depth");
                    break;
                }

                InsertTriggerItem(trig_type);
                if((Buf_Swap) || (i==0))
                    Frame_Index++;

                firstItem = 32;
                if(Bit_Depth == 5 || Bit_Depth == 7)
                    index = Pat_Num*(Bit_Depth+1);
                else
                    index = Pat_Num * Bit_Depth;

                for(j=0; j < 32; j++)
                {
                    if(firstItem > 31) //first set bit not found yet
                    {
                        if( (BitPlanes[index][Bit_Depth-1] & (1 << j)) == (1<<j))
                        {
                            firstItem = j;
                        }
                    }
                    else if((BitPlanes[index][Bit_Depth-1] & (1 << j)) == 0)
                    {
                        break;
                    }
                }

                lastItem = j-1;

                if(isExtPatDisplayMode)
                    ui->listWidget_PatSeqLUT->addItem(GenerateItemText(Frame_Index, Bit_Depth, firstItem, lastItem, Invert_Pat));
                else
                    ui->listWidget_PatSeqLUT->addItem(GenerateItemText(splashLut[Frame_Index], Bit_Depth, firstItem, lastItem, Invert_Pat));

                if(LED_Select <= 0 || LED_Select > 7)
                {
                    ShowError("Received unexpected value for Color selection");
                    break;
                }
                bgColor = GetColorFromIndex(LED_Select);
                ui->listWidget_PatSeqLUT->item(ui->listWidget_PatSeqLUT->count()-1)->setBackgroundColor(bgColor);

                if(isExtPatDisplayMode)
                    UpdateSeqItemData(trig_type, Pat_Num, Bit_Depth, LED_Select, Frame_Index, Invert_Pat, Insert_Black, Buf_Swap, TrigOutPrev);
                else
                    UpdateSeqItemData(trig_type, Pat_Num, Bit_Depth, LED_Select, splashLut[Frame_Index], Invert_Pat, Insert_Black, Buf_Swap, TrigOutPrev);

                if(ui->listWidget_PatSeqLUT->count() > 1)
                {
                    if(ui->listWidget_PatSeqLUT->item(ui->listWidget_PatSeqLUT->count()-1)->data(Qt::UserRole+trigOutPrev).toBool() == true)
                    {
                        CombineItemWithPrev(ui->listWidget_PatSeqLUT->count()-1);
                    }
                }
            }
        }
        else
        {
            ShowError("System is configured in Variable Exposure Pattern Sequence Mode");
        }
    }
}

void MainWindow::on_listWidget_PatSeqLUT_customContextMenuRequested(const QPoint &pos)
{
    int itemRow=0;
    QMenu seqListMenu(this);

    QAction *removeAct;
    QAction *invertAct;
    QAction *insertAct = new QAction(tr("Insert Trigger-In"), this);
    QAction *combineAct = new QAction(tr("Share exposure time with prev pattern"), this);
    QAction *toggleInsBlkAct = new QAction(tr("Clear DMD after exposure time"), this);

    toggleInsBlkAct->setCheckable(true);

    itemRow = ui->listWidget_PatSeqLUT->row((ui->listWidget_PatSeqLUT->itemAt(pos)));
    if(itemRow < 0 || itemRow > 127)
        return;

    if(!ui->listWidget_PatSeqLUT->item(itemRow)->icon().isNull())
    {
        invertAct = new QAction(tr("Invert Trigger-In"), this);
        removeAct = new QAction("Remove Trigger-In", this);
    }
    else
    {
        invertAct = new QAction(tr("Invert Pattern"), this);
        removeAct = new QAction("Remove Pattern", this);
    }

    //For trigger Items
    if(!ui->listWidget_PatSeqLUT->item(itemRow)->icon().isNull())
    {
        //Invert option not applicable for Internal trigger. Also, for straming mode, external trigger can only be positive.
        if(ui->listWidget_PatSeqLUT->item(itemRow+1)->data(Qt::UserRole+TrigType).toInt() != IntTrig && ui->radioButton_PatSeqSrcFrmFlash->isChecked() == 1)
            seqListMenu.addAction(invertAct);
    }
    else
    {
        if(itemRow < ui->listWidget_PatSeqLUT->count()-1)//if not last item
        {
            //check if next items are combined (hidden/trigOutPrev=1) with this one. If yes, don't give the option to invert this item
            if(!ui->listWidget_PatSeqLUT->item(itemRow+1)->isHidden())
            {
                seqListMenu.addAction(invertAct);
            }

        }
        else
            seqListMenu.addAction(invertAct);

        //If not first item and previous item is also a pattern item
        if(itemRow != 0 && ui->listWidget_PatSeqLUT->item(itemRow-1)->icon().isNull())
        {
            seqListMenu.addAction(combineAct);
        }
        seqListMenu.addAction(toggleInsBlkAct);
        if(ui->listWidget_PatSeqLUT->item(itemRow)->data(Qt::UserRole+InsertBlack).toInt() != 0)
            toggleInsBlkAct->setChecked(true);
        else
            toggleInsBlkAct->setChecked(false);
    }

    if(!ui->listWidget_PatSeqLUT->item(itemRow)->icon().isNull() && itemRow == 0)
    {
        /* First trigger item cannot be removed */
    }
    else
    {
        seqListMenu.addAction(removeAct);
    }

    if(ui->radioButton_PatSeqSrcFrmFlash->isChecked())
    {
        //If pattern item and trigger type is "No Trigger"
        if(ui->listWidget_PatSeqLUT->item(itemRow)->icon().isNull() && ui->listWidget_PatSeqLUT->item(itemRow)->data(Qt::UserRole+TrigType).toInt() == NoTrig)
        {
            seqListMenu.addAction(insertAct);
        }
    }

    QAction* selectedAct = seqListMenu.exec(QCursor::pos());

    if(selectedAct == removeAct)
    {
        RemoveSelectedItemFromSeqList();
    }
    else if(selectedAct == invertAct)
    {
        if(!ui->listWidget_PatSeqLUT->item(itemRow)->icon().isNull()) //Invert Trigger-In
        {
            if(ui->listWidget_PatSeqLUT->item(itemRow+1)->data(Qt::UserRole+TrigType).toInt() == ExtPos)
            {
                ui->listWidget_PatSeqLUT->item(itemRow+1)->setData(Qt::UserRole+TrigType, ExtNeg);
                QIcon trigInvIcon (":/new/prefix1/Icons/trigger_inv.png");
                ui->listWidget_PatSeqLUT->item(itemRow)->setIcon(trigInvIcon);
            }
            else if(ui->listWidget_PatSeqLUT->item(itemRow+1)->data(Qt::UserRole+TrigType).toInt() == ExtNeg)
            {
                ui->listWidget_PatSeqLUT->item(itemRow+1)->setData(Qt::UserRole+TrigType, ExtPos);
                QIcon trigInvIcon (":/new/prefix1/Icons/trigger.png");
                ui->listWidget_PatSeqLUT->item(itemRow)->setIcon(trigInvIcon);
            }
        }
        else //Invert Pattern
        {
            if(ui->listWidget_PatSeqLUT->item(itemRow)->text().contains("~"))
            {
                QString itemText = ui->listWidget_PatSeqLUT->item(itemRow)->text();
                itemText.chop(6);
                ui->listWidget_PatSeqLUT->item(itemRow)->setText(itemText);
                ui->listWidget_PatSeqLUT->item(itemRow)->setData(Qt::UserRole+InvertPat, 0);
            }
            else
            {
                ui->listWidget_PatSeqLUT->item(itemRow)->setText(ui->listWidget_PatSeqLUT->item(itemRow)->text().append("\n    ~"));
                ui->listWidget_PatSeqLUT->item(itemRow)->setData(Qt::UserRole+InvertPat, 1);
            }
        }
    }
    else if(selectedAct == insertAct)
    {
        if(ui->comboBox_PatSeqSelTrigType->currentIndex() == ExtNeg)
        {
            ui->listWidget_PatSeqLUT->item(itemRow)->setData(Qt::UserRole+TrigType, ExtNeg);
            ui->listWidget_PatSeqLUT->insertItem(itemRow, "");
            QIcon trigIcon (":/new/prefix1/Icons/trigger_inv.png");
            ui->listWidget_PatSeqLUT->item(itemRow)->setIcon(trigIcon);
        }
        else
        {
            if(ui->comboBox_PatSeqSelTrigType->currentIndex() == ExtPos)
                ui->listWidget_PatSeqLUT->item(itemRow)->setData(Qt::UserRole+TrigType, ExtPos);
            else if(ui->comboBox_PatSeqSelTrigType->currentIndex() == IntTrig)
                ui->listWidget_PatSeqLUT->item(itemRow)->setData(Qt::UserRole+TrigType, IntTrig);
            else
            {
                ShowError("Please select trigger type to be inserted using Trigger_In_Type combobox");
                return;
            }
            ui->listWidget_PatSeqLUT->insertItem(itemRow, "");
            QIcon trigIcon (":/new/prefix1/Icons/trigger.png");
            ui->listWidget_PatSeqLUT->item(itemRow)->setIcon(trigIcon);
        }
    }
    else if(selectedAct == combineAct)
    {
        CombineItemWithPrev(itemRow);
    }
    else if(selectedAct == toggleInsBlkAct)
    {
        if(toggleInsBlkAct->isChecked())
            ui->listWidget_PatSeqLUT->item(itemRow)->setData(Qt::UserRole+InsertBlack, 1);
        else
            ui->listWidget_PatSeqLUT->item(itemRow)->setData(Qt::UserRole+InsertBlack, 0);
    }
}

void MainWindow::on_pushButton_PatSeqClearLUTFrmGUI_clicked()
{
    m_numExtraSplashLutEntries = 0;

    while(ui->listWidget_PatSeqLUT->count() != 0)
        ui->listWidget_PatSeqLUT->takeItem(0);
    ui->spinBox_PatSeqFrameImgIndex->setValue(0);

    ui->pushButton_PatSeqValIndExpOOR->setEnabled(false);
    ui->pushButton_PatSeqValIndPatNumOOR->setEnabled(false);
    ui->pushButton_PatSeqValIndTrigOutOverlap->setEnabled(false);
    ui->pushButton_PatSeqValIndBlkVecMiss->setEnabled(false);
    ui->pushButton_PatSeqValPatPeriodShort->setEnabled(false);

    ui->pushButton_PatSeqValStatus->setEnabled(false);
    ui->pushButton_PatSeqCtrlStart->setEnabled(false);
    ui->pushButton_PatSeqCtrlPause->setEnabled(false);
    ui->pushButton_PatSeqCtrlStop->setEnabled(false);

}

/* Variable Exposure Sequence Settings */

void MainWindow::on_radioButton_VarExpPatSeqSrcFrmFlash_clicked()
{
    ui->spinBox_VarExpPatSeqFrameImgIndex->setValue(0);
    ui->comboBox_VarExpPatSeqSelTrigType->setDisabled(false);
    ui->comboBox_VarExpPatSeqSelTrigType->setCurrentIndex(IntTrig);
    ui->checkBox_VarExpPatSeqInsBlackPat->setChecked(true);

    ui->spinBox_VarExpPatSeqFrameImgIndex->setRange(0,(m_numImgInFlash-1));

    ui->radioButton_VarExpPatSeqTrigTypeIntExt->setChecked(true);
    ui->radioButton_VarExpPatSeqTrigTypeVSync->setChecked(false);
    ui->radioButton_VarExpPatSeqSrcFrmVideoPort->setChecked(false);
    this->VarExpUpdateFrameTitle(false);
    //if there are entries in the SeqLut it must be cleared
    if(ui->listWidget_VarExpPatSeqLUT->count() != 0)
        emit on_pushButton_VarExpPatSeqClearLUTFrmGUI_clicked();
}

void MainWindow::on_radioButton_VarExpPatSeqSrcFrmVideoPort_clicked()
{
    ui->spinBox_VarExpPatSeqFrameImgIndex->setValue(0);
    ui->comboBox_VarExpPatSeqSelTrigType->setDisabled(true);
    ui->comboBox_VarExpPatSeqSelTrigType->setCurrentIndex(ExtPos);
    ui->checkBox_VarExpPatSeqInsBlackPat->setChecked(false);

    ui->spinBox_VarExpPatSeqFrameImgIndex->setRange(0,255);

    ui->radioButton_VarExpPatSeqTrigTypeIntExt->setChecked(false);
    ui->radioButton_VarExpPatSeqTrigTypeVSync->setChecked(true);
    ui->radioButton_VarExpPatSeqSrcFrmFlash->setChecked(false);
    this->VarExpUpdateFrameTitle(true);
    //if there are entries in the SeqLut it must be cleared
    if(ui->listWidget_VarExpPatSeqLUT->count() != 0)
        emit on_pushButton_VarExpPatSeqClearLUTFrmGUI_clicked();
}

void MainWindow::on_radioButton_VarExpPatSeqTrigTypeIntExt_clicked()
{
    ui->spinBox_VarExpPatSeqFrameImgIndex->setValue(0);
    ui->comboBox_VarExpPatSeqSelTrigType->setDisabled(false);
    ui->comboBox_VarExpPatSeqSelTrigType->setCurrentIndex(IntTrig);
    ui->checkBox_VarExpPatSeqInsBlackPat->setChecked(true);

    ui->spinBox_VarExpPatSeqFrameImgIndex->setRange(0,(m_numImgInFlash-1));

    ui->radioButton_VarExpPatSeqSrcFrmVideoPort->setChecked(false);
    ui->radioButton_VarExpPatSeqSrcFrmFlash->setChecked(true);
    ui->radioButton_VarExpPatSeqTrigTypeVSync->setChecked(false);
    this->VarExpUpdateFrameTitle(false);
    //if there are entries in the SeqLut it must be cleared
    if(ui->listWidget_VarExpPatSeqLUT->count() != 0)
        emit on_pushButton_VarExpPatSeqClearLUTFrmGUI_clicked();
}

void MainWindow::on_radioButton_VarExpPatSeqTrigTypeVSync_clicked()
{
    ui->spinBox_VarExpPatSeqFrameImgIndex->setValue(0);
    ui->comboBox_VarExpPatSeqSelTrigType->setDisabled(true);
    ui->comboBox_VarExpPatSeqSelTrigType->setCurrentIndex(ExtPos);
    ui->checkBox_VarExpPatSeqInsBlackPat->setChecked(false);

    ui->spinBox_VarExpPatSeqFrameImgIndex->setRange(0,255);

    ui->radioButton_VarExpPatSeqSrcFrmVideoPort->setChecked(true);
    ui->radioButton_VarExpPatSeqSrcFrmFlash->setChecked(false);
    ui->radioButton_VarExpPatSeqTrigTypeIntExt->setChecked(false);
    this->VarExpUpdateFrameTitle(true);
    //if there are entries in the SeqLut it must be cleared
    if(ui->listWidget_VarExpPatSeqLUT->count() != 0)
        emit on_pushButton_VarExpPatSeqClearLUTFrmGUI_clicked();
}

void MainWindow::on_spinBox_VarExpPatSeqFrameImgIndex_valueChanged(int index)
{
    g_VarExpFrameIdx = index;
}

void MainWindow::on_comboBox_VarExpPatSeqPatBitDepthSel_currentIndexChanged(int index)
{
    int i, j;
    QColor bgColor = Qt::gray;
    int index_orig = index;
    if(index == 4 || index == 6)
        index +=1;

    ui->listWidget_VarExpPatSeqBitPlanes->selectionModel()->clearSelection();

    for(i=0; i<24;)
    {
        for(j=0; j<=index; j++,i++)
        {
            ui->listWidget_VarExpPatSeqBitPlanes->item(i)->setBackgroundColor(bgColor);
        }
        if(bgColor == Qt::gray)
            bgColor = Qt::lightGray;
        else
            bgColor = Qt::gray;
    }

    if(index_orig == 4)  //Special case of bit-depth 5
    {
        ui->listWidget_VarExpPatSeqBitPlanes->item(0)->setBackgroundColor(Qt::black);
        ui->listWidget_VarExpPatSeqBitPlanes->item(6)->setBackgroundColor(Qt::black);
        ui->listWidget_VarExpPatSeqBitPlanes->item(12)->setBackgroundColor(Qt::black);
        ui->listWidget_VarExpPatSeqBitPlanes->item(18)->setBackgroundColor(Qt::black);
    }

    if(index_orig == 6)  //Special case of bit-depth 7
    {
        ui->listWidget_VarExpPatSeqBitPlanes->item(0)->setBackgroundColor(Qt::black);
        ui->listWidget_VarExpPatSeqBitPlanes->item(8)->setBackgroundColor(Qt::black);
        ui->listWidget_VarExpPatSeqBitPlanes->item(16)->setBackgroundColor(Qt::black);
    }
}

void MainWindow::on_listWidget_VarExpPatSeqBitPlanes_itemClicked(QListWidgetItem *item)
{
    int selectedRow = item->listWidget()->row(item);
    int i;
    unsigned int selectionMask;

    item->listWidget()->selectionModel()->clearSelection();
    selectionMask = BitPlanes[selectedRow][ui->comboBox_VarExpPatSeqPatBitDepthSel->currentIndex()];
    for(i=0; i < 24; i++)
    {
        if(selectionMask & 1)
        {
            item->listWidget()->item(i)->setSelected(true);
        }
        selectionMask >>= 1;
    }
}

void MainWindow::VarExpUpdateFrameTitle(bool isFrame)
{
    if(isFrame)
        ui->label_VarExpPatSeqFrmOrImgIndex->setText("Frame Index");
    else
        ui->label_VarExpPatSeqFrmOrImgIndex->setText("Flash Index");
}

int MainWindow::VarExpInsertTriggerItem(int trig_type)
{
    QIcon trigIcon;

    //If not "No Trigger"
    if(trig_type != NoTrig)
    {
        ui->listWidget_VarExpPatSeqLUT->addItem("");
        if(trig_type != ExtNeg)
        {
            trigIcon.addFile(":/new/prefix1/Icons/trigger.png");
        }
        else
        {
            trigIcon.addFile(":/new/prefix1/Icons/trigger_inv.png");
        }
        ui->listWidget_VarExpPatSeqLUT->item(ui->listWidget_VarExpPatSeqLUT->count()-1)->setIcon(trigIcon);
        return 0;
    }
    return -1;
}

QString MainWindow::VarExpGenerateItemText(int frame_Index, int bitDepth, int firstItem, int lastItem, bool invert, int patExp, int patPeriod)
{
    QString itemText;

    if(ui->radioButton_VarExpPatSeqSrcFrmFlash->isChecked())
        itemText = "S";
    else
        itemText = "F";

    itemText.append(numToStr(frame_Index));
    itemText.append("::");
    itemText.append(ui->listWidget_VarExpPatSeqBitPlanes->item(firstItem)->text());
    if(lastItem != firstItem)
    {
        itemText.append("-");
        itemText.append(ui->listWidget_VarExpPatSeqBitPlanes->item(lastItem)->text());
    }
    else
        itemText.prepend("   ");

    itemText.append("\n");
    itemText.append("  (");
    itemText.append(numToStr(bitDepth));
    itemText.append(")\n");
    itemText.append("Exp: ");
    itemText.append(numToStr(patExp));
    itemText.append("\n");
    itemText.append("Prd: ");
    itemText.append(numToStr(patPeriod));
    itemText.append("\n");

    if(invert)
        itemText.append("\n    ~");

    return itemText;
}

void MainWindow::VarExpUpdateSeqItemData(int trigType, int pat_num,int bitDepth,int Led_Select,int frame_index, bool invertPat, bool insertBlack,bool bufSwap, bool TrigOutPrev, int PatExp, int PatPrd)
{
    //Update the Data of the listItem with all relevant information for retrieval later.
    ui->listWidget_VarExpPatSeqLUT->item(ui->listWidget_VarExpPatSeqLUT->count()-1)->setData(Qt::UserRole+TrigType, trigType);
    ui->listWidget_VarExpPatSeqLUT->item(ui->listWidget_VarExpPatSeqLUT->count()-1)->setData(Qt::UserRole+PatNum, pat_num);
    ui->listWidget_VarExpPatSeqLUT->item(ui->listWidget_VarExpPatSeqLUT->count()-1)->setData(Qt::UserRole+BitDepth, bitDepth);
    ui->listWidget_VarExpPatSeqLUT->item(ui->listWidget_VarExpPatSeqLUT->count()-1)->setData(Qt::UserRole+LEDSelect, Led_Select);
    ui->listWidget_VarExpPatSeqLUT->item(ui->listWidget_VarExpPatSeqLUT->count()-1)->setData(Qt::UserRole+InvertPat, invertPat);
    ui->listWidget_VarExpPatSeqLUT->item(ui->listWidget_VarExpPatSeqLUT->count()-1)->setData(Qt::UserRole+InsertBlack, insertBlack);
    ui->listWidget_VarExpPatSeqLUT->item(ui->listWidget_VarExpPatSeqLUT->count()-1)->setData(Qt::UserRole+frameIndex, frame_index);
    ui->listWidget_VarExpPatSeqLUT->item(ui->listWidget_VarExpPatSeqLUT->count()-1)->setData(Qt::UserRole+BufSwap, bufSwap);
    ui->listWidget_VarExpPatSeqLUT->item(ui->listWidget_VarExpPatSeqLUT->count()-1)->setData(Qt::UserRole+trigOutPrev, TrigOutPrev);
    ui->listWidget_VarExpPatSeqLUT->item(ui->listWidget_VarExpPatSeqLUT->count()-1)->setData(Qt::UserRole+PatExposure, PatExp);
    ui->listWidget_VarExpPatSeqLUT->item(ui->listWidget_VarExpPatSeqLUT->count()-1)->setData(Qt::UserRole+PatPeriod, PatPrd);
}

void MainWindow::VarExpRemoveSelectedItemFromSeqList()
{
    int i;
    unsigned int buffer_swap=0;
    bool foundSelection = false;
    bool done = true;   //Single selection only

    for(i=0; i<ui->listWidget_VarExpPatSeqLUT->count(); i++)
    {
        while(ui->listWidget_VarExpPatSeqLUT->item(i)->isSelected())
        {
            if(ui->listWidget_VarExpPatSeqLUT->item(i)->icon().isNull()) //Selected item is not a trigger Icon
            {
                if( i > 0 && ui->listWidget_VarExpPatSeqLUT->item(i-1)->icon().isNull() == false) //if the previous item is a trigger icon; remove that first
                {
                    i--;
                    done = false;
                }
            }

            buffer_swap=0;
            //Item to be removed now is a pattern
            if(ui->listWidget_VarExpPatSeqLUT->item(i)->icon().isNull())
            {
                if(ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+BufSwap) == 1)
                    buffer_swap=1;

                //if the next item is also a pattern and if it shares the exposure time with the item being removed; then remove that also
                while((i+1) < ui->listWidget_VarExpPatSeqLUT->count()) //make sure we don't read past the array
                {
                    if(ui->listWidget_VarExpPatSeqLUT->item(i+1)->icon().isNull())
                    {
                        if(ui->listWidget_VarExpPatSeqLUT->item(i+1)->data(Qt::UserRole+trigOutPrev) == 1)
                        {
                            ui->listWidget_VarExpPatSeqLUT->takeItem(i+1);
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else //Item to be removed now is a trigger
            {
                //Set trigger type to "No Trigger" when trigger is removed.
                ui->listWidget_VarExpPatSeqLUT->item(i+1)->setData(Qt::UserRole+TrigType, NoTrig);
            }

            ui->listWidget_VarExpPatSeqLUT->takeItem(i);
            foundSelection = true;
            if(i>=ui->listWidget_VarExpPatSeqLUT->count())
            {
                ui->listWidget_VarExpPatSeqLUT->selectionModel()->clearSelection();
                return;
            }
            if(buffer_swap) //If BufSwap item was removed, the bufSwap needs to be inserted in the next item
                ui->listWidget_VarExpPatSeqLUT->item(i)->setData(Qt::UserRole+BufSwap, buffer_swap);

            if(done)
                break;

            done = true;
        }
    }
    if(!foundSelection)
        ShowError("None Selected");
    ui->listWidget_VarExpPatSeqLUT->selectionModel()->clearSelection();
}

void MainWindow::VarExpCombineItemWithPrev(unsigned int itemRow)
{
    unsigned int lastVisibleItem;

    if(itemRow == 0)
        return;

    //Return if prev item is trigger item
    if(ui->listWidget_VarExpPatSeqLUT->item(itemRow-1)->icon().isNull() == false)
        return;

    lastVisibleItem = itemRow-1;

    while(ui->listWidget_VarExpPatSeqLUT->item(lastVisibleItem)->isHidden())
        lastVisibleItem--;

    //Append color name to the end of prev item text
    if (ui->listWidget_VarExpPatSeqLUT->item(lastVisibleItem)->backgroundColor() != Qt::gray)
    {
        switch (ui->listWidget_VarExpPatSeqLUT->item(lastVisibleItem)->data(Qt::UserRole+LEDSelect).toUInt())
        {
        case 1:
            ui->listWidget_VarExpPatSeqLUT->item(lastVisibleItem)->setText(ui->listWidget_VarExpPatSeqLUT->item(itemRow-1)->text().append("Red"));;
            break;
        case 2:
            ui->listWidget_VarExpPatSeqLUT->item(lastVisibleItem)->setText(ui->listWidget_VarExpPatSeqLUT->item(itemRow-1)->text().append("Green"));;
            break;
        case 3:
            ui->listWidget_VarExpPatSeqLUT->item(lastVisibleItem)->setText(ui->listWidget_VarExpPatSeqLUT->item(itemRow-1)->text().append("Yellow"));;
            break;
        case 4:
            ui->listWidget_VarExpPatSeqLUT->item(lastVisibleItem)->setText(ui->listWidget_VarExpPatSeqLUT->item(itemRow-1)->text().append("Blue"));;
            break;
        case 5:
            ui->listWidget_VarExpPatSeqLUT->item(lastVisibleItem)->setText(ui->listWidget_VarExpPatSeqLUT->item(itemRow-1)->text().append("Magenta"));;
            break;
        case 6:
            ui->listWidget_VarExpPatSeqLUT->item(lastVisibleItem)->setText(ui->listWidget_VarExpPatSeqLUT->item(itemRow-1)->text().append("Cyan"));;
            break;
        case 7:
            ui->listWidget_VarExpPatSeqLUT->item(lastVisibleItem)->setText(ui->listWidget_VarExpPatSeqLUT->item(itemRow-1)->text().append("White"));;
            break;
        }
    }

    //Append color name to the end of selected item text
    if (ui->listWidget_VarExpPatSeqLUT->item(itemRow)->backgroundColor() != Qt::gray)
    {
        switch (ui->listWidget_VarExpPatSeqLUT->item(itemRow)->data(Qt::UserRole+LEDSelect).toUInt())
        {
        case 1:
            ui->listWidget_VarExpPatSeqLUT->item(itemRow)->setText(ui->listWidget_VarExpPatSeqLUT->item(itemRow)->text().append("Red"));;
            break;
        case 2:
            ui->listWidget_VarExpPatSeqLUT->item(itemRow)->setText(ui->listWidget_VarExpPatSeqLUT->item(itemRow)->text().append("Green"));;
            break;
        case 3:
            ui->listWidget_VarExpPatSeqLUT->item(itemRow)->setText(ui->listWidget_VarExpPatSeqLUT->item(itemRow)->text().append("Yellow"));;
            break;
        case 4:
            ui->listWidget_VarExpPatSeqLUT->item(itemRow)->setText(ui->listWidget_VarExpPatSeqLUT->item(itemRow)->text().append("Blue"));;
            break;
        case 5:
            ui->listWidget_VarExpPatSeqLUT->item(itemRow)->setText(ui->listWidget_VarExpPatSeqLUT->item(itemRow)->text().append("Magenta"));;
            break;
        case 6:
            ui->listWidget_VarExpPatSeqLUT->item(itemRow)->setText(ui->listWidget_VarExpPatSeqLUT->item(itemRow)->text().append("Cyan"));;
            break;
        case 7:
            ui->listWidget_VarExpPatSeqLUT->item(itemRow)->setText(ui->listWidget_VarExpPatSeqLUT->item(itemRow)->text().append("White"));;
            break;
        }
    }

    ui->listWidget_VarExpPatSeqLUT->item(lastVisibleItem)->setText(ui->listWidget_VarExpPatSeqLUT->item(lastVisibleItem)->text().append("\n"));
    ui->listWidget_VarExpPatSeqLUT->item(lastVisibleItem)->setText(ui->listWidget_VarExpPatSeqLUT->item(lastVisibleItem)->text().append(ui->listWidget_VarExpPatSeqLUT->item(itemRow)->text()));
    ui->listWidget_VarExpPatSeqLUT->item(itemRow)->setData(Qt::UserRole+trigOutPrev, 1);
    ui->listWidget_VarExpPatSeqLUT->item(lastVisibleItem)->setBackgroundColor(Qt::gray);
    ui->listWidget_VarExpPatSeqLUT->setRowHidden(itemRow, true);
    ui->listWidget_VarExpPatSeqLUT->selectionModel()->clearSelection();

    return;
}

void MainWindow::on_listWidget_VarExpPatSeqLUT_customContextMenuRequested(const QPoint &pos)
{
    int itemRow=0;
    QMenu seqListMenu(this);

    QAction *removeAct;
    QAction *invertAct;
    QAction *insertAct = new QAction(tr("Insert Trigger-In"), this);
    QAction *combineAct = new QAction(tr("Share exposure time with prev pattern"), this);
    QAction *toggleInsBlkAct = new QAction(tr("Clear DMD after exposure time"), this);

    toggleInsBlkAct->setCheckable(true);

    itemRow = ui->listWidget_VarExpPatSeqLUT->row((ui->listWidget_VarExpPatSeqLUT->itemAt(pos)));
    if(itemRow < 0 || itemRow > 1823)
        return;

    if(!ui->listWidget_VarExpPatSeqLUT->item(itemRow)->icon().isNull())
    {
        invertAct = new QAction(tr("Invert Trigger-In"), this);
        removeAct = new QAction("Remove Trigger-In", this);
    }
    else
    {
        invertAct = new QAction(tr("Invert Pattern"), this);
        removeAct = new QAction("Remove Pattern", this);
    }

    //For trigger Items
    if(!ui->listWidget_VarExpPatSeqLUT->item(itemRow)->icon().isNull())
    {
        //Invert option not applicable for Internal trigger. Also, for straming mode, external trigger can only be positive.
        if(ui->listWidget_VarExpPatSeqLUT->item(itemRow+1)->data(Qt::UserRole+TrigType).toInt() != IntTrig && ui->radioButton_VarExpPatSeqSrcFrmFlash->isChecked() == 1)
            seqListMenu.addAction(invertAct);
    }
    else
    {
        if(itemRow < ui->listWidget_VarExpPatSeqLUT->count()-1)//if not last item
        {
            //check if next items are combined (hidden/trigOutPrev=1) with this one. If yes, don't give the option to invert this item
            if(!ui->listWidget_VarExpPatSeqLUT->item(itemRow+1)->isHidden())
            {
                seqListMenu.addAction(invertAct);
            }

        }
        else
            seqListMenu.addAction(invertAct);

        //If not first item and previous item is also a pattern item
        if(itemRow != 0 && ui->listWidget_VarExpPatSeqLUT->item(itemRow-1)->icon().isNull())
        {
            seqListMenu.addAction(combineAct);
        }
        seqListMenu.addAction(toggleInsBlkAct);
        if(ui->listWidget_VarExpPatSeqLUT->item(itemRow)->data(Qt::UserRole+InsertBlack).toInt() != 0)
            toggleInsBlkAct->setChecked(true);
        else
            toggleInsBlkAct->setChecked(false);
    }

    if(!ui->listWidget_VarExpPatSeqLUT->item(itemRow)->icon().isNull() && itemRow == 0)
    {
        /* First trigger item cannot be removed */
    }
    else
    {
        seqListMenu.addAction(removeAct);
    }

    if(ui->radioButton_VarExpPatSeqSrcFrmFlash->isChecked())
    {
        //If pattern item and trigger type is "No Trigger"
        if(ui->listWidget_VarExpPatSeqLUT->item(itemRow)->icon().isNull() && ui->listWidget_VarExpPatSeqLUT->item(itemRow)->data(Qt::UserRole+TrigType).toInt() == NoTrig)
        {
            seqListMenu.addAction(insertAct);
        }
    }

    QAction* selectedAct = seqListMenu.exec(QCursor::pos());

    if(selectedAct == removeAct)
    {
        VarExpRemoveSelectedItemFromSeqList();
    }
    else if(selectedAct == invertAct)
    {
        if(!ui->listWidget_VarExpPatSeqLUT->item(itemRow)->icon().isNull()) //Invert Trigger-In
        {
            if(ui->listWidget_VarExpPatSeqLUT->item(itemRow+1)->data(Qt::UserRole+TrigType).toInt() == ExtPos)
            {
                ui->listWidget_VarExpPatSeqLUT->item(itemRow+1)->setData(Qt::UserRole+TrigType, ExtNeg);
                QIcon trigInvIcon (":/new/prefix1/Icons/trigger_inv.png");
                ui->listWidget_VarExpPatSeqLUT->item(itemRow)->setIcon(trigInvIcon);
            }
            else if(ui->listWidget_VarExpPatSeqLUT->item(itemRow+1)->data(Qt::UserRole+TrigType).toInt() == ExtNeg)
            {
                ui->listWidget_VarExpPatSeqLUT->item(itemRow+1)->setData(Qt::UserRole+TrigType, ExtPos);
                QIcon trigInvIcon (":/new/prefix1/Icons/trigger.png");
                ui->listWidget_VarExpPatSeqLUT->item(itemRow)->setIcon(trigInvIcon);
            }
        }
        else //Invert Pattern
        {
            if(ui->listWidget_VarExpPatSeqLUT->item(itemRow)->text().contains("~"))
            {
                QString itemText = ui->listWidget_VarExpPatSeqLUT->item(itemRow)->text();
                itemText.chop(6); //SK: TBD
                ui->listWidget_VarExpPatSeqLUT->item(itemRow)->setText(itemText);
                ui->listWidget_VarExpPatSeqLUT->item(itemRow)->setData(Qt::UserRole+InvertPat, 0);
            }
            else
            {
                ui->listWidget_VarExpPatSeqLUT->item(itemRow)->setText(ui->listWidget_VarExpPatSeqLUT->item(itemRow)->text().append("\n    ~"));
                ui->listWidget_VarExpPatSeqLUT->item(itemRow)->setData(Qt::UserRole+InvertPat, 1);
            }
        }
    }
    else if(selectedAct == insertAct)
    {
        if(ui->comboBox_VarExpPatSeqSelTrigType->currentIndex() == ExtNeg)
        {
            ui->listWidget_VarExpPatSeqLUT->item(itemRow)->setData(Qt::UserRole+TrigType, ExtNeg);
            ui->listWidget_VarExpPatSeqLUT->insertItem(itemRow, "");
            QIcon trigIcon (":/new/prefix1/Icons/trigger_inv.png");
            ui->listWidget_VarExpPatSeqLUT->item(itemRow)->setIcon(trigIcon);
        }
        else
        {
            if(ui->comboBox_VarExpPatSeqSelTrigType->currentIndex() == ExtPos)
                ui->listWidget_VarExpPatSeqLUT->item(itemRow)->setData(Qt::UserRole+TrigType, ExtPos);
            else if(ui->comboBox_VarExpPatSeqSelTrigType->currentIndex() == IntTrig)
                ui->listWidget_VarExpPatSeqLUT->item(itemRow)->setData(Qt::UserRole+TrigType, IntTrig);
            else
            {
                ShowError("Please select trigger type to be inserted using Trigger_In_Type combobox");
                return;
            }
            ui->listWidget_VarExpPatSeqLUT->insertItem(itemRow, "");
            QIcon trigIcon (":/new/prefix1/Icons/trigger.png");
            ui->listWidget_VarExpPatSeqLUT->item(itemRow)->setIcon(trigIcon);
        }
    }
    else if(selectedAct == combineAct)
    {
        VarExpCombineItemWithPrev(itemRow);
    }
    else if(selectedAct == toggleInsBlkAct)
    {
        if(toggleInsBlkAct->isChecked())
            ui->listWidget_VarExpPatSeqLUT->item(itemRow)->setData(Qt::UserRole+InsertBlack, 1);
        else
            ui->listWidget_VarExpPatSeqLUT->item(itemRow)->setData(Qt::UserRole+InsertBlack, 0);
    }
}

void MainWindow::on_pushButton_VarExpPatSeqAddPatToLut_clicked()
{
    int i;
    int firstItem=25, lastItem;
    unsigned int pat_num=0;
    unsigned int prevFrameIdx=0;
    QColor bgColor;
    bool bufSwap;
    QListWidgetItem *curItem;
    unsigned int seqListLength;
    int trigger_type;

    if(ui->comboBox_VarExpPatSeqSelPatColor->currentIndex()==0)
    {
        ShowError("Select a Color");
        return;
    }

    if(ui->listWidget_VarExpPatSeqBitPlanes->selectedItems().count() != (ui->comboBox_VarExpPatSeqPatBitDepthSel->currentIndex()+1))
    {
        ShowError("Number of bits selected must equal Bit Depth");
        return;
    }

    //Make sure the Pattern Exposure and Pattern Period timings are within the spec

    //Don't allow Pattern Exposure > Pattern Period
    if((ui->lineEdit_VarExpPatSeqPatExpTime->text().toInt() > ui->lineEdit_VarExpPatSeqPatPeriod->text().toInt()))
    {
        ShowError("Pattern exposure setting voilation, it should be, Pattern Exposure = Pattern Period or (Pattern Period - Pattern Exposure) > 230us");
        return;
    }

    //If Pattern Exposure != Pattern Period then (Pattern Period - Pattern Exposure) > 230us
    if( ((ui->lineEdit_VarExpPatSeqPatExpTime->text().toInt() != ui->lineEdit_VarExpPatSeqPatPeriod->text().toInt())) &&
        ((ui->lineEdit_VarExpPatSeqPatPeriod->text().toInt() - (ui->lineEdit_VarExpPatSeqPatExpTime->text().toInt())) <= 230))
    {
        ShowError("Pattern exposure setting voilation, it should be, Pattern Exposure = Pattern Period or (Pattern Period - Pattern Exposure) > 230us");
        return;
    }

    for(i=0; i<24; i++)
    {
        if(ui->listWidget_VarExpPatSeqBitPlanes->item(i)->isSelected())
        {
            if(firstItem == 25)
                firstItem = i;
            lastItem = i;
        }
    }

    //If first item
    if(ui->listWidget_VarExpPatSeqLUT->count() == 0)
    {
        if(ui->radioButton_VarExpPatSeqSrcFrmFlash->isChecked())
        {
            //Trigger type can't be "No Trigger"
            if(ui->comboBox_VarExpPatSeqSelTrigType->currentIndex() == NoTrig)
            {
                ShowError("First Item must be triggered. Please select a Trigger_In_Type other than No Trigger");
                return;
            }
            bufSwap = true;//false;
        }
        else
        {
            //In streaming mode, first item has to be triggered by vsync
            bufSwap = true;
        }
    }
    else
    {
        prevFrameIdx = ui->listWidget_VarExpPatSeqLUT->item(ui->listWidget_VarExpPatSeqLUT->count()-1)->data(Qt::UserRole+frameIndex).toUInt();
        if(prevFrameIdx != g_VarExpFrameIdx)
        {
            bufSwap = true;
        }
        else
        {
            bufSwap = false;
        }
    }

    if(ui->radioButton_VarExpPatSeqSrcFrmFlash->isChecked())
    {
        trigger_type = ui->comboBox_VarExpPatSeqSelTrigType->currentIndex();
    }
    else
    {
        if(bufSwap == true)
            trigger_type = ExtPos;
        else
            trigger_type = NoTrig;
    }

    VarExpInsertTriggerItem(trigger_type);

    ui->listWidget_VarExpPatSeqLUT->addItem(VarExpGenerateItemText(g_VarExpFrameIdx, ui->comboBox_VarExpPatSeqPatBitDepthSel->currentIndex()+1, firstItem, lastItem,ui->checkBox_VarExpPatSeqInvPatData->isChecked(),ui->lineEdit_VarExpPatSeqPatExpTime->text().toInt(0),ui->lineEdit_VarExpPatSeqPatPeriod->text().toInt(0)));

    seqListLength = ui->listWidget_VarExpPatSeqLUT->count();
    curItem = ui->listWidget_VarExpPatSeqLUT->item(seqListLength-1);

    bgColor = GetColorFromIndex(ui->comboBox_VarExpPatSeqSelPatColor->currentIndex());
    curItem->setBackgroundColor(bgColor);

    //Update the Data of the listItem with all relevant information for retrieval later.
    pat_num = firstItem/(ui->comboBox_VarExpPatSeqPatBitDepthSel->currentIndex()+1);
    VarExpUpdateSeqItemData(trigger_type, pat_num, ui->comboBox_VarExpPatSeqPatBitDepthSel->currentIndex()+1, ui->comboBox_VarExpPatSeqSelPatColor->currentIndex(),g_VarExpFrameIdx, ui->checkBox_VarExpPatSeqInvPatData->isChecked(), ui->checkBox_VarExpPatSeqInsBlackPat->isChecked(), bufSwap, 0, ui->lineEdit_VarExpPatSeqPatExpTime->text().toInt(0),ui->lineEdit_VarExpPatSeqPatPeriod->text().toInt(0));
    ui->listWidget_VarExpPatSeqLUT->scrollToItem(ui->listWidget_VarExpPatSeqLUT->item(seqListLength-1));
}

void MainWindow::on_pushButton_VarExpPatSeqSendLUT_clicked()
{
    int i;
    char errorStr[256];
    unsigned char splashLut[256];
    int numSplashLutEntries = 0;
    int numPatLutEntries = 0;
    int numPatsPerTrigOut2 = 1;
    int trigMode=0;
    unsigned int min_pat_exposure[8] = {235, 700, 1570, 1700, 2000, 2500, 4500, 8333};

    if(ui->listWidget_VarExpPatSeqLUT->count() == 0)
        return;

	m_dlpAPI->DLPC350_ClearExpLut();

    for(i=0; i<ui->listWidget_VarExpPatSeqLUT->count(); i++)
    {
        if(ui->listWidget_VarExpPatSeqLUT->item(i)->icon().isNull()) //only if pattern item (skip trigger items)
        {

            if(i==0)
            {
                ShowError("First Item must be triggered. Please select a Trigger_In_Type other than No Trigger");
                return;
            }


            //Check if expsoure time is meets the minimum requirement
            if(ui->lineEdit_VarExpPatSeqPatExpTime->text().toUInt() < min_pat_exposure[ui->comboBox_VarExpPatSeqPatBitDepthSel->currentIndex()])
            {
                sprintf(errorStr, "Exposure time %d < Minimum Exposure time %d for bit depth %d", ui->lineEdit_VarExpPatSeqPatExpTime->text().toUInt(), min_pat_exposure[ui->comboBox_VarExpPatSeqPatBitDepthSel->currentIndex()], ui->comboBox_VarExpPatSeqPatBitDepthSel->currentIndex());
                ShowError(errorStr);
                return;
            }

            //Check for share exposure with previous pattern
            if(ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+trigOutPrev).toBool() == true)
            {
                if(i>1 && (ui->listWidget_VarExpPatSeqLUT->item((i-1))->data(Qt::UserRole+InsertBlack).toBool() == true))
                {
                    ShowError("Error Pattern cannot be shared with previous exposure; because previous pattern has black-fill pattern inserted");
                    return;
                }
            }

            //If there is a buffer swap or if this is the first pattern
            if((ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+BufSwap).toBool()) || (numSplashLutEntries == 0))
            {
                if (numSplashLutEntries >= 256)
                {
                    ShowError("Image LUT entries(256) reached maximum. Will not add anymore entries\n");
                    return;
                }
                else
                {
                    splashLut[numSplashLutEntries++] = ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+frameIndex).toInt();
                }
            }

#ifdef DEBUG_LOG_EN
            qDebug() << "[" << i << "] " << "TrigType = " << ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+TrigType).toInt() << "," \
                     << "PatNum = " << ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+PatNum).toInt() << "," \
                     << "BitDepth = " << ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+BitDepth).toInt() << "," \
                     << "LEDSelect = " << ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+LEDSelect).toInt() << "," \
                     << "InvertPattern = " << ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+InvertPat).toBool() << "," \
                     << "Insert Black = " << ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+InsertBlack).toBool() << "," \
                     << "BufSwap = " << ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+BufSwap).toBool() << "," \
                     << "TrigOutPrev = " << ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+trigOutPrev).toBool() << "," \
                     << "PatExposure = " <<  ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+PatExposure).toInt() << "," \
                     << "PatPeriod = "  << ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+PatPeriod).toInt();
#endif

			if (m_dlpAPI->DLPC350_AddToExpLut(\
                        ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+TrigType).toInt(), \
                        ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+PatNum).toInt(), \
                        ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+BitDepth).toInt(), \
                        ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+LEDSelect).toInt(), \
                        ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+InvertPat).toBool(), \
                        ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+InsertBlack).toBool(), \
                        ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+BufSwap).toBool(), \
                        ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+trigOutPrev).toBool(), \
                        ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+PatExposure).toInt(), \
                        ui->listWidget_VarExpPatSeqLUT->item(i)->data(Qt::UserRole+PatPeriod).toInt()) < 0)
            {
                ShowError("Error Variable Exposure Pattern LUT");
                return;
            }

            numPatLutEntries++;
        }
    }

    //Set Pattern Mode - Video or Flash
	m_dlpAPI->DLPC350_SetPatternDisplayMode(ui->radioButton_VarExpPatSeqSrcFrmVideoPort->isChecked());

    //if play once is selected
    if(ui->radioButton_VarExpPatSeqDispRunOnce->isChecked())
    {
        numPatsPerTrigOut2 = numPatLutEntries;
    }
    else
    {
        numPatsPerTrigOut2 = ui->spinBox_TrigOut2PulsePerNumPat->value();
    }

    //Pattern Sequence Configuration
	if (m_dlpAPI->DLPC350_SetVarExpPatternConfig(numPatLutEntries, numPatsPerTrigOut2, numSplashLutEntries, ui->radioButton_VarExpPatSeqDispRunContinuous->isChecked()) < 0)
    {
        ShowError("Error Sending Pattern Config");
        return;
    }

    if(ui->radioButton_VarExpPatSeqTrigTypeIntExt->isChecked())
    {
        trigMode = 3;
    }
    else
    {
        trigMode = 4;
    }

    //Configure Trigger Mode - 3 or 4 //Applicable for Variable Exposure pat sequence
	if (m_dlpAPI->DLPC350_SetPatternTriggerMode(trigMode) < 0)
    {
        ShowError("Error Sending trigger Mode");
        return;
    }

    //Send Variable Exposure pattern LUT
	if (m_dlpAPI->DLPC350_SendVarExpPatLut() < 0)
    {
        ShowError("Error Sending Variable Exposure Pattern LUT");
        return;
    }

	if (m_dlpAPI->DLPC350_SendVarExpImageLut(&splashLut[0], numSplashLutEntries) < 0)
    {
        ShowError("Error Sending Variable Exposure Image LUT");
        return;
    }

    ui->pushButton_PatSeqValIndExpOOR->setEnabled(false);
    ui->pushButton_PatSeqValIndPatNumOOR->setEnabled(false);
    ui->pushButton_PatSeqValIndTrigOutOverlap->setEnabled(false);
    ui->pushButton_PatSeqValIndBlkVecMiss->setEnabled(false);
    ui->pushButton_PatSeqValPatPeriodShort->setEnabled(false);

    ui->pushButton_PatSeqValStatus->setEnabled(false);
    ui->pushButton_PatSeqCtrlStart->setEnabled(false);
    ui->pushButton_PatSeqCtrlPause->setEnabled(false);
    ui->pushButton_PatSeqCtrlStop->setEnabled(false);

    // Select the pattern stop start pause tab
    ui->tabWidget_2->setCurrentIndex(2);
}

void MainWindow::on_pushButton_VarExpPatSeqReadLUTFrmHW_clicked()
{
    int trigMode=0;
    bool isExtPatDisplayMode;
    unsigned int NumPatLutEntries;
    bool Repeat;
    unsigned int NumPatsPerTrigOut2;
    unsigned int NumImages;
    unsigned char NumImagesLUT[256];
    int Pat_Exposure;
    int Pat_Period;
    int trig_type, Pat_Num, Bit_Depth, LED_Select, Frame_Index;
    bool Invert_Pat, Insert_Black, Buf_Swap, TrigOutPrev;
    unsigned int firstItem=32, lastItem, index;
    QColor bgColor;
    unsigned int i, j;

    //Read update the trigger mode and then update
	if (m_dlpAPI->DLPC350_GetPatternTriggerMode(&trigMode) == 0)
    {
        if(trigMode == 3 || trigMode == 4)
        {
            //Read the display Mode
			if (m_dlpAPI->DLPC350_GetPatternDisplayMode(&isExtPatDisplayMode) == 0)
            {
                if (isExtPatDisplayMode)
                {
                    ui->radioButton_VarExpPatSeqSrcFrmVideoPort->setChecked(true);
                    emit on_radioButton_VarExpPatSeqSrcFrmVideoPort_clicked();
                }
                else
                {
                    ui->radioButton_VarExpPatSeqSrcFrmFlash->setChecked(true);
                    emit on_radioButton_VarExpPatSeqSrcFrmFlash_clicked();
                }
            }
            else
            {
                ShowError("Error Unable to read the Pattern Display Mode");
            }

            //Get the variable Exposure Pattern Configurarion
			if (m_dlpAPI->DLPC350_GetVarExpPatternConfig(&NumPatLutEntries, &NumPatsPerTrigOut2, &NumImages, &Repeat) == 0)
            {
                if(Repeat == true)
                {
                    ui->radioButton_VarExpPatSeqDispRunContinuous->setChecked(true);
                    ui->spinBox_TrigOut2PulsePerNumPat->setValue(NumPatsPerTrigOut2); //SK: Is it proper?
                }
                else
                {
                    ui->radioButton_VarExpPatSeqDispRunOnce->setChecked(true);
                }
            }

			if (m_dlpAPI->DLPC350_GetVarExpPatLut(NumPatLutEntries) < 0)
            {
                ShowError("Could not read variable exposure pattern LUT from target");
                return;
            }

			if (m_dlpAPI->DLPC350_GetvarExpImageLut(&NumImagesLUT[0], NumImages) < 0)
            {
                ShowError("Could not read image LUT from target");
                return;
            }
            else
            {
                // Values read correctly so check for special 2 numSplash case and adjust so GUI displays correctly
                if(NumImages == 2)
                {
                    unsigned char temp_val = NumImagesLUT[0];
                    NumImagesLUT[0] = NumImagesLUT[1];
                    NumImagesLUT[1] = temp_val;
                }

#ifdef DEBUG_LOG_EN
                //Readout the VarExpImageLut table
                for(unsigned int i = 0; i<NumImages; i++)
                    qDebug() << "VarExpImageLut[" << i << "] " << NumImagesLUT[i] ;
#endif
            }

            emit on_pushButton_VarExpPatSeqClearLUTFrmGUI_clicked();

            Frame_Index = -1;

            for(i=0; i<NumPatLutEntries; i++)
            {
				m_dlpAPI->DLPC350_GetVarExpPatLutItem(i, &trig_type, &Pat_Num, &Bit_Depth, &LED_Select, &Invert_Pat, &Insert_Black, &Buf_Swap, &TrigOutPrev, &Pat_Exposure, &Pat_Period);

#ifdef DEBUG_LOG_EN
                qDebug() << "Pat_Num[" << i << "] " << "TrigType = " << trig_type << "," << "PatIndex = " << Pat_Num << "," \
                         << "Bit-Depth = " << Bit_Depth << "," << "LED_Color = " << LED_Select << "Invert = " << Invert_Pat << "," \
                         << "Insert_Black = " << Insert_Black << "," << "Buffer-Swap = " << Buf_Swap << "," << "TrigOutPrev = " << TrigOutPrev \
                         << "," << "PatterExposure = " <<  Pat_Exposure << "," << "PatternPeriod = " << Pat_Period;
#endif
                if(Bit_Depth <= 0 || Bit_Depth > 8)
                {
                    ShowError("Received unexpected value for Bit depth");
                    break;
                }

                //Add trigger type in the GUI
                VarExpInsertTriggerItem(trig_type);

                if((Buf_Swap) || (i==0))
                    Frame_Index++;

                firstItem = 32;

                if(Bit_Depth == 5 || Bit_Depth == 7)
                    index = Pat_Num*(Bit_Depth+1);
                else
                    index = Pat_Num * Bit_Depth;

                for(j=0; j < 32; j++)
                {
                    if(firstItem > 31) //first set bit not found yet
                    {
                        if( (BitPlanes[index][Bit_Depth-1] & (1 << j)) == (1<<j))
                        {
                            firstItem = j;
                        }
                    }
                    else if((BitPlanes[index][Bit_Depth-1] & (1 << j)) == 0)
                    {
                        break;
                    }
                }

                lastItem = j-1;

                if(isExtPatDisplayMode)
                    ui->listWidget_VarExpPatSeqLUT->addItem(VarExpGenerateItemText(Frame_Index, Bit_Depth, firstItem, lastItem, Invert_Pat, Pat_Exposure, Pat_Period));
                else
                    ui->listWidget_VarExpPatSeqLUT->addItem(VarExpGenerateItemText(NumImagesLUT[Frame_Index], Bit_Depth, firstItem, lastItem, Invert_Pat, Pat_Exposure, Pat_Period));

                if(LED_Select <= 0 || LED_Select > 7)
                {
                    ShowError("Received unexpected value for Color selection");
                    break;
                }

                bgColor = GetColorFromIndex(LED_Select);

                ui->listWidget_VarExpPatSeqLUT->item(ui->listWidget_VarExpPatSeqLUT->count()-1)->setBackgroundColor(bgColor);

                if(isExtPatDisplayMode)
                    VarExpUpdateSeqItemData(trig_type, Pat_Num, Bit_Depth, LED_Select, Frame_Index, Invert_Pat, Insert_Black, Buf_Swap, TrigOutPrev, Pat_Exposure, Pat_Period);
                else
                    VarExpUpdateSeqItemData(trig_type, Pat_Num, Bit_Depth, LED_Select, NumImagesLUT[Frame_Index], Invert_Pat, Insert_Black, Buf_Swap, TrigOutPrev,  Pat_Exposure, Pat_Period);

                if(ui->listWidget_VarExpPatSeqLUT->count() > 1)
                {
                    if(ui->listWidget_VarExpPatSeqLUT->item(ui->listWidget_VarExpPatSeqLUT->count()-1)->data(Qt::UserRole+trigOutPrev).toBool() == true)
                    {
                        ;//CombineItemWithPrev(ui->Seq_listWidget->count()-1); //SK: Pending
                    }
                }
            }
        }
        else
        {
            ShowError("System is configured in legacy Pattern Sequence Mode");
        }
    }
}

void MainWindow::on_pushButton_VarExpPatSeqClearLUTFrmGUI_clicked()
{
    m_numExtraSplashLutEntries = 0;

    while(ui->listWidget_VarExpPatSeqLUT->count() != 0)
        ui->listWidget_VarExpPatSeqLUT->takeItem(0);
    ui->spinBox_VarExpPatSeqFrameImgIndex->setValue(0);

    ui->pushButton_PatSeqValIndExpOOR->setEnabled(false);
    ui->pushButton_PatSeqValIndPatNumOOR->setEnabled(false);
    ui->pushButton_PatSeqValIndTrigOutOverlap->setEnabled(false);
    ui->pushButton_PatSeqValIndBlkVecMiss->setEnabled(false);
    ui->pushButton_PatSeqValPatPeriodShort->setEnabled(false);

    ui->pushButton_PatSeqValStatus->setEnabled(false);
    ui->pushButton_PatSeqCtrlStart->setEnabled(false);
    ui->pushButton_PatSeqCtrlPause->setEnabled(false);
    ui->pushButton_PatSeqCtrlStop->setEnabled(false);
}

/* Patter Sequence Play/Pause/Stop & Validate Control */

void MainWindow::on_pushButton_ValidatePatSeq_clicked()
{
    int i = 0;
    unsigned int status;
    bool ready;

    if((ui->radioButton_SLMode->isChecked() == false) && (ui->radioButton_VariableExpSLMode->isChecked() == false))
    {
        ShowError("Please change operating mode to Pattern Sequence before validating sequence");
        return;
    }

    //if pattern sequence is already running it must be stopped first
    emit on_pushButton_PatSeqCtrlStop_clicked();

    //Clear the flags
    ui->pushButton_PatSeqValIndExpOOR->setEnabled(false);
    ui->pushButton_PatSeqValIndPatNumOOR->setEnabled(false);
    ui->pushButton_PatSeqValIndTrigOutOverlap->setEnabled(false);
    ui->pushButton_PatSeqValIndBlkVecMiss->setEnabled(false);
    ui->pushButton_PatSeqValPatPeriodShort->setEnabled(false);
    ui->pushButton_PatSeqCtrlStart->setEnabled(false);
    ui->pushButton_PatSeqCtrlPause->setEnabled(false);
    ui->pushButton_PatSeqCtrlStop->setEnabled(false);


	if (m_dlpAPI->DLPC350_StartPatLutValidate())
    {
        ShowError("Error validating LUT data");
        return;
    }

    QEventLoop loop;

    do
    {
		if (m_dlpAPI->DLPC350_CheckPatLutValidate(&ready, &status) < 0)
        {
            ShowError("Error validating LUT data");
            return;
        }

        if(ready)
        {
            break;
        }
        else
        {
            QTimer::singleShot(1000, &loop, SLOT(quit()));
            loop.exec();
        }

        if(i++ > MAX_NUM_RETRIES)
            break;
    } while(1);

    ui->pushButton_PatSeqValIndExpOOR->setEnabled((status & BIT0) == BIT0);
    ui->pushButton_PatSeqValIndPatNumOOR->setEnabled((status & BIT1) == BIT1);
    ui->pushButton_PatSeqValIndTrigOutOverlap->setEnabled((status & BIT2) == BIT2);
    ui->pushButton_PatSeqValIndBlkVecMiss->setEnabled((status & BIT3) == BIT3);
    ui->pushButton_PatSeqValPatPeriodShort->setEnabled((status & BIT4) == BIT4);
    //Except BIT0 and BIT1 flags enable pattern sequence
    if(!(status & BIT0) && !(status & BIT1))
    {
        ui->pushButton_PatSeqValStatus->setEnabled(true);
        ui->pushButton_PatSeqCtrlStart->setEnabled(true);
        ui->pushButton_PatSeqCtrlPause->setEnabled(true);
        ui->pushButton_PatSeqCtrlStop->setEnabled(true);

		m_bPatSeqValidated = true;
    }
    else
    {
        ui->pushButton_PatSeqValStatus->setEnabled(false);
        ui->pushButton_PatSeqCtrlStart->setEnabled(false);
        ui->pushButton_PatSeqCtrlPause->setEnabled(false);
        ui->pushButton_PatSeqCtrlStop->setEnabled(false);

		m_bPatSeqValidated = false;
    }
}

void MainWindow::on_pushButton_PatSeqCtrlStart_clicked()
{
    int i = 0;
    unsigned int patMode;
	m_dlpAPI->DLPC350_PatternDisplay(2);
    SleeperThread::msleep(100);
    while(1)
    {
		m_dlpAPI->DLPC350_GetPatternDisplay(&patMode);
        if(patMode == 2)
            break;
        else
			m_dlpAPI->DLPC350_PatternDisplay(2);
        SleeperThread::msleep(100);

        if(i++ > MAX_NUM_RETRIES)
            break;
    }
}

void MainWindow::on_pushButton_PatSeqCtrlPause_clicked()
{
    int i = 0;
    unsigned int patMode;
	m_dlpAPI->DLPC350_PatternDisplay(1);
    SleeperThread::msleep(100);
    while(1)
    {
		m_dlpAPI->DLPC350_GetPatternDisplay(&patMode);
        if(patMode == 1)
            break;
        else
			m_dlpAPI->DLPC350_PatternDisplay(1);
        SleeperThread::msleep(100);
        if(i++ > MAX_NUM_RETRIES)
            break;
    }
}

void MainWindow::on_pushButton_PatSeqCtrlStop_clicked()
{
    int i = 0;
    unsigned int patMode;

	m_dlpAPI->DLPC350_PatternDisplay(0);
    SleeperThread::msleep(100);
    while(1)
    {
		m_dlpAPI->DLPC350_GetPatternDisplay(&patMode);
        if(patMode == 0)
            break;
        else
			m_dlpAPI->DLPC350_PatternDisplay(0);
        SleeperThread::msleep(100);
        if(i++ > MAX_NUM_RETRIES)
            break;
    }
}

void MainWindow::on_checkBox_PatSeqCtrlGlobalDataInvert_toggled(bool checked)
{
	m_dlpAPI->DLPC350_SetInvertData(checked);
}

/* Image Load timing information retrive */

void MainWindow::on_pushButton_GetImgLoadTimingInfo_clicked()
{
    unsigned int pTimingData;
    unsigned char HWStatus, SysStatus, MainStatus;

    ui->lineEdit_ImgLoadTmInMs->clear();

	if (m_dlpAPI->DLPC350_MeasureImageLoadTiming(ui->spinBox_ImgLdTmImgIndex->value(), 1) >= 0)
    {

		if (m_dlpAPI->DLPC350_GetStatus(&HWStatus, &SysStatus, &MainStatus) != 0)
        {
            ShowError("Unable to read status");
            return;
        }

		if (m_dlpAPI->DLPC350_ReadImageLoadTiming(&pTimingData) >= 0)
        {
            ui->lineEdit_ImgLoadTmInMs->setText(numToStr(pTimingData/18667));
        }
    }
}


/* Trigger Input/Output Control */

void MainWindow::on_pushButton_SetTrigConfig_clicked() //Also sends trig in1delay
{

    bool mode;
    unsigned int patMode;

    //Allow settings to be applied only in Pattern Mode
	m_dlpAPI->DLPC350_GetMode(&mode);
    if(mode == true)
    {
       //First stop pattern sequence
		m_dlpAPI->DLPC350_GetPatternDisplay(&patMode);
        //if it is in PAUSE or RUN mode
        if(patMode != 0)
        {
            emit on_pushButton_PatSeqCtrlStop_clicked();
        }

        //Apply the settings

        // Send trigger 1 input delay
		m_dlpAPI->DLPC350_SetTrigIn1Delay(ui->spinBox_TrigIn1->value());

        // Send trigger 2 input polarity
		m_dlpAPI->DLPC350_SetTrigIn2Pol(ui->comboBox_TrigIn2Pol->currentIndex());

        // Send trigger 1 output delays
		m_dlpAPI->DLPC350_SetTrigOutConfig(1, ui->checkBox_InvertTrig1Out->isChecked(), \
                                    ui->spinBox_Trig1OutRDly->value(),\
                                    ui->spinBox_Trig1OutFDly->value());

        // Send trigger 2 output delay
		m_dlpAPI->DLPC350_SetTrigOutConfig(2, ui->checkBox_InvertTrig2Out->isChecked(), \
                                    ui->spinBox_Trig2OutRDly->value(),0);

        ui->pushButton_PatSeqValIndExpOOR->setEnabled(false);
        ui->pushButton_PatSeqValIndPatNumOOR->setEnabled(false);
        ui->pushButton_PatSeqValIndTrigOutOverlap->setEnabled(false);
        ui->pushButton_PatSeqValIndBlkVecMiss->setEnabled(false);
        ui->pushButton_PatSeqValPatPeriodShort->setEnabled(false);

        ui->pushButton_PatSeqValStatus->setEnabled(false);
        ui->pushButton_PatSeqCtrlStart->setEnabled(false);
        ui->pushButton_PatSeqCtrlPause->setEnabled(false);
        ui->pushButton_PatSeqCtrlStop->setEnabled(false);

        // Select the pattern stop start pause tab
        ui->tabWidget_2->setCurrentIndex(2);
    }

}

void MainWindow::on_pushButton_GetTrigConfig_clicked()
{
    unsigned int risingDelay, fallingDelay;
    unsigned int delay;
    bool invert;
    bool isFallingEdge;

    // Get trigger 1 output delays
	if (m_dlpAPI->DLPC350_GetTrigOutConfig(1, &invert, &risingDelay, &fallingDelay) == 0)
    {
        ui->checkBox_InvertTrig1Out->setChecked(invert);
        ui->spinBox_Trig1OutRDly->setValue(risingDelay);
        ui->spinBox_Trig1OutFDly->setValue(fallingDelay);
        emit on_spinBox_Trig1OutRDly_valueChanged(risingDelay);
        emit on_spinBox_Trig1OutFDly_valueChanged(fallingDelay);
    }
#ifdef DEBUG_LOG_EN
    else
    {
        qDebug() << "DLPC350_GetTrigOutConfig() FAIL";
    }
#endif

    // Get trigger 2 output delay
	if (m_dlpAPI->DLPC350_GetTrigOutConfig(2, &invert, &risingDelay, &fallingDelay) == 0)
    {
        ui->checkBox_InvertTrig2Out->setChecked(invert);
        ui->spinBox_Trig2OutRDly->setValue(risingDelay);
        emit on_spinBox_Trig2OutRDly_valueChanged(risingDelay);
    }
#ifdef DEBUG_LOG_EN
    else
    {
        qDebug() << "DLPC350_GetTrigOutConfig() FAIL";
    }
#endif

    // Get trigger 1 input delay
	if (m_dlpAPI->DLPC350_GetTrigIn1Delay(&delay) == 0)
    {
        ui->spinBox_TrigIn1->setValue(delay);
        emit on_spinBox_TrigIn1_valueChanged(delay);
    }
#ifdef DEBUG_LOG_EN
    else
    {
        qDebug() << "DLPC350_GetTrigIn1Delay() FAIL";
    }
#endif

    //get trigger 2 input polarity
	if (m_dlpAPI->DLPC350_GetTrigIn2Pol(&isFallingEdge) == 0)
    {
        if(isFallingEdge == true)
            ui->comboBox_TrigIn2Pol->setCurrentIndex(1);
        else
            ui->comboBox_TrigIn2Pol->setCurrentIndex(0);
    }
#ifdef DEBUG_LOG_EN
    else
    {
        qDebug() << "DLPC350_GetTrigIn2Pol() FAIL";
    }
#endif

}

void MainWindow::on_spinBox_TrigIn1_valueChanged(int arg1)
{
    char displayStr[255];
    float val;

    val = (arg1)*107.136;

    sprintf(displayStr, "%2.2f us", val/1000);
    ui->label_TrigIn1Value->setText(displayStr);
    ui->horizontalSlider_TrigIn1->setValue(arg1);
}

void MainWindow::on_spinBox_Trig1OutRDly_valueChanged(int arg1)
{
    char displayStr[255];
    float val;

    val = (arg1-187)*107.2;

    sprintf(displayStr, "%2.2f us", val/1000);
    ui->label_Trig1OutRDlyVal->setText(displayStr);
    ui->horizontalSlider_Trig1OutRDly->setValue(arg1);
}

void MainWindow::on_spinBox_Trig1OutFDly_valueChanged(int arg1)
{
    char displayStr[255];
    float val;

    val = (arg1-187)*107.2;

    sprintf(displayStr, "%2.2f us", val/1000);
    ui->label_Trig1OutFDlyVal->setText(displayStr);
    ui->horizontalSlider_Trig1OutFDly->setValue(arg1);
}

void MainWindow::on_spinBox_Trig2OutRDly_valueChanged(int arg1)
{
    char displayStr[255];
    float val;

    val = (arg1-187)*107.2;

    sprintf(displayStr, "%2.2f us", val/1000);
    ui->label_Trig2OutRDlyVal->setText(displayStr);
    ui->horizontalSlider_Trig2OutRDly->setValue(arg1);
}

void MainWindow::on_horizontalSlider_TrigIn1_valueChanged(int value)
{
    ui->spinBox_TrigIn1->setValue(value);
}

void MainWindow::on_horizontalSlider_Trig1OutRDly_valueChanged(int value)
{
    ui->spinBox_Trig1OutRDly->setValue(value);
}

void MainWindow::on_horizontalSlider_Trig1OutFDly_valueChanged(int value)
{
    ui->spinBox_Trig1OutFDly->setValue(value);
}

void MainWindow::on_horizontalSlider_Trig2OutRDly_valueChanged(int value)
{
    ui->spinBox_Trig2OutRDly->setValue(value);
}

/* LED delay Control */
void MainWindow::on_spinBox_LedDlyCtrlRedREdgeDly_valueChanged(int arg1)
{
    char displayStr[255];
    float val;
    ui->horizontalSlider_LedDlyCtrlRedREdgeDly->setValue(arg1);

    val = (arg1-187)*107.2;

    sprintf(displayStr, "%2.2f us", val/1000);
    ui->label_LedDlyCtrlRedREdgeDlyVal->setText(displayStr);
}

void MainWindow::on_horizontalSlider_LedDlyCtrlRedREdgeDly_valueChanged(int value)
{
    ui->spinBox_LedDlyCtrlRedREdgeDly->setValue(value);
}

void MainWindow::on_spinBox_LedDlyCtrlRedFEdgeDly_valueChanged(int arg1)
{
    char displayStr[255];
    float val;
    ui->horizontalSlider_LedDlyCtrlRedFEdgeDly->setValue(arg1);

    val = (arg1-187)*107.2;

    sprintf(displayStr, "%2.2f us", val/1000);
    ui->label_LedDlyCtrlRedFEdgeDlyVal->setText(displayStr);
}

void MainWindow::on_horizontalSlider_LedDlyCtrlRedFEdgeDly_valueChanged(int value)
{
    ui->spinBox_LedDlyCtrlRedFEdgeDly->setValue(value);
}

void MainWindow::on_spinBox_LedDlyCtrlGreenREdgeDly_valueChanged(int arg1)
{
    char displayStr[255];
    float val;
    ui->horizontalSlider_LedDlyCtrlGreenREdgeDly->setValue(arg1);

    val = (arg1-187)*107.2;

    sprintf(displayStr, "%2.2f us", val/1000);
    ui->label_LedDlyCtrlGreenREdgeDlyVal->setText(displayStr);
}

void MainWindow::on_horizontalSlider_LedDlyCtrlGreenREdgeDly_valueChanged(int value)
{
    ui->spinBox_LedDlyCtrlGreenREdgeDly->setValue(value);
}

void MainWindow::on_spinBox_LedDlyCtrlGreenFEdgeDly_valueChanged(int arg1)
{
    char displayStr[255];
    float val;
    ui->horizontalSlider_LedDlyCtrlGreenFEdgeDly->setValue(arg1);

    val = (arg1-187)*107.2;

    sprintf(displayStr, "%2.2f us", val/1000);
    ui->label_LedDlyCtrlGreenFEdgeDlyVal->setText(displayStr);
}

void MainWindow::on_horizontalSlider_LedDlyCtrlGreenFEdgeDly_valueChanged(int value)
{
    ui->spinBox_LedDlyCtrlGreenFEdgeDly->setValue(value);
}

void MainWindow::on_spinBox_LedDlyCtrlBlueREdgeDly_valueChanged(int arg1)
{
    char displayStr[255];
    float val;
    ui->horizontalSlider_LedDlyCtrlBlueREdgeDly->setValue(arg1);

    val = (arg1-187)*107.2;

    sprintf(displayStr, "%2.2f us", val/1000);
    ui->label_LedDlyCtrlBlueREdgeDlyVal->setText(displayStr);
}

void MainWindow::on_horizontalSlider_LedDlyCtrlBlueREdgeDly_valueChanged(int value)
{
    ui->spinBox_LedDlyCtrlBlueREdgeDly->setValue(value);
}

void MainWindow::on_spinBox_LedDlyCtrlBlueFEdgeDly_valueChanged(int arg1)
{
    char displayStr[255];
    float val;
    ui->horizontalSlider_LedDlyCtrlBlueFEdgeDly->setValue(arg1);

    val = (arg1-187)*107.2;

    sprintf(displayStr, "%2.2f us", val/1000);
    ui->label_LedDlyCtrlBlueFEdgeDlyVal->setText(displayStr);
}

void MainWindow::on_horizontalSlider_LedDlyCtrlBlueFEdgeDly_valueChanged(int value)
{
    ui->spinBox_LedDlyCtrlBlueFEdgeDly->setValue(value);
}

void MainWindow::on_pushButton_GetLEDDlyCtrlConfig_clicked()
{
    unsigned char rising, falling;

	if (m_dlpAPI->DLPC350_GetRedLEDStrobeDelay(&rising, &falling) == 0)
    {
        ui->spinBox_LedDlyCtrlRedREdgeDly->setValue(rising);
        ui->spinBox_LedDlyCtrlRedFEdgeDly->setValue(falling);
    }
	if (m_dlpAPI->DLPC350_GetGreenLEDStrobeDelay(&rising, &falling) == 0)
    {
        ui->spinBox_LedDlyCtrlGreenREdgeDly->setValue(rising);
        ui->spinBox_LedDlyCtrlGreenFEdgeDly->setValue(falling);
    }
	if (m_dlpAPI->DLPC350_GetBlueLEDStrobeDelay(&rising, &falling) == 0)
    {
        ui->spinBox_LedDlyCtrlBlueREdgeDly->setValue(rising);
        ui->spinBox_LedDlyCtrlBlueFEdgeDly->setValue(falling);
    }

}

void MainWindow::on_pushButton_SetLedDlyCtrlConfig_clicked()
{
	m_dlpAPI->DLPC350_SetRedLEDStrobeDelay(ui->spinBox_LedDlyCtrlRedREdgeDly->value(), ui->spinBox_LedDlyCtrlRedFEdgeDly->value());
	m_dlpAPI->DLPC350_SetGreenLEDStrobeDelay(ui->spinBox_LedDlyCtrlGreenREdgeDly->value(), ui->spinBox_LedDlyCtrlGreenFEdgeDly->value());
	m_dlpAPI->DLPC350_SetBlueLEDStrobeDelay(ui->spinBox_LedDlyCtrlBlueREdgeDly->value(), ui->spinBox_LedDlyCtrlBlueFEdgeDly->value());
}

/* Peripheral Control */

void MainWindow::on_pushButton_SetPWMConfig_clicked()
{
    unsigned int channel;

    if(ui->comboBox_PWMChannel->currentIndex() != 0)
        channel = 2;
    else
        channel = 0;

	m_dlpAPI->DLPC350_SetPWMConfig(channel, ui->lineEdit_PWMPeriod->text().toUInt(), ui->spinBox_PWMDutyCycle->value());
	m_dlpAPI->DLPC350_SetPWMEnable(channel, ui->checkBox_PWMEnable->isChecked());
}

void MainWindow::on_pushButton_GetPWMConfig_clicked()
{
    unsigned int channel, pulsePeriod, dutyCycle;
    bool enable;

    if(ui->comboBox_PWMChannel->currentIndex() != 0)
        channel = 2;
    else
        channel = 0;

	if (m_dlpAPI->DLPC350_GetPWMConfig(channel, &pulsePeriod, &dutyCycle) == 0)
    {
        ui->lineEdit_PWMPeriod->setText(numToStr(pulsePeriod));
        ui->spinBox_PWMDutyCycle->setValue(dutyCycle);
    }
	if (m_dlpAPI->DLPC350_GetPWMEnable(channel, &enable) == 0)
        ui->checkBox_PWMEnable->setChecked(enable);
}

void MainWindow::on_pushButton_SetPWMCapConfig_clicked()
{
	m_dlpAPI->DLPC350_SetPWMCaptureConfig(ui->comboBox_PWMCapChannel->currentIndex(), ui->checkBox_PWMCapEnable->isChecked(), ui->lineEdit_PWMCapSampleRate->text().toUInt());
}

void MainWindow::on_pushButton_GetPWMCapConfig_clicked()
{
    bool enabled;
    unsigned int sampleRate;

	if (m_dlpAPI->DLPC350_GetPWMCaptureConfig(ui->comboBox_PWMCapChannel->currentIndex(), &enabled, &sampleRate) == 0)
    {
        ui->checkBox_PWMCapEnable->setChecked(enabled);
        ui->lineEdit_PWMCapSampleRate->setText(numToStr(sampleRate));
    }
}

void MainWindow::on_pushButton_PWMCapRead_clicked()
{
    unsigned int lowPeriod, highPeriod, dutyCycle;

	if (m_dlpAPI->DLPC350_PWMCaptureRead(ui->comboBox_PWMCapChannel->currentIndex(), \
                              &lowPeriod, &highPeriod) == 0)
    {
        if((highPeriod + lowPeriod) == 0)
            dutyCycle = 0;
        else
            dutyCycle = highPeriod*100/(highPeriod + lowPeriod);
        ui->lineEdit_PWMCapDutyCycleReadback->setText(numToStr(dutyCycle));
    }
}

void MainWindow::on_pushBox_SetGPIOConfig_clicked()
{
    bool alternativeMode = false;

    switch ( ui->comboBox_GPIOPin->currentIndex() )
    {
    /* LCR functions, it will just return, no modification allowed */
    case 1  :
    case 3  :
    case 4  :
    case 7  :
    case 8  :
    case 9  :
    case 10 :
    case 16 :
    case 17 :
    case 18 :
    case 19 :
    case 22 :
    case 23 :
    case 26 :
    case 31 :
    case 32 :
    case 37 :
        ShowError("Modification not permitted on this pin");
        return;
        /* only GPIO functions , if asking for alternate, return */
    case 13:
    case 14:
    case 15:
    case 20:
    case 21:
    case 24:
    case 25:
    case 27:
    case 28:
    case 29:
    case 30:
    case 33:
    case 34:
    case 35:
    case 36:
        if (  ui->checkBox_GPIOEnAltFun->isChecked() == true )
        {
            ShowError("This pin can be used as GPIO Only");
            return;
        }
        /* only alternate function = 0 is possible */
    case 0:
    case 2:
    case 5:
    case 6:
    case 11:
    case 12:
        alternativeMode = false;
        break;
    default:
        break;
    }
	m_dlpAPI->DLPC350_SetGPIOConfig(ui->comboBox_GPIOPin->currentIndex(), \
                          ui->checkBox_GPIOEnAltFun->isChecked(), \
                          alternativeMode, \
                          ui->comboBox_GPIOPinDir->currentIndex(),\
                          ui->comboBox_GPIOOutType->currentIndex(),\
                          ui->comboBox_GPIOPinState->currentIndex());
}

void MainWindow::on_pushBox_GetGPIOConfig_clicked()
{
    bool enAltFunc, altFunc1, dirOutput, outTypeOpenDrain, state;

	if (m_dlpAPI->DLPC350_GetGPIOConfig(ui->comboBox_GPIOPin->currentIndex(), \
                             &enAltFunc, &altFunc1, &dirOutput, \
                             &outTypeOpenDrain, &state) == 0)
    {
        ui->checkBox_GPIOEnAltFun->setChecked(enAltFunc);
        ui->comboBox_GPIOPinDir->setCurrentIndex(dirOutput);
        ui->comboBox_GPIOOutType->setCurrentIndex(outTypeOpenDrain);
        ui->comboBox_GPIOPinState->setCurrentIndex(state);
    }
}

void MainWindow::on_spinBox_GpClk_valueChanged(int arg1)
{
    char textStr[64];

    sprintf(textStr, "%2.3f Mhz", 96.0/arg1);
    ui->label_GpClkFreqVal->setText(textStr);
}

void MainWindow::on_pushButton_SetGpClk_clicked()
{
	m_dlpAPI->DLPC350_SetGeneralPurposeClockOutFreq(ui->comboBox_GpClk->currentIndex() + 1, \
                                          ui->checkBox_GpClk->isChecked(), \
                                          ui->spinBox_GpClk->value());
}

void MainWindow::on_pushButton_GetGpClk_clicked()
{
    bool enabled;
    unsigned int divider;

	if (m_dlpAPI->DLPC350_GetGeneralPurposeClockOutFreq(\
                ui->comboBox_GpClk->currentIndex()+1, &enabled, &divider) == 0)
    {
        ui->checkBox_GpClk->setChecked(enabled);
        ui->spinBox_GpClk->setValue(divider);
    }
}

void MainWindow::on_checkBox_GPIOEnAltFun_toggled(bool checked)
{
    ui->comboBox_GPIOPinDir->setEnabled(!checked);
    ui->comboBox_GPIOPinState->setEnabled(!checked);
    ui->comboBox_GPIOOutType->setEnabled(!checked);
}

void MainWindow::on_pushButton_i2cWrite_clicked()
{
    QString data;
    QStringList datalist;
    unsigned char wdata[256];
    unsigned int numWriteBytes;
    bool ok;

    bool is7BitAddr = (ui->comboBox_i2cAddrMode->currentIndex() == 0) ? true : false;

    unsigned int i2cClk = ui->lineEdit_i2cClockRate->text().toULong(&ok);

    if(i2cClk < 18194 || i2cClk > 400000 || (ok == false))
    {
        ShowError("Please enter valid clk setting. Available range 18194 to 400000");
        return;
    }

    unsigned int devAddr = ui->lineEdit_i2cDevAddr->text().toULong(&ok,16);
    if(ok == false)
    {
        ShowError("Enter valid device address in Hex format");
        return;
    }

    if(devAddr&0x01)
    {
        if(is7BitAddr)
            ShowError("Enter device address in correct format: Address[b6:b0]; set Address = ([b6:b0] << 1). Note the lsb R/W bit will be appended");
        else
            ShowError("Enter device address in correct format: Address[b9:b0]; set Address = ([b9:b0] << 1). Note the lsb R/W bit will be appended");

        return;
    }

    data = ui->lineEdit_i2cWriteData->text();
    if(data.isEmpty() == false)
    {
        datalist = data.split(' ');
        numWriteBytes = datalist.length();
    }
    else
    {
        numWriteBytes = 0;
    }

    if(numWriteBytes == 0 || numWriteBytes > 256)
    {
        ShowError("Incorrect number of data bytes received. Valid range 1 to 256");
        return;
    }

    for(unsigned int i = 0; i < numWriteBytes; i++)
    {
        wdata[i] = datalist[i].toULong(&ok,16);
        if(ok == false)
        {
            ShowError("Enter Hex (0xXX) data only in the (Write Data) section ");
            return;
        }
    }

	if (m_dlpAPI->DLPC350_I2C0WriteData(is7BitAddr, i2cClk, devAddr, numWriteBytes, &wdata[0]) < 0)
    {
        ShowError("Failed to Write command over I2C0 port");
        return;
    }

    unsigned char status;
	if (m_dlpAPI->DLPC350_I2C0TranStat(&status) < 0)
        return;

    if(status)
    {
        //Put the error message
        char tmpBuf[128];
        switch(status)
        {
            case 0x01:
                sprintf(tmpBuf,"NO ACK received from Slave");
                break;
            case 0x02:
                sprintf(tmpBuf,"I2C bus arbitration lost");
                break;
            case 0x04:
                sprintf(tmpBuf,"I2C Write timeout error occured");
                break;
            case 0x08:
                sprintf(tmpBuf,"I2C Read timeout error occured");
                break;
            case 0x10:
                sprintf(tmpBuf,"I2C error -0x10 occured");
                break;
            case 0x20:
                sprintf(tmpBuf,"I2C Internal error occured");
                break;
            default:
                break;
        }
        ShowError(tmpBuf);
    }

    return;
}

void MainWindow::on_pushButton_i2cRead_clicked()
{
    QString data;
    QStringList datalist;
    QString str;
    char hexbuf[3];
    unsigned char wdata[256];
    unsigned char rdata[256];
    unsigned int numWriteBytes;
    unsigned int numReadBytes;
    bool ok;

    bool is7BitAddr = (ui->comboBox_i2cAddrMode->currentIndex() == 0) ? true : false;

    unsigned int i2cClk = ui->lineEdit_i2cClockRate->text().toULong(&ok);
    if(i2cClk < 18194 || i2cClk > 400000 || (ok == false))
    {
        ShowError("Please enter valid clk setting. Available range 18194 to 400000");
        return;
    }

    unsigned int devAddr = ui->lineEdit_i2cDevAddr->text().toULong(&ok,16);
    if(ok == false)
    {
        ShowError("Enter valid device address in Hex format");
        return;
    }

    if(devAddr&0x01)
    {
        if(is7BitAddr)
            ShowError("Enter device address in correct format: Address[b6:b0]; set Address = ([b6:b0] << 1). Note the lsb R/W bit will be appended");
        else
            ShowError("Enter device address in correct format: Address[b9:b0]; set Address = ([b9:b0] << 1). Note the lsb R/W bit will be appended");

        return;
    }

    data = ui->lineEdit_i2cWriteData->text();

    if(data.isEmpty() == false)
    {
        datalist = data.split(' ');
        numWriteBytes = datalist.length();
    }
    else
    {
        numWriteBytes = 0;
    }

    if(numWriteBytes > 256)
    {
        ShowError("Incorrect number of data bytes received. Valid range 1 to 256");
        return;
    }

    if(numWriteBytes > 0)
    {
        for(unsigned int i = 0; i < numWriteBytes; i++)
        {
            wdata[i] = datalist[i].toULong(&ok,16);
            if(ok == false)
            {
                ShowError("Enter Hex (0xXX) data only in the (Write Data) section ");
                return;
            }
        }
    }

    numReadBytes = ui->lineEdit_i2cBytesToRead->text().toULong();

    if(numReadBytes < 1 || numReadBytes > 256)
    {
        ShowError("Enter valid number of bytes to be Read. Valid range 1 to 256");
        return;
    }

	if (m_dlpAPI->DLPC350_I2C0ReadData(is7BitAddr, i2cClk, devAddr, numWriteBytes, numReadBytes, &wdata[0], &rdata[0]) < 0)
    {
        ShowError("Failed to Read response over I2C0 port");
        unsigned char status;
		if (m_dlpAPI->DLPC350_I2C0TranStat(&status) < 0)
            return;
        //Put error message
        char tmpBuf[128];
        switch(status)
        {
            case 0x01:
                sprintf(tmpBuf,"NO ACK received from Slave");
                break;
            case 0x02:
                sprintf(tmpBuf,"I2C bus arbitration lost");
                break;
            case 0x04:
                sprintf(tmpBuf,"I2C Write timeout error occured");
                break;
            case 0x08:
                sprintf(tmpBuf,"I2C Read timeout error occured");
                break;
            case 0x10:
                sprintf(tmpBuf,"I2C error -0x10 occured");
                break;
            case 0x20:
                sprintf(tmpBuf,"I2C Internal error occured");
                break;
            default:
                break;
        }
        ShowError(tmpBuf);
        return;
    }

    datalist.clear();
    for(unsigned int i = 0; i < numReadBytes; i++)
    {
        sprintf(hexbuf, "%02X", rdata[i]);
        str = QString(hexbuf);
        str = "0x" + str;
        datalist.append( str );
    }

    ui->lineEdit_i2cReadData->setText( datalist.join(" ") );

}

/* Create Images */

void MainWindow::SaveImage()
{
    BMP_Image_t splashImage;
    QFile outFile(m_outFileName);

	m_dlpBMPParser->BMP_InitImage(&splashImage, PTN_WIDTH, PTN_HEIGHT, 8 * BYTES_PER_PIXEL);
    if(outFile.open(QIODevice::ReadWrite))
    {
		m_dlpBMPParser->BMP_StoreImage(&splashImage, (BMP_DataFunc_t *)My_FileWrite, \
                       &outFile, (BMP_PixelFunc_t *)My_ImgeGet, NULL);
        outFile.close();
    }
    else
    {
        ShowError("File Save Error");
        return;
    }

    QImage img(PTN_WIDTH, PTN_HEIGHT, QImage::Format_RGB32);
    QSize imgSize(ui->label_CreaImgOutImgPreview->size());

    img.load(outFile.fileName());

    ui->label_CreaImgOutImgPreview->setPixmap(\
                QPixmap::fromImage(img).scaled(imgSize));
}

void MainWindow::on_pushButton_CreaImgSelectBMPFile_clicked()
{
    QString startPath;
    QString fileName;

    if(m_patternFile.isFile())
        startPath = m_patternFile.absolutePath();
    else
        startPath = m_ptnImagePath;

    fileName = QFileDialog::getOpenFileName(this,
                                            QString("Select Image(s) for Pattern : "),
                                            startPath,
                                            "*.bmp");

    m_patternFile.setFile(fileName);
    m_ptnImagePath = m_patternFile.absolutePath();

    ui->lineEdit_CreaImgInputBmpFileName->setText(m_patternFile.fileName());

    QImage img = QImage(fileName);
    if(img.width() != 912 || img.height() != 1140)
    {
        ShowError("Only images of resolution 912x1140 are supported");
        return;
    }
    QSize imgSize(ui->label_CreaImgInputBMPFileImgPreview->size());

    if(m_patternFile.isFile())
    {
        img.load(m_patternFile.absoluteFilePath());
    }

    ui->label_CreaImgInputBMPFileImgPreview->setPixmap(QPixmap::fromImage(img).scaled(imgSize));
}

void MainWindow::on_pushButton_CreaImgSelectDestFileName_clicked()
{

    m_outFileName = QFileDialog::getSaveFileName(this,
                                                 QString("Select File to Save the output bitmap : "),
                                                 m_outFileName,
                                                 "*.bmp");

    m_settings.setValue("outFileName", m_outFileName);
    ui->lineEdit_CreaImgOutputBmpFileName->setText(m_outFileName);
}

void MainWindow::on_pushButton_CreaImgClearOutFileContents_clicked()
{
    memset(g_pImageBuffer, 0, PTN_WIDTH*PTN_HEIGHT*BYTES_PER_PIXEL);
    emit SaveImage();
}

void MainWindow::on_pushButton_CreaImgAddToOutFile_clicked()
{
    QFile file(m_patternFile.absoluteFilePath());
    QDataStream in(&file);
    unsigned char writeParam;


    //Refer to DLPC350 Programmer's Guide Rev 'D', Table 2-29, Pattern Number Mapping
    if(((ui->comboBox_CreaImgFileBitDepth->currentIndex()+1) == 5) || ((ui->comboBox_CreaImgFileBitDepth->currentIndex()+1) == 7))
    {
        writeParam = (((ui->comboBox_CreaImgFileBitDepth->currentIndex()+1)*ui->comboBox_CreaImgFilePatPos->currentIndex()) + (ui->comboBox_CreaImgFilePatPos->currentIndex()+1)) & 0x1F;
    }
    else
    {
        writeParam = ((ui->comboBox_CreaImgFileBitDepth->currentIndex()+1)*ui->comboBox_CreaImgFilePatPos->currentIndex()) & 0x1F;
    }

    writeParam |= ((ui->comboBox_CreaImgFileBitDepth->currentIndex() & 7) << 5);

    file.open(QIODevice::ReadOnly);
	m_dlpBMPParser->BMP_ParseImage((BMP_DataFunc_t *)My_FileRead, &in, (BMP_PixelFunc_t *)My_ImgeDraw, &writeParam, ui->comboBox_CreaImgFileBitDepth->currentIndex() + 1);
    file.close();

    emit SaveImage();

}

void MainWindow::on_comboBox_CreaImgFileBitDepth_currentIndexChanged(int index)
{
    // Adjust comboBox_CreaImgFilePatPos
    ui->comboBox_CreaImgFilePatPos->clear();

    switch(index)
    {
    case 0:     // Bit Depth = 1
        ui->comboBox_CreaImgFilePatPos->addItem("[ G0 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ G1 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ G2 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ G3 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ G4 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ G5 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ G6 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ G7 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ R0 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ R1 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ R2 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ R3 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ R4 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ R5 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ R6 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ R7 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B0 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B1 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B2 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B3 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B4 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B5 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B6 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B7 ]");
        break;
    case 1:     // Bit Depth = 2
        ui->comboBox_CreaImgFilePatPos->addItem("[ G1 G0 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ G3 G2 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ G5 G4 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ G7 G6 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ R1 R0 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ R3 R2 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ R5 R4 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ R7 R6 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B1 B0 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B3 B2 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B5 B4 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B7 B6 ]");
        break;
    case 2:     // Bit Depth = 3
        ui->comboBox_CreaImgFilePatPos->addItem("[ G2 G1 G0 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ G5 G4 G3 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ R0 G7 G6 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ R3 R2 R1 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ R6 R5 R4 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B1 B0 R7 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B4 B3 B2 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B7 B6 B5 ]");
        break;
    case 3:     // Bit Depth = 4
        ui->comboBox_CreaImgFilePatPos->addItem("[ G3 G2 G1 G0 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ G7 G6 G5 G4 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ R3 R2 R1 R0 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ R7 R6 R5 R4 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B3 B2 B1 B0 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B7 B6 B5 B4 ]");
        break;
    case 4:     // Bit Depth = 5
        ui->comboBox_CreaImgFilePatPos->addItem("[ G5 G4 G3 G2 G1 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ R3 R2 R1 R0 G7 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B1 B0 R7 R6 R5 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B7 B6 B5 B4 B3 ]");
        break;
    case 5:     // Bit Depth = 6
        ui->comboBox_CreaImgFilePatPos->addItem("[ G5 G4 G3 G2 G1 G0 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ R3 R2 R1 R0 G7 G6 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B1 B0 R7 R6 R5 R4 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B7 B6 B5 B4 B3 B2 ]");
        break;
    case 6:     // Bit Depth = 7
        ui->comboBox_CreaImgFilePatPos->addItem("[ G7 G6 G5 G4 G3 G2 G1 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ R7 R6 R5 R4 R3 R2 R1 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B7 B6 B5 B4 B3 B2 B1 ]");
        break;
    case 7:     // Bit Depth = 8
        ui->comboBox_CreaImgFilePatPos->addItem("[ G7 G6 G5 G4 G3 G2 G1 G0 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ R7 R6 R5 R4 R3 R2 R1 R0 ]");
        ui->comboBox_CreaImgFilePatPos->addItem("[ B7 B6 B5 B4 B3 B2 B1 B0 ]");
        break;
    }
}

void MainWindow::on_lineEdit_CreaImgOutputBmpFileName_textEdited(const QString &arg1)
{
    m_settings.setValue("outFileName", arg1);
    m_outFileName = arg1;
}

/* Common - Firmware Build & Upload */

int MainWindow::GetSectorNum(unsigned int Addr)
{
    unsigned int i;
    for(i=0; i < g_FlashDevice.numSectors; i++)
    {
        if(g_FlashDevice.SectorArr[i] > Addr)
            break;
    }

    return (i==0)? 0 : (i-1);
}

/* Firmware Build */

bool MainWindow::ProcessFlashParamsLine(QString line)
{
    unsigned int MfgID, DevID, i;
    bool ok;

    line = line.trimmed();
    if(line.startsWith('/'))
        return false;

    if(line.isEmpty())
        return false;

    MfgID = line.section(',',1,1).trimmed().toUInt(&ok, 0);
    if(!ok)
        return false;
    DevID = line.section(',',3,3).trimmed().toUInt(&ok, 0);
    if(!ok)
        return false;

    if((MfgID == g_FlashDevice.Mfg_ID) && (DevID == g_FlashDevice.Dev_ID))
    {
        g_FlashDevice.Mfg = line.section(',', 0, 0).trimmed();
        g_FlashDevice.Dev = line.section(',', 2, 2).trimmed();
        g_FlashDevice.Size_MBit = line.section(',', 4, 4).trimmed().toUInt(&ok, 0);
        if(!ok)
            return false;
        g_FlashDevice.Type = line.section(',', 5, 5).trimmed().toUInt(&ok, 0);
        if(!ok)
            return false;
        g_FlashDevice.numSectors = line.section(',', 7, 7).trimmed().toUInt(&ok, 0);
        if(!ok)
            return false;

        for(i=0; i<g_FlashDevice.numSectors; i++)
        {
            g_FlashDevice.SectorArr[i] = line.section(',', 8+i, 8+i).trimmed().toUInt(&ok, 0);
            if(!ok)
                return false;
        }

        return true;
    }
    return false;
}

void MainWindow::on_pushButton_FWSelectFWBin_clicked()
{
    QString fileName;
    int count;

    fileName = QFileDialog::getOpenFileName(this,
                                            QString("Select Image to load"),
                                            m_firmwarePath,
                                            "*.img *.bin");

    count = ui->comboBox_FWSplashImageIndex->count();
    ui->comboBox_FWSplashImageIndex->setEnabled(false);
    ui->pushButton_FWAddSplashImage->setEnabled(false);
    ui->pushButton_FWRemoveSplashImage->setEnabled(false);
    ui->pushButton_FWChangeSplashImage->setEnabled(false);
    ui->pushButton_FWBuildNewFrmwImage->setEnabled(false);
    ui->pushButton_FWSelectIniFile->setEnabled(false);
    //    ui->pushButton_FWSplashImageUpload->setEnabled(true);
    ui->label_NewFWBuildPath->clear();
    ui->label_NewFWBuildPath->setEnabled(false);
    ui->label_FWPreviewSelImage->clear();
    ui->label_FWOrgSplashImageCount->setText(g_displayStr_splashImageCount);
    ui->label_FWNewSplashImageAddedCount->setText(g_displayStr_splashImageAddedCount + QString::number(0));
    ui->label_FWSplashImageRemovedCount->setText(g_displayStr_splashImageRemovedCount + QString::number(0));
    ui->label_FWAvaSplashImageCount->setText(g_displayStr_splashImageTotalCount);
    ui->progressBar_FWFileParsing->setValue(0);
    for (int i = 0; i < count - 1; i++)
        ui->comboBox_FWSplashImageIndex->removeItem(1);

    m_isPrevModeStandBy = false;
    m_splashImageAddIndex = 0;
    m_splashImageCount = 0;
    m_splashImageAdded = 0;
    m_splashImageRemoved = 0;
    if(!fileName.isEmpty())
    {
        QFile imgFileIn(fileName);
        int splash_count, actual_splash_count, ret;
        char displayStr[255];
        unsigned char *pByteArray;
        int fileLen;
        int i;
        ui->lineEdit_FWFileSelected->setText(fileName);
        QFileInfo firmwareFileInfo;
        firmwareFileInfo.setFile(fileName);
        m_firmwarePath = firmwareFileInfo.absolutePath();
        if(!imgFileIn.open(QIODevice::ReadWrite))
        {
            ShowError("Unable to open image file. Copy image file to a folder with Admin/read/write permission and try again\n");
            return;
        }
        for (i = 0; i < MAX_SPLASH_IMAGES; i++)
            m_addedSplashImages[i].clear();

        fileLen = imgFileIn.size();
        pByteArray = (unsigned char *)malloc(fileLen);

        if(pByteArray == NULL)
        {
            imgFileIn.close();
            ShowError("Memory alloc for file read failed");
            return;
        }
        imgFileIn.read((char *)pByteArray, fileLen);
        imgFileIn.close();

		ret = m_dlpFrm->DLPC350_Frmw_CopyAndVerifyImage(pByteArray, fileLen);
        if (ret)
        {
            switch(ret)
            {
            case ERROR_FRMW_FLASH_TABLE_SIGN_MISMATCH:
                ShowError("ERROR: Flash Table Signature doesn't match! Bad Firmware Image!\n");
                free(pByteArray);
                return;
            case ERROR_NO_MEM_FOR_MALLOC:
                ShowError("Fatal Error! System Run out of memory\n");
                free(pByteArray);
                return;
            default:
                break;
            }
        }

        free(pByteArray);

		if ((m_dlpFrm->DLPC350_Frmw_GetVersionNumber() & 0xFFFFFF) < RELEASE_FW_VERSION)
            ShowError("WARNING: Old version of Firmware detected.\n\nDownload the latest release from http://www.ti.com/tool/dlpr350.");

		splash_count = m_dlpFrm->DLPC350_Frmw_GetSplashCount();
        actual_splash_count = splash_count;
        if (splash_count < 0)
        {
            ShowError("Firmware file image header format not recognized\n");
        }
        else
        {
            ui->label_FWFileParseStatus->setText("Retrieving Pattern Images\n");

            for (i = 0; i < splash_count; i++)
            {
                char file_name[11];

                QApplication::processEvents(); //Update GUI

                ui->progressBar_FWFileParsing->setValue(i*100/(splash_count - 1));

                sprintf(file_name, "temp_%d.bmp", i);
                QFile outFile(file_name);

                if(outFile.open(QIODevice::ReadWrite))
                {
                    BMP_Image_t splashImage;

					ret = m_dlpFrm->DLPC350_Frmw_GetSpashImage(g_pImageBuffer, i);
                    if (ret)
                    {
                        outFile.close();
                        memset(g_pImageBuffer, 0, PTN_WIDTH*PTN_HEIGHT*BYTES_PER_PIXEL);
                        switch(ret)
                        {
                        case ERROR_NO_MEM_FOR_MALLOC:
                            ShowError("Fatal Error! System Run out of memory\n");
                            return;
                        case ERROR_NO_SPLASH_IMAGE:
                            actual_splash_count--;
                            continue;
                        default:
                            continue;
                        }
                    }
					m_dlpBMPParser->BMP_InitImage(&splashImage, PTN_WIDTH, PTN_HEIGHT, 8 * BYTES_PER_PIXEL);
					m_dlpBMPParser->BMP_StoreImage(&splashImage, (BMP_DataFunc_t *)My_FileWrite, &outFile, (BMP_PixelFunc_t *)My_ImgeGet, NULL);
                    outFile.close();
                    memset(g_pImageBuffer, 0, PTN_WIDTH*PTN_HEIGHT*BYTES_PER_PIXEL);
                    m_addedSplashImages[i] = outFile.fileName();
                    if (i)
                        ui->comboBox_FWSplashImageIndex->addItem(QString::number(i));
                }
                else
                    ShowError("Cannot save pattern image as .bmp to display. Try relocating GUI to a folder that has write permission");
            }

            m_splashImageAddIndex = actual_splash_count;
            m_splashImageCount = actual_splash_count;

            sprintf(displayStr, "%d", actual_splash_count);
            ui->label_FWOrgSplashImageCount->setText(g_displayStr_splashImageCount + displayStr);
            ui->label_FWAvaSplashImageCount->setText(g_displayStr_splashImageTotalCount + displayStr);
            ui->comboBox_FWSplashImageIndex->setEnabled(true);
            ui->pushButton_FWAddSplashImage->setEnabled(true);
            ui->pushButton_FWRemoveSplashImage->setEnabled(true);
            ui->pushButton_FWChangeSplashImage->setEnabled(true);
            ui->pushButton_FWBuildNewFrmwImage->setEnabled(true);
            ui->pushButton_FWSelectIniFile->setEnabled(true);
            //	    ui->pushButton_FWSplashImageUpload->setEnabled(true);
            ui->label_NewFWBuildPath->setEnabled(true);
            ui->comboBox_FWSplashImageIndex->setCurrentIndex(0);
            emit on_comboBox_FWSplashImageIndex_currentIndexChanged(0);
            ui->label_FWFileParseStatus->setText("Retrieved Pattern Images\n");
        }
    }
}

void MainWindow::on_pushButton_FWAddSplashImage_clicked()
{
    QString fileName;
    static QString filePath;

    if (m_splashImageAddIndex == MAX_SPLASH_IMAGES)
    {
        ShowError("Only 256 images can be added \n");
        return;
    }

    if (filePath == NULL)
        filePath = m_ptnImagePath;
    fileName = QFileDialog::getOpenFileName(this,
                                            QString("Select Splash Image"),
                                            filePath,
                                            "*.bmp");
    if(!fileName.isEmpty())
    {
        QFileInfo patternFileInfo;
        patternFileInfo.setFile(fileName);
        m_ptnImagePath = patternFileInfo.absolutePath();

        QImage img = QImage(fileName);
        if(img.width() != 912 || img.height() != 1140)
        {
            ShowError("Only images of resolution 912x1140 are supported");
            return;
        }
        m_addedSplashImages[m_splashImageAddIndex] = fileName;
        filePath = fileName;
        if (m_splashImageAddIndex)
            ui->comboBox_FWSplashImageIndex->addItem(QString::number(m_splashImageAddIndex));
        ui->comboBox_FWSplashImageIndex->setCurrentIndex(m_splashImageAddIndex);
        emit on_comboBox_FWSplashImageIndex_currentIndexChanged(m_splashImageAddIndex);
        m_splashImageAddIndex++;

        ui->label_FWAvaSplashImageCount->setText(g_displayStr_splashImageTotalCount + QString::number(++m_splashImageCount));
        ui->label_FWNewSplashImageAddedCount->setText(g_displayStr_splashImageAddedCount + QString::number(++m_splashImageAdded));
    }
    return;
}

void MainWindow::on_pushButton_FWRemoveSplashImage_clicked()
{
    int i;
    int index = ui->comboBox_FWSplashImageIndex->currentIndex();

    if (!m_splashImageAddIndex)
        return;

    m_addedSplashImages[index].clear();
    for(i = index; i < MAX_SPLASH_IMAGES - 1; i++)
    {
        if((m_addedSplashImages[i] == NULL) && (m_addedSplashImages[i + 1] == NULL))
            break;
        m_addedSplashImages[i] = m_addedSplashImages[i + 1];
    }

    emit on_comboBox_FWSplashImageIndex_currentIndexChanged(index);
    m_splashImageAddIndex--;
    if (m_splashImageAddIndex)
        ui->comboBox_FWSplashImageIndex->removeItem(m_splashImageAddIndex);
    ui->label_FWAvaSplashImageCount->setText(g_displayStr_splashImageTotalCount + QString::number(--m_splashImageCount));
    ui->label_FWSplashImageRemovedCount->setText(g_displayStr_splashImageRemovedCount + QString::number(++m_splashImageRemoved));

    return;
}

void MainWindow::on_comboBox_FWSplashImageIndex_currentIndexChanged(int index)
{
    if (m_addedSplashImages[index] == NULL)
    {
        ui->label_FWPreviewSelImage->clear();
        return;
    }

    QFile imgFile(m_addedSplashImages[index]);
    QImage img(PTN_WIDTH, PTN_HEIGHT, QImage::Format_RGB32);
    QSize imgSize(ui->label_FWPreviewSelImage->size());

    imgFile.open(QIODevice::ReadOnly);
    img.load(imgFile.fileName());

    ui->label_FWPreviewSelImage->setPixmap(QPixmap::fromImage(img).scaled(imgSize));
    return;
}

void MainWindow::on_pushButton_FWChangeSplashImage_clicked()
{
    QString fileName;
    static QString filePath;
    int index = ui->comboBox_FWSplashImageIndex->currentIndex();

    if (m_addedSplashImages[index] == NULL)
    {
        ShowError("Use Add Button to add patterns in sequence\n");
        return;
    }

    if (filePath == NULL)
        filePath = m_ptnImagePath;
    fileName = QFileDialog::getOpenFileName(this,
                                            QString("Select Image"),
                                            filePath,
                                            "*.bmp");

    if(!fileName.isEmpty())
    {
        QFileInfo patternFileInfo;
        patternFileInfo.setFile(fileName);
        m_ptnImagePath = patternFileInfo.absolutePath();

        QImage img = QImage(fileName);
        if(img.width() != 912 || img.height() != 1140)
        {
            ShowError("Only images of resolution 912x1140 are supported");
            return;
        }

        m_addedSplashImages[index] = fileName;
        filePath = fileName;
        emit on_comboBox_FWSplashImageIndex_currentIndexChanged(index);
    }
    return;
}

void MainWindow::on_pushButton_FWSelectIniFile_clicked()
{
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this,
                                            QString("Select .ini file"),
                                            m_firmwarePath,
                                            tr("ini files(*.ini)"));
    if(fileName.isEmpty())
        return;

    QFileInfo firmwareFileInfo;
    firmwareFileInfo.setFile(fileName);
    m_firmwarePath = firmwareFileInfo.absolutePath();

    ui->lineEdit_FWIniFileSelected->setText(fileName);
}

void MainWindow::on_pushButton_FWClearSelIniFile_clicked()
{
    ui->lineEdit_FWIniFileSelected->clear();
}

void MainWindow::on_pushButton_FWClearFWTag_clicked()
{
    ui->lineEdit_firmwareTagName->clear();
}

void MainWindow::on_radioButton_FWIllumSelColor_toggled(bool checked)
{
    if(checked)
    {
        ui->radioButton_FWIllumSelMono->setChecked(false);
        ui->checkBox_FWIllumSelMonoRedCh->setChecked(false);
        ui->checkBox_FWIllumSelMonoGreenCh->setChecked(false);
        ui->checkBox_FWIllumSelMonoBlueCh->setChecked(false);
        ui->checkBox_FWIllumSelMonoRedCh->setEnabled(false);
        ui->checkBox_FWIllumSelMonoGreenCh->setEnabled(false);
        ui->checkBox_FWIllumSelMonoBlueCh->setEnabled(false);

    }
}

void MainWindow::on_radioButton_FWIllumSelMono_toggled(bool checked)
{
    if(checked)
    {
        ui->radioButton_FWIllumSelColor->setChecked(false);
        ui->checkBox_FWIllumSelMonoRedCh->setEnabled(true);
        ui->checkBox_FWIllumSelMonoGreenCh->setEnabled(true);
        ui->checkBox_FWIllumSelMonoBlueCh->setEnabled(true);
    }
}

void MainWindow::on_checkBox_FWIllumSelMonoRedCh_toggled(bool checked)
{
    if(checked)
    {
        ui->checkBox_FWIllumSelMonoGreenCh->setChecked(false);
        ui->checkBox_FWIllumSelMonoBlueCh->setChecked(false);
    }
}

void MainWindow::on_checkBox_FWIllumSelMonoGreenCh_toggled(bool checked)
{
    if(checked)
    {
        ui->checkBox_FWIllumSelMonoRedCh->setChecked(false);
        ui->checkBox_FWIllumSelMonoBlueCh->setChecked(false);
    }
}

void MainWindow::on_checkBox_FWIllumSelMonoBlueCh_toggled(bool checked)
{
    if(checked)
    {
        ui->checkBox_FWIllumSelMonoRedCh->setChecked(false);
        ui->checkBox_FWIllumSelMonoGreenCh->setChecked(false);
    }
}

void MainWindow::on_pushButton_FWSplashImageUpload_clicked()
{
    int i, count = 0, startSector=0, lastSectorToErase = 0, dataLen = 0, dataLen_full = 0, bytesSent = 0;
    unsigned char *newSplashBuffer;
    uint32 newSplashBufferSize = 0;
    uint32 flashSkipSize;
    QFile flashParamFile(":/new/prefix1/Flash/FlashDeviceParameters.txt");
    unsigned short manID;
    unsigned long long devID;


    for(i = 0; i < MAX_SPLASH_IMAGES; i++)
    {
        if (m_addedSplashImages[i] != NULL)
            count++;
    }

	m_dlpFrm->DLPC350_Frmw_SPLASH_InitBuffer(count);

	if (m_dlpAPI->DLPC350_EnterProgrammingMode() < 0)
    {
        ShowError("Unable to enter Programming mode");
        return;
    }

    ui->label_FWFileParseStatus->setText("Waiting to enter programming mode");

    QTime waitEndTime = QTime::currentTime().addSecs(5);
    //If going from normal mode to bootloader mode, wait until connection closes; then wait again for reconnection
    while(ui->pushButton_Connect->isEnabled() == true)
    {
        QApplication::processEvents(); //Update the GUI timeout in case the target is already in bootloader mode.
        if(QTime::currentTime() > waitEndTime)
            break;
    }

    while(ui->pushButton_Connect->isEnabled() == false)
    {
        QApplication::processEvents(); //Update the GUI
    }

    m_usbPollTimer->stop();

	if (m_dlpAPI->DLPC350_GetFlashManID(&manID) < 0)
    {
        ShowError("Unable to read Flash Manufacturer ID");
        return;
    }
	if (m_dlpAPI->DLPC350_GetFlashDevID(&devID) < 0)
    {
        ShowError("Unable to read Flash Device ID");
        return;
    }

    devID &= 0xFFFF;

    g_FlashDevice.Mfg_ID = manID;
    g_FlashDevice.Dev_ID = devID;

    if (!flashParamFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ShowError("Unable to open FlashDeviceParameters.txt");
        return;
    }

    QTextStream in(&flashParamFile);
    bool found = false;
    while (!in.atEnd())
    {
        QString line = in.readLine();
        if(ProcessFlashParamsLine(line))
        {
            found = true;
            break;
        }
    }

    if(found == false)
    {
        char displayStr[255];

        sprintf(displayStr, "Unsupported Flash Device : Manufacturer ID = 0x%x & Device ID = 0x%llx", manID, devID);
        ShowError(displayStr);
        return;
    }

	m_dlpAPI->DLPC350_SetFlashType(g_FlashDevice.Type);

	flashSkipSize = m_dlpFrm->DLPC350_Frmw_GetSPlashFlashStartAddress() - FLASH_BASE_ADDRESS;
    startSector = GetSectorNum(flashSkipSize);

    if(flashSkipSize != g_FlashDevice.SectorArr[startSector])
    {
        unsigned char *flashTableSector_Buffer = (unsigned char *) malloc(128 * 1024);

        startSector++;
		m_dlpFrm->DLPC350_Frmw_UpdateFlashTableSplashAddress(flashTableSector_Buffer, g_FlashDevice.SectorArr[startSector]);
        ui->label_FWFileParseStatus->setText("Updating Flash Table");
        ui->progressBar_FWFileParsing->setValue(0);

		m_dlpAPI->DLPC350_SetFlashAddr(128 * 1024);
		m_dlpAPI->DLPC350_FlashSectorErase();
		m_dlpAPI->DLPC350_WaitForFlashReady();

        dataLen = 128 * 1024;
        dataLen_full = dataLen;

		m_dlpAPI->DLPC350_SetFlashAddr(128 * 1024);
		m_dlpAPI->DLPC350_SetUploadSize(dataLen);

        while(dataLen > 0)
        {
			bytesSent = m_dlpAPI->DLPC350_UploadData(flashTableSector_Buffer + dataLen_full - dataLen, dataLen);

            if(bytesSent < 0)
            {
                ShowError("Flash Table Data Download Failed");
                free(flashTableSector_Buffer);
                return;
            }
            /*        for(i=0; i<bytesSent; i++)
        {
            expectedChecksum += pByteArray[BLsize+dataLen_full-dataLen+i];
        } */

            dataLen -= bytesSent;
            ui->progressBar_FWFileParsing->setValue(((dataLen_full-dataLen)*100/dataLen_full));
            QApplication::processEvents(); //Update the GUI
        }

        ui->label_FWFileParseStatus->setText("Flash Table Updated");
		m_dlpAPI->DLPC350_WaitForFlashReady();
        flashSkipSize = g_FlashDevice.SectorArr[startSector] - FLASH_BASE_ADDRESS;
    }

    lastSectorToErase = GetSectorNum(newSplashBufferSize + flashSkipSize);

    if((newSplashBufferSize + flashSkipSize) == g_FlashDevice.SectorArr[lastSectorToErase]) //If perfectly aligned with last sector start addr, no need to erase last sector.
        lastSectorToErase -= 1;


    ui->progressBar_FWFileParsing->setValue(0);
    ui->label_FWFileParseStatus->setText("Erasing Flash Sectors to download images");

    for(i=startSector; i <= lastSectorToErase; i++)
    {
		m_dlpAPI->DLPC350_SetFlashAddr(g_FlashDevice.SectorArr[i]);
		m_dlpAPI->DLPC350_FlashSectorErase();
		m_dlpAPI->DLPC350_WaitForFlashReady();    //Wait for flash busy flag to go off
        ui->progressBar_FWFileParsing->setValue(i*100/lastSectorToErase);
        QApplication::processEvents(); //Update the GUI
    }

    ui->label_FWFileParseStatus->setText("Erasing Flash Sectors Complete");

    dataLen = dataLen_full = bytesSent = 0;

    for(i = 0; i < MAX_SPLASH_IMAGES; i++)
    {
        if (m_addedSplashImages[i] == NULL)
            continue;

        QFile imgFile(m_addedSplashImages[i]);
        unsigned char *pByteArray;
        uint8 compression;
        uint32 compSize;
        int fileLen, ret;

        imgFile.open(QIODevice::ReadOnly);
        fileLen = imgFile.size();
        pByteArray = (unsigned char *)malloc(fileLen);

        if(pByteArray == NULL)
        {
            imgFile.close();
            ShowError("Memory alloc for file read failed");
            return;
        }
        imgFile.read((char *)pByteArray, fileLen);
        imgFile.close();
        if (m_addedSplashImages[i].contains("_nocomp.bmp"))
            compression = SPLASH_UNCOMPRESSED;
        else if (m_addedSplashImages[i].contains("_rle.bmp"))
            compression = SPLASH_RLE_COMPRESSION;
        else if (m_addedSplashImages[i].contains("_4line.bmp"))
            compression = SPLASH_4LINE_COMPRESSION;
        else
            compression = SPLASH_NOCOMP_SPECIFIED;

		ret = m_dlpFrm->DLPC350_Frmw_SPLASH_AddSplash(pByteArray, &compression, &compSize);
        if (ret < 0)
            ShowError("Error in buiding flash image with specified images\n");
        free(pByteArray);
    }

	m_dlpFrm->DLPC350_Frmw_Get_NewSplashBuffer(&newSplashBuffer, &newSplashBufferSize);

    dataLen = newSplashBufferSize;
    dataLen_full = dataLen;

	m_dlpAPI->DLPC350_SetFlashAddr(g_FlashDevice.SectorArr[startSector]);
	m_dlpAPI->DLPC350_SetUploadSize(dataLen);

    ui->progressBar_FWFileParsing->setValue(0);
    ui->label_FWFileParseStatus->setText("Downloading Images");

    while(dataLen > 0)
    {
		bytesSent = m_dlpAPI->DLPC350_UploadData(newSplashBuffer + dataLen_full - dataLen, dataLen);

        if(bytesSent < 0)
        {
            ShowError("Image Data Download Failed");
            return;
        }

        dataLen -= bytesSent;
        ui->progressBar_FWFileParsing->setValue(((dataLen_full-dataLen)*100/dataLen_full));
        QApplication::processEvents(); //Update the GUI
    }

    ui->label_FWFileParseStatus->setText("Downloading Images Complete");
	m_dlpAPI->DLPC350_WaitForFlashReady();

	m_dlpAPI->DLPC350_ExitProgrammingMode();
    m_usbPollTimer->start();
}

void MainWindow::on_pushButton_FWBuildNewFrmwImage_clicked()
{
    int i, count = 0;
    QString fileName;
    unsigned char *newFrmwImage;
    uint32 newFrmwSize;

    fileName = QFileDialog::getSaveFileName(this,
                                            QString("Enter name of new Image to be built"),
                                            m_firmwarePath,
                                            "*.img *.bin");

    if(fileName.isEmpty())
        return;

    QFileInfo firmwareFileInfo;
    firmwareFileInfo.setFile(fileName);
    m_firmwarePath = firmwareFileInfo.absolutePath();

    /* Check if .ini file is selected and if yes build it into the new firmware */
    if (!ui->lineEdit_FWIniFileSelected->text().isEmpty())
    {
        QFile iniFile(ui->lineEdit_FWIniFileSelected->text());
        if(!iniFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            ShowError("Unable to open .ini file");
            return;
        }
        QTextStream in(&iniFile);

        QString firstIniToken;
        uint32 iniParams[MAX_VAR_EXP_PAT_LUT_ENTRIES*3];//For Variable Exposure there are 1824 entries with each entry taking 3 32-bit params
        int numIniParams, ret;
        QByteArray byteArray;
        char cFirstIniToken[128];
        char *pCh;

        while(!in.atEnd())
        {
            QString line = in.readLine();
            byteArray = line.toLocal8Bit();
            pCh = byteArray.data();

			if (!m_dlpFrm->DLPC350_Frmw_ParseIniLines(pCh))
            {
				m_dlpFrm->DLPC350_Frmw_GetCurrentIniLineParam(&cFirstIniToken[0], iniParams, &numIniParams);
                firstIniToken = QString(&cFirstIniToken[0]);
				ret = m_dlpFrm->DLPC350_Frmw_WriteApplConfigData(&cFirstIniToken[0], iniParams, numIniParams);
                if (ret)
                {
                    char errString[255];
                    sprintf(errString, "wrong token %s or wrong parameters for the token %s\n", firstIniToken.toLocal8Bit().constData(), firstIniToken.toLocal8Bit().constData());
                    ShowError(errString);
                }
            }
        }
    }

    /* Add firmare tag information */
    if(ui->lineEdit_firmwareTagName->text().isEmpty() || (ui->lineEdit_firmwareTagName->text().length() > 32))
    {
        ShowError("Enter valid firmware tag information; maximum length upto 32 ASCII characters allowed");
        return;
    }
    else
    {
        const char *pCh = ui->lineEdit_firmwareTagName->text().toUtf8().data();

        uint32 params[32];

        for(i = 0; i < ui->lineEdit_firmwareTagName->text().length(); i++)
        {
            params[i] = (pCh[i]&0xFF);

            if(i > 31)
                break;
        }

		if (m_dlpFrm->DLPC350_Frmw_WriteApplConfigData("DEFAULT.FIRMWARE_TAG", &params[0], i))
        {
            ShowError("Unable to add firmware tag information");
            return;
        }
    }

    /* Based on the user selection adjust illumination system RGB Color or Monochrome*/
    if(ui->radioButton_FWIllumSelMono->isChecked())
    {
        uint32 param;

        param = 0x01;

		if (m_dlpFrm->DLPC350_Frmw_WriteApplConfigData("DEFAULT.LED_ENABLE_MAN_MODE", &param, 1))
        {
            ShowError("Unable to add illumination configuration color/monochrome information");
            return;
        }

        if(ui->checkBox_FWIllumSelMonoRedCh->isChecked())
        {
			if (m_dlpFrm->DLPC350_Frmw_WriteApplConfigData("DEFAULT.MAN_ENABLE_RED_LED", &param, 1))
            {
                ShowError("Unable to add illumination configuration color/monochrome information");
                return;
            }

            param = 0x02;

        }

        if(ui->checkBox_FWIllumSelMonoGreenCh->isChecked())
        {
			if (m_dlpFrm->DLPC350_Frmw_WriteApplConfigData("DEFAULT.MAN_ENABLE_GRN_LED", &param, 1))
            {
                ShowError("Unable to add illumination configuration color/monochrome information");
                return;
            }

            param = 0x03;

        }

        if(ui->checkBox_FWIllumSelMonoBlueCh->isChecked())
        {
			if (m_dlpFrm->DLPC350_Frmw_WriteApplConfigData("DEFAULT.MAN_ENABLE_BLU_LED", &param, 1))
            {
                ShowError("Unable to add illumination configuration color/monochrome information");
                return;
            }

            param = 0x04;

        }

		if (m_dlpFrm->DLPC350_Frmw_WriteApplConfigData("MACHINE_DATA.COLORPROFILE_0_BRILLIANTCOLORLOOK", &param, 1))
        {
            ShowError("Unable to add illumination configuration color/monochrome information");
            return;
        }

    }
    else
    {
        uint32 param;

        param = 0x00;

		if (m_dlpFrm->DLPC350_Frmw_WriteApplConfigData("DEFAULT.LED_ENABLE_MAN_MODE", &param, 1))
        {
            ShowError("Unable to add illumination configuration color/monochrome information");
            return;
        }

		if (m_dlpFrm->DLPC350_Frmw_WriteApplConfigData("DEFAULT.MAN_ENABLE_RED_LED", &param, 1))
        {
            ShowError("Unable to add illumination configuration color/monochrome information");
            return;
        }

		if (m_dlpFrm->DLPC350_Frmw_WriteApplConfigData("DEFAULT.MAN_ENABLE_GRN_LED", &param, 1))
        {
            ShowError("Unable to add illumination configuration color/monochrome information");
            return;
        }

		if (m_dlpFrm->DLPC350_Frmw_WriteApplConfigData("DEFAULT.MAN_ENABLE_BLU_LED", &param, 1))
        {
            ShowError("Unable to add illumination configuration color/monochrome information");
            return;
        }

		if (m_dlpFrm->DLPC350_Frmw_WriteApplConfigData("MACHINE_DATA.COLORPROFILE_0_BRILLIANTCOLORLOOK", &param, 1))
        {
            ShowError("Unable to add illumination configuration color/monochrome information");
            return;
        }

    }

    for(i = 0; i < MAX_SPLASH_IMAGES; i++)
    {
        if (m_addedSplashImages[i] != NULL)
            count++;
    }

	m_dlpFrm->DLPC350_Frmw_SPLASH_InitBuffer(count);

    QFile logFile("Frmw-build.log");
    logFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&logFile);
    out << "Building Images from specified BMPs\n\n";
    for(i = 0; i < MAX_SPLASH_IMAGES; i++)
    {
        if (m_addedSplashImages[i] == NULL)
            continue;

        QFile imgFile(m_addedSplashImages[i]);
        unsigned char *pByteArray;
        uint8 compression;
        uint32 compSize;
        int fileLen, ret;
        char dbgStr[1024];
        QFileInfo imageFileInfo;
        imageFileInfo.setFile(imgFile.fileName());

        imgFile.open(QIODevice::ReadOnly);
        fileLen = imgFile.size();
        pByteArray = (unsigned char *)malloc(fileLen);

        if(pByteArray == NULL)
        {
            imgFile.close();
            ShowError("Memory alloc for file read failed");
            return;
        }
        imgFile.read((char *)pByteArray, fileLen);
        imgFile.close();

        out << m_addedSplashImages[i] << "\n";
        out << "\t" << "Uncompressed Size = " << fileLen << " Compression type : ";

        if (m_addedSplashImages[i].contains("_nocomp.bmp"))
            compression = SPLASH_UNCOMPRESSED;
        else if (m_addedSplashImages[i].contains("_rle.bmp"))
            compression = SPLASH_RLE_COMPRESSION;
        else if (m_addedSplashImages[i].contains("_4line.bmp"))
            compression = SPLASH_4LINE_COMPRESSION;
        else
            compression = SPLASH_NOCOMP_SPECIFIED;

		ret = m_dlpFrm->DLPC350_Frmw_SPLASH_AddSplash(pByteArray, &compression, &compSize);
        if (ret < 0)
        {
            switch(ret)
            {
            case ERROR_NOT_BMP_FILE:
                sprintf(dbgStr, "Error building firmware - %s not in BMP format", qPrintable(imageFileInfo.fileName()));
                break;
            case ERROR_NOT_24bit_BMP_FILE:
                sprintf(dbgStr, "Error building firmware - %s not in 24-bit format", qPrintable(imageFileInfo.fileName()));
                break;
            case ERROR_NO_MEM_FOR_MALLOC:
                sprintf(dbgStr, "Error building firmware with %s - Insufficient memory", qPrintable(imageFileInfo.fileName()));
                break;
            default:
                sprintf(dbgStr, "Error building firmware with %s - error code %d", qPrintable(imageFileInfo.fileName()), ret);
                break;
            }
            ShowError(dbgStr);
            return;
        }

        switch(compression)
        {
        case SPLASH_UNCOMPRESSED:
            out << "Uncompressed";
            break;
        case SPLASH_RLE_COMPRESSION:
            out << "RLE Compression";
            break;
        case SPLASH_4LINE_COMPRESSION:
            out << "4 Line Compression";
            break;
        default:
            break;
        }
        out << " Compressed Size = " << compSize << "\n\n";

        free(pByteArray);
    }

    logFile.close();
	m_dlpFrm->DLPC350_Frmw_Get_NewFlashImage(&newFrmwImage, &newFrmwSize);

    QFile outFile(fileName);
    if(outFile.open(QIODevice::ReadWrite))
    {
        if(outFile.write((char*)newFrmwImage, newFrmwSize) <= 0)
            ShowError("Error in writing to output file\n");
        outFile.close();
    }
    else
        ShowError("Cannot Open output file for writing\n");

    ui->label_NewFWBuildPath->setText(fileName);
    ShowError("Build Complete\n");
}

/* Firmware Upload Function */
void MainWindow::on_pushButton_FWFileSelect_clicked()
{
    QString fileName;

    fileName = QFileDialog::getOpenFileName(this,
                                            QString("Select Image to load"),
                                            m_firmwarePath,
                                            "*.img *.bin");

    ui->pushButton_FWUpload->setEnabled(false);

    if(!fileName.isEmpty())
    {
        ui->FirmwareFile_2->setText(fileName);
        QFileInfo firmwareFileInfo;
        firmwareFileInfo.setFile(fileName);
        m_firmwarePath = firmwareFileInfo.absolutePath();
    }
    ui->pushButton_FWUpload->setEnabled(true);
}

void MainWindow::on_pushButton_FWUpload_clicked()
{
    QFile imgFile(ui->FirmwareFile_2->text());
    QFile flashParamFile(":/new/prefix1/Flash/FlashDeviceParameters.txt");
    unsigned short manID;
    unsigned long long devID;
    char displayStr[255];
    int startSector=0, i, BLsize=0, lastSectorToErase;
    unsigned char *pByteArray=NULL;
    long long dataLen, dataLen_full;
    int bytesSent;
    unsigned int expectedChecksum=0, checksum;
    long long percent_completion = 0;

    if(!imgFile.open(QIODevice::ReadOnly))
    {
        ShowError("Unable to open image file. Copy image file to a folder with Admin/read/write permission and try again\n");
        return;
    }

    //
    //1. Parse the supplied binary image for validity
    //
    dataLen = imgFile.size();
    pByteArray = (unsigned char *)malloc(dataLen);
    if(pByteArray == NULL)
    {
        imgFile.close();
        ShowError("Unable to get enough memory to read the firmware image on the system");
        return;
    }
    
	imgFile.read((char *)pByteArray, dataLen);

	int ret = m_dlpFrm->DLPC350_Frmw_CopyAndVerifyImage(pByteArray, dataLen);
    if (ret)
    {
        switch(ret)
        {
        case ERROR_FRMW_FLASH_TABLE_SIGN_MISMATCH:
            ShowError("ERROR: Flash Table Signature doesn't match! Bad Firmware Image!\n");
            free(pByteArray);
            imgFile.close();
            return;
        case ERROR_NO_MEM_FOR_MALLOC:
            ShowError("Fatal Error! System Run out of memory\n");
            free(pByteArray);
            imgFile.close();
            return;
        default:
            break;
        }
    }

    //
    //2. Check if user selected to update bootloader code
    //

    if(ui->checkBox_SkipBootLoader->isChecked() == false)
    {
        QString text("You have selected to update Bootloader code. Select [Yes] to update Bootloader and [No] to skip update.");
        QMessageBox msgBox(QMessageBox::Warning, "LightCrafter Message", text, QMessageBox::Yes|QMessageBox::No, this);
        if(msgBox.exec() == QMessageBox::Yes)
            ui->checkBox_SkipBootLoader->setChecked(false);
        else
            ui->checkBox_SkipBootLoader->setChecked(true);
    }

    //
    //3. Enter programming mode
    //
	if (m_dlpAPI->DLPC350_EnterProgrammingMode() < 0)
    {
        ShowError("Unable to enter Programming mode");
        return;
    }

    ui->label_FWUploadProgressBar->setText("Waiting to enter programming mode");

    QTime waitEndTime = QTime::currentTime().addSecs(5);
    //If going from normal mode to bootloader mode, wait until connection closes; then wait again for reconnection
    while(ui->pushButton_Connect->isEnabled() == true)
    {
        QApplication::processEvents(); //Update the GUI
        //timeout in case the target is already in bootloader mode.
        if(QTime::currentTime() > waitEndTime)
            break;
    }

    while(ui->pushButton_Connect->isEnabled() == false)
    {
        QApplication::processEvents(); //Update the GUI
    }

    m_usbPollTimer->stop();

    //
    //4. Read Flash information
    //
	if (m_dlpAPI->DLPC350_GetFlashManID(&manID) < 0)
    {
        ShowError("Unable to read Flash Manufacturer ID");
        return;
    }

	if (m_dlpAPI->DLPC350_GetFlashDevID(&devID) < 0)
    {
        ShowError("Unable to read Flash Device ID");
        return;
    }
    devID &= 0xFFFF;

    g_FlashDevice.Mfg_ID = manID;
    g_FlashDevice.Dev_ID = devID;

    //
    //5. Populate flash layout from FlashDeviceParameters for given g_FlashDevice Mfg_ID & Dev_ID
    //   

    if (!flashParamFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ShowError("Unable to open FlashDeviceParameters.txt");
        return;
    }

    QTextStream in(&flashParamFile);
    bool found = false;
    while (!in.atEnd())
    {
        QString line = in.readLine();
        if(ProcessFlashParamsLine(line))
        {
            found = true;
            break;
        }
    }

    if(found == false)
    {
        sprintf(displayStr, "Unsupported Flash Device : Manufacturer ID = 0x%x & Device ID = 0x%llx", manID, devID);
        ShowError(displayStr);
        return;
    }

    //
    //6. Program Flash
    //

    //DLPC350_BLSpecialMode(BIT0); //Skip Software Write Response from Bootloader

    if(ui->checkBox_SkipBootLoader->isChecked())
    {
        BLsize = 128 * 1024;
    }

    startSector = GetSectorNum(BLsize);
    lastSectorToErase = GetSectorNum(imgFile.size());
    if(imgFile.size() == g_FlashDevice.SectorArr[lastSectorToErase]) //If perfectly aligned with last sector start addr, no need to erase last sector.
        lastSectorToErase -= 1;

	m_dlpAPI->DLPC350_SetFlashType(g_FlashDevice.Type);
    ui->prorgessBar_FWUpload->setValue(0);
    ui->label_FWUploadProgressBar->setText("Erasing Flash Sectors");

    for(i=startSector; i <= lastSectorToErase; i++)
    {
		m_dlpAPI->DLPC350_SetFlashAddr(g_FlashDevice.SectorArr[i]);
		m_dlpAPI->DLPC350_FlashSectorErase();
		m_dlpAPI->DLPC350_WaitForFlashReady();    //Wait for flash busy flag to go off
        ui->prorgessBar_FWUpload->setValue(i*100/lastSectorToErase);
        QApplication::processEvents(); //Update the GUI
    }

    ui->label_FWUploadProgressBar->setText("Erasing Flash Sectors Complete");


    dataLen -= BLsize;

	m_dlpAPI->DLPC350_SetFlashAddr(BLsize);
	m_dlpAPI->DLPC350_SetUploadSize(dataLen);

    dataLen_full = dataLen;
    ui->prorgessBar_FWUpload->setValue(0);
    ui->label_FWUploadProgressBar->setText("Downloading Firmware Image");

    while(dataLen > 0)
    {
		bytesSent = m_dlpAPI->DLPC350_UploadData(pByteArray + BLsize + dataLen_full - dataLen, dataLen);

        if(bytesSent < 0)
        {
            ShowError("Flash Data Download Failed");
            imgFile.close();
            free(pByteArray);
            return;
        }
        for(i=0; i<bytesSent; i++)
        {
            expectedChecksum += pByteArray[BLsize+dataLen_full-dataLen+i];
        }

        dataLen -= bytesSent;
        if(percent_completion != (((dataLen_full-dataLen)*100)/dataLen_full))
        {
            percent_completion = (((dataLen_full-dataLen)*100)/dataLen_full);
            ui->prorgessBar_FWUpload->setValue(percent_completion);
        }
        QApplication::processEvents(); //Update the GUI
    }
    ui->label_FWUploadProgressBar->setText("Waiting for checksum verification");
    QApplication::processEvents(); //Update the GUI

    //
    //7. Compute checksum
    //

	m_dlpAPI->DLPC350_CalculateFlashChecksum();

#if 0
    /* More wait time for bigger files */
    if(imgFile.size() > 16*1024*1024)
        waitEndTime = QTime::currentTime().addSecs(6);
    else
        waitEndTime = QTime::currentTime().addSecs(3);

    while(true)
    {
        QApplication::processEvents(); //Update the GUI
        if(QTime::currentTime() > waitEndTime)
            break;
    }
#endif

	m_dlpAPI->DLPC350_WaitForFlashReady();

	if (m_dlpAPI->DLPC350_GetFlashChecksum(&checksum) < 0)
    {
        ShowError("Error reading checksum from target");
    }
    else  if(checksum != expectedChecksum)
    {
        sprintf(displayStr, "Checksum mismatch: Expected %x; Received %x", expectedChecksum, checksum);
        ShowError(displayStr);
    }
    else
    {
		m_dlpAPI->DLPC350_ExitProgrammingMode(); //Exit programming mode; Start application.
        ShowError("Download Complete");
    }

    ui->checkBox_SkipBootLoader->setChecked(true);
    ui->label_FWUploadProgressBar->setText("Download Complete");
    ui->prorgessBar_FWUpload->setValue(0);
    imgFile.close();
    free(pByteArray);
    m_usbPollTimer->start();

}

