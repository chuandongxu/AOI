#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>

#include "InspLeadWidget.h"
#include "../Common/SystemData.h"
#include "DataStoreAPI.h"
#include "../include/IVisionUI.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../Common/CommonFunc.h"
#include "../DataModule/QDetectObj.h"
#include "InspWindowWidget.h"
#include "../DataModule/CalcUtils.hpp"

using namespace NFG::AOI;

enum BASIC_PARAM
{
    PAD_RECORD_ID,
    LEAD_RECORD_ID,
    PAD_LEAD_DIST,
    MAX_OFFSET_X,
    MAX_OFFSET_Y,
};

InspLeadWidget::InspLeadWidget(InspWindowWidget *parent)
    : EditInspWindowBaseWidget(parent)
{
    ui.setupUi(this);

    m_pEditPadRecordID = std::make_unique<QLineEdit>(ui.tableWidget);
    ui.tableWidget->setCellWidget(PAD_RECORD_ID, DATA_COLUMN, m_pEditPadRecordID.get());
    m_pEditPadRecordID->setEnabled(false);

    m_pEditLeadRecordID = std::make_unique<QLineEdit>(ui.tableWidget);
    ui.tableWidget->setCellWidget(LEAD_RECORD_ID, DATA_COLUMN, m_pEditLeadRecordID.get());
    m_pEditLeadRecordID->setEnabled(false);

    m_pEditPadLeadDist = std::make_unique<QLineEdit>(ui.tableWidget);
    ui.tableWidget->setCellWidget(PAD_LEAD_DIST, DATA_COLUMN, m_pEditPadLeadDist.get());
    m_pEditPadLeadDist->setEnabled(false);

    m_pSpecAndResultMaxOffsetX = std::make_unique<SpecAndResultWidget>(ui.tableWidget, 0, 10000);
    ui.tableWidget->setCellWidget(MAX_OFFSET_X, DATA_COLUMN, m_pSpecAndResultMaxOffsetX.get());

    m_pSpecAndResultMaxOffsetY = std::make_unique<SpecAndResultWidget>(ui.tableWidget, 0, 10000);
    ui.tableWidget->setCellWidget(MAX_OFFSET_Y, DATA_COLUMN, m_pSpecAndResultMaxOffsetY.get());
}

InspLeadWidget::~InspLeadWidget() {
}

void InspLeadWidget::setDefaultValue() {
    m_pEditPadRecordID->clear();
    m_pEditLeadRecordID->clear();
    m_pEditPadLeadDist->clear();
    m_pSpecAndResultMaxOffsetX->setSpec(500.f);
    m_pSpecAndResultMaxOffsetY->setSpec(500.f);
}

void InspLeadWidget::setCurrentWindow(const Engine::Window &window) {
    m_currentWindow = window;

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError)
        return;

    if (!parse_doucment.isObject())
        return;
    QJsonObject json = parse_doucment.object();

    m_pEditPadRecordID->setText(QString::number(json.take("PadRecordId").toInt()));
    m_pEditLeadRecordID->setText(QString::number(json.take("LeadRecordId").toInt()));
    m_pEditPadLeadDist->setText(QString::number(json.take("PadLeadDist").toDouble()));
    m_pSpecAndResultMaxOffsetX->setSpec(json.take("MaxOffsetX").toDouble());
    m_pSpecAndResultMaxOffsetY->setSpec(json.take("MaxOffsetY").toDouble());
}

void InspLeadWidget::tryInsp() {
    Vision::PR_INSP_LEAD_TMPL_CMD stCmd;
    Vision::PR_INSP_LEAD_TMPL_RPY stRpy;

    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    stCmd.matInputImg = pUI->getImage();

    stCmd.rectROI = pUI->getSelectedROI();
    stCmd.nLeadRecordId = m_pEditLeadRecordID->text().toInt();
    stCmd.nPadRecordId = m_pEditPadRecordID->text().toInt();
    stCmd.fLrnedPadLeadDist = m_pEditPadLeadDist->text().toFloat() / dResolutionX;
    stCmd.fMaxLeadOffsetX = m_pSpecAndResultMaxOffsetX->getSpec() / dResolutionX;
    stCmd.fMaxLeadOffsetY = m_pSpecAndResultMaxOffsetY->getSpec() / dResolutionX;
    Vision::PR_InspLeadTmpl(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK == stRpy.enStatus)
        pUI->displayImage(stRpy.matResultImg);
    m_pSpecAndResultMaxOffsetX->setResult(stRpy.fLeadOffsetX * dResolutionX);
    m_pSpecAndResultMaxOffsetY->setResult(stRpy.fLeadOffsetY * dResolutionY);
}

void InspLeadWidget::confirmWindow(OPERATION enOperation) {
}

