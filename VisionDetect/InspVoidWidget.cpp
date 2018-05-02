#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>

#include "InspVoidWidget.h"
#include "../Common/SystemData.h"
#include "DataStoreAPI.h"
#include "VisionAPI.h"
#include "../include/IVisionUI.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../DataModule/QDetectObj.h"
#include "InspWindowWidget.h"

using namespace AOI;

enum BASIC_PARAM {
    INSP_MODE,
    MAX_AREA_RATIO,
    MIN_AREA_RATIO,
    MAX_HOLE_COUNT,
    MIN_HOLE_COUNT,
    MAX_HOLE_AREA,
    MIN_HOLE_AREA,
};

InspVoidWidget::InspVoidWidget(InspWindowWidget *parent)
    : EditInspWindowBaseWidget(parent)
{
    ui.setupUi(this);

    m_pComboBoxInspMode = std::make_unique<QComboBox>(this);
    m_pComboBoxInspMode->addItem("Ratio Mode");
    m_pComboBoxInspMode->addItem("Area Modes");
    ui.tableWidget->setCellWidget(INSP_MODE, DATA_COLUMN, m_pComboBoxInspMode.get());
    connect(m_pComboBoxInspMode.get(), SIGNAL(currentIndexChanged(int)), this, SLOT(on_inspModeChanged(int)));

    m_pEditMaxAreaRatio = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditMaxAreaRatio->setValidator(new QDoubleValidator(1, 100, 2, m_pEditMaxAreaRatio.get()));
    ui.tableWidget->setCellWidget(MAX_AREA_RATIO, DATA_COLUMN, m_pEditMaxAreaRatio.get());

    m_pEditMinAreaRatio = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditMinAreaRatio->setValidator(new QDoubleValidator(0, 99, 2, m_pEditMinAreaRatio.get()));
    ui.tableWidget->setCellWidget(MIN_AREA_RATIO, DATA_COLUMN, m_pEditMinAreaRatio.get());

    m_pEditMaxHoleCount = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditMaxHoleCount->setValidator(new QIntValidator(1, 1000, m_pEditMaxHoleCount.get()));
    ui.tableWidget->setCellWidget(MAX_HOLE_COUNT, DATA_COLUMN, m_pEditMaxHoleCount.get());

    m_pEditMinHoleCount = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditMinHoleCount->setValidator(new QIntValidator(m_pEditMinHoleCount.get()));
    ui.tableWidget->setCellWidget(MIN_HOLE_COUNT, DATA_COLUMN, m_pEditMinHoleCount.get());

    m_pEditMaxHoleArea = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditMaxHoleArea->setValidator(new QDoubleValidator(m_pEditMaxHoleArea.get()));
    ui.tableWidget->setCellWidget(MAX_HOLE_AREA, DATA_COLUMN, m_pEditMaxHoleArea.get());

    m_pEditMinHoleArea = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditMinHoleArea->setValidator(new QDoubleValidator(m_pEditMinHoleArea.get()));
    ui.tableWidget->setCellWidget(MIN_HOLE_AREA, DATA_COLUMN, m_pEditMinHoleArea.get());
}

InspVoidWidget::~InspVoidWidget()
{
}

void InspVoidWidget::on_inspModeChanged(int index)
{
    if (static_cast<Vision::PR_INSP_HOLE_MODE> (index) == Vision::PR_INSP_HOLE_MODE::RATIO) {
        ui.tableWidget->showRow(MAX_AREA_RATIO);
        ui.tableWidget->showRow(MIN_AREA_RATIO);
        ui.tableWidget->hideRow(MAX_HOLE_COUNT);
        ui.tableWidget->hideRow(MIN_HOLE_COUNT);
        ui.tableWidget->hideRow(MAX_HOLE_AREA);
        ui.tableWidget->hideRow(MAX_HOLE_COUNT);
        ui.tableWidget->hideRow(MIN_HOLE_AREA);
    }
    else if (static_cast<Vision::PR_INSP_HOLE_MODE> (index) == Vision::PR_INSP_HOLE_MODE::BLOB) {
        ui.tableWidget->hideRow(MAX_AREA_RATIO);
        ui.tableWidget->hideRow(MIN_AREA_RATIO);
        ui.tableWidget->showRow(MAX_HOLE_COUNT);
        ui.tableWidget->showRow(MIN_HOLE_COUNT);
        ui.tableWidget->showRow(MAX_HOLE_AREA);
        ui.tableWidget->showRow(MAX_HOLE_COUNT);
        ui.tableWidget->showRow(MIN_HOLE_AREA);
    }
}

