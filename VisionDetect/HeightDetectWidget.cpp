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
#include "../DataModule/CalcUtils.hpp"

using namespace NFG::AOI;
using namespace AOI;

enum BASIC_PARAM
{
    MEASURE_TYPE_ATTRI,
    BASE_TYPE_ATTRI,
    BASE_SCALE_ATTRI,
    RANGE_MIN_ATTRI,
    RANGE_MAX_ATTRI,
    MAX_REL_HT,
    MIN_REL_HT,
};

enum class HDW_MEASURE_TYPE {
    EN_MEASURE_TYPE,
    EN_BASE_TYPE,
};

enum class HDW_BASE_TYPE {
    EN_GLOBAL_BASE_TYPE,
    EN_MANUAL_TYPE,
};

HeightDetectWidget::HeightDetectWidget(InspWindowWidget *parent)
:EditInspWindowBaseWidget(parent) {
    ui.setupUi(this);   

    m_pComboxMeasureType = std::make_unique<QComboBox>(ui.tableWidget);
    m_pComboxMeasureType->addItem(QStringLiteral("测量面"));
    m_pComboxMeasureType->addItem(QStringLiteral("基准面"));
    ui.tableWidget->setCellWidget(MEASURE_TYPE_ATTRI, DATA_COLUMN, m_pComboxMeasureType.get());
    connect(m_pComboxMeasureType.get(), SIGNAL(currentIndexChanged(int)), this, SLOT(on_measureChanged(int)));

    m_pComboxBaseType = std::make_unique<QComboBox>(ui.tableWidget);
    m_pComboxBaseType->addItem(QStringLiteral("自动生成"));
    m_pComboxBaseType->addItem(QStringLiteral("手动添加"));
    ui.tableWidget->setCellWidget(BASE_TYPE_ATTRI, DATA_COLUMN, m_pComboxBaseType.get());
    connect(m_pComboxBaseType.get(), SIGNAL(currentIndexChanged(int)), this, SLOT(on_baseTypeChanged(int)));

    m_pEditBaseScale = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditBaseScale->setValidator(new QIntValidator(2, 5, m_pEditBaseScale.get()));
    ui.tableWidget->setCellWidget(BASE_SCALE_ATTRI, DATA_COLUMN, m_pEditBaseScale.get());

    m_pEditMinRange = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditMinRange->setValidator(new QDoubleValidator(0, 100, 2, m_pEditMinRange.get()));
    ui.tableWidget->setCellWidget(RANGE_MIN_ATTRI, DATA_COLUMN, m_pEditMinRange.get());

    m_pEditMaxRange = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditMaxRange->setValidator(new QDoubleValidator(0, 100, 2, m_pEditMaxRange.get()));
    ui.tableWidget->setCellWidget(RANGE_MAX_ATTRI, DATA_COLUMN, m_pEditMaxRange.get());

    m_pSpecAndResultMaxRelHt = std::make_unique<SpecAndResultWidget>(ui.tableWidget, -10000, 10000);
    ui.tableWidget->setCellWidget(MAX_REL_HT, DATA_COLUMN, m_pSpecAndResultMaxRelHt.get());

    m_pSpecAndResultMinRelHt = std::make_unique<SpecAndResultWidget>(ui.tableWidget, -10000, 10000);
    ui.tableWidget->setCellWidget(MIN_REL_HT, DATA_COLUMN, m_pSpecAndResultMinRelHt.get());
}

HeightDetectWidget::~HeightDetectWidget() {
}

void HeightDetectWidget::setDefaultValue() {
    m_pComboxMeasureType->setCurrentIndex(0);
    m_pComboxBaseType->setCurrentIndex(0);
    m_pEditBaseScale->setText("2");
    m_pEditMinRange->setText("30");
    m_pEditMaxRange->setText("70");
    m_pSpecAndResultMaxRelHt->setSpec(3000);
    m_pSpecAndResultMinRelHt->setSpec(1000);
}

