#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>

#include "InspBridgeWidget.h"
#include "../Common/SystemData.h"
#include "DataStoreAPI.h"
#include "VisionAPI.h"
#include "../include/IVisionUI.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../DataModule/QDetectObj.h"
#include "InspWindowWidget.h"
#include "../DataModule/CalcUtils.hpp"
#include "highgui.hpp"

using namespace AOI;

enum BASIC_PARAM
{
    INSP_MODE,
    CHECK_LEFT,
    CHECK_RIGHT,
    CHECK_UP,
    CHECK_DOWN,
    EDIT_MAX_WIDTH,
    EDIT_MAX_HEIGHT,
};

InspBridgeWidget::InspBridgeWidget(InspWindowWidget *parent)
: EditInspWindowBaseWidget(parent) {
    ui.setupUi(this);

    m_pComboBoxInspMode = std::make_unique<QComboBox>(this);
    m_pComboBoxInspMode->addItem("Inner");
    m_pComboBoxInspMode->addItem("Outer");
    ui.tableWidget->setCellWidget(INSP_MODE, DATA_COLUMN, m_pComboBoxInspMode.get());
    connect(m_pComboBoxInspMode.get(), SIGNAL(currentIndexChanged(int)), this, SLOT(on_inspModeChanged(int)));

    m_pCheckLeft = std::make_unique<QCheckBox>(ui.tableWidget);
    m_pCheckLeft->setChecked(true);
    ui.tableWidget->setCellWidget(CHECK_LEFT, DATA_COLUMN, m_pCheckLeft.get());

    m_pCheckRight = std::make_unique<QCheckBox>(ui.tableWidget);
    m_pCheckRight->setChecked(true);
    ui.tableWidget->setCellWidget(CHECK_RIGHT, DATA_COLUMN, m_pCheckRight.get());

    m_pCheckUp = std::make_unique<QCheckBox>(ui.tableWidget);
    m_pCheckUp->setChecked(true);
    ui.tableWidget->setCellWidget(CHECK_UP, DATA_COLUMN, m_pCheckUp.get());

    m_pCheckDown = std::make_unique<QCheckBox>(ui.tableWidget);
    m_pCheckDown->setChecked(true);
    ui.tableWidget->setCellWidget(CHECK_DOWN, DATA_COLUMN, m_pCheckDown.get());

    m_pEditMaxWidth = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditMaxWidth->setValidator(new QDoubleValidator(0, 10000, 2, m_pEditMaxWidth.get()));
    ui.tableWidget->setCellWidget(EDIT_MAX_WIDTH, DATA_COLUMN, m_pEditMaxWidth.get());

    m_pEditMaxHeight = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditMaxHeight->setValidator(new QDoubleValidator(0, 10000, 2, m_pEditMaxHeight.get()));
    ui.tableWidget->setCellWidget(EDIT_MAX_HEIGHT, DATA_COLUMN, m_pEditMaxHeight.get());

    on_inspModeChanged(m_pComboBoxInspMode->currentIndex());
}

InspBridgeWidget::~InspBridgeWidget() {
}

void InspBridgeWidget::on_inspModeChanged(int index) {
    if (static_cast<Vision::PR_INSP_BRIDGE_MODE> (index) == Vision::PR_INSP_BRIDGE_MODE::INNER) {
        ui.tableWidget->showRow(EDIT_MAX_WIDTH);
        ui.tableWidget->showRow(EDIT_MAX_HEIGHT);

        ui.tableWidget->hideRow(CHECK_LEFT);
        ui.tableWidget->hideRow(CHECK_RIGHT);
        ui.tableWidget->hideRow(CHECK_UP);
        ui.tableWidget->hideRow(CHECK_DOWN);
    }
    else if (static_cast<Vision::PR_INSP_BRIDGE_MODE> (index) == Vision::PR_INSP_BRIDGE_MODE::OUTER) {
        ui.tableWidget->hideRow(EDIT_MAX_WIDTH);
        ui.tableWidget->hideRow(EDIT_MAX_HEIGHT);

        ui.tableWidget->showRow(CHECK_LEFT);
        ui.tableWidget->showRow(CHECK_RIGHT);
        ui.tableWidget->showRow(CHECK_UP);
        ui.tableWidget->showRow(CHECK_DOWN);
    }
}

void InspBridgeWidget::setDefaultValue() {
    m_pComboBoxInspMode->setCurrentIndex(0);
    m_pCheckLeft->setChecked(true);
    m_pCheckRight->setChecked(true);
    m_pCheckUp->setChecked(true);
    m_pCheckDown->setChecked(true);
    m_pEditMaxWidth->setText(QString::number(100.f));
    m_pEditMaxHeight->setText(QString::number(100.f));
}

