#include "HeightDetectWidget.h"
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>

#include "../Common/SystemData.h"
#include "DataStoreAPI.h"
#include "VisionAPI.h"
#include "../include/IVisionUI.h"
#include "../include/IData.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../DataModule/QDetectObj.h"
#include "InspWindowWidget.h"
#include "../DataModule/CalcUtils.hpp"
#include "../DataModule/DataUtils.h"

using namespace NFG::AOI;
using namespace AOI;

enum BASIC_PARAM
{
    MEASURE_TYPE_ATTRI,
    BASE_TYPE_ATTRI,
    BASE_SCALE_ATTRI,
    RANGE_MIN_ATTRI,
    RANGE_MAX_ATTRI,
    MAX_ABS_HT,
    MAX_MAXERR_HT,
    MIN_MINERR_HT,
    MAX_REL_HT,
    MAX_LR_REL_HT,
    MAX_TB_REL_HT,
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

    m_pSpecAndResultAbsHt = std::make_unique<SpecAndResultWidget>(ui.tableWidget, -10000, 10000);
    ui.tableWidget->setCellWidget(MAX_ABS_HT, DATA_COLUMN, m_pSpecAndResultAbsHt.get());

    m_pSpecAndResultMaxHtErr = std::make_unique<SpecAndResultWidget>(ui.tableWidget, 0, 10000);
    ui.tableWidget->setCellWidget(MAX_MAXERR_HT, DATA_COLUMN, m_pSpecAndResultMaxHtErr.get());

    m_pSpecAndResultMinHtErr = std::make_unique<SpecAndResultWidget>(ui.tableWidget, -10000, 0);
    ui.tableWidget->setCellWidget(MIN_MINERR_HT, DATA_COLUMN, m_pSpecAndResultMinHtErr.get());

    m_pCheckBoxRelHt = std::make_unique<QCheckBox>(ui.tableWidget);
    ui.tableWidget->setCellWidget(MAX_REL_HT, DATA_COLUMN, m_pCheckBoxRelHt.get());
    connect(m_pCheckBoxRelHt.get(), SIGNAL(toggled(bool)), this, SLOT(onRelHtChanged(bool)));

    m_pSpecAndResultLftRgtRelHt = std::make_unique<SpecAndResultWidget>(ui.tableWidget, -10000, 10000);
    ui.tableWidget->setCellWidget(MAX_LR_REL_HT, DATA_COLUMN, m_pSpecAndResultLftRgtRelHt.get());

    m_pSpecAndResultTopBtmRelHt = std::make_unique<SpecAndResultWidget>(ui.tableWidget, -10000, 10000);
    ui.tableWidget->setCellWidget(MAX_TB_REL_HT, DATA_COLUMN, m_pSpecAndResultTopBtmRelHt.get());

    m_pCheckBoxRelHt->setChecked(false);
    m_pCheckBoxRelHt->setEnabled(false);
    m_pSpecAndResultLftRgtRelHt->setEnabled(false);
    m_pSpecAndResultTopBtmRelHt->setEnabled(false);
}

HeightDetectWidget::~HeightDetectWidget() {
}

void HeightDetectWidget::setDefaultValue() {
    m_pComboxMeasureType->setCurrentIndex(0);
    m_pComboxBaseType->setCurrentIndex(0);
    m_pEditBaseScale->setText("2");
    m_pEditMinRange->setText("30");
    m_pEditMaxRange->setText("70");
    m_pSpecAndResultAbsHt->setSpec(2000);
    m_pSpecAndResultMaxHtErr->setSpec(200);
    m_pSpecAndResultMinHtErr->setSpec(-200);
    m_pCheckBoxRelHt->setChecked(false);
    m_pCheckBoxRelHt->setEnabled(false);
    m_pSpecAndResultLftRgtRelHt->setEnabled(false);
    m_pSpecAndResultTopBtmRelHt->setEnabled(false);
    m_pSpecAndResultLftRgtRelHt->setSpec(100);
    m_pSpecAndResultTopBtmRelHt->setSpec(100);
    m_bIsTryInspected = false;
    m_currentWindow.usage = Engine::Window::Usage::UNDEFINED;
}