void HeightDetectWidget::tryInsp() {
    if (static_cast<HDW_MEASURE_TYPE> (m_pComboxMeasureType->currentIndex()) == HDW_MEASURE_TYPE::EN_BASE_TYPE) {
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
    auto rectROI = pUI->getSelectedROI();
    if (rectROI.width <= 0 || rectROI.height <= 0) {
        QMessageBox::critical(this, QStringLiteral("高度检测框"), QStringLiteral("Please select a ROI to do inspection."));
        return;
    }
    stCmd.rectROI = rectROI;

    bool bGlobalBase = (static_cast<HDW_BASE_TYPE> (m_pComboxBaseType->currentIndex()) == HDW_BASE_TYPE::EN_GLOBAL_BASE_TYPE);
    if (bGlobalBase)
    {     
        Engine::WindowVector vecWindow;
        auto result = Engine::GetAllWindows(vecWindow);
        if (result != Engine::OK) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            System->setTrackInfo(QString("Error at GetAllWindows, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
            return;
        }

        Engine::Window window;
        for each (Engine::Window win in vecWindow)
        {
            if (win.usage == Engine::Window::Usage::HEIGHT_BASE_GLOBAL)
            {
                window = win;
                break;
            }
        }

        QJsonParseError json_error;
        QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
        if (json_error.error != QJsonParseError::NoError) {
            QMessageBox::critical(this, QStringLiteral("高度检测框"), QStringLiteral("Please check global base params to do inspection."));
            return;
        }
        QJsonObject jsonValue = parse_doucment.object();

        cv::Vec3b color; int nRn = 0, nTn = 0;
        color[0] = jsonValue["ClrRVal"].toInt();
        color[1] = jsonValue["ClrGVal"].toInt();
        color[2] = jsonValue["ClrBVal"].toInt();
        nRn = jsonValue["RnValue"].toInt();
        nTn = jsonValue["TnValue"].toInt();

        auto pColorWidget = m_pParent->getColorWidget();
        cv::Mat matImage = pUI->getImage();

        int nBigImgWidth = matImage.cols / dCombinedImageScale;
        int nBigImgHeight = matImage.rows / dCombinedImageScale;
        auto x = m_currentWindow.x / dResolutionX;
        auto y = m_currentWindow.y / dResolutionY;
        if (bBoardRotated)
            x = nBigImgWidth  - x;
        else
            y = nBigImgHeight - y; //In cad, up is positive, but in image, down is positive.

        auto width  = m_currentWindow.width  / dResolutionX;
        auto height = m_currentWindow.height / dResolutionY;

        double dBaseScale = m_pEditBaseScale->text().toInt();

        cv::Rect2f rectBase(x, y, width, height);
        cv::Rect rectBaseDetectWin = CalcUtils::resizeRect(rectBase, cv::Size2f(rectBase.width * dBaseScale, rectBase.height * dBaseScale));
        if (rectBaseDetectWin.x < 0) rectBaseDetectWin.x = 0;
        else if ((rectBaseDetectWin.x + rectBaseDetectWin.width) >= matImage.cols) rectBaseDetectWin.width = rectBase.width;
        if (rectBaseDetectWin.y < 0) rectBaseDetectWin.y = 0;
        else if ((rectBaseDetectWin.y + rectBaseDetectWin.height) >= matImage.rows) rectBaseDetectWin.height = rectBase.height;
        cv::Mat matROI(matImage, rectBaseDetectWin);
        pColorWidget->setImage(matROI);

        pColorWidget->holdColorImage(color, nRn, nTn);
        cv::Mat matMask = pColorWidget->getProcessedImage();
        pColorWidget->releaseColorImage();

        cv::Mat matBigMask = cv::Mat::ones(matImage.size(), CV_8UC1);
        matBigMask *= Vision::PR_MAX_GRAY_LEVEL;
        cv::Mat matMaskROI(matBigMask, cv::Rect(rectBaseDetectWin));
        matMask.copyTo(matMaskROI);
        stCmd.matMask = matBigMask;

        stCmd.vecRectBases.push_back(rectBaseDetectWin);
    }
    else
    {
        auto matImage = pUI->getImage();
        int nBigImgWidth = matImage.cols / dCombinedImageScale;
        int nBigImgHeight = matImage.rows / dCombinedImageScale;

        for (const auto &window : m_windowGroup.vecWindows) {
            if (Engine::Window::Usage::HEIGHT_BASE == window.usage) {
                auto x = window.x / dResolutionX;
                auto y = window.y / dResolutionY;
                if (bBoardRotated)
                    x = nBigImgWidth  - x;
                else
                    y = nBigImgHeight - y; //In cad, up is positive, but in image, down is positive.

                auto width = window.width / dResolutionX;
                auto height = window.height / dResolutionY;
                cv::RotatedRect rrBaseRect(cv::Point2f(x, y), cv::Size2f(width, height), window.angle);
                stCmd.vecRectBases.push_back(rrBaseRect.boundingRect());
            }
        }
    }

    float fMaxHeight = m_pSpecAndResultMaxRelHt->getSpec();
    float fMinHeigth = m_pSpecAndResultMinRelHt->getSpec();
    Vision::PR_Calc3DHeightDiff(&stCmd, &stRpy);
    float height = stRpy.fHeightDiff * MM_TO_UM;
    m_pSpecAndResultMaxRelHt->setResult(height);
    m_pSpecAndResultMinRelHt->setResult(height);
    bool bPassed = (height < fMaxHeight) && (height > fMinHeigth);
    if (! bPassed) {
        QString strMsg;
        strMsg.sprintf("Inspect Status %d, %s, height (%f)", Vision::ToInt32(stRpy.enStatus), bPassed ? "pass" : "not pass", height);
        QMessageBox::information(this, "Height Detect", strMsg);
    }
}

void HeightDetectWidget::confirmWindow(OPERATION enOperation) {
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();
    auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();

    QJsonObject json;
    json.insert("MinRange", m_pEditMinRange->text().toFloat() / ONE_HUNDRED_PERCENT);
    json.insert("MaxRange", m_pEditMaxRange->text().toFloat() / ONE_HUNDRED_PERCENT);
    json.insert("MaxRelHt", m_pSpecAndResultMaxRelHt->getSpec());
    json.insert("MinRelHt", m_pSpecAndResultMinRelHt->getSpec());
    json.insert("GlobalBase", (static_cast<HDW_BASE_TYPE> (m_pComboxBaseType->currentIndex()) == HDW_BASE_TYPE::EN_GLOBAL_BASE_TYPE) ? true : false);
    json.insert("GlobalBaseScale", m_pEditBaseScale->text().toInt());

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
    window.usage = (static_cast<HDW_MEASURE_TYPE> (m_pComboxMeasureType->currentIndex()) == HDW_MEASURE_TYPE::EN_MEASURE_TYPE) ? Engine::Window::Usage::HEIGHT_MEASURE : Engine::Window::Usage::HEIGHT_BASE;
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

    m_pParent->updateInspWindowList();
}

void HeightDetectWidget::setCurrentWindow(const Engine::Window &window) {
    m_currentWindow = window;

    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    m_pComboxMeasureType->setCurrentIndex(static_cast<int>((window.usage == Engine::Window::Usage::HEIGHT_MEASURE) ? HDW_MEASURE_TYPE::EN_MEASURE_TYPE : HDW_MEASURE_TYPE::EN_BASE_TYPE));

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError)
        return;

    if (parse_doucment.isObject()) {
        QJsonObject obj = parse_doucment.object();

        m_pEditMinRange->setText(QString::number(obj.take("MinRange").toDouble() * ONE_HUNDRED_PERCENT));
        m_pEditMaxRange->setText(QString::number(obj.take("MaxRange").toDouble() * ONE_HUNDRED_PERCENT));
        m_pSpecAndResultMaxRelHt->setSpec(obj.take("MaxRelHt").toDouble());
        m_pSpecAndResultMaxRelHt->clearResult();
        m_pSpecAndResultMinRelHt->setSpec(obj.take("MinRelHt").toDouble());
        m_pSpecAndResultMinRelHt->clearResult();

        m_pComboxBaseType->setCurrentIndex(static_cast<int>(obj.take("GlobalBase").toBool() ? HDW_BASE_TYPE::EN_GLOBAL_BASE_TYPE : HDW_BASE_TYPE::EN_MANUAL_TYPE)); 
        m_pEditBaseScale->setText(QString::number(obj.take("GlobalBaseScale").toInt()));
    }
}

void HeightDetectWidget::on_measureChanged(int index) {

    if (static_cast<HDW_MEASURE_TYPE> (index) == HDW_MEASURE_TYPE::EN_MEASURE_TYPE)
    {
        ui.tableWidget->showRow(BASE_TYPE_ATTRI);
        ui.tableWidget->showRow(BASE_SCALE_ATTRI);
        ui.tableWidget->showRow(MAX_REL_HT);
        ui.tableWidget->showRow(MIN_REL_HT);
    }
    else if (static_cast<HDW_MEASURE_TYPE> (index) == HDW_MEASURE_TYPE::EN_BASE_TYPE)
    {
        ui.tableWidget->hideRow(BASE_TYPE_ATTRI);
        ui.tableWidget->hideRow(BASE_SCALE_ATTRI);
        ui.tableWidget->hideRow(MAX_REL_HT);
        ui.tableWidget->hideRow(MIN_REL_HT);
    }  
}

void HeightDetectWidget::on_baseTypeChanged(int index) {
    if (static_cast<HDW_BASE_TYPE> (index) == HDW_BASE_TYPE::EN_GLOBAL_BASE_TYPE)
    {
        //m_pComboxMeasureType->setEnabled(false);
        m_pEditBaseScale->setEnabled(true);
    }
    else if (static_cast<HDW_BASE_TYPE> (index) == HDW_BASE_TYPE::EN_MANUAL_TYPE)
    {
        //m_pComboxMeasureType->setEnabled(true);
        m_pEditBaseScale->setEnabled(false);
    }
}


