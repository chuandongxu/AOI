#include "InspVoidWidget.h"
#include <QStackedWidget>
#include <QLineEdit>
#include <QMessageBox>

#include "../Common/SystemData.h"
#include "DataStoreAPI.h"
#include "VisionAPI.h"
#include "json.h"
#include "../include/IVisionUI.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../DataModule/QDetectObj.h"

using namespace NFG::AOI;
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

static const int DATA_COLUMN = 1;

InspVoidWidget::InspVoidWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    m_pComboBoxInspMode = std::make_unique<QComboBox>(this);
    m_pComboBoxInspMode->addItem("Ratio Mode");
    m_pComboBoxInspMode->addItem("Area Modes");
    ui.tableWidget->setCellWidget ( INSP_MODE, DATA_COLUMN, m_pComboBoxInspMode.get() );
    connect( m_pComboBoxInspMode.get(), SIGNAL(currentIndexChanged(int)), SLOT(on_testModeChanged(int)));

    m_pEditMaxAreaRatio = std::make_unique<QLineEdit>( ui.tableWidget );
    m_pEditMaxAreaRatio->setValidator(new QDoubleValidator( 1, 100, 2, m_pEditMaxAreaRatio.get() ) );
    m_pEditMaxAreaRatio->setText("100");
    ui.tableWidget->setCellWidget(MAX_AREA_RATIO, DATA_COLUMN, m_pEditMaxAreaRatio.get() );

    m_pEditMinAreaRatio = std::make_unique<QLineEdit>( ui.tableWidget );
    m_pEditMinAreaRatio->setValidator(new QDoubleValidator( 0, 99, 2, m_pEditMinAreaRatio.get() ) );
    m_pEditMinAreaRatio->setText("80");
    ui.tableWidget->setCellWidget(MIN_AREA_RATIO, DATA_COLUMN, m_pEditMinAreaRatio.get() );

    m_pEditMaxHoleCount = std::make_unique<QLineEdit>( ui.tableWidget );
    m_pEditMaxHoleCount->setValidator(new QIntValidator( 1, 1000, m_pEditMaxHoleCount.get() ) );
    m_pEditMaxHoleCount->setText("10");
    ui.tableWidget->setCellWidget(MAX_HOLE_COUNT, DATA_COLUMN, m_pEditMaxHoleCount.get() );

    m_pEditMinHoleCount = std::make_unique<QLineEdit>( ui.tableWidget );
    m_pEditMinHoleCount->setValidator(new QIntValidator( m_pEditMinHoleCount.get() ) );
    m_pEditMinHoleCount->setText("1");
    ui.tableWidget->setCellWidget(MIN_HOLE_COUNT, DATA_COLUMN, m_pEditMinHoleCount.get() );

    m_pEditMaxHoleArea = std::make_unique<QLineEdit>( ui.tableWidget );
    m_pEditMaxHoleArea->setValidator(new QDoubleValidator( m_pEditMaxHoleArea.get() ) );
    m_pEditMaxHoleArea->setText("5000");
    ui.tableWidget->setCellWidget(MAX_HOLE_AREA, DATA_COLUMN, m_pEditMaxHoleArea.get() );

    m_pEditMinHoleArea = std::make_unique<QLineEdit>( ui.tableWidget );
    m_pEditMinHoleArea->setValidator(new QDoubleValidator( m_pEditMinHoleArea.get() ) );
    m_pEditMinHoleArea->setText("500");
    ui.tableWidget->setCellWidget(MIN_HOLE_AREA, DATA_COLUMN, m_pEditMinHoleArea.get() );    
}

InspVoidWidget::~InspVoidWidget()
{
}

void InspVoidWidget::on_inspModeChanged(int index)
{
}

