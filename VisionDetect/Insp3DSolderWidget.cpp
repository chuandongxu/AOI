#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>

#include "Insp3DSolderWidget.h"
#include "InspWindowWidget.h"
#include "../Common/SystemData.h"
#include "DataStoreAPI.h"
#include "VisionAPI.h"
#include "../include/IVisionUI.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../DataModule/QDetectObj.h"
#include "../DataModule/CalcUtils.hpp"
#include "../DataModule/DataUtils.h"

using namespace NFG::AOI;
using namespace AOI;

enum BASIC_PARAM
{
    CONDUCTOR_ABS_HEIGHT,
    CONDUCTOR_ABS_HEIGHT_UP_LIMIT,
    CONDUCTOR_ABS_HEIGHT_LO_LIMIT,
    CONDUCTOR_REL_HEIGHT_UP_LIMIT,
    SOLDER_HEIGHT_LO_LIMIT,
    SOLDER_HEIGHT_RATIO_LO_LIMIT,
    SOLDER_COV_RATIO_LO_LIMIT,
    SOLDER_HEIGHT_UP_LIMIT,
    SOLDER_WETTING_WIDTH,
};

Insp3DSolderWidget::Insp3DSolderWidget(InspWindowWidget *parent)
    : EditInspWindowBaseWidget(parent)
{
    ui.setupUi(this);

    m_pConductorAbsHeight = std::make_unique<SpecAndResultWidget>(ui.tableWidget, 0, 5000);
    ui.tableWidget->setCellWidget(CONDUCTOR_ABS_HEIGHT, DATA_COLUMN, m_pConductorAbsHeight.get());

    m_pConductorAbsHeightUpLimit = std::make_unique<SpecAndResultWidget>(ui.tableWidget, 0, 5000);
    ui.tableWidget->setCellWidget(CONDUCTOR_ABS_HEIGHT_UP_LIMIT, DATA_COLUMN, m_pConductorAbsHeightUpLimit.get());

    m_pConductorAbsHeightLoLimit = std::make_unique<SpecAndResultWidget>(ui.tableWidget, 0, 5000);
    ui.tableWidget->setCellWidget(CONDUCTOR_ABS_HEIGHT_LO_LIMIT, DATA_COLUMN, m_pConductorAbsHeightLoLimit.get());

    m_pConductorRelHeightUpLimit = std::make_unique<SpecAndResultWidget>(ui.tableWidget, 0, 5000);
    ui.tableWidget->setCellWidget(CONDUCTOR_REL_HEIGHT_UP_LIMIT, DATA_COLUMN, m_pConductorRelHeightUpLimit.get());

    m_pSolderHeightLoLimit = std::make_unique<SpecAndResultWidget>(ui.tableWidget, 0, 5000);
    ui.tableWidget->setCellWidget(SOLDER_HEIGHT_LO_LIMIT, DATA_COLUMN, m_pSolderHeightLoLimit.get());

    m_pSolderHeightRatioLoLimit = std::make_unique<SpecAndResultWidget>(ui.tableWidget, 0, 100);
    ui.tableWidget->setCellWidget(SOLDER_HEIGHT_RATIO_LO_LIMIT, DATA_COLUMN, m_pSolderHeightRatioLoLimit.get());

    m_pSolderCovRatioLoLimit = std::make_unique<SpecAndResultWidget>(ui.tableWidget, 0, 100);
    ui.tableWidget->setCellWidget(SOLDER_COV_RATIO_LO_LIMIT, DATA_COLUMN, m_pSolderCovRatioLoLimit.get());

    m_pSolderHeightUpLimit = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pSolderHeightUpLimit->setValidator(new QDoubleValidator(300, 1000, 2, m_pSolderHeightUpLimit.get()));
    ui.tableWidget->setCellWidget(SOLDER_HEIGHT_UP_LIMIT, DATA_COLUMN, m_pSolderHeightUpLimit.get());

    m_pSolderWettingWidth = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pSolderWettingWidth->setValidator(new QDoubleValidator(100, 500, 2, m_pSolderWettingWidth.get()));
    ui.tableWidget->setCellWidget(SOLDER_WETTING_WIDTH, DATA_COLUMN, m_pSolderWettingWidth.get());
}