void HeightDetectWidget::tryInsp() {
    if (static_cast<HDW_MEASURE_TYPE> (m_pComboxMeasureType->currentIndex()) == HDW_MEASURE_TYPE::EN_BASE_TYPE) {
        QString strMsg;
        strMsg.sprintf("Select Measure Window to measure height!");
        QMessageBox::information(this, "Measure Height", strMsg);
        return;
    }

    QString strTitle(QStringLiteral("3D 高度检测"));
    auto pUI = getModule<IVisionUI>(UI_MODEL);

    if (Engine::Window::Usage::UNDEFINED == m_currentWindow.usage) {
        auto nReturn = System->showInteractMessage(strTitle, QStringLiteral("是否检测相对高度？"));
        if (nReturn == QDialog::Accepted)
        {
            //nReturn = System->showInteractMessage(strTitle, QStringLiteral("请拖动鼠标选择检测窗口"));
            //if (nReturn != QDialog::Accepted)
            //    return;

            pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_SELECT_SUB_ROI);
            nReturn = System->showInteractMessage(strTitle, QStringLiteral("请拖动鼠标选择第一个小检测窗口"));
            if (nReturn != QDialog::Accepted)
                return;

            auto vecSubROIs = pUI->getSubROIs();
            if (vecSubROIs.empty()) {
                System->showMessage(strTitle, QStringLiteral("请选择小检测窗口!"));
                return;
            }

            auto lastSubROI = vecSubROIs.back();
            cv::Point2f ptWindowCtr(lastSubROI.x + lastSubROI.width / 2.f, lastSubROI.y + lastSubROI.height / 2.f);
            auto selectedDevice = pUI->getSelectedDevice();
            auto ptCenter = selectedDevice.getWindow().center;
            auto rectDevice = selectedDevice.getWindow().boundingRect();

            cv::Point ptNewWindowCtr;
            ptNewWindowCtr.x = 2 * ptCenter.x - ptWindowCtr.x;
            ptNewWindowCtr.y = ptWindowCtr.y;
            cv::Rect rectNewWindow(ptNewWindowCtr.x - lastSubROI.width / 2, ptNewWindowCtr.y - lastSubROI.height / 2, lastSubROI.width, lastSubROI.height);
            vecSubROIs.push_back(rectNewWindow);
 
            ptNewWindowCtr.x = ptWindowCtr.x;
            ptNewWindowCtr.y = 2 * ptCenter.y - ptWindowCtr.y;
            rectNewWindow = cv::Rect(ptNewWindowCtr.x - lastSubROI.width / 2, ptNewWindowCtr.y - lastSubROI.height / 2, lastSubROI.width, lastSubROI.height);
            vecSubROIs.push_back(rectNewWindow);

            ptNewWindowCtr.x = 2 * ptCenter.x - ptWindowCtr.x;
            ptNewWindowCtr.y = 2 * ptCenter.y - ptWindowCtr.y;
            rectNewWindow = cv::Rect(ptNewWindowCtr.x - lastSubROI.width / 2, ptNewWindowCtr.y - lastSubROI.height / 2, lastSubROI.width, lastSubROI.height);
            vecSubROIs.push_back(rectNewWindow); 

            pUI->setSubROIs(vecSubROIs);

            m_pCheckBoxRelHt->setChecked(true);
        }
        else
        {
            m_pCheckBoxRelHt->setChecked(false);
        }
    }

    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    Int32 bBoardRotated = 0; Engine::GetParameter("BOARD_ROTATED", bBoardRotated, false);
    float fCombinedImageScale = 1.f; Engine::GetParameter("ScanImageZoomFactor", fCombinedImageScale, 1.f);

    Vision::PR_CALC_3D_HEIGHT_DIFF_CMD stCmd;
    Vision::PR_CALC_3D_HEIGHT_DIFF_RPY stRpy;

    stCmd.fEffectHRatioStart = m_pEditMinRange->text().toFloat() / ONE_HUNDRED_PERCENT;
    stCmd.fEffectHRatioEnd = m_pEditMaxRange->text().toFloat() / ONE_HUNDRED_PERCENT;
 
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
        stCmd.matMask = getGlobalBaseMask(m_currentWindow);
        stCmd.vecRectBases.push_back(calcWinBaseRect(m_currentWindow));
    }
    else {
        auto matImage = pUI->getImage();
        int nBigImgWidth  = matImage.cols / fCombinedImageScale;
        int nBigImgHeight = matImage.rows / fCombinedImageScale;

        for (const auto &window : m_windowGroup.vecWindows) {
            if (Engine::Window::Usage::HEIGHT_BASE == window.usage) {
                auto x = window.x / dResolutionX;
                auto y = window.y / dResolutionY;
                if (bBoardRotated)
                    x = nBigImgWidth  - x;
                else
                    y = nBigImgHeight - y; //In cad, up is positive, but in image, down is positive.

                auto width  = window.width  / dResolutionX;
                auto height = window.height / dResolutionY;
                cv::RotatedRect rrBaseRect(cv::Point2f(x, y), cv::Size2f(width, height), window.angle);
                stCmd.vecRectBases.push_back(rrBaseRect.boundingRect());
            }
        }
    }

    float fAbsHeight = m_pSpecAndResultAbsHt->getSpec();
    float fMaxHeight = m_pSpecAndResultMaxHtErr->getSpec();
    float fMinHeigth = m_pSpecAndResultMinHtErr->getSpec();

    Vision::PR_Calc3DHeightDiff(&stCmd, &stRpy);
    float height = stRpy.fHeightDiff * MM_TO_UM;
    float heightErr = height - fAbsHeight;

    m_pSpecAndResultAbsHt->setResult(height);
    m_pSpecAndResultMaxHtErr->setResult(heightErr > 0 ? heightErr : 0);
    m_pSpecAndResultMinHtErr->setResult(heightErr < 0 ? heightErr : 0);

    if (bGlobalBase) {
        cv::Mat matDisplayImage = pUI->getImage().clone();
        matDisplayImage.setTo(cv::Scalar(0, 255, 255), stCmd.matMask);
        pUI->displayImage(matDisplayImage);
    }

    bool bPassed = (heightErr <= fMaxHeight) && (heightErr >= fMinHeigth);
    if (! bPassed) {
        QString strMsg;
        strMsg.sprintf("Inspect Status %d, %s, height (%f)", Vision::ToInt32(stRpy.enStatus), bPassed ? "pass" : "not pass", height);
        QMessageBox::information(this, "Height Detect", strMsg);
    }

    calcRelativeValue();

    m_bIsTryInspected = true;
}