void InspBridgeWidget::setCurrentWindow(const Engine::Window &window) {
    m_currentWindow = window;

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        System->setTrackInfo(QString("Invalid inspection parameter encounted."));
        return;
    }
    QJsonObject jsonValue = parse_doucment.object();

    m_pComboBoxInspMode->setCurrentIndex(jsonValue["InspMode"].toInt());

    if (Vision::PR_INSP_BRIDGE_MODE::INNER == static_cast<Vision::PR_INSP_BRIDGE_MODE>(jsonValue["InspMode"].toInt())) {
        QJsonObject jsonInnerMode = jsonValue["InnerMode"].toObject();
        m_pEditMaxWidth->setText(QString::number(jsonInnerMode["MaxWidth"].toDouble()));
        m_pEditMaxHeight->setText(QString::number(jsonInnerMode["MaxHeight"].toDouble()));
    }
    else {
        QJsonObject jsonOuterMode = jsonValue["OuterMode"].toObject();
        m_pCheckLeft->setChecked(jsonOuterMode["CheckLeft"].toBool());
        m_pCheckRight->setChecked(jsonOuterMode["CheckRight"].toBool());
        m_pCheckUp->setChecked(jsonOuterMode["CheckUp"].toBool());
        m_pCheckDown->setChecked(jsonOuterMode["CheckDown"].toBool());
    }

    on_inspModeChanged(m_pComboBoxInspMode->currentIndex());
}

void InspBridgeWidget::tryInsp() {
    Vision::PR_INSP_BRIDGE_CMD stCmd;
    Vision::PR_INSP_BRIDGE_RPY stRpy;

    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    stCmd.enInspMode = static_cast<Vision::PR_INSP_BRIDGE_MODE>(m_pComboBoxInspMode->currentIndex());

    if (Vision::PR_INSP_BRIDGE_MODE::INNER == stCmd.enInspMode) {
        stCmd.stInnerInspCriteria.fMaxLengthX = m_pEditMaxWidth->text().toFloat() / dResolutionX;
        stCmd.stInnerInspCriteria.fMaxLengthY = m_pEditMaxHeight->text().toFloat() / dResolutionY;
    }
    else {
        // Ask user to select the search window
        auto pUI = getModule<IVisionUI>(UI_MODEL);
        if (Engine::Window::Usage::INSP_BRIDGE != m_currentWindow.usage) {
            auto rectROI = pUI->getSelectedROI();
            cv::Rect rectDefaultSrchWindow = CalcUtils::resizeRect(rectROI, cv::Size2f(rectROI.width + 30, rectROI.height + 30));
            pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_SRCH_WINDOW);
            pUI->setSrchWindow(rectDefaultSrchWindow);
            auto nReturn = System->showInteractMessage(QStringLiteral("测试锡桥检测框"), QStringLiteral("请拖动鼠标选择向外搜寻锡桥范围"));
            if (nReturn != QDialog::Accepted)
                return;
        }

        stCmd.rectOuterSrchWindow = pUI->getSrchWindow();

        if (m_pCheckLeft->isChecked())
            stCmd.vecOuterInspDirection.push_back(Vision::PR_DIRECTION::LEFT);
        if (m_pCheckRight->isChecked())
            stCmd.vecOuterInspDirection.push_back(Vision::PR_DIRECTION::RIGHT);
        if (m_pCheckUp->isChecked())
            stCmd.vecOuterInspDirection.push_back(Vision::PR_DIRECTION::UP);
        if (m_pCheckDown->isChecked())
            stCmd.vecOuterInspDirection.push_back(Vision::PR_DIRECTION::DOWN);
    }

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    stCmd.matInputImg = pUI->getImage();
    cv::cvtColor(stCmd.matInputImg, stCmd.matInputImg, CV_BGR2GRAY);
    stCmd.rectROI = pUI->getSelectedROI();

    cv::Mat matProcessedROI = m_pParent->getColorWidget()->getProcessedImage();
    cv::Mat matOrigianlROI;
    if (Vision::PR_INSP_BRIDGE_MODE::INNER == stCmd.enInspMode)
        matOrigianlROI = cv::Mat(stCmd.matInputImg, stCmd.rectROI);
    else
        matOrigianlROI = cv::Mat(stCmd.matInputImg, stCmd.rectOuterSrchWindow);
    matProcessedROI.copyTo(matOrigianlROI);

    if (stCmd.rectROI.width <= 0 || stCmd.rectROI.height <= 0) {
        QMessageBox::critical(this, QStringLiteral("Add Inspect Bridge Window"), QStringLiteral("Please select a ROI to do inspection."));
        return;
    }

    Vision::PR_InspBridge(&stCmd, &stRpy);
    QString strMsg;
    strMsg.sprintf("Inspect Status %d, bridge count %d", Vision::ToInt32(stRpy.enStatus), stRpy.vecBridgeWindow.size());
    QMessageBox::information(this, "Insp void", strMsg);
}