Insp3DSolderWidget::~Insp3DSolderWidget() {
}

void Insp3DSolderWidget::setDefaultValue() {
    m_pConductorAbsHeight->setSpec(500.f);
    m_pConductorAbsHeightUpLimit->setSpec(100.f);
    m_pConductorAbsHeightLoLimit->setSpec(100.f);
    m_pConductorRelHeightUpLimit->setSpec(50.f);
    m_pSolderHeightLoLimit->setSpec(150.f);
    m_pSolderHeightRatioLoLimit->setSpec(25);
    m_pSolderCovRatioLoLimit->setSpec(20);
    m_pSolderHeightUpLimit->setText("500");
    m_pSolderWettingWidth->setText("180");
    m_bIsTryInspected = false;
    m_currentWindow.usage = Engine::Window::Usage::UNDEFINED;
}

void Insp3DSolderWidget::setCurrentWindow(const Engine::Window &window) {
    m_currentWindow = window;

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError)
        return;

    if (! parse_doucment.isObject())
        return;

    QJsonObject obj = parse_doucment.object();

    m_pConductorAbsHeight->setSpec(obj["ConductorAbsHeight"].toDouble());
    m_pConductorAbsHeightUpLimit->setSpec(obj["ConductorAbsHeightUpLimit"].toDouble());
    m_pConductorAbsHeightLoLimit->setSpec(obj["ConductorAbsHeightLoLimit"].toDouble());
    m_pConductorRelHeightUpLimit->setSpec(obj["ConductorRelHeightUpLimit"].toDouble());
    m_pSolderHeightLoLimit->setSpec(obj["SolderHeightLoLimit"].toDouble());
    m_pSolderHeightRatioLoLimit->setSpec(obj["SolderHeightRatioLoLimit"].toDouble());
    m_pSolderCovRatioLoLimit->setSpec(obj["SolderCovRatioLoLimit"].toDouble());

    m_pSolderHeightUpLimit->setText(QString::number(obj["SolderHeightUpLimit"].toDouble()));
    m_pSolderWettingWidth->setText(QString::number(obj["SolderWettingWidth"].toDouble()));

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    auto vecRectSubROIS = pUI->getSubROIs();
    int nNumSubROI = obj["NumSubROI"].toInt();
    
    auto rectROI = pUI->getSelectedROI();
    Vision::VectorOfRect vecSubROIs;
    for (int index = 0; index < nNumSubROI; ++ index) {
        QString strKey = "SubROI_" + QString::number(index);
        auto strRect = obj[strKey].toString().toStdString();
        auto rectSubROI = DataUtils::parseRect(strRect);
        rectSubROI.x += rectROI.x;
        rectSubROI.y += rectROI.y;
        vecSubROIs.push_back(rectSubROI);
    }
    pUI->setSubROIs(vecSubROIs);

    m_bIsTryInspected = false;
}