void InspLeadWidget::on_btnAutoLocateLead_clicked() {
    QString strTitle(QStringLiteral("自动生成检测框"));
    auto pUI = getModule<IVisionUI>(UI_MODEL);
    auto vvDevice = pUI->getSelectedDevice();
    if (vvDevice.getId() <= 0) {
        System->showMessage(strTitle, QStringLiteral("请先选择一个元件!"));
        return;
    }

    auto rectDevice = vvDevice.getWindow().boundingRect();

     cv::Rect rectDefaultSrchWindow = CalcUtils::resizeRect(rectDevice, cv::Size(rectDevice.width + 50, rectDevice.height + 50));
     pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_SRCH_WINDOW);
     pUI->setSrchWindow(rectDefaultSrchWindow);
     auto nReturn = System->showInteractMessage(strTitle, QStringLiteral("请拖动鼠标选择搜寻窗口"));
     if (nReturn != QDialog::Accepted)
         return;

    auto rectSrchWindow = pUI->getSrchWindow();

    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_INSP_WINDOW);
    nReturn = System->showInteractMessage(strTitle, QStringLiteral("请拖动鼠标选择Pad"));
    if (nReturn != QDialog::Accepted)
        return;

    auto rectPad = pUI->getSelectedROI();

    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_INSP_WINDOW);
    nReturn = System->showInteractMessage(strTitle, QStringLiteral("请拖动鼠标选择Lead"));
    if (nReturn != QDialog::Accepted)
        return;

    auto rectLead = pUI->getSelectedROI();

    Vision::PR_AUTO_LOCATE_LEAD_CMD stCmd;
    Vision::PR_AUTO_LOCATE_LEAD_RPY stRpy;
    stCmd.matInputImg = pUI->getImage();
    stCmd.rectChipBody = rectDevice;
    stCmd.rectSrchWindow = rectSrchWindow;
    stCmd.rectLeadWindow = rectLead;
    stCmd.rectPadWindow = rectPad;
    stCmd.enMethod = Vision::PR_AUTO_LOCATE_LEAD_METHOD::TEMPLATE_MATCH;
    if (ui.checkBoxLeft->isChecked())
        stCmd.vecSrchLeadDirections.push_back(Vision::PR_DIRECTION::LEFT);
    if (ui.checkBoxRight->isChecked())
        stCmd.vecSrchLeadDirections.push_back(Vision::PR_DIRECTION::RIGHT);
    if (ui.checkBoxUp->isChecked())
        stCmd.vecSrchLeadDirections.push_back(Vision::PR_DIRECTION::UP);
    if (ui.checkBoxDown->isChecked())
        stCmd.vecSrchLeadDirections.push_back(Vision::PR_DIRECTION::DOWN);
    Vision::PR_AutoLocateLead(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK == stRpy.enStatus) {
        pUI->displayImage(stRpy.matResultImg);
        QString strMsg = QStringLiteral("自动定位引脚框成功, 确定自动添加检测框?");
        auto nReturn = System->showInteractMessage(strTitle, strMsg);
        if (nReturn != QDialog::Accepted) {
            for (const auto recordId : stRpy.vecLeadRecordId)
                Vision::PR_FreeRecord(recordId);
            for (const auto recordId : stRpy.vecPadRecordId)
                Vision::PR_FreeRecord(recordId);
            return;
        }
        _autoAddInspWindows(stRpy);
    }else {
        QString strMsg = QStringLiteral("自动定位引脚框失败, 错误消息: ");
        Vision::PR_GET_ERROR_INFO_RPY stErrorRpy;
        Vision::PR_GetErrorInfo(stRpy.enStatus, &stErrorRpy);
        strMsg += stErrorRpy.achErrorStr;
        System->showMessage(strTitle, strMsg);
    }
}

