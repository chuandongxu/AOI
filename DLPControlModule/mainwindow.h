/*
 * mainwindow.h
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#ifdef Q_OS_WIN32
#include <qt_windows.h>
#endif
#include <QListWidgetItem>
#include <QFileInfo>
#include <QThread>

#include "dlpc350_common.h"
#include "dlpc350_firmware.h"
#include "dlpc350_flashDevice.h"

namespace Ui {
class MainWindow;
}

class SleeperThread : public QThread
{
public:
    static void msleep(unsigned long msecs)
    {
        QThread::msleep(msecs);
    }
};

class DLPUsb;
class DLPApi;
class DLPFrmw;
class DLPBMPParser;
class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    void ApplyDefaultSolution();
    bool isConnected();
    bool isDataValiated();
    bool startUpCapture();
    bool endUpCapture();

    void setDLP(int nIndex);
    void setUSBSerialNB(std::string& serialNumber);
    std::string getUSBSerialNB();
    bool trigger();

private slots:

    void timerTimeout(void);
    void on_checkBox_updateStatus_toggled(bool checked);

    void on_pushButton_Connect_clicked();
    void on_pushButton_Reset_clicked();
    void on_radioButton_VideoMode_clicked();
    void on_radioButton_SLMode_clicked();
    void on_radioButton_VariableExpSLMode_clicked();
    void on_radioButton_StandbyMode_clicked();
    void on_pushButton_GetLEDConfiguration_clicked();
    void on_pushButton_SetLEDConfiguration_clicked();
    void on_pushButton_GetFlip_clicked();
    void on_pushButton_SetFlip_clicked();
    void on_pushButton_InitPatternSeq_clicked();
    void on_radioButton_ColorDisplayAuto_clicked();
    void on_radioButton_ColorDisplayManual_clicked();
    void on_pushButton_ApplySolution_clicked();
    void on_pushButton_SaveSolution_clicked();
    void on_pushButton_ApplyDefaultSolution_clicked();

    /* Video Port Settings */
    void on_pushButton_SetPortSource_clicked();
    void on_pushButton_GetPortSource_clicked();
    void on_comboBox_InputSourceList_currentIndexChanged(int index);
    void on_pushButton_SetPortSwap_clicked();
    void on_pushButton_GetPortSwap_clicked();
    void on_pushButton_SetPortPixelFormat_clicked();
    void on_pushButton_GetPortPixelFormat_clicked();
    void on_pushButton_SetPortClock_clicked();
    void on_pushButton_GetPortClock_clicked();
    void on_pushButton_SetFPDMode_clicked();
    void on_pushButton_GetFPDMode_clicked();

    /* Internal TPG Color */
    void on_pushButton_SetTPGColor_clicked();
    void on_pushButton_GetTPGColor_clicked();

    /* Display Function */
    void on_pushButton_SetDisplayConfiguration_clicked();
    void on_pushButton_GetDisplayConfiguration_clicked();

    /* Read Video Singal information */
    void on_pushButton_GetVideoSingalInfo_clicked();

    /* Sequence Settings */
    void on_radioButton_PatSeqSrcFrmFlash_clicked();
    void on_radioButton_PatSeqSrcFrmVideoPort_clicked();
    void on_radioButton_PatSeqTrigTypeIntExt_clicked();
    void on_radioButton_PatSeqTrigTypeVSync_clicked();
    void on_spinBox_PatSeqFrameImgIndex_valueChanged(int arg1);
    void on_comboBox_PatSeqPatBitDepthSel_currentIndexChanged(int index);
    void on_listWidget_PatSeqBitPlanes_itemClicked(QListWidgetItem *item);
    void on_pushButton_PatSeqAddPatToLut_clicked();
    void on_listWidget_PatSeqLUT_customContextMenuRequested(const QPoint &pos);
    void on_pushButton_PatSeqSendLUT_clicked();
    void on_pushButton_PatSeqReadLUTFrmHW_clicked();
    void on_pushButton_PatSeqClearLUTFrmGUI_clicked();

    /* Variable Exposure Sequence Settings */
    void on_radioButton_VarExpPatSeqSrcFrmFlash_clicked();
    void on_radioButton_VarExpPatSeqSrcFrmVideoPort_clicked();
    void on_radioButton_VarExpPatSeqTrigTypeIntExt_clicked();
    void on_radioButton_VarExpPatSeqTrigTypeVSync_clicked();
    void on_spinBox_VarExpPatSeqFrameImgIndex_valueChanged(int arg1);
    void on_comboBox_VarExpPatSeqPatBitDepthSel_currentIndexChanged(int index);
    void on_listWidget_VarExpPatSeqBitPlanes_itemClicked(QListWidgetItem *item);
    void on_pushButton_VarExpPatSeqAddPatToLut_clicked();
    void on_listWidget_VarExpPatSeqLUT_customContextMenuRequested(const QPoint &pos);
    void on_pushButton_VarExpPatSeqSendLUT_clicked();
    void on_pushButton_VarExpPatSeqReadLUTFrmHW_clicked();
    void on_pushButton_VarExpPatSeqClearLUTFrmGUI_clicked();

    /* Patter Sequence Play/Pause/Stop & Validate Control */
    void on_pushButton_ValidatePatSeq_clicked();
    void on_pushButton_PatSeqCtrlStart_clicked();
    void on_pushButton_PatSeqCtrlPause_clicked();
    void on_pushButton_PatSeqCtrlStop_clicked();
    void on_checkBox_PatSeqCtrlGlobalDataInvert_toggled(bool checked);

    /* Image Load timing information retrive */
    void on_pushButton_GetImgLoadTimingInfo_clicked();

    /* Trigger Input/Output Control */
    void on_pushButton_SetTrigConfig_clicked();
    void on_pushButton_GetTrigConfig_clicked();
    void on_spinBox_TrigIn1_valueChanged(int arg1);
    void on_spinBox_Trig1OutRDly_valueChanged(int arg1);
    void on_spinBox_Trig1OutFDly_valueChanged(int arg1);
    void on_spinBox_Trig2OutRDly_valueChanged(int arg1);
    void on_horizontalSlider_TrigIn1_valueChanged(int value);
    void on_horizontalSlider_Trig1OutRDly_valueChanged(int value);
    void on_horizontalSlider_Trig1OutFDly_valueChanged(int value);
    void on_horizontalSlider_Trig2OutRDly_valueChanged(int value);

    /* LED Delay Control */
    void on_spinBox_LedDlyCtrlRedREdgeDly_valueChanged(int arg1);
    void on_horizontalSlider_LedDlyCtrlRedREdgeDly_valueChanged(int value);
    void on_spinBox_LedDlyCtrlRedFEdgeDly_valueChanged(int arg1);
    void on_horizontalSlider_LedDlyCtrlRedFEdgeDly_valueChanged(int value);
    void on_spinBox_LedDlyCtrlGreenREdgeDly_valueChanged(int arg1);
    void on_horizontalSlider_LedDlyCtrlGreenREdgeDly_valueChanged(int value);
    void on_spinBox_LedDlyCtrlGreenFEdgeDly_valueChanged(int arg1);
    void on_horizontalSlider_LedDlyCtrlGreenFEdgeDly_valueChanged(int value);
    void on_spinBox_LedDlyCtrlBlueREdgeDly_valueChanged(int arg1);
    void on_horizontalSlider_LedDlyCtrlBlueREdgeDly_valueChanged(int value);
    void on_spinBox_LedDlyCtrlBlueFEdgeDly_valueChanged(int arg1);
    void on_horizontalSlider_LedDlyCtrlBlueFEdgeDly_valueChanged(int value);
    void on_pushButton_GetLEDDlyCtrlConfig_clicked();
    void on_pushButton_SetLedDlyCtrlConfig_clicked();

    /* Peripheral Control*/
    void on_pushButton_SetPWMConfig_clicked();
    void on_pushButton_GetPWMConfig_clicked();
    void on_pushButton_SetPWMCapConfig_clicked();
    void on_pushButton_GetPWMCapConfig_clicked();
    void on_pushButton_PWMCapRead_clicked();
    void on_pushBox_SetGPIOConfig_clicked();
    void on_pushBox_GetGPIOConfig_clicked();
    void on_spinBox_GpClk_valueChanged(int arg1);
    void on_pushButton_SetGpClk_clicked();
    void on_pushButton_GetGpClk_clicked();
    void on_checkBox_GPIOEnAltFun_toggled(bool checked);
    void on_pushButton_i2cWrite_clicked();
    void on_pushButton_i2cRead_clicked();

    /* Create Images */
    void on_pushButton_CreaImgSelectBMPFile_clicked();
    void on_pushButton_CreaImgSelectDestFileName_clicked();
    void on_pushButton_CreaImgClearOutFileContents_clicked();
    void on_pushButton_CreaImgAddToOutFile_clicked();
    void on_comboBox_CreaImgFileBitDepth_currentIndexChanged(int index);
    void on_lineEdit_CreaImgOutputBmpFileName_textEdited(const QString &arg1);

    /* Firmware Build */
    void on_pushButton_FWSelectFWBin_clicked();
    void on_pushButton_FWAddSplashImage_clicked();
    void on_pushButton_FWRemoveSplashImage_clicked();
    void on_comboBox_FWSplashImageIndex_currentIndexChanged(int index);
    void on_pushButton_FWChangeSplashImage_clicked();
    void on_pushButton_FWSelectIniFile_clicked();
    void on_pushButton_FWClearSelIniFile_clicked();
    void on_pushButton_FWClearFWTag_clicked();
    void on_radioButton_FWIllumSelColor_toggled(bool checked);
    void on_radioButton_FWIllumSelMono_toggled(bool checked);
    void on_checkBox_FWIllumSelMonoRedCh_toggled(bool checked);
    void on_checkBox_FWIllumSelMonoGreenCh_toggled(bool checked);
    void on_checkBox_FWIllumSelMonoBlueCh_toggled(bool checked);
    void on_pushButton_FWSplashImageUpload_clicked();
    void on_pushButton_FWBuildNewFrmwImage_clicked();

    /* Firmware Upload */
    void on_pushButton_FWFileSelect_clicked();
    void on_pushButton_FWUpload_clicked();

