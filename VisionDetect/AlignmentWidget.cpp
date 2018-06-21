#include "AlignmentWidget.h"
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>

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
    ALGORITHM_ATTRI,
    RECORDID_ATTRI,
    SUBPIXEL_ATTRI,
    MOTION_ATTRI,
    MINSCORE_ATTRI,
};

AlignmentWidget::AlignmentWidget(InspWindowWidget *parent)
:EditInspWindowBaseWidget(parent) {
    ui.setupUi(this);

    m_pComboBoxAlgorithm = std::make_unique<QComboBox>(ui.tableWidget);
    m_pComboBoxAlgorithm->addItem("SQUARE DIFF");
    m_pComboBoxAlgorithm->addItem("HIERARCHICAL EDGE");
    m_pComboBoxAlgorithm->addItem("HIERARCHICAL AREA");
    ui.tableWidget->setCellWidget(ALGORITHM_ATTRI, DATA_COLUMN, m_pComboBoxAlgorithm.get());

    m_pEditRecordID = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditRecordID->setValidator(new QIntValidator(0, 1000, m_pEditRecordID.get()));
    ui.tableWidget->setCellWidget(RECORDID_ATTRI, DATA_COLUMN, m_pEditRecordID.get());
    m_pEditRecordID->setEnabled(false);

    m_pCheckBoxSubPixel = std::make_unique<QCheckBox>(ui.tableWidget);
    ui.tableWidget->setCellWidget(SUBPIXEL_ATTRI, DATA_COLUMN, m_pCheckBoxSubPixel.get());

    m_pComboBoxMotion = std::make_unique<QComboBox>(ui.tableWidget);
    m_pComboBoxMotion->addItem("TRANSLATION");
    m_pComboBoxMotion->addItem("EUCLIDEAN");
    m_pComboBoxMotion->addItem("AFFINE");
    m_pComboBoxMotion->addItem("HOMOGRAPHY");
    ui.tableWidget->setCellWidget(MOTION_ATTRI, DATA_COLUMN, m_pComboBoxMotion.get());

    m_pEditMinScore = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditMinScore->setValidator(new QIntValidator(1, 100, m_pEditMinScore.get()));
    ui.tableWidget->setCellWidget(MINSCORE_ATTRI, DATA_COLUMN, m_pEditMinScore.get());
}

AlignmentWidget::~AlignmentWidget() {
}

void AlignmentWidget::setDefaultValue() {
    m_pComboBoxAlgorithm->setCurrentIndex(0);
    m_pEditRecordID->setText("0");

    m_pCheckBoxSubPixel->setChecked(false);
    m_pComboBoxMotion->setCurrentIndex(0);
    m_pEditMinScore->setText("60");

    m_bIsTryInspected = false;
    m_currentWindow.recordId = 0;
}

bool AlignmentWidget::_learnTemplate(int &recordId) {
    Vision::PR_LRN_TEMPLATE_CMD stCmd;
    Vision::PR_LRN_TEMPLATE_RPY stRpy;

    stCmd.enAlgorithm = static_cast<Vision::PR_MATCH_TMPL_ALGORITHM>(m_pComboBoxAlgorithm->currentIndex());

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    stCmd.matInputImg = pUI->getImage();
    cv::Rect rectROI = pUI->getSelectedROI();
    if (rectROI.width <= 0 || rectROI.height <= 0) {
        QMessageBox::critical(this, QStringLiteral("Add Alignment Window"), QStringLiteral("Please select a ROI to do inspection."));
        return false;
    }

    stCmd.rectROI = rectROI;

    Vision::PR_LrnTmpl(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK != stRpy.enStatus) {
        System->showMessage(QStringLiteral("定位框"), QStringLiteral("学习模板失败"));
        recordId = 0;
        return false;
    }

    recordId = stRpy.nRecordId;
    return true;
}

bool AlignmentWidget::_srchTemplate(int recordId, bool bShowResult) {
    Vision::PR_MATCH_TEMPLATE_CMD stCmd;
    Vision::PR_MATCH_TEMPLATE_RPY stRpy;

    stCmd.enAlgorithm = static_cast<Vision::PR_MATCH_TMPL_ALGORITHM>(m_pComboBoxAlgorithm->currentIndex());
    stCmd.bSubPixelRefine = m_pCheckBoxSubPixel->isChecked();
    stCmd.enMotion = static_cast<Vision::PR_OBJECT_MOTION>(m_pComboBoxMotion->currentIndex());
    stCmd.fMinMatchScore = m_pEditMinScore->text().toFloat();
    stCmd.nRecordId = recordId;

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    stCmd.matInputImg = pUI->getImage();
    cv::Rect rectROI = pUI->getSelectedROI();
    if (rectROI.width <= 0 || rectROI.height <= 0) {
        QMessageBox::critical(this, QStringLiteral("Add Alignment Window"), QStringLiteral("Please select a ROI to do inspection."));
        return false;
    }

    stCmd.rectSrchWindow = pUI->getSrchWindow();

    Vision::PR_MatchTmpl(&stCmd, &stRpy);
    if (bShowResult) {
        QString strMsg;
        strMsg.sprintf("Inspect Status %d, center(%f, %f), rotation(%f), score(%f)", Vision::ToInt32(stRpy.enStatus), stRpy.ptObjPos.x, stRpy.ptObjPos.y, stRpy.fRotation, stRpy.fMatchScore);
        QMessageBox::information(this, "Alignment", strMsg);
    }

    return Vision::VisionStatus::OK == stRpy.enStatus;
}

