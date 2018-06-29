#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>

#include "InspPolarityWidget.h"
#include "InspWindowWidget.h"
#include "../Common/SystemData.h"
#include "DataStoreAPI.h"
#include "VisionAPI.h"
#include "../include/IVisionUI.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../DataModule/QDetectObj.h"

using namespace NFG::AOI;
using namespace AOI;

enum BASIC_PARAM
{
    WINDOW_TYPE,
    ATTRIBUTE,
    INTENSITY_DIFF_TOL,
};

InspPolarityWidget::InspPolarityWidget(InspWindowWidget *parent)
: EditInspWindowBaseWidget(parent) {
    ui.setupUi(this);

    m_pComboBoxType = std::make_unique<QComboBox>(ui.tableWidget);
    m_pComboBoxType->addItem(QStringLiteral("检测"));
    m_pComboBoxType->addItem(QStringLiteral("基准"));
    ui.tableWidget->setCellWidget(WINDOW_TYPE, DATA_COLUMN, m_pComboBoxType.get());
    connect(m_pComboBoxType.get(), SIGNAL(currentIndexChanged(int)), this, SLOT(onTypeChange(int)));

    m_pComboBoxAttribute = std::make_unique<QComboBox>(ui.tableWidget);
    m_pComboBoxAttribute->addItem("Bright");
    m_pComboBoxAttribute->addItem("Dark");
    ui.tableWidget->setCellWidget(ATTRIBUTE, DATA_COLUMN, m_pComboBoxAttribute.get());

    m_pSpecAndResultIntensityDiffTol = std::make_unique<SpecAndResultWidget>(ui.tableWidget, 1, 255, 0);
    ui.tableWidget->setCellWidget(INTENSITY_DIFF_TOL, DATA_COLUMN, m_pSpecAndResultIntensityDiffTol.get());

    setDefaultValue();
}

InspPolarityWidget::~InspPolarityWidget() {
}

void InspPolarityWidget::setDefaultValue() {
    m_pComboBoxType->setCurrentIndex(0);
    m_pComboBoxAttribute->setCurrentIndex(0);
    m_pSpecAndResultIntensityDiffTol->setSpec(50);
}

void InspPolarityWidget::setCurrentWindow(const Engine::Window &window) {
    m_currentWindow = window;

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        System->setTrackInfo(QString("Invalid inspection parameter encounted."));
        return;
    }

    if (!parse_doucment.isObject()) {
        System->setTrackInfo(QString("Invalid inspection parameter encounted."));
        return;
    }

    QJsonObject obj = parse_doucment.object();

    m_pComboBoxType->setCurrentIndex(obj.take("Type").toInt());
    m_pComboBoxAttribute->setCurrentIndex(obj.take("Attribute").toInt());
    m_pSpecAndResultIntensityDiffTol->setSpec(obj.take("IntensityDiffTol").toInt());
    m_pSpecAndResultIntensityDiffTol->clearResult();
}

void InspPolarityWidget::tryInsp() {
    if (m_pComboBoxType->currentIndex() != 0) {
        QString strMsg;
        strMsg.sprintf("Select inspection window to inspect polarity!");
        QMessageBox::information(this, "Inspect Polarity", strMsg);
        return;
    }

    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();
    auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();

    Vision::PR_INSP_POLARITY_CMD stCmd;
    Vision::PR_INSP_POLARITY_RPY stRpy;

    stCmd.enInspROIAttribute = static_cast<Vision::PR_OBJECT_ATTRIBUTE>(m_pComboBoxAttribute->currentIndex());
    stCmd.nGrayScaleDiffTol = Vision::ToInt32(m_pSpecAndResultIntensityDiffTol->getSpec());

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    stCmd.matInputImg = pUI->getImage();
    cv::Rect rectROI = pUI->getSelectedROI();
    if (rectROI.width <= 0 || rectROI.height <= 0) {
        QMessageBox::critical(this, QStringLiteral("极性检测框"), QStringLiteral("Please select a ROI to do inspection."));
        return;
    }
    stCmd.rectInspROI = rectROI;

    auto matImage = pUI->getImage();
    int nBigImgWidth = matImage.cols / dCombinedImageScale;
    int nBigImgHeight = matImage.rows / dCombinedImageScale;

    for (const auto &window : m_windowGroup.vecWindows) {
        if (Engine::Window::Usage::INSP_POLARITY_REF == window.usage) {
            auto x = window.x / dResolutionX;
            auto y = window.y / dResolutionY;
            if (bBoardRotated)
                x = nBigImgWidth - x;
            else
                y = nBigImgHeight - y; //In cad, up is positive, but in image, down is positive.

            auto width = window.width / dResolutionX;
            auto height = window.height / dResolutionY;
            cv::RotatedRect rrBaseRect(cv::Point2f(x, y), cv::Size2f(width, height), window.angle);
            stCmd.rectCompareROI = rrBaseRect.boundingRect();
            break;
        }
    }

    Vision::PR_InspPolarity(&stCmd, &stRpy);
    QString strMsg;
    strMsg.sprintf("Inspect Status %d, Intensity difference(%d)", Vision::ToInt32(stRpy.enStatus), stRpy.nGrayScaleDiff);
    QMessageBox::information(this, "Inspect Polarity", strMsg);
}

void InspPolarityWidget::confirmWindow(OPERATION enOperation) {
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();
    auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();

    QJsonObject json;
    json.insert("Type", m_pComboBoxType->currentIndex());
    json.insert("Attribute", m_pComboBoxAttribute->currentIndex());
    json.insert("IntensityDiffTol", Vision::ToInt32(m_pSpecAndResultIntensityDiffTol->getSpec()));

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
    window.usage = m_pComboBoxType->currentIndex() == 0 ? Engine::Window::Usage::INSP_POLARITY : Engine::Window::Usage::INSP_POLARITY_REF;
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
        if (Engine::Window::Usage::INSP_POLARITY == window.usage)
            _snprintf(windowName, sizeof(windowName), "Inspect Polarity [%d, %d] @ %s", Vision::ToInt32(window.x), Vision::ToInt32(window.y), pUI->getSelectedDevice().getName().c_str());
        else
            _snprintf(windowName, sizeof(windowName), "Inspect Polarity Ref [%d, %d] @ %s", Vision::ToInt32(window.x), Vision::ToInt32(window.y), pUI->getSelectedDevice().getName().c_str());

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

void InspPolarityWidget::onTypeChange(int index) {
    if (0 == index) {
        ui.tableWidget->showRow(ATTRIBUTE);
        ui.tableWidget->showRow(INTENSITY_DIFF_TOL);
    }
    else {
        ui.tableWidget->hideRow(ATTRIBUTE);
        ui.tableWidget->hideRow(INTENSITY_DIFF_TOL);
    }
}