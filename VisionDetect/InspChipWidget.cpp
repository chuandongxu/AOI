#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>

#include "InspChipWidget.h"
#include "../Common/SystemData.h"
#include "DataStoreAPI.h"
#include "VisionAPI.h"
#include "../include/IVisionUI.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../Common/CommonFunc.h"
#include "../DataModule/QDetectObj.h"
#include "InspWindowWidget.h"
#include "../DataModule/CalcUtils.hpp"

using namespace NFG::AOI;
using namespace AOI;

enum BASIC_PARAM
{
    INSP_MODE,
    RECORD_ID,
};

InspChipWidget::InspChipWidget(InspWindowWidget *parent)
: EditInspWindowBaseWidget(parent) {
    ui.setupUi(this);

    m_pComboBoxInspMode = std::make_unique<QComboBox>(this);
    m_pComboBoxInspMode->addItem("HEAD");
    m_pComboBoxInspMode->addItem("BODY");
    m_pComboBoxInspMode->addItem("SQUARE");
    m_pComboBoxInspMode->addItem("CAE");
    m_pComboBoxInspMode->addItem("CIRCULAR");
    ui.tableWidget->setCellWidget(INSP_MODE, DATA_COLUMN, m_pComboBoxInspMode.get());

    m_pEditRecordID = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditRecordID->setValidator(new QIntValidator(0, 1000, m_pEditRecordID.get()));
    ui.tableWidget->setCellWidget(RECORD_ID, DATA_COLUMN, m_pEditRecordID.get());
    m_pEditRecordID->setEnabled(false);
}

InspChipWidget::~InspChipWidget() {
}

void InspChipWidget::setDefaultValue() {
    m_pComboBoxInspMode->setCurrentIndex(0);
    m_pEditRecordID->setText("0");
}

bool InspChipWidget::_learnChip(int &recordId) {
    Vision::PR_LRN_CHIP_CMD stCmd;
    Vision::PR_LRN_CHIP_RPY stRpy;

    stCmd.enInspMode = static_cast<Vision::PR_INSP_CHIP_MODE>(m_pComboBoxInspMode->currentIndex());

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    stCmd.matInputImg = pUI->getImage();
    cv::Rect rectROI = pUI->getSelectedROI();
    if (rectROI.width <= 0 || rectROI.height <= 0) {
        QMessageBox::critical(this, QStringLiteral("元件检测框"), QStringLiteral("Please select a ROI to do inspection."));
        return false;
    }

    stCmd.rectChip = rectROI;

    Vision::PR_LrnChip(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK != stRpy.enStatus) {
        System->showMessage(QStringLiteral("元件检测框"), QStringLiteral("学习元件失败."));
        recordId = 0;
        return false;
    }

    recordId = stRpy.nRecordId;
    return true;
}

bool InspChipWidget::_inspChip(int recordId, bool bShowResult) {
    Vision::PR_INSP_CHIP_CMD stCmd;
    Vision::PR_INSP_CHIP_RPY stRpy;

    stCmd.enInspMode = static_cast<Vision::PR_INSP_CHIP_MODE>(m_pComboBoxInspMode->currentIndex());
    stCmd.nRecordId = recordId;

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    stCmd.matInputImg = pUI->getImage();
    cv::Rect rectROI = pUI->getSelectedROI();
    if (rectROI.width <= 0 || rectROI.height <= 0) {
        QMessageBox::critical(this, QStringLiteral("Add Alignment Window"), QStringLiteral("Please select a ROI to do inspection."));
        return false;
    }

    stCmd.rectSrchWindow = pUI->getSrchWindow();

    Vision::PR_InspChip(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK == stRpy.enStatus)
        pUI->displayImage(stRpy.matResultImg);
    if (bShowResult) {
        QString strMsg;
        strMsg.sprintf("Inspect Status %d, center(%f, %f), rotation(%f), score(%f)", Vision::ToInt32(stRpy.enStatus), stRpy.rotatedRectResult.center.x, stRpy.rotatedRectResult.center.y, stRpy.rotatedRectResult.angle);
        QMessageBox::information(this, "Alignment", strMsg);
    }

    return Vision::VisionStatus::OK == stRpy.enStatus;
}

void InspChipWidget::tryInsp() {
    if (Engine::Window::Usage::INSP_CHIP != m_currentWindow.usage) {
        // Ask user to select the search window
        auto pUI = getModule<IVisionUI>(UI_MODEL);
        auto rectROI = pUI->getSelectedROI();
        cv::Rect rectDefaultSrchWindow = CalcUtils::resizeRect(rectROI, cv::Size2f(rectROI.width * 1.5f, rectROI.height * 1.5f));
        pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_SRCH_WINDOW);
        pUI->setSrchWindow(rectDefaultSrchWindow);
        auto nReturn = System->showInteractMessage(QStringLiteral("元件检测框"), QStringLiteral("请拖动鼠标选择搜寻窗口"));
        if (nReturn != QDialog::Accepted)
            return;
    }

    int nRecordId = 0;
    bool bNewRecord = false;
    if (m_currentWindow.recordId > 0)
        nRecordId = m_currentWindow.recordId;
    else {
        if (!_learnChip(nRecordId))
            return;
        else
            bNewRecord = true;
    }

    if (_inspChip(nRecordId))
        m_bIsTryInspected = true;
    else
        m_bIsTryInspected = false;

    if (bNewRecord)
        Vision::PR_FreeRecord(nRecordId);
}