void InspVoidWidget::setDefaultValue()
{
    m_pComboBoxInspMode->setCurrentIndex(0);
    m_pEditMaxAreaRatio->setText("100");
    m_pEditMinAreaRatio->setText("80");
    m_pEditMaxHoleCount->setText("10");
    m_pEditMinHoleCount->setText("1");
    m_pEditMaxHoleArea->setText("5000");
    m_pEditMinHoleArea->setText("500");

    on_inspModeChanged(m_pComboBoxInspMode->currentIndex());
}

void InspVoidWidget::setCurrentWindow(const Engine::Window &window)
{
    m_currentWindow = window;

    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
	if (json_error.error != QJsonParseError::NoError) {
        System->setTrackInfo(QString("Invalid inspection parameter encounted."));
		return;
    }
    QJsonObject jsonValue = parse_doucment.object();

    m_pComboBoxInspMode->setCurrentIndex(jsonValue["InspMode"].toInt());
    
    if ( Vision::PR_INSP_HOLE_MODE::RATIO == static_cast<Vision::PR_INSP_HOLE_MODE>( jsonValue["InspMode"].toInt() ) ) {
        QJsonObject jsonRatioMode = jsonValue["RatioMode"].toObject();
        m_pEditMaxAreaRatio->setText(QString::number(jsonRatioMode["MaxAreaRatio"].toDouble()));
        m_pEditMinAreaRatio->setText(QString::number(jsonRatioMode["MinAreaRatio"].toDouble()));
    }else {
        QJsonObject jsonBlobMode = jsonValue["BlobMode"].toObject();
        m_pEditMaxHoleCount->setText(QString::number(jsonBlobMode["MaxHoleCount"].toInt()));
        m_pEditMinHoleCount->setText(QString::number(jsonBlobMode["MinHoleCount"].toInt()));
        m_pEditMaxHoleArea->setText(QString::number(jsonBlobMode["MaxHoleArea"].toDouble()));
        m_pEditMinHoleArea->setText(QString::number(jsonBlobMode["MinHoleArea"].toDouble()));
    }

    on_inspModeChanged(m_pComboBoxInspMode->currentIndex());
}

void InspVoidWidget::tryInsp()
{
    Vision::PR_INSP_HOLE_CMD stCmd;
    Vision::PR_INSP_HOLE_RPY stRpy;

    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    stCmd.enInspMode = static_cast<Vision::PR_INSP_HOLE_MODE>(m_pComboBoxInspMode->currentIndex());
    
    if ( Vision::PR_INSP_HOLE_MODE::RATIO == stCmd.enInspMode ) {
        stCmd.stRatioModeCriteria.fMaxRatio = m_pEditMaxAreaRatio->text().toFloat() / ONE_HUNDRED_PERCENT;
        stCmd.stRatioModeCriteria.fMinRatio = m_pEditMinAreaRatio->text().toFloat() / ONE_HUNDRED_PERCENT;
    }else {
        stCmd.stBlobModeCriteria.nMaxBlobCount = m_pEditMaxHoleCount->text().toInt();
        stCmd.stBlobModeCriteria.nMinBlobCount = m_pEditMinHoleCount->text().toInt();
        stCmd.stBlobModeCriteria.fMaxArea = m_pEditMaxHoleArea->text().toFloat() / dResolutionX / dResolutionX;
        stCmd.stBlobModeCriteria.fMinArea = m_pEditMinHoleArea->text().toFloat() / dResolutionY / dResolutionY;
    }
    auto pUI = getModule<IVisionUI>(UI_MODEL);
    stCmd.matInputImg = pUI->getImage();
    cv::cvtColor(stCmd.matInputImg, stCmd.matInputImg, CV_BGR2GRAY);
    stCmd.rectROI = pUI->getSelectedROI();
    cv::Mat matProcessedROI = m_pParent->getColorWidget()->getProcessedImage();
    cv::Mat matOrigianlROI(stCmd.matInputImg, stCmd.rectROI);
    matProcessedROI.copyTo(matOrigianlROI);
    stCmd.bPreprocessedImg = true;

    if (stCmd.rectROI.width <= 0 || stCmd.rectROI.height <= 0) {
        QMessageBox::critical(this, QStringLiteral("Add Insp Hole Window"), QStringLiteral("Please select a ROI to do inspection."));
        return;
    }

    Vision::PR_InspHole ( &stCmd, &stRpy );
    QString strMsg;
    strMsg.sprintf("Inspect Status %d", Vision::ToInt32(stRpy.enStatus));
    if ( Vision::PR_INSP_HOLE_MODE::RATIO == stCmd.enInspMode )
        strMsg += QString(", ratio mode result: bright area ratio %1").arg(stRpy.stRatioModeResult.fRatio);
    else
        strMsg += QString(", blob mode result: blob counts %1").arg(stRpy.stBlobModeResult.vecBlobs.size());
    QMessageBox::information(this, "Insp void", strMsg);
}

