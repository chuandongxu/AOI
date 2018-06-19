#include "HeightDetectWidget.h"
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>

#include "../Common/SystemData.h"
#include "DataStoreAPI.h"
#include "VisionAPI.h"
#include "../include/IVisionUI.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../DataModule/QDetectObj.h"
#include "InspWindowWidget.h"

using namespace NFG::AOI;
using namespace AOI;

enum BASIC_PARAM
{
    MEASURE_TYPE_ATTRI,
    RANGE_MIN_ATTRI,
    RANGE_MAX_ATTRI,
    MAX_REL_HT,
    MIN_REL_HT,
};

HeightDetectWidget::HeightDetectWidget(InspWindowWidget *parent)
:EditInspWindowBaseWidget(parent) {
    ui.setupUi(this);

    m_pCheckBoxMeasure = std::make_unique<QCheckBox>(ui.tableWidget);
    ui.tableWidget->setCellWidget(MEASURE_TYPE_ATTRI, DATA_COLUMN, m_pCheckBoxMeasure.get());

    m_pEditMinRange = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditMinRange->setValidator(new QDoubleValidator(0, 100, 2, m_pEditMinRange.get()));
    ui.tableWidget->setCellWidget(RANGE_MIN_ATTRI, DATA_COLUMN, m_pEditMinRange.get());

    m_pEditMaxRange = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditMaxRange->setValidator(new QDoubleValidator(0, 100, 2, m_pEditMaxRange.get()));
    ui.tableWidget->setCellWidget(RANGE_MAX_ATTRI, DATA_COLUMN, m_pEditMaxRange.get());

    m_pEditMaxRelHt = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditMaxRelHt->setValidator(new QDoubleValidator(0, 10, -10, m_pEditMaxRelHt.get()));
    ui.tableWidget->setCellWidget(MAX_REL_HT, DATA_COLUMN, m_pEditMaxRelHt.get());

    m_pEditMinRelHt = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditMinRelHt->setValidator(new QDoubleValidator(0, 100, 2, m_pEditMinRelHt.get()));
    ui.tableWidget->setCellWidget(MIN_REL_HT, DATA_COLUMN, m_pEditMinRelHt.get());
}

HeightDetectWidget::~HeightDetectWidget() {
}

void HeightDetectWidget::setDefaultValue() {
    m_pCheckBoxMeasure->setChecked(true);
    m_pEditMinRange->setText("30");
    m_pEditMaxRange->setText("70");
}

void HeightDetectWidget::tryInsp() {
    if (!m_pCheckBoxMeasure->isChecked()) {
        QString strMsg;
        strMsg.sprintf("Select Measure Window to measure height!");
        QMessageBox::information(this, "Measure Height", strMsg);
        return;
    }

    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();
    auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();

    Vision::PR_CALC_3D_HEIGHT_DIFF_CMD stCmd;
    Vision::PR_CALC_3D_HEIGHT_DIFF_RPY stRpy;

    stCmd.fEffectHRatioStart = m_pEditMinRange->text().toFloat() / ONE_HUNDRED_PERCENT;
    stCmd.fEffectHRatioEnd = m_pEditMaxRange->text().toFloat() / ONE_HUNDRED_PERCENT;

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    stCmd.matHeight = pUI->getHeightData();
    cv::Rect rectROI = pUI->getSelectedROI();
    if (rectROI.width <= 0 || rectROI.height <= 0) {
        QMessageBox::critical(this, QStringLiteral("高度检测框"), QStringLiteral("Please select a ROI to do inspection."));
        return;
    }
    stCmd.rectROI = rectROI;

    auto matImage = pUI->getImage();
    int nBigImgWidth = matImage.cols / dCombinedImageScale;
    int nBigImgHeight = matImage.rows / dCombinedImageScale;

    for (const auto &window : m_windowGroup.vecWindows) {
        if (Engine::Window::Usage::HEIGHT_BASE == window.usage) {
            auto x = window.x / dResolutionX;
            auto y = window.y / dResolutionY;
            if (bBoardRotated)
                x = nBigImgWidth - x;
            else
                y = nBigImgHeight - y; //In cad, up is positive, but in image, down is positive.

            auto width = window.width / dResolutionX;
            auto height = window.height / dResolutionY;
            cv::RotatedRect rrBaseRect(cv::Point2f(x, y), cv::Size2f(width, height), window.angle);
            stCmd.vecRectBases.push_back(rrBaseRect.boundingRect());
        }
    }

    float fMaxHeight = m_pEditMaxRelHt->text().toFloat();
    float fMinHeigth = m_pEditMinRelHt->text().toFloat();
    Vision::PR_Calc3DHeightDiff(&stCmd, &stRpy);
    bool bPassed = (stRpy.fHeightDiff < fMaxHeight) && (stRpy.fHeightDiff > fMinHeigth);
    QString strMsg;
    strMsg.sprintf("Inspect Status %d, %s, height (%f)", Vision::ToInt32(stRpy.enStatus), bPassed ? "pass" : "not pass", stRpy.fHeightDiff);
    QMessageBox::information(this, "Height Detect", strMsg);
}