void HeightDetectWidget::calcRelativeValue()
{
    // Get the relative height value 
    m_pSpecAndResultLftRgtRelHt->setResult(0);
    m_pSpecAndResultTopBtmRelHt->setResult(0);

    bool bRelHeight = m_pCheckBoxRelHt->isChecked();
    if (bRelHeight)
    {
        auto pUI = getModule<IVisionUI>(UI_MODEL);

        auto vecRectSubROIS = pUI->getSubROIs();

        double dPosXMaxHt = 0, dPosXMinHt = 0;
        double dPosYMaxHt = 0, dPosYMinHt = 0;

        int nIndex = 0;
        for (auto &rectROI : vecRectSubROIS) {
            auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
            auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
            auto bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();
            auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();

            Vision::PR_CALC_3D_HEIGHT_DIFF_CMD stCmd;
            Vision::PR_CALC_3D_HEIGHT_DIFF_RPY stRpy;

            stCmd.fEffectHRatioStart = m_pEditMinRange->text().toFloat() / ONE_HUNDRED_PERCENT;
            stCmd.fEffectHRatioEnd = m_pEditMaxRange->text().toFloat() / ONE_HUNDRED_PERCENT;

            stCmd.matHeight = pUI->getHeightData();
            stCmd.rectROI = rectROI;

            bool bGlobalBase = (static_cast<HDW_BASE_TYPE> (m_pComboxBaseType->currentIndex()) == HDW_BASE_TYPE::EN_GLOBAL_BASE_TYPE);
            if (bGlobalBase)
            {
                stCmd.matMask = getGlobalBaseMask(m_currentWindow);
                stCmd.vecRectBases.push_back(calcWinBaseRect(m_currentWindow));
            }
            else {
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
            }

            Vision::PR_Calc3DHeightDiff(&stCmd, &stRpy);
            float height = stRpy.fHeightDiff * MM_TO_UM;   

            if (0 == nIndex)
            {
                dPosXMinHt = height;
                dPosYMinHt = height;
            }
            else if (1 == nIndex)
            {
                dPosXMaxHt = height;             
            }
            else if (2 == nIndex)
            {
                dPosYMaxHt = height;
            }

            nIndex++;
        }

        // Result Final
        m_pSpecAndResultLftRgtRelHt->setResult(dPosXMaxHt - dPosXMinHt);
        m_pSpecAndResultTopBtmRelHt->setResult(dPosYMaxHt - dPosYMinHt);
    }
}