void InspVoidWidget::on_btnTryInsp_clicked()
{
    Vision::PR_INSP_HOLE_CMD stCmd;
    Vision::PR_INSP_HOLE_RPY stRpy;

    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    auto pComboBoxInspMode = dynamic_cast<QComboBox *>(ui.tableWidget->cellWidget(INSP_MODE, 1));
    stCmd.enInspMode = static_cast<Vision::PR_INSP_HOLE_MODE>(pComboBoxInspMode->currentIndex());
    
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
    stCmd.rectROI = pUI->getSelectedROI();
    if ( stCmd.rectROI.width <= 0 || stCmd.rectROI.height <= 0 ) {
        QMessageBox::critical(this, QStringLiteral("Add Insp Hole Window"), QStringLiteral("Please select a ROI to do inspection."));
        return;
    }
    stCmd.enSegmentMethod = Vision::PR_IMG_SEGMENT_METHOD::GRAY_SCALE_RANGE;
    stCmd.stGrayScaleRange.nStart = 100;
    stCmd.stGrayScaleRange.nEnd = 255;
    Vision::PR_InspHole ( &stCmd, &stRpy );
    QString strMsg;
    strMsg.sprintf("Inspect Status %d", Vision::ToInt32 ( stRpy.enStatus ) );
    if ( Vision::PR_INSP_HOLE_MODE::RATIO == stCmd.enInspMode )
        strMsg += QString(", ratio mode result: bright area ratio %1").arg(stRpy.stRatioModeResult.fRatio);
    else
        strMsg += QString(", blob mode result: blob counts %1").arg(stRpy.stBlobModeResult.vecBlobs.size());
    QMessageBox::information(this, "Insp void", strMsg);
}

void InspVoidWidget::on_btnConfirmWindow_clicked()
{
    auto pComboBoxInspMode = dynamic_cast<QComboBox *>(ui.tableWidget->cellWidget(INSP_MODE, 1));
    auto enInspMode = static_cast<Vision::PR_INSP_HOLE_MODE>(pComboBoxInspMode->currentIndex());

    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    Json::Value jsonValue;
    jsonValue["InspMode"] = pComboBoxInspMode->currentIndex();
    if ( Vision::PR_INSP_HOLE_MODE::RATIO == static_cast<Vision::PR_INSP_HOLE_MODE>( pComboBoxInspMode->currentIndex() ) ) {
        jsonValue["RatioMode"]["MaxAreaRatio"] = m_pEditMaxAreaRatio->text().toStdString();
        jsonValue["RatioMode"]["MinAreaRatio"] = m_pEditMinAreaRatio->text().toStdString();
    }else {
        jsonValue["BlobMode"]["MaxHoleCount"] = m_pEditMaxHoleCount->text().toStdString();
        jsonValue["BlobMode"]["MinHoleCount"] = m_pEditMinHoleCount->text().toStdString();
        jsonValue["BlobMode"]["MaxHoleArea"] = m_pEditMaxHoleArea->text().toStdString();
        jsonValue["BlobMode"]["MinHoleArea"] = m_pEditMinHoleArea->text().toStdString();
    }
    auto pUI = getModule<IVisionUI>(UI_MODEL);  
    auto rectROI = pUI->getSelectedROI();
    if ( rectROI.width <= 0 || rectROI.height <= 0 ) {
        QMessageBox::critical(this, QStringLiteral("Add Insp Hole Window"), QStringLiteral("Please select a ROI to do inspection."));
        return;
    }
    Engine::Window window;
    window.usage = Engine::Window::Usage::INSP_HOLE;
    window.xmlParams = jsonValue.toStyledString();
    window.x = ( rectROI.x + rectROI.width  / 2.f ) * dResolutionX;
    window.y = ( rectROI.y + rectROI.height / 2.f ) * dResolutionY;
    window.width = rectROI.width * dResolutionX;
    window.height = rectROI.height * dResolutionY;
    char windowName[100];
    _snprintf(windowName, sizeof(windowName), "InspHole [%d, %d]", Vision::ToInt32(window.x), Vision::ToInt32(window.y));
    window.name = windowName;
    window.angle = 0;
    auto result = Engine::CreateWindow ( window );
    if (result != Engine::OK) {
		String errorType, errorMessage;
		Engine::GetErrorDetail(errorType, errorMessage);
		System->setTrackInfo(QString("Error at CreateWindow, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
		return;
	}else {
        System->setTrackInfo(QString("Success to Create Window: %1.").arg ( windowName ) );
    }

    QDetectObj detectObj ( window.Id, window.name.c_str() );
    cv::Point2f ptCenter ( window.x / dResolutionX, window.y / dResolutionY );
    cv::Size2f szROI ( window.width / dResolutionX, window.height / dResolutionY );
    detectObj.setFrame ( cv::RotatedRect ( ptCenter, szROI, window.angle ) );
    auto vecDetectObjs = pUI->getDetectObjs();
    vecDetectObjs.push_back ( detectObj );
    pUI->setDetectObjs ( vecDetectObjs );
}