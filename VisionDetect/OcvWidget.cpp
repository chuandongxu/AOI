#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>

#include "OcvWidget.h"
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
    CHAR_COUNT,
    MIN_SCORE,
};

OcvWidget::OcvWidget(InspWindowWidget *parent)
    : EditInspWindowBaseWidget(parent)
{
    ui.setupUi(this);
    m_pEditCharCount = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditCharCount->setValidator(new QIntValidator(1, 1000, m_pEditCharCount.get()));
    ui.tableWidget->setCellWidget(CHAR_COUNT, DATA_COLUMN, m_pEditCharCount.get());

    m_pSpecAndResultMinScore = std::make_unique<SpecAndResultWidget>(ui.tableWidget, 40, 100);
    ui.tableWidget->setCellWidget(MIN_SCORE, DATA_COLUMN, m_pSpecAndResultMinScore.get());

    setDefaultValue();
}

OcvWidget::~OcvWidget() {
}

void OcvWidget::setDefaultValue() {
    m_pEditCharCount->setText(QString::number(4));
    m_pSpecAndResultMinScore->setSpec(60);
    ui.listWidgetRecordId->clear();
    m_bIsTryInspected = false;

    m_currentWindow.usage = Engine::Window::Usage::UNDEFINED;
}

void OcvWidget::setCurrentWindow(const Engine::Window &window) {
    m_currentWindow = window;

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError)
        return;

    if (! parse_doucment.isObject())
        return;

    QJsonObject obj = parse_doucment.object();

    m_pEditCharCount->setText(QString::number(obj.take("CharCount").toInt()));
    m_pSpecAndResultMinScore->setSpec(obj.take("MinScore").toDouble());

    ui.listWidgetRecordId->clear();
    auto strRecordList = obj.take("RecordList").toString();
    auto datalist = strRecordList.split(',');
    for (const auto &strRecordId : datalist) {
        ui.listWidgetRecordId->addItem(strRecordId);
    }

    m_bIsTryInspected = false;
}

void OcvWidget::tryInsp() {
    if (ui.listWidgetRecordId->count() <= 0) {
        System->showMessage(QStringLiteral("OCV字符检测"), QStringLiteral("请先学习OCV模板"));
        return;
    }

    std::vector<Int32> vecRecordId;
    for (int i = 0; i < ui.listWidgetRecordId->count(); ++ i) {
        vecRecordId.push_back(ui.listWidgetRecordId->item(i)->text().toInt());
    }

    if (_inspOcv(vecRecordId))
        m_bIsTryInspected = true;
    else
        m_bIsTryInspected = false;
}