void HeightDetectWidget::confirmWindow(OPERATION enOperation) {
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
        tryInsp();
        if (!m_bIsTryInspected)
            return;
    }

    QJsonObject json;
    json.insert("MinRange", m_pEditMinRange->text().toFloat() / ONE_HUNDRED_PERCENT);
    json.insert("MaxRange", m_pEditMaxRange->text().toFloat() / ONE_HUNDRED_PERCENT);
    json.insert("AbsHt", m_pSpecAndResultAbsHt->getSpec());
    json.insert("MaxAbsHt", m_pSpecAndResultMaxHtErr->getSpec());
    json.insert("MinAbsHt", m_pSpecAndResultMinHtErr->getSpec());
    json.insert("RelHt", m_pCheckBoxRelHt->isChecked());
    json.insert("LftRgtRelHt", m_pSpecAndResultLftRgtRelHt->getSpec());
    json.insert("TopBtmRelHt", m_pSpecAndResultTopBtmRelHt->getSpec());
    json.insert("GlobalBase", (static_cast<HDW_BASE_TYPE> (m_pComboxBaseType->currentIndex()) == HDW_BASE_TYPE::EN_GLOBAL_BASE_TYPE) ? true : false);
    json.insert("GlobalBaseScale", m_pEditBaseScale->text().toInt());
   
    if (m_pCheckBoxRelHt->isChecked())
    {
        auto selectedDevice = pUI->getSelectedDevice();
        auto ptCenter = selectedDevice.getWindow().center;

        auto vecRectSubROIS = pUI->getSubROIs();
        json.insert("NumSubROI", Vision::ToInt32(vecRectSubROIS.size()));
        int index = 0;
        for (auto &rectSubROI : vecRectSubROIS) {
            rectSubROI.x -= ptCenter.x;
            rectSubROI.y -= ptCenter.y;
            QString strKey = "SubROI_" + QString::number(index);
            json.insert(strKey, DataUtils::formatRect(rectSubROI).c_str());
            ++index;
        }
    }

    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);
  
    Engine::Window window;
    window.lightId = m_pParent->getSelectedLighting() + 1;
    window.usage = (static_cast<HDW_MEASURE_TYPE> (m_pComboxMeasureType->currentIndex()) == HDW_MEASURE_TYPE::EN_MEASURE_TYPE) ? Engine::Window::Usage::HEIGHT_MEASURE : Engine::Window::Usage::HEIGHT_BASE;
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

    updateWindowToUI(window, enOperation);
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
        
        m_pSpecAndResultAbsHt->setSpec(obj.take("AbsHt").toDouble());
        m_pSpecAndResultAbsHt->clearResult();
        m_pSpecAndResultMaxHtErr->setSpec(obj.take("MaxAbsHt").toDouble());
        m_pSpecAndResultMaxHtErr->clearResult();
        m_pSpecAndResultMinHtErr->setSpec(obj.take("MinAbsHt").toDouble());
        m_pSpecAndResultMinHtErr->clearResult();

        m_pCheckBoxRelHt->setChecked(obj.take("RelHt").toBool());
        m_pSpecAndResultLftRgtRelHt->setSpec(obj.take("LftRgtRelHt").toDouble());
        m_pSpecAndResultLftRgtRelHt->clearResult();
        m_pSpecAndResultTopBtmRelHt->setSpec(obj.take("TopBtmRelHt").toDouble());
        m_pSpecAndResultTopBtmRelHt->clearResult();

        m_pComboxBaseType->setCurrentIndex(static_cast<int>(obj.take("GlobalBase").toBool() ? HDW_BASE_TYPE::EN_GLOBAL_BASE_TYPE : HDW_BASE_TYPE::EN_MANUAL_TYPE)); 
        m_pEditBaseScale->setText(QString::number(obj.take("GlobalBaseScale").toInt()));

        if (m_pCheckBoxRelHt->isChecked())
        {
            auto pUI = getModule<IVisionUI>(UI_MODEL);

            auto selectedDevice = pUI->getSelectedDevice();
            auto ptCenter = selectedDevice.getWindow().center;

            auto vecRectSubROIS = pUI->getSubROIs();
            int nNumSubROI = obj["NumSubROI"].toInt();

            Vision::VectorOfRect vecSubROIs;
            for (int index = 0; index < nNumSubROI; ++index) {
                QString strKey = "SubROI_" + QString::number(index);
                auto strRect = obj[strKey].toString().toStdString();
                auto rectSubROI = DataUtils::parseRect(strRect);
                rectSubROI.x += ptCenter.x;
                rectSubROI.y += ptCenter.y;
                vecSubROIs.push_back(rectSubROI);
            }
            pUI->setSubROIs(vecSubROIs);
        }       
    }   

    m_bIsTryInspected = false;
}