void Insp3DSolderWidget::tryInsp() {
    QString strTitle(QStringLiteral("3D Solder检测"));

    cv::Scalar scalarValue;
    int nColorDiff = 0, nGrayDiff = 0;
    bool bWithGlobalBase = getGlobalHeightBaseParams(scalarValue, nColorDiff, nGrayDiff);
    if (!bWithGlobalBase) {
        System->showMessage(strTitle, QStringLiteral("请先设置全局高度基准窗口."));
        return;
    }

    auto pUI = getModule<IVisionUI>(UI_MODEL);

    if (Engine::Window::Usage::UNDEFINED == m_currentWindow.usage) {
        auto nReturn = System->showInteractMessage(strTitle, QStringLiteral("请拖动鼠标选择检测窗口"));
        if (nReturn != QDialog::Accepted)
            return;

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

        nReturn = System->showInteractMessage(strTitle, QStringLiteral("需要自动添加另一个小检测框吗?"));
        if (nReturn == QDialog::Accepted) {
            auto selectedDevice = pUI->getSelectedDevice();
            auto ptCenter = selectedDevice.getWindow().center;
            auto rectDevice = selectedDevice.getWindow().boundingRect();
            cv::Point ptNewWindowCtr;
            if (rectDevice.width > rectDevice.height) {
                ptNewWindowCtr.x = 2 * ptCenter.x - ptWindowCtr.x;
                ptNewWindowCtr.y = ptWindowCtr.y;
            }
            else {
                ptNewWindowCtr.x = ptWindowCtr.x;
                ptNewWindowCtr.y = 2 * ptCenter.y - ptWindowCtr.y;
            }
            cv::Rect rectNewWindow(ptNewWindowCtr.x - lastSubROI.width / 2, ptNewWindowCtr.y - lastSubROI.height / 2, lastSubROI.width, lastSubROI.height);
            vecSubROIs.push_back(rectNewWindow);
            pUI->setSubROIs(vecSubROIs);
        }
        else
        {
            pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_SELECT_SUB_ROI);
            auto nReturn = System->showInteractMessage(strTitle, QStringLiteral("请拖动鼠标选择另一个小检测窗口"));
            if (nReturn != QDialog::Accepted)
                return;
        }
    }

    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();
    auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();

    Vision::PR_INSP_3D_SOLDER_CMD stCmd;
    Vision::PR_INSP_3D_SOLDER_RPY stRpy;
    stCmd.matHeight = pUI->getHeightData();
    stCmd.matColorImg = pUI->getImage();
    stCmd.rectDeviceROI = pUI->getSelectedROI();
    stCmd.vecRectCheckROIs = pUI->getSubROIs();
    stCmd.scalarBaseColor = scalarValue;
    stCmd.nBaseColorDiff = nColorDiff;
    stCmd.nBaseGrayDiff = nGrayDiff;
    stCmd.nWettingWidth = Vision::ToInt32(m_pSolderWettingWidth->text().toDouble() / dResolutionX);

    Vision::PR_Insp3DSolder(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK != stRpy.enStatus) {
        Vision::PR_GET_ERROR_INFO_RPY stErrStrRpy;
        Vision::PR_GetErrorInfo(stRpy.enStatus, &stErrStrRpy);
        QString strMsg(QStringLiteral("检测3D Solder失败, 错误消息: "));
        strMsg += stErrStrRpy.achErrorStr;
        System->showMessage(strTitle, strMsg);
        m_bIsTryInspected = false;
        return;
    }
    pUI->displayImage(stRpy.matResultImg);
    m_pConductorAbsHeight->setResult(stRpy.vecResults[0].fComponentHeight * MM_TO_UM);
    float fHeightDiff = stRpy.vecResults[0].fComponentHeight * MM_TO_UM - m_pConductorAbsHeight->getSpec();
    if (fHeightDiff >= 0) {
        m_pConductorAbsHeightUpLimit->setResult(fHeightDiff);
        m_pConductorAbsHeightLoLimit->setResult(0);
    }else {
        m_pConductorAbsHeightUpLimit->setResult(0);
        m_pConductorAbsHeightLoLimit->setResult(fHeightDiff);
    }

    if (stRpy.vecResults.size() >= 2) {
        float fRelHeight = fabs(stRpy.vecResults[0].fComponentHeight - stRpy.vecResults[1].fComponentHeight);
        m_pConductorRelHeightUpLimit->setResult(fRelHeight * MM_TO_UM);
    }

    m_pSolderHeightLoLimit->setResult(stRpy.vecResults[0].fSolderHeight * MM_TO_UM);
    float fFullRatioHeight = m_pSolderHeightUpLimit->text().toFloat() < stRpy.vecResults[0].fComponentHeight * MM_TO_UM ?
        m_pSolderHeightUpLimit->text().toFloat() : stRpy.vecResults[0].fComponentHeight * MM_TO_UM;
    float fRatio = stRpy.vecResults[0].fSolderHeight * MM_TO_UM / fFullRatioHeight * ONE_HUNDRED_PERCENT;
    m_pSolderHeightRatioLoLimit->setResult(fRatio);
    m_pSolderCovRatioLoLimit->setResult(stRpy.vecResults[0].fSolderRatio * ONE_HUNDRED_PERCENT);
    m_bIsTryInspected = true;
}