void InspLeadWidget::_autoAddInspWindows(const Vision::PR_AUTO_LOCATE_LEAD_RPY &stRpy) {
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();
    auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();

    auto pUI = getModule<IVisionUI>(UI_MODEL);  

    Engine::Window window;
    window.lightId = m_pParent->getSelectedLighting() + 1;
    window.usage = Engine::Window::Usage::INSP_LEAD;

    auto matBigImage = pUI->getImage();
    int nBigImgWidth  = matBigImage.cols / dCombinedImageScale;
    int nBigImgHeight = matBigImage.rows / dCombinedImageScale;
    
    window.deviceId = pUI->getSelectedDevice().getId();
    window.angle = 0;

    int result = Engine::OK;

    auto selectDevice = pUI->getSelectedDevice();
    window.deviceId = selectDevice.getId();
    char windowName[100];
    
    auto vecDetectObjs = pUI->getDetectObjs();
    QString strTitle(QStringLiteral("创建引脚检测框"));
    for (const auto &stLeadInfo : stRpy.vecLeadInfo) {
        QJsonObject json;
        json.insert("PadRecordId", stLeadInfo.nPadRecordId);
        json.insert("LeadRecordId", stLeadInfo.nLeadRecordId);
        float fPadLeadDist = 0;
        if (Vision::PR_DIRECTION::UP == stLeadInfo.enDir || Vision::PR_DIRECTION::DOWN == stLeadInfo.enDir)
            fPadLeadDist = abs((stLeadInfo.rectPadWindow.y + stLeadInfo.rectPadWindow.height / 2) - 
            (stLeadInfo.rectLeadWindow.y + stLeadInfo.rectLeadWindow.height / 2)) * dResolutionY;
        else
            fPadLeadDist = abs((stLeadInfo.rectPadWindow.x + stLeadInfo.rectPadWindow.width / 2) - 
            (stLeadInfo.rectLeadWindow.x + stLeadInfo.rectLeadWindow.width / 2)) * dResolutionX;

        json.insert("PadLeadDist", fPadLeadDist);
        json.insert("MaxOffsetX", 500.f);
        json.insert("MaxOffsetY", 500.f);

        QJsonDocument document;
        document.setObject(json);
        QByteArray byte_array = document.toJson(QJsonDocument::Compact);
        window.inspParams = byte_array;

        cv::Point ptWindowCtr(stLeadInfo.rectSrchWindow.x + stLeadInfo.rectSrchWindow.width / 2,
            stLeadInfo.rectSrchWindow.y + stLeadInfo.rectSrchWindow.height / 2);

        if (bBoardRotated) {
            window.x = (nBigImgWidth - ptWindowCtr.x)  * dResolutionX;
            window.y = ptWindowCtr.y * dResolutionY;
        }
        else {
            window.x = ptWindowCtr.x * dResolutionX;
            window.y = (nBigImgHeight - ptWindowCtr.y) * dResolutionY;
        }
        window.width  = stLeadInfo.rectSrchWindow.width  * dResolutionX;
        window.height = stLeadInfo.rectSrchWindow.height * dResolutionY;

        _snprintf(windowName, sizeof(windowName), "Insp Lead [%d, %d] @ %s", Vision::ToInt32(window.x), Vision::ToInt32(window.y), selectDevice.getName().c_str());
        window.name = windowName;
        result = Engine::CreateWindow(window);
        if (result != Engine::OK) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            QString strMsg = QStringLiteral("创建检测框失败, 错误消息: ") + errorMessage.c_str();
            System->showMessage(strTitle, strMsg);
            return;
        }
        else {
            System->setTrackInfo(QString("Success to Create Window: %1.").arg(window.name.c_str()));
        }

        QDetectObj detectObj(window.Id, window.name.c_str());
        cv::Point2f ptCenter(window.x / dResolutionX, window.y / dResolutionY);
        if (bBoardRotated)
            ptCenter.x = nBigImgWidth - ptCenter.x;
        else
            ptCenter.y = nBigImgHeight - ptCenter.y; //In cad, up is positive, but in image, down is positive.
        cv::Size2f szROI(window.width / dResolutionX, window.height / dResolutionY);
        detectObj.setFrame(cv::RotatedRect(ptCenter, szROI, window.angle));
        vecDetectObjs.push_back(detectObj);
    }

    for (const auto recordId : stRpy.vecLeadRecordId) {
        Binary recordData;
        if (ReadBinaryFile(FormatRecordName(recordId), recordData) != 0) {
            QString strMsg = QString(QStringLiteral("读取Vision Record失败, record Id %1")).arg(recordId);
            System->showMessage(strTitle, strMsg);
            return;
        }
        result = Engine::AddRecord(recordId, recordData);
        if (result != Engine::OK) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            QString strMsg = QStringLiteral("创建record到数据库失败, 错误消息: ") + errorMessage.c_str();
            System->showMessage(strTitle, strMsg);
            return;
        }
    }
                
    for (const auto recordId : stRpy.vecPadRecordId) {
        Binary recordData;
        if (ReadBinaryFile(FormatRecordName(recordId), recordData) != 0) {
            QString strMsg = QString(QStringLiteral("读取Vision Record失败, record Id %1")).arg(recordId);
            System->showMessage(strTitle, strMsg);
            return;
        }
        result = Engine::AddRecord(recordId, recordData);
        if (result != Engine::OK) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            QString strMsg = QStringLiteral("创建record到数据库失败, 错误消息: ") + errorMessage.c_str();
            System->showMessage(strTitle, strMsg);
            return;
        }
    }

    pUI->setDetectObjs(vecDetectObjs);
    m_pParent->updateInspWindowList();
}