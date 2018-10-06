#include "InspHeightBaseWidget.h"
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <qthread.h>

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
    COLOR_ATTRI,
    RANGE_MIN_ATTRI,
    RANGE_MAX_ATTRI,
    RN_ATTRI,
    TN_ATTRI,
};

InspHeightBaseWidget::InspHeightBaseWidget(InspWindowWidget *parent)
    : EditInspWindowBaseWidget(parent)
{
    ui.setupUi(this);

    m_pEditColor = std::make_unique<QLineEdit>(ui.tableWidget); 
    m_pEditColor->setEnabled(false);
    ui.tableWidget->setCellWidget(COLOR_ATTRI, DATA_COLUMN, m_pEditColor.get());

    m_pEditMinRange = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditMinRange->setValidator(new QDoubleValidator(0, 100, 2, m_pEditMinRange.get()));
    ui.tableWidget->setCellWidget(RANGE_MIN_ATTRI, DATA_COLUMN, m_pEditMinRange.get());

    m_pEditMaxRange = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditMaxRange->setValidator(new QDoubleValidator(0, 100, 2, m_pEditMaxRange.get()));
    ui.tableWidget->setCellWidget(RANGE_MAX_ATTRI, DATA_COLUMN, m_pEditMaxRange.get());

    m_pEditRnParam = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditRnParam->setValidator(new QIntValidator(0, 255, m_pEditRnParam.get()));
    ui.tableWidget->setCellWidget(RN_ATTRI, DATA_COLUMN, m_pEditRnParam.get());

    m_pEditTnParam = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditTnParam->setValidator(new QIntValidator(0, 255, m_pEditTnParam.get()));
    ui.tableWidget->setCellWidget(TN_ATTRI, DATA_COLUMN, m_pEditTnParam.get());
}

InspHeightBaseWidget::~InspHeightBaseWidget()
{
}

void InspHeightBaseWidget::setDefaultValue() {
    m_pEditColor->setText("r:0,g:0,b:0");
    m_pEditMinRange->setText("10");
    m_pEditMaxRange->setText("90");
    m_pEditRnParam->setText("20");
    m_pEditTnParam->setText("20");
}

void InspHeightBaseWidget::tryInsp() {
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    Int32 bBoardRotated = 0; Engine::GetParameter("BOARD_ROTATED", bBoardRotated, false);
    float dCombinedImageScale = 1.f; Engine::GetParameter("ScanImageZoomFactor", dCombinedImageScale, 1.f);

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    cv::Rect rectROI = pUI->getSelectedROI();
    if (rectROI.width <= 0 || rectROI.height <= 0) {
        QMessageBox::critical(this, QStringLiteral("高度全局基面框"), QStringLiteral("Please select a ROI to do inspection."));
        return;
    }

    auto matImage = pUI->getImage();
    auto matSelect = matImage(rectROI);

    cv::Scalar tempVal = cv::mean(matSelect);

    m_color[0] = tempVal.val[0];
    m_color[1] = tempVal.val[1];
    m_color[2] = tempVal.val[2];

    QString strColorMsg;
    strColorMsg.sprintf("r:%d,g:%d,b:%d", m_color[2], m_color[1], m_color[0]);
    m_pEditColor->setText(strColorMsg);

    QString strMsg("Inspect Status, Color ");
    strMsg += strColorMsg;
    QMessageBox::information(this, "Height Base Detect", strMsg);
}

