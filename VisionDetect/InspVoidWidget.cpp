#include "InspVoidWidget.h"
#include <QStackedWidget>
#include <QLineEdit>
#include <QMessageBox>

#include "../Common/SystemData.h"
#include "DataStoreAPI.h"
#include "VisionAPI.h"
#include "json.h"
#include "../include/VisionUI.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"

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

    m_pComboBoxInspMode = new QComboBox(this);
    m_pComboBoxInspMode->addItem("Ratio Mode");
    m_pComboBoxInspMode->addItem("Area Modes");
    ui.tableWidget->setCellWidget ( INSP_MODE, DATA_COLUMN, m_pComboBoxInspMode );
    connect( m_pComboBoxInspMode, SIGNAL(currentIndexChanged(int)), SLOT(on_testModeChanged(int)));

    QLineEdit *pEditMaxAreaRatio = new QLineEdit( ui.tableWidget );
    pEditMaxAreaRatio->setValidator(new QDoubleValidator( pEditMaxAreaRatio ) );
    pEditMaxAreaRatio->setText("100");
    ui.tableWidget->setCellWidget(MAX_AREA_RATIO, DATA_COLUMN, pEditMaxAreaRatio );

    QLineEdit *pEditMinAreaRatio = new QLineEdit( ui.tableWidget );
    pEditMinAreaRatio->setValidator(new QDoubleValidator( pEditMinAreaRatio ) );
    pEditMinAreaRatio->setText("80");
    ui.tableWidget->setCellWidget(MIN_AREA_RATIO, DATA_COLUMN, pEditMinAreaRatio );

    QLineEdit *pEditMaxHoleCount = new QLineEdit( ui.tableWidget );
    pEditMaxHoleCount->setValidator(new QIntValidator( pEditMaxHoleCount ) );
    pEditMaxHoleCount->setText("10");
    ui.tableWidget->setCellWidget(MAX_HOLE_COUNT, DATA_COLUMN, pEditMaxHoleCount );

    QLineEdit *pEditMinHoleCount = new QLineEdit( ui.tableWidget );
    pEditMinHoleCount->setValidator(new QIntValidator( pEditMinHoleCount ) );
    pEditMinHoleCount->setText("1");
    ui.tableWidget->setCellWidget(MIN_HOLE_COUNT, DATA_COLUMN, pEditMinHoleCount );

    QLineEdit *pEditMaxHoleArea = new QLineEdit( ui.tableWidget );
    pEditMaxHoleArea->setValidator(new QDoubleValidator( pEditMaxHoleArea ) );
    pEditMaxHoleArea->setText("5000");
    ui.tableWidget->setCellWidget(MAX_HOLE_AREA, DATA_COLUMN, pEditMaxHoleArea );

    QLineEdit *pEditMinHoleArea = new QLineEdit( ui.tableWidget );
    pEditMinHoleArea->setValidator(new QDoubleValidator( pEditMinHoleArea ) );
    pEditMinHoleArea->setText("500");
    ui.tableWidget->setCellWidget(MIN_HOLE_AREA, DATA_COLUMN, pEditMinHoleArea );    
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

    auto pComboBoxInspMode = dynamic_cast<QComboBox *>(ui.tableWidget->cellWidget(INSP_MODE, 1));
    stCmd.enInspMode = static_cast<Vision::PR_INSP_HOLE_MODE>(pComboBoxInspMode->currentIndex());
    
    if ( Vision::PR_INSP_HOLE_MODE::RATIO == stCmd.enInspMode ) {
        stCmd.stRatioModeCriteria.fMaxRatio = dynamic_cast<QLineEdit *>(ui.tableWidget->cellWidget(MAX_AREA_RATIO, DATA_COLUMN))->text().toFloat() / ONE_HUNDRED_PERCENT;
        stCmd.stRatioModeCriteria.fMinRatio = dynamic_cast<QLineEdit *>(ui.tableWidget->cellWidget(MIN_AREA_RATIO, DATA_COLUMN))->text().toFloat() / ONE_HUNDRED_PERCENT;
    }else {
        stCmd.stBlobModeCriteria.nMaxBlobCount = dynamic_cast<QLineEdit *>(ui.tableWidget->cellWidget(MAX_HOLE_COUNT, DATA_COLUMN))->text().toInt();
        stCmd.stBlobModeCriteria.nMinBlobCount = dynamic_cast<QLineEdit *>(ui.tableWidget->cellWidget(MIN_HOLE_COUNT, DATA_COLUMN))->text().toInt();
        stCmd.stBlobModeCriteria.fMaxArea = dynamic_cast<QLineEdit *>(ui.tableWidget->cellWidget(MAX_HOLE_AREA, DATA_COLUMN))->text().toFloat();
        stCmd.stBlobModeCriteria.fMinArea = dynamic_cast<QLineEdit *>(ui.tableWidget->cellWidget(MIN_HOLE_AREA, DATA_COLUMN))->text().toFloat();
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
        jsonValue["RatioMode"]["MaxAreaRatio"] = dynamic_cast<QLineEdit *>(ui.tableWidget->cellWidget(MAX_AREA_RATIO, DATA_COLUMN))->text().toStdString();
        jsonValue["RatioMode"]["MinAreaRatio"] = dynamic_cast<QLineEdit *>(ui.tableWidget->cellWidget(MIN_AREA_RATIO, DATA_COLUMN))->text().toStdString();
    }else {
        jsonValue["BlobMode"]["MaxHoleCount"] = dynamic_cast<QLineEdit *>(ui.tableWidget->cellWidget(MAX_HOLE_COUNT, DATA_COLUMN))->text().toStdString();
        jsonValue["BlobMode"]["MinHoleCount"] = dynamic_cast<QLineEdit *>(ui.tableWidget->cellWidget(MIN_HOLE_COUNT, DATA_COLUMN))->text().toStdString();
        jsonValue["BlobMode"]["MaxHoleArea"] = dynamic_cast<QLineEdit *>(ui.tableWidget->cellWidget(MAX_HOLE_AREA, DATA_COLUMN))->text().toStdString();
        jsonValue["BlobMode"]["MinHoleArea"] = dynamic_cast<QLineEdit *>(ui.tableWidget->cellWidget(MIN_HOLE_AREA, DATA_COLUMN))->text().toStdString();
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
    window.x = rectROI.x * dResolutionX;
    window.y = rectROI.y * dResolutionY;
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
	}
}