void HeightDetectWidget::on_measureChanged(int index) {
    if (static_cast<HDW_MEASURE_TYPE> (index) == HDW_MEASURE_TYPE::EN_MEASURE_TYPE)
    {
        ui.tableWidget->showRow(BASE_TYPE_ATTRI);
        ui.tableWidget->showRow(BASE_SCALE_ATTRI);
        ui.tableWidget->showRow(MAX_ABS_HT);
        ui.tableWidget->showRow(MAX_MAXERR_HT);
        ui.tableWidget->showRow(MIN_MINERR_HT);
        ui.tableWidget->showRow(MAX_REL_HT);
        ui.tableWidget->showRow(MAX_LR_REL_HT);
        ui.tableWidget->showRow(MAX_TB_REL_HT);
    }
    else if (static_cast<HDW_MEASURE_TYPE> (index) == HDW_MEASURE_TYPE::EN_BASE_TYPE)
    {
        ui.tableWidget->hideRow(BASE_TYPE_ATTRI);
        ui.tableWidget->hideRow(BASE_SCALE_ATTRI);
        ui.tableWidget->hideRow(MAX_ABS_HT);
        ui.tableWidget->hideRow(MAX_MAXERR_HT);
        ui.tableWidget->hideRow(MIN_MINERR_HT);
        ui.tableWidget->hideRow(MAX_REL_HT);
        ui.tableWidget->hideRow(MAX_LR_REL_HT);
        ui.tableWidget->hideRow(MAX_TB_REL_HT);
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

void HeightDetectWidget::onRelHtChanged(bool bInsp) {
    if (bInsp) {
        m_pSpecAndResultLftRgtRelHt->setEnabled(true);
        m_pSpecAndResultTopBtmRelHt->setEnabled(true);
    }
    else {
        m_pSpecAndResultLftRgtRelHt->setEnabled(false);
        m_pSpecAndResultTopBtmRelHt->setEnabled(false);
    }
}

cv::Mat HeightDetectWidget::getGlobalBaseMask(Engine::Window& window)
{
    Engine::WindowVector vecWindow;
    auto result = Engine::GetAllWindows(vecWindow);
    if (result != Engine::OK) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        System->setTrackInfo(QString("Error at GetAllWindows, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
        return cv::Mat();
    }

    Engine::Window windowBase;
    for each (Engine::Window win in vecWindow)
    {
        if (win.usage == Engine::Window::Usage::HEIGHT_BASE_GLOBAL)
        {
            windowBase = win;
            break;
        }
    }

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(windowBase.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        QMessageBox::critical(this, QStringLiteral("高度检测框"), QStringLiteral("Please check global base params to do inspection."));
        return cv::Mat();
    }
    QJsonObject jsonValue = parse_doucment.object();

    cv::Vec3b color; int nRn = 0, nTn = 0;
    color[0] = jsonValue["ClrBVal"].toInt();
    color[1] = jsonValue["ClrGVal"].toInt();
    color[2] = jsonValue["ClrRVal"].toInt();
    nRn = jsonValue["RnValue"].toInt();
    nTn = jsonValue["TnValue"].toInt();

    auto pData = getModule<IData>(DATA_MODEL);
    int index = windowBase.lightId - 1;
    auto vecCombinedBigImage = pData->getCombinedBigImages();
    cv::Mat matBigImage;
    if (index >= 0 && index < vecCombinedBigImage.size() && !vecCombinedBigImage[index].empty())
        matBigImage = vecCombinedBigImage[index];
    else
        return cv::Mat();

    cv::Rect rectBaseDetectWin = calcWinBaseRect(window);
    cv::Mat matROI(matBigImage, rectBaseDetectWin);

    auto pColorWidget = m_pParent->getColorWidget();
    pColorWidget->setImage(matROI);
    pColorWidget->holdColorImage(color, nRn, nTn);
    cv::Mat matMask = pColorWidget->getProcessedImage();
    pColorWidget->releaseColorImage();

    cv::Mat matBigMask = cv::Mat::zeros(matBigImage.size(), CV_8UC1);
    cv::Mat matMaskROI(matBigMask, cv::Rect(rectBaseDetectWin));
    matMask.copyTo(matMaskROI);

    return matBigMask;
}

cv::Rect HeightDetectWidget::calcWinBaseRect(Engine::Window& window) {
    auto pUI = getModule<IVisionUI>(UI_MODEL);
    cv::Mat matImage = pUI->getImage();

    double dBaseScale = m_pEditBaseScale->text().toDouble();

    auto selctROI = pUI->getSelectedROI();
    cv::Rect rectBaseDetectWin = CalcUtils::resizeRect(selctROI, cv::Size2f(selctROI.width * dBaseScale, selctROI.height * dBaseScale));
    CalcUtils::adjustRectROI(rectBaseDetectWin, matImage);

    return rectBaseDetectWin;
}