void InspVoidWidget::confirmWindow(OPERATION enOperation)
{
    auto pComboBoxInspMode = dynamic_cast<QComboBox *>(ui.tableWidget->cellWidget(INSP_MODE, 1));
    auto enInspMode = static_cast<Vision::PR_INSP_HOLE_MODE>(pComboBoxInspMode->currentIndex());

    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();
    auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();

    QJsonObject jsonValue;

    jsonValue["InspMode"] = pComboBoxInspMode->currentIndex();
    if ( Vision::PR_INSP_HOLE_MODE::RATIO == static_cast<Vision::PR_INSP_HOLE_MODE>( pComboBoxInspMode->currentIndex() ) ) {
        QJsonObject jsonRatioMode;
        jsonRatioMode["MaxAreaRatio"] = m_pEditMaxAreaRatio->text().toFloat();
        jsonRatioMode["MinAreaRatio"] = m_pEditMinAreaRatio->text().toFloat();
        jsonValue["RatioMode"] = jsonRatioMode;
    }else {
        QJsonObject jsonBlobMode;
        jsonBlobMode["MaxHoleCount"] = m_pEditMaxHoleCount->text().toInt();
        jsonBlobMode["MinHoleCount"] = m_pEditMinHoleCount->text().toInt();
        jsonBlobMode["MaxHoleArea"] = m_pEditMaxHoleArea->text().toFloat();
        jsonBlobMode["MinHoleArea"] = m_pEditMinHoleArea->text().toFloat();
        jsonValue["BlobMode"] = jsonBlobMode;
    }
    QJsonDocument document;
	document.setObject(jsonValue);
	QByteArray byteArray = document.toJson(QJsonDocument::Compact);

    auto pUI = getModule<IVisionUI>(UI_MODEL);  
    auto rectROI = pUI->getSelectedROI();
    if ( rectROI.width <= 0 || rectROI.height <= 0 ) {
        QMessageBox::critical(this, QStringLiteral("Add Insp Hole Window"), QStringLiteral("Please select a ROI first to add inspection window."));
        return;
    }
    Engine::Window window;
    window.lightId = m_pParent->getSelectedLighting() + 1;
    window.usage = Engine::Window::Usage::INSP_HOLE;
    window.inspParams = byteArray;

    cv::Point2f ptWindowCtr(rectROI.x + rectROI.width  / 2.f, rectROI.y + rectROI.height / 2.f);
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
    window.colorParams = m_pParent->getColorWidget()->getJsonFormattedParams();
    int result = Engine::OK;
    if ( OPERATION::ADD == enOperation ) {
        window.deviceId = pUI->getSelectedDevice().getId();
        char windowName[100];
        _snprintf(windowName, sizeof(windowName), "InspHole [%d, %d] @ %s", Vision::ToInt32(window.x), Vision::ToInt32(window.y), pUI->getSelectedDevice().getName().c_str());
        window.name = windowName;
        result = Engine::CreateWindow ( window );
        if (result != Engine::OK) {
		    String errorType, errorMessage;
		    Engine::GetErrorDetail(errorType, errorMessage);
		    System->setTrackInfo(QString("Error at CreateWindow, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
		    return;
	    }else
            System->setTrackInfo(QString("Success to Create Window: %1.").arg(window.name.c_str()));

        QDetectObj detectObj(window.Id, window.name.c_str());
        cv::Point2f ptCenter(window.x / dResolutionX, window.y / dResolutionY);
        if (bBoardRotated)
            ptCenter.x = nBigImgWidth  - ptCenter.x;
        else
            ptCenter.y = nBigImgHeight - ptCenter.y; //In cad, up is positive, but in image, down is positive.
        cv::Size2f szROI(window.width / dResolutionX, window.height / dResolutionY);
        detectObj.setFrame(cv::RotatedRect(ptCenter, szROI, window.angle));
        auto vecDetectObjs = pUI->getDetectObjs();
        vecDetectObjs.push_back(detectObj);
        pUI->setDetectObjs(vecDetectObjs);
    }else {
        window.Id = m_currentWindow.Id;
        window.name = m_currentWindow.name;
        result = Engine::UpdateWindow(window);
        if (result != Engine::OK) {
		    String errorType, errorMessage;
		    Engine::GetErrorDetail(errorType, errorMessage);
		    System->setTrackInfo(QString("Error at UpdateWindow, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
		    return;
	    }else
            System->setTrackInfo(QString("Success to update window: %1.").arg(window.name.c_str()));
    }

    m_pParent->UpdateInspWindowList();
}