void InspHeightBaseWidget::confirmWindow(OPERATION enOperation) { 
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    Int32 bBoardRotated = 0; Engine::GetParameter("BOARD_ROTATED", bBoardRotated, false);
    float dCombinedImageScale = 1.f; Engine::GetParameter("ScanImageZoomFactor", dCombinedImageScale, 1.f);

    QJsonObject json;
    json.insert("MinRange", m_pEditMinRange->text().toFloat() / ONE_HUNDRED_PERCENT);
    json.insert("MaxRange", m_pEditMaxRange->text().toFloat() / ONE_HUNDRED_PERCENT);
    json.insert("RnValue", m_pEditRnParam->text().toInt());
    json.insert("TnValue", m_pEditTnParam->text().toInt());    
    json.insert("ClrBVal", m_color[0]);
    json.insert("ClrGVal", m_color[1]);
    json.insert("ClrRVal", m_color[2]);

    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    auto rectROI = pUI->getSelectedROI();
    if (rectROI.width <= 0 || rectROI.height <= 0) {
        QMessageBox::critical(this, QStringLiteral("Add Height Base ROI Window"), QStringLiteral("Please select a ROI to do inspection."));
        return;
    }

    Engine::Window window;
    window.lightId = m_pParent->getSelectedLighting() + 1;
    window.usage = Engine::Window::Usage::HEIGHT_BASE_GLOBAL;
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
    //window.colorParams = m_pParent->getColorWidget()->getJsonFormattedParams();

    int result = Engine::OK;
    if (OPERATION::ADD == enOperation) {
        if (!isGlobalBaseNotExist()) return;

        window.deviceId = pUI->getSelectedDevice().getId();
        char windowName[100];       
        _snprintf(windowName, sizeof(windowName), "Height Global Base [%d, %d] @ %s", Vision::ToInt32(window.x), Vision::ToInt32(window.y), pUI->getSelectedDevice().getName().c_str());
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

void InspHeightBaseWidget::setCurrentWindow(const Engine::Window &window) {
    m_currentWindow = window;

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError)
        return;

    if (parse_doucment.isObject()) {
        QJsonObject obj = parse_doucment.object();

        m_pEditMinRange->setText(QString::number(obj.take("MinRange").toDouble() * ONE_HUNDRED_PERCENT));
        m_pEditMaxRange->setText(QString::number(obj.take("MaxRange").toDouble() * ONE_HUNDRED_PERCENT));
        m_pEditRnParam->setText(QString::number(obj.take("RnValue").toInt()));
        m_pEditTnParam->setText(QString::number(obj.take("TnValue").toInt()));

        m_color[0] = obj.take("ClrBVal").toInt();
        m_color[1] = obj.take("ClrGVal").toInt();
        m_color[2] = obj.take("ClrRVal").toInt();
    }
}

void InspHeightBaseWidget::on_btnSelectROI_clicked()
{
    // Ask user to select the search window
    auto nReturn = System->showInteractMessage(QStringLiteral("Base检测框"), QStringLiteral("请拖动鼠标选择检测窗口"));
    if (nReturn != QDialog::Accepted)
        return;

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    auto detectWinROI = pUI->getSelectedROI();

    auto pColorWidget = m_pParent->getColorWidget();
    cv::Mat matImage = pUI->getImage();
    cv::Mat matROI(matImage, detectWinROI);
    pColorWidget->setImage(matROI);

    pColorWidget->holdColorImage(m_color, m_pEditRnParam->text().toInt(), m_pEditTnParam->text().toInt());
    pColorWidget->show();
    while (!pColorWidget->isHidden())
    {
        QThread::msleep(100);
        QApplication::processEvents();
    }
    pColorWidget->releaseColorImage();

    int nRn = 0, nTn = 0;
    pColorWidget->getColorParams(nRn, nTn);

    m_pEditRnParam->setText(QString::number(nRn));
    m_pEditTnParam->setText(QString::number(nTn));
}

bool InspHeightBaseWidget::isGlobalBaseNotExist()
{
    Engine::DeviceVector vecDevices;
    QString strTitle(QStringLiteral("全局Base设置"));
    auto result = Engine::GetAllDevices(vecDevices);
    if (Engine::OK != result) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        QString strMsg(QStringLiteral("读取元件信息失败, 错误消息: "));
        strMsg += errorMessage.c_str();
        System->showMessage(strTitle, strMsg);
        return false;
    }
  
    int windowId = -1;
    for (const auto &device : vecDevices) {
        if (device.type.empty())
            continue;

        Engine::WindowVector vecWindows;
        Engine::GetDeviceWindows(device.Id, vecWindows);
        if (vecWindows.empty())
            continue;

        for (const auto &window : vecWindows) {
            if (Engine::Window::Usage::HEIGHT_BASE_GLOBAL == window.usage)
            {
                windowId = window.Id;
                break;
            }
        }      

        if (windowId > -1) break;
    }

    if (windowId > -1)
    {
        if (QMessageBox::Ok == QMessageBox::question(NULL, QStringLiteral("信息提示"),
            QStringLiteral("全局Base已经存在，是否重新设置？"), QMessageBox::Ok, QMessageBox::Cancel))
        {
            result = Engine::DeleteWindow(windowId);
            if (result != Engine::OK) {
                String errorType, errorMessage;
                Engine::GetErrorDetail(errorType, errorMessage);
                QString msg(QStringLiteral("删除检测框失败, 错误消息: "));
                msg += errorMessage.c_str();
                System->showMessage(QStringLiteral("检测框"), msg);
                return false;
            }

            IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
            auto vecDetectObjs = pUI->getDetectObjs();
            auto it = std::remove_if(vecDetectObjs.begin(), vecDetectObjs.end(), [windowId](QDetectObj &detectObj) { return detectObj.getID() == windowId; });
            vecDetectObjs.erase(it);
            pUI->setDetectObjs(vecDetectObjs);

            return true;
        }
        return false;
    }
    return true;
}