private:
    FlashDevice g_FlashDevice;
    int g_FrameIdx;
    int g_VarExpFrameIdx ;    
    QString g_displayStr_splashImageCount;
    QString g_displayStr_splashImageAddedCount;
    QString g_displayStr_splashImageRemovedCount;
    QString g_displayStr_splashImageTotalCount;
    QStringList g_iniGUITokens;

private:
    Ui::MainWindow *ui;

    DLPUsb* m_dlpUSB;
    DLPApi* m_dlpAPI;
    DLPFrmw* m_dlpFrm;
    DLPBMPParser* m_dlpBMPParser;

    QSettings m_settings;
    QString m_firmwarePath;
    QString m_ptnImagePath;
    QString m_outFileName;
    QString m_csvFileName;
    QFileInfo m_patternFile;
    QTimer *m_usbPollTimer;
    QString m_addedSplashImages[MAX_SPLASH_IMAGES];
    bool m_isPrevModeStandBy;
    int m_splashImageAddIndex;
    int m_splashImageCount;
    int m_splashImageAdded;
    int m_splashImageRemoved;
    int m_numExtraSplashLutEntries;
    int m_extraSplashLutEntries[64];
    unsigned int m_numImgInFlash;
    bool m_bApplyDefaultSolution;
    bool m_bConnected;
    bool m_bPatSeqValidated;
    std::string m_usbSerialNumber;
    int m_nDLPIndex;

    void ShowError(const char *str);
    int GetSectorNum(unsigned int Addr);
    bool ProcessFlashParamsLine(QString line);
    void SaveImage();
    void ApplyIniParam(QString token, uint32 *params, int numParams);
    void RefreshGUISettingsFromDLPC350();
    void ApplyGUISettingToDLPC350();

    /* Pat Sequence */
    QColor GetColorFromIndex(int index);
    void UpdateFrameTitle(bool isFrame);
    int InsertTriggerItem(int TrigType);
    void RemoveSelectedItemFromSeqList();
    void CombineItemWithPrev(unsigned int itemRow);
    QString GenerateItemText(int frameIndex, int bitDepth, int firstItem, \
                             int lastItem, bool invert);
    void UpdateSeqItemData(int TrigType, int PatNum,int BitDepth, \
                           int LEDSelect,int frameIndex, bool InvertPat, \
                           bool InsertBlack,bool BufSwap, bool trigOutPrev);
    /* Variable Exposure Pat Sequence */
    void VarExpUpdateFrameTitle(bool isFrame);
    int VarExpInsertTriggerItem(int trig_type);
    void VarExpRemoveSelectedItemFromSeqList();
    void VarExpCombineItemWithPrev(unsigned int itemRow);
    QString VarExpGenerateItemText(int frame_Index, int bitDepth, \
                                   int firstItem, int lastItem, bool invert, \
                                   int patExp, int patPeriod);
    void VarExpUpdateSeqItemData(int TrigType, int PatNum,int BitDepth, \
                                 int LEDSelect,int frameIndex, \
                                 bool InvertPat, bool InsertBlack, \
                                 bool BufSwap, bool trigOutPrev, \
                                 int PatExp, int PatPrd);

    /* Read DLPC350 Status */
    int GetDLPC350Status();
    /* DLPC350 group calls */
    void SetDLPC350InVideoMode();
    void SetDLPC350InPatternMode();
};


#endif // MAINWINDOW_H