void AlignmentWidget::tryInsp() {
    if (Engine::Window::Usage::ALIGNMENT != m_currentWindow.usage) {
        // Ask user to select the search window
        auto pUI = getModule<IVisionUI>(UI_MODEL);
        auto rectROI = pUI->getSelectedROI();
        cv::Rect rectDefaultSrchWindow = CalcUtils::resizeRect(rectROI, cv::Size2f(rectROI.width * 1.5f, rectROI.height * 1.5f));
        pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_SRCH_WINDOW);
        pUI->setSrchWindow(rectDefaultSrchWindow);
        auto nReturn = System->showInteractMessage(QStringLiteral("定位框"), QStringLiteral("请拖动鼠标选择搜寻窗口"));
        if (nReturn != QDialog::Accepted)
            return;
    }

    int nRecordId = 0;
    bool bNewRecord = false;
    if (m_currentWindow.recordId > 0)
        nRecordId = m_currentWindow.recordId;
    else {
        if (!_learnTemplate(nRecordId))
            return;
        else
            bNewRecord = true;
    }

    if (_srchTemplate(nRecordId))
        m_bIsTryInspected = true;
    else
        m_bIsTryInspected = false;

    if (bNewRecord)
        Vision::PR_FreeRecord(nRecordId);
}

void AlignmentWidget::confirmWindow(OPERATION enOperation) {
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();
    auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    auto rectROI = pUI->getSelectedROI();
    if (rectROI.width <= 0 || rectROI.height <= 0) {
        QMessageBox::critical(this, QStringLiteral("Add Alignment Window"), QStringLiteral("Please select a ROI to do inspection."));
        return;
    }

    if (! m_bIsTryInspected) {
        // Ask user to select the search window
        pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_SRCH_WINDOW);
        if (OPERATION::ADD == enOperation) {
            cv::Rect rectSrchWindow = CalcUtils::resizeRect(rectROI, cv::Size2f(rectROI.width * 1.5f, rectROI.height * 1.5f));
            pUI->setSrchWindow(rectSrchWindow);
        }
        auto nReturn = System->showInteractMessage(QStringLiteral("定位框"), QStringLiteral("请拖动鼠标选择搜寻窗口"));
        if (nReturn != QDialog::Accepted)
            return;
    }

    int nRecordId = 0;
    if (!_learnTemplate(nRecordId)) {
        return;
    }

    if (!_srchTemplate(nRecordId, false)) {
        System->showInteractMessage(QStringLiteral("定位框"), QStringLiteral("搜寻模板失败"));
        return;
    }

    auto rectSrchWindow = pUI->getSrchWindow();

    QJsonObject json;
    json.insert("Algorithm", m_pComboBoxAlgorithm->currentIndex());
    json.insert("SubPixel", m_pCheckBoxSubPixel->isChecked());
    json.insert("Motion", m_pComboBoxMotion->currentIndex());
    json.insert("MinScore", m_pEditMinScore->text().toFloat());

    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);

    Engine::Window window;
    if (OPERATION::EDIT == enOperation)
        window = m_currentWindow;
    window.lightId = m_pParent->getSelectedLighting() + 1;
    window.usage = Engine::Window::Usage::ALIGNMENT;
    window.inspParams = byte_array;

    cv::Point2f ptWindowCtr(rectROI.x + rectROI.width / 2.f, rectROI.y + rectROI.height / 2.f);
    auto matBigImage = pUI->getImage();
    int nBigImgWidth = matBigImage.cols / dCombinedImageScale;
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
    window.srchWidth  = rectSrchWindow.width  * dResolutionX;
    window.srchHeight = rectSrchWindow.height * dResolutionY;
    window.deviceId = pUI->getSelectedDevice().getId();
    window.angle = 0;
    window.recordId = nRecordId;
    m_pEditRecordID->setText(QString::number(nRecordId));

    if (ReadBinaryFile(FormatRecordName(window.recordId), window.recordData) != 0) {
        QMessageBox::critical(this, QStringLiteral("Add Alignment Window"), QStringLiteral("Failed to read record data."));
        return;
    }

    QDetectObj detectObj(window.Id, window.name.c_str());
    cv::Point2f ptCenter(window.x / dResolutionX, window.y / dResolutionY);
    if (bBoardRotated)
        ptCenter.x = nBigImgWidth - ptCenter.x;
    else
        ptCenter.y = nBigImgHeight - ptCenter.y; //In cad, up is positive, but in image, down is positive.
    detectObj.setFrame(cv::RotatedRect(ptCenter, rectROI.size(), window.angle));
    detectObj.setSrchWindow(cv::RotatedRect(ptCenter, rectSrchWindow.size(), window.angle));
    auto vecDetectObjs = pUI->getDetectObjs();

    int result = Engine::OK;
    if (OPERATION::ADD == enOperation) {
        window.deviceId = pUI->getSelectedDevice().getId();
        char windowName[100];
        _snprintf(windowName, sizeof(windowName), "Alignment [%d, %d] @ %s", Vision::ToInt32(window.x), Vision::ToInt32(window.y), pUI->getSelectedDevice().getName().c_str());
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
    m_pParent->UpdateInspWindowList();
}

void AlignmentWidget::setCurrentWindow(const Engine::Window &window) {
    m_currentWindow = window;

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError)
        return;

    if (! parse_doucment.isObject())
        return;

    QJsonObject obj = parse_doucment.object();

    m_pComboBoxAlgorithm->setCurrentIndex(obj.take("Algorithm").toInt());

    m_pCheckBoxSubPixel->setChecked(obj.take("SubPixel").toBool());
    m_pComboBoxMotion->setCurrentIndex(obj.take("Motion").toInt());
    m_pEditMinScore->setText(QString::number(obj.take("MinScore").toDouble()));

    m_pEditRecordID->setText(QString::number(window.recordId));

    m_bIsTryInspected = false;
}