void InspChipWidget::confirmWindow(OPERATION enOperation) {
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    Int32 bBoardRotated = 0; Engine::GetParameter("BOARD_ROTATED", bBoardRotated, false);
    float fCombinedImageScale = 1.f; Engine::GetParameter("ScanImageZoomFactor", fCombinedImageScale, 1.f);

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    auto rectROI = pUI->getSelectedROI();
    if (rectROI.width <= 0 || rectROI.height <= 0) {
        QMessageBox::critical(this, QStringLiteral("Add Alignment Window"), QStringLiteral("Please select a ROI to do inspection."));
        return;
    }

    if (!m_bIsTryInspected) {
        // Ask user to select the search window
        pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_SRCH_WINDOW);
        if (OPERATION::ADD == enOperation) {
            cv::Rect rectSrchWindow = CalcUtils::resizeRect(rectROI, cv::Size2f(rectROI.width * 1.5f, rectROI.height * 1.5f));
            pUI->setSrchWindow(rectSrchWindow);
        }
        auto nReturn = System->showInteractMessage(QStringLiteral("元件检测框"), QStringLiteral("请拖动鼠标选择搜寻窗口"));
        if (nReturn != QDialog::Accepted)
            return;
    }

    int nRecordId = 0;
    if (! _learnChip(nRecordId)) {
        return;
    }

    if (! _inspChip(nRecordId, false)) {
        System->showInteractMessage(QStringLiteral("元件检测框"), QStringLiteral("搜寻模板失败"));
        return;
    }

    auto rectSrchWindow = pUI->getSrchWindow();

    QJsonObject json;
    json.insert("InspMode", m_pComboBoxInspMode->currentIndex());

    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);

    Engine::Window window;
    window.lightId = m_pParent->getSelectedLighting() + 1;
    window.usage = Engine::Window::Usage::INSP_CHIP;
    window.inspParams = byte_array;

    cv::Point2f ptWindowCtr(rectROI.x + rectROI.width / 2.f, rectROI.y + rectROI.height / 2.f);
    auto matBigImage = pUI->getImage();
    int nBigImgWidth = matBigImage.cols / fCombinedImageScale;
    int nBigImgHeight = matBigImage.rows / fCombinedImageScale;
    if (bBoardRotated) {
        window.x = (nBigImgWidth - ptWindowCtr.x)  * dResolutionX;
        window.y = ptWindowCtr.y * dResolutionY;
    }
    else {
        window.x = ptWindowCtr.x * dResolutionX;
        window.y = (nBigImgHeight - ptWindowCtr.y) * dResolutionY;
    }
    window.width  = rectROI.width  * dResolutionX;
    window.height = rectROI.height * dResolutionY;
    window.srchWidth = rectSrchWindow.width  * dResolutionX;
    window.srchHeight = rectSrchWindow.height * dResolutionY;
    window.deviceId = pUI->getSelectedDevice().getId();
    window.angle = 0;
    window.recordId = nRecordId;
    m_pEditRecordID->setText(QString::number(nRecordId));

    if (ReadBinaryFile(FormatRecordName(window.recordId), window.recordData) != 0) {
        QMessageBox::critical(this, QStringLiteral("Add Alignment Window"), QStringLiteral("Failed to read record data."));
        return;
    }

    int result = Engine::OK;
    if (OPERATION::ADD == enOperation) {
        window.deviceId = pUI->getSelectedDevice().getId();
        char windowName[100];
        _snprintf(windowName, sizeof(windowName), "Insp Chip [%d, %d] @ %s", Vision::ToInt32(window.x), Vision::ToInt32(window.y), pUI->getSelectedDevice().getName().c_str());
        window.name = windowName;
        result = Engine::CreateWindow(window);
        if (result != Engine::OK) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            QString strMsg = QStringLiteral("创建检测框失败, 错误消息: ") + errorMessage.c_str();
            System->showMessage(QStringLiteral("创建元件检测框"), strMsg);
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
        auto vecDetectObjs = pUI->getDetectObjs();
        vecDetectObjs.push_back(detectObj);
        pUI->setDetectObjs(vecDetectObjs);
    }
    else {
        window.Id = m_currentWindow.Id;
        window.name = m_currentWindow.name;
        result = Engine::UpdateWindow(window);
        if (result != Engine::OK) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            System->setTrackInfo(QString("Error at UpdateWindow, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
            return;
        }
        else {
            System->setTrackInfo(QString("Success to update window: %1.").arg(window.name.c_str()));
        }
    }

    m_pParent->updateInspWindowList();
}

void InspChipWidget::setCurrentWindow(const Engine::Window &window) {
    m_currentWindow = window;

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError)
        return;

    if (!parse_doucment.isObject())
        return;
    QJsonObject obj = parse_doucment.object();

    m_pComboBoxInspMode->setCurrentIndex(obj.take("InspMode").toInt());

    m_pEditRecordID->setText(QString::number(window.recordId));
}