void InspBridgeWidget::confirmWindow(OPERATION enOperation) {
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();
    auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();

    QString strTitle(QStringLiteral("确认锡桥检测框"));
    auto pUI = getModule<IVisionUI>(UI_MODEL);
    auto rectROI = pUI->getSelectedROI();
    if (rectROI.width <= 0 || rectROI.height <= 0) {
        QMessageBox::critical(this, strTitle, QStringLiteral("Please select a ROI to do inspection."));
        return;
    }

    Vision::PR_INSP_BRIDGE_MODE enInspMode = static_cast<Vision::PR_INSP_BRIDGE_MODE>(m_pComboBoxInspMode->currentIndex());
    if (Vision::PR_INSP_BRIDGE_MODE::OUTER == enInspMode) {
        // Ask user to select the search window
        pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_SRCH_WINDOW);
        if (OPERATION::ADD == enOperation) {
            cv::Rect rectSrchWindow = CalcUtils::resizeRect(rectROI, cv::Size2f(rectROI.width + 30, rectROI.height + 30));
            pUI->setSrchWindow(rectSrchWindow);
        }
        auto nReturn = System->showInteractMessage(strTitle, QStringLiteral("请拖动鼠标选择向外搜寻锡桥范围"));
        if (nReturn != QDialog::Accepted)
            return;
    }

    cv::Rect rectSrchWindow;

    QJsonObject jsonValue;
    jsonValue.insert("InspMode", m_pComboBoxInspMode->currentIndex());
    if (Vision::PR_INSP_HOLE_MODE::RATIO == static_cast<Vision::PR_INSP_HOLE_MODE>(m_pComboBoxInspMode->currentIndex())) {
        QJsonObject jsonInnerMode;
        jsonInnerMode["MaxWidth"] = m_pEditMaxWidth->text().toFloat();
        jsonInnerMode["MaxHeight"] = m_pEditMaxHeight->text().toFloat();
        jsonValue["InnerMode"] = jsonInnerMode;
    }
    else {
        QJsonObject jsonOuterMode;
        jsonOuterMode["CheckLeft"] = m_pCheckLeft->isChecked();
        jsonOuterMode["CheckRight"] = m_pCheckRight->isChecked();
        jsonOuterMode["CheckUp"] = m_pCheckUp->isChecked();
        jsonOuterMode["CheckDown"] = m_pCheckDown->isChecked();
        jsonValue["OuterMode"] = jsonOuterMode;
        rectSrchWindow = pUI->getSrchWindow();
    }

    QJsonDocument document;
    document.setObject(jsonValue);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);

    Engine::Window window;
    window.lightId = m_pParent->getSelectedLighting() + 1;
    window.usage = Engine::Window::Usage::INSP_BRIDGE;
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
    window.colorParams = m_pParent->getColorWidget()->getJsonFormattedParams();

    QDetectObj detectObj(window.Id, window.name.c_str());
    cv::Point2f ptCenter(window.x / dResolutionX, window.y / dResolutionY);
    if (bBoardRotated)
        ptCenter.x = nBigImgWidth - ptCenter.x;
    else
        ptCenter.y = nBigImgHeight - ptCenter.y; //In cad, up is positive, but in image, down is positive.

    detectObj.setFrame(cv::RotatedRect(ptCenter, rectROI.size(), window.angle));
    detectObj.setSrchWindow(cv::RotatedRect(ptCenter, rectSrchWindow.size(), window.angle));

    int result = Engine::OK;
    auto vecDetectObjs = pUI->getDetectObjs();
    if (OPERATION::ADD == enOperation) {
        window.deviceId = pUI->getSelectedDevice().getId();
        char windowName[100];
        _snprintf(windowName, sizeof(windowName), "Insp Bridge [%d, %d] @ %s", Vision::ToInt32(window.x), Vision::ToInt32(window.y), pUI->getSelectedDevice().getName().c_str());
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
        window.Id = m_currentWindow.Id;
        window.name = m_currentWindow.name;
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