void HeightDetectWidget::confirmWindow(OPERATION enOperation) {
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();
    auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();

    QJsonObject json;
    json.insert("MinRange", m_pEditMinRange->text().toFloat() / ONE_HUNDRED_PERCENT);
    json.insert("MaxRange", m_pEditMaxRange->text().toFloat() / ONE_HUNDRED_PERCENT);
    json.insert("MaxRelHt", m_pEditMaxRelHt->text().toFloat());
    json.insert("MinRelHt", m_pEditMinRelHt->text().toFloat());

    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    auto rectROI = pUI->getSelectedROI();
    if (rectROI.width <= 0 || rectROI.height <= 0) {
        QMessageBox::critical(this, QStringLiteral("Add Height Detect Window"), QStringLiteral("Please select a ROI to do inspection."));
        return;
    }

    Engine::Window window;
    window.lightId = m_pParent->getSelectedLighting() + 1;
    window.usage = m_pCheckBoxMeasure->isChecked() ? Engine::Window::Usage::HEIGHT_MEASURE : Engine::Window::Usage::HEIGHT_BASE;
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
    window.width = rectROI.width  * dResolutionX;
    window.height = rectROI.height * dResolutionY;
    window.deviceId = pUI->getSelectedDevice().getId();
    window.angle = 0;

    int result = Engine::OK;
    if (OPERATION::ADD == enOperation) {
        window.deviceId = pUI->getSelectedDevice().getId();
        char windowName[100];
        if (Engine::Window::Usage::HEIGHT_MEASURE == window.usage)
            _snprintf(windowName, sizeof(windowName), "Height Detect [%d, %d] @ %s", Vision::ToInt32(window.x), Vision::ToInt32(window.y), pUI->getSelectedDevice().getName().c_str());
        else
            _snprintf(windowName, sizeof(windowName), "Height Detect Base [%d, %d] @ %s", Vision::ToInt32(window.x), Vision::ToInt32(window.y), pUI->getSelectedDevice().getName().c_str());

        window.name = windowName;
        result = Engine::CreateWindow(window);
        if (result != Engine::OK) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            System->setTrackInfo(QString("Error at CreateWindow, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
            return;
        }
        else
            System->setTrackInfo(QString("Success to Create Window: %1.").arg(window.name.c_str()));

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

    m_pParent->UpdateInspWindowList();
}

void HeightDetectWidget::setCurrentWindow(const Engine::Window &window) {
    m_currentWindow = window;

    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    m_pCheckBoxMeasure->setChecked(window.usage == Engine::Window::Usage::HEIGHT_MEASURE);

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError)
        return;

    if (parse_doucment.isObject()) {
        QJsonObject obj = parse_doucment.object();

        m_pEditMinRange->setText(QString::number(obj.take("MinRange").toDouble() * ONE_HUNDRED_PERCENT));
        m_pEditMaxRange->setText(QString::number(obj.take("MaxRange").toDouble() * ONE_HUNDRED_PERCENT));
        m_pEditMaxRelHt->setText(QString::number(obj.take("MaxRelHt").toDouble()));
        m_pEditMinRelHt->setText(QString::number(obj.take("MinRelHt").toDouble()));
    }
}