void Insp3DSolderWidget::confirmWindow(OPERATION enOperation) {
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

    if (!m_bIsTryInspected) {
        tryInsp();
        if (!m_bIsTryInspected)
            return;
    }    

    QJsonObject json;
    json.insert("ConductorAbsHeight", m_pConductorAbsHeight->getSpec());
    json.insert("ConductorAbsHeightUpLimit", m_pConductorAbsHeightUpLimit->getSpec());
    json.insert("ConductorAbsHeightLoLimit", m_pConductorAbsHeightLoLimit->getSpec());
    json.insert("ConductorRelHeightUpLimit", m_pConductorRelHeightUpLimit->getSpec());
    json.insert("SolderHeightLoLimit", m_pSolderHeightLoLimit->getSpec());
    json.insert("SolderHeightRatioLoLimit", m_pSolderHeightRatioLoLimit->getSpec());
    json.insert("SolderCovRatioLoLimit", m_pSolderCovRatioLoLimit->getSpec());

    json.insert("SolderHeightUpLimit", m_pSolderHeightUpLimit->text().toFloat());
    json.insert("SolderWettingWidth", m_pSolderWettingWidth->text().toFloat());

    auto vecRectSubROIS = pUI->getSubROIs();
    json.insert("NumSubROI", Vision::ToInt32(vecRectSubROIS.size()));
    
    int index = 0;
    for (auto &rectSubROI : vecRectSubROIS) {
        rectSubROI.x -= rectROI.x;
        rectSubROI.y -= rectROI.y;
        QString strKey = "SubROI_" + QString::number(index);
        json.insert(strKey, DataUtils::formatRect(rectSubROI).c_str());
        ++ index;
    }

    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);

    Engine::Window window;
    if (OPERATION::EDIT == enOperation)
        window = m_currentWindow;
    window.lightId = m_pParent->getSelectedLighting() + 1;
    window.usage = Engine::Window::Usage::INSP_3D_SOLDER;
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
    window.deviceId = pUI->getSelectedDevice().getId();
    window.angle = 0;

    int result = Engine::OK;
    if (OPERATION::ADD == enOperation) {
        window.deviceId = pUI->getSelectedDevice().getId();
        char windowName[100];
        _snprintf(windowName, sizeof(windowName), "Inspect 3D Solder [%d, %d] @ %s", Vision::ToInt32(window.x), Vision::ToInt32(window.y), pUI->getSelectedDevice().getName().c_str());
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
    }

    updateWindowToUI(window, enOperation);
    m_pParent->updateInspWindowList();
}

/*static*/ bool Insp3DSolderWidget::getGlobalHeightBaseParams(cv::Scalar &scalarValue, int &nColorDiff, int &nGrayDiff) {
    Engine::WindowVector vecWindows;
    auto result = Engine::GetAllWindows(vecWindows);
    QString strTitle(QStringLiteral("3D Base"));
    if (Engine::OK != result) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        QString strMsg(QStringLiteral("读取检测框失败, 错误消息: "));
        strMsg += errorMessage.c_str();
        System->showMessage(strTitle, strMsg);
        return false;
    }
    bool bFound = false;
    Engine::Window windowHeightBase;
    for (const auto& window : vecWindows) {
        if (window.usage == Engine::Window::Usage::HEIGHT_BASE_GLOBAL) {
            windowHeightBase = window;
            bFound = true;
            break;
        }
    }

    if (!bFound)
        return false;

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(windowHeightBase.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        System->showMessage(strTitle, QString("Window insp parameters: ") + windowHeightBase.inspParams.c_str() + " is invalid.");
        return false;
    }

    if (!parse_doucment.isObject())
        return false;

    QJsonObject obj = parse_doucment.object();

    nColorDiff = obj.take("RnValue").toInt();
    nGrayDiff = obj.take("TnValue").toInt();

    scalarValue = cv::Scalar(obj["ClrBVal"].toInt(), obj["ClrGVal"].toInt(), obj["ClrRVal"].toInt());
    return bFound;
}