void OcvWidget::confirmWindow(OPERATION enOperation) {
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();
    auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();

    QString strTitle(QStringLiteral("OCV字符检测"));

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    auto rectROI = pUI->getSelectedROI();
    if (rectROI.width <= 0 || rectROI.height <= 0) {
        QMessageBox::critical(this, strTitle, QStringLiteral("Please select a ROI to do inspection."));
        return;
    }

    if (ui.listWidgetRecordId->count() <= 0) {
        System->showMessage(strTitle, QStringLiteral("请先学习OCV模板"));
        return;
    }

    std::vector<Int32> vecRecordId;
    for (int i = 0; i < ui.listWidgetRecordId->count(); ++ i) {
        vecRecordId.push_back(ui.listWidgetRecordId->item(i)->text().toInt());
    }

    if (! m_bIsTryInspected) {
        if (!_inspOcv(vecRecordId, false)) {
            System->showMessage(strTitle, QStringLiteral("OCV检测失败"));
            return;
        }
    }

    QJsonObject json;
    json.insert("CharCount", m_pEditCharCount->text().toInt());
    json.insert("MinScore", m_pSpecAndResultMinScore->getSpec());
    String strRecordList;
    for (auto recordId : vecRecordId) {
        strRecordList += std::to_string(recordId) + ",";
    }
    strRecordList = strRecordList.substr(0, strRecordList.length() - 1);
    json.insert("RecordList", strRecordList.c_str());

    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);

    Engine::Window window;
    if (OPERATION::EDIT == enOperation)
        window = m_currentWindow;
    window.lightId = m_pParent->getSelectedLighting() + 1;
    window.usage = Engine::Window::Usage::OCV;
    window.inspParams = byte_array;

    cv::Point2f ptWindowCtr(rectROI.x + rectROI.width / 2.f, rectROI.y + rectROI.height / 2.f);
    auto matBigImage = pUI->getImage();
    int nBigImgWidth  = matBigImage.cols / dCombinedImageScale;
    int nBigImgHeight = matBigImage.rows / dCombinedImageScale;
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
    window.srchWidth  = 0;
    window.srchHeight = 0;
    window.deviceId = pUI->getSelectedDevice().getId();
    window.angle = 0;

    QDetectObj detectObj(window.Id, window.name.c_str());
    cv::Point2f ptCenter(window.x / dResolutionX, window.y / dResolutionY);
    if (bBoardRotated)
        ptCenter.x = nBigImgWidth  - ptCenter.x;
    else
        ptCenter.y = nBigImgHeight - ptCenter.y; //In cad, up is positive, but in image, down is positive.
    detectObj.setFrame(cv::RotatedRect(ptCenter, rectROI.size(), window.angle));
    auto vecDetectObjs = pUI->getDetectObjs();

    int result = Engine::OK;
    if (OPERATION::ADD == enOperation) {
        window.deviceId = pUI->getSelectedDevice().getId();
        char windowName[100];
        _snprintf(windowName, sizeof(windowName), "Ocv [%d, %d] @ %s", Vision::ToInt32(window.x), Vision::ToInt32(window.y), pUI->getSelectedDevice().getName().c_str());
        window.name = windowName;
        result = Engine::CreateWindow(window);
        if (result != Engine::OK) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            System->setTrackInfo(QString("Error at CreateWindow, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
            return;
        }
        else {
            System->setTrackInfo(QString("Success to Create Window: %1.").arg(window.name.c_str()));
        }

        for (auto recordId : vecRecordId) {
            Binary recordData;
            if (ReadBinaryFile(FormatRecordName(recordId), recordData) != 0) {
                QMessageBox::critical(this, strTitle, QStringLiteral("Failed to read record data."));
                return;
            }
            Engine::AddRecord(recordId, recordData);
        }

        vecDetectObjs.push_back(detectObj);
    }
    else {
        result = Engine::UpdateWindow(window);
        if (result != Engine::OK) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            System->setTrackInfo(QString("Error at UpdateWindow, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
            return;
        }
        else
            System->setTrackInfo(QString("Success to update window: %1.").arg(window.name.c_str()));

        auto iter = std::find_if(vecDetectObjs.begin(), vecDetectObjs.end(), [window](const QDetectObj& obj) { return window.Id == obj.getID(); });
        if (iter != vecDetectObjs.end()) {
            *iter = detectObj;
        }
    }

    pUI->setDetectObjs(vecDetectObjs);
    m_pParent->updateInspWindowList();
}

void OcvWidget::on_btnLrnOcv_clicked() {
    int nRecordId = 0;
    if (! _learnOcv(nRecordId))
        return;

    QString strTitle(QStringLiteral("OCV字符检测"));
    QString strMsg = QStringLiteral("OCV字符模板学习成功，reocrd Id ") + QString::number(nRecordId) + QStringLiteral(", 确认添加到当前窗口?");
    if (QDialog::Accepted != System->showInteractMessage(strTitle, strMsg)) {
        Vision::PR_FreeRecord(nRecordId);
        return;
    }

    ui.listWidgetRecordId->addItem(QString::number(nRecordId));

    if (m_currentWindow.usage != Engine::Window::Usage::OCV)
        return;

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(m_currentWindow.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError)
        return;

    if (! parse_doucment.isObject())
        return;

    QJsonObject obj = parse_doucment.object();
    obj["RecordList"] = obj["RecordList"].toString() + "," + QString::number(nRecordId);

    QJsonDocument document;
    document.setObject(obj);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);

    m_currentWindow.inspParams = byte_array;

    auto result = Engine::UpdateWindow(m_currentWindow);
    if (result != Engine::OK) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        System->setTrackInfo(QString("Error at CreateWindow, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
        return;
    }

    Binary recordData;
    if (ReadBinaryFile(FormatRecordName(nRecordId), recordData) != 0) {
        System->showMessage( strTitle, QStringLiteral("Failed to read record data."));
        return;
    }
    Engine::AddRecord(nRecordId, recordData);
}

bool OcvWidget::_learnOcv(int &recordId) {
    Vision::PR_LRN_OCV_CMD stCmd;
    Vision::PR_LRN_OCV_RPY stRpy;

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    stCmd.matInputImg = pUI->getImage();
    cv::Rect rectROI = pUI->getSelectedROI();
    if (rectROI.width <= 0 || rectROI.height <= 0) {
        QMessageBox::critical(this, QStringLiteral("学习OCV"), QStringLiteral("Please select a ROI to do inspection."));
        return false;
    }

    stCmd.rectROI = rectROI;
    stCmd.nCharCount = m_pEditCharCount->text().toInt();
    Vision::PR_LrnOcv(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK != stRpy.enStatus) {
        System->showMessage(QStringLiteral("学习OCV"), QStringLiteral("学习OCV失败"));
        recordId = 0;
        return false;
    }

    pUI->displayImage(stRpy.matResultImg);

    recordId = stRpy.nRecordId;
    return true;
}

bool OcvWidget::_inspOcv(const std::vector<Int32> &vecRecordId, bool bShowResult /*= true*/) {
    Vision::PR_OCV_CMD stCmd;
    Vision::PR_OCV_RPY stRpy;

    stCmd.vecRecordId = vecRecordId;

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    stCmd.matInputImg = pUI->getImage();
    cv::Rect rectROI = pUI->getSelectedROI();
    if (rectROI.width <= 0 || rectROI.height <= 0) {
        QMessageBox::critical(this, QStringLiteral("Add OCV Window"), QStringLiteral("Please select a ROI to do inspection."));
        return false;
    }
    stCmd.rectROI = rectROI;
    Vision::PR_Ocv(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK == stRpy.enStatus)
        pUI->displayImage(stRpy.matResultImg);
    if (bShowResult) {
        m_pSpecAndResultMinScore->setResult(stRpy.fOverallScore);
        QString strMsg;
        strMsg.sprintf("Inspect Status %d, overall score(%f)", Vision::ToInt32(stRpy.enStatus), stRpy.fOverallScore);
        QMessageBox::information(this, "Alignment", strMsg);
    }

    return Vision::VisionStatus::OK == stRpy.enStatus;
}