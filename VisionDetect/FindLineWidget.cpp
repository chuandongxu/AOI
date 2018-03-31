#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>

#include "FindLineWidget.h"
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

enum BASIC_PARAM {
    FIND_PAIR,
    FIND_DIRECTION,
    CALIPER_COUNT,
    CALIPER_WIDTH,
    EDGE_THRESHOLD,
    EDGE_SELECT_METHOD,
    REMOVE_STRAY_POINT_RATIO,
    GAUSSIAN_DIFF_HALF_WIDTH,
    GAUSSIAN_DIFF_SIGMA,
    CHECK_LINERITY,
    POINT_MAX_OFFSET,
    MIN_LINERITY,
    CHECK_ANGLE,
    EXPECTED_ANGLE,
    ANGLE_DIFF_TOL,
};

FindLineWidget::FindLineWidget(InspWindowWidget *parent)
    : EditInspWindowBaseWidget(parent)
{
    ui.setupUi(this);

    m_pCheckBoxFindPair = std::make_unique<QCheckBox>(ui.tableWidget);
    ui.tableWidget->setCellWidget(FIND_PAIR, DATA_COLUMN, m_pCheckBoxFindPair.get());

    m_pComboBoxFindLineDirection = std::make_unique<QComboBox>(ui.tableWidget);
    m_pComboBoxFindLineDirection->addItem("Dark to Bright");
    m_pComboBoxFindLineDirection->addItem("Bright to Dark");
    ui.tableWidget->setCellWidget(FIND_DIRECTION, DATA_COLUMN, m_pComboBoxFindLineDirection.get());

    m_pEditCaliperCount = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditCaliperCount->setValidator(new QIntValidator(2, 1000, m_pEditCaliperCount.get()));
    ui.tableWidget->setCellWidget(CALIPER_COUNT, DATA_COLUMN, m_pEditCaliperCount.get());

    m_pEditCaliperWidth = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditCaliperWidth->setValidator(new QDoubleValidator(1, 1000, 0.1, m_pEditCaliperWidth.get()));
    ui.tableWidget->setCellWidget(CALIPER_WIDTH, DATA_COLUMN, m_pEditCaliperWidth.get());

    m_pEditEdgeThreshold = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditEdgeThreshold->setValidator(new QIntValidator(1, 255, m_pEditEdgeThreshold.get()));
    ui.tableWidget->setCellWidget(EDGE_THRESHOLD, DATA_COLUMN, m_pEditEdgeThreshold.get());

    m_pComboBoxEdgeSelectMethod = std::make_unique<QComboBox>(ui.tableWidget);
    m_pComboBoxEdgeSelectMethod->addItem("Max Edge");
    m_pComboBoxEdgeSelectMethod->addItem("First Edge");
    ui.tableWidget->setCellWidget(EDGE_SELECT_METHOD, DATA_COLUMN, m_pComboBoxEdgeSelectMethod.get());

    m_pEditRmStrayPointRatio = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditRmStrayPointRatio->setValidator(new QDoubleValidator(0, 1, 2, m_pEditRmStrayPointRatio.get()));
    ui.tableWidget->setCellWidget(REMOVE_STRAY_POINT_RATIO, DATA_COLUMN, m_pEditRmStrayPointRatio.get());

    m_pEditDiffFilterHalfW = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditDiffFilterHalfW->setValidator(new QIntValidator(1, 10, m_pEditDiffFilterHalfW.get()));
    ui.tableWidget->setCellWidget(GAUSSIAN_DIFF_HALF_WIDTH, DATA_COLUMN, m_pEditDiffFilterHalfW.get());

    m_pEditDiffFilterSigma = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditDiffFilterSigma->setValidator(new QDoubleValidator(0.1, 100, 2, m_pEditDiffFilterSigma.get()));
    ui.tableWidget->setCellWidget(GAUSSIAN_DIFF_SIGMA, DATA_COLUMN, m_pEditDiffFilterSigma.get());

    m_pCheckLinerity = std::make_unique<QCheckBox>(ui.tableWidget);
    ui.tableWidget->setCellWidget(CHECK_LINERITY, DATA_COLUMN, m_pCheckLinerity.get());

    m_pEditPointMaxOffset = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditPointMaxOffset->setValidator(new QDoubleValidator(1, 5000, 2, m_pEditPointMaxOffset.get()));
    ui.tableWidget->setCellWidget(POINT_MAX_OFFSET, DATA_COLUMN, m_pEditPointMaxOffset.get());

    m_pEditMinLinearity = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditMinLinearity->setValidator(new QDoubleValidator(0, 100, 2, m_pEditMinLinearity.get()));
    ui.tableWidget->setCellWidget(MIN_LINERITY, DATA_COLUMN, m_pEditMinLinearity.get());

    m_pEditCheckAngle = std::make_unique<QCheckBox>(ui.tableWidget);
    ui.tableWidget->setCellWidget(CHECK_ANGLE, DATA_COLUMN, m_pEditCheckAngle.get());

    m_pEditExpectedAngle = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditExpectedAngle->setValidator(new QDoubleValidator(-360, 360, 2, m_pEditExpectedAngle.get()));
    ui.tableWidget->setCellWidget(EXPECTED_ANGLE, DATA_COLUMN, m_pEditExpectedAngle.get());

    m_pEditAngleDiffTolerance = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditAngleDiffTolerance->setValidator(new QDoubleValidator(-100, 100, 2, m_pEditAngleDiffTolerance.get()));
    ui.tableWidget->setCellWidget(ANGLE_DIFF_TOL, DATA_COLUMN, m_pEditAngleDiffTolerance.get());
}

FindLineWidget::~FindLineWidget()
{
}

void FindLineWidget::setDefaultValue()
{
    m_pCheckBoxFindPair->setChecked(false);
    m_pComboBoxFindLineDirection->setCurrentIndex(0);
    m_pEditCaliperCount->setText("20");
    m_pEditCaliperWidth->setText("10");
    m_pEditEdgeThreshold->setText("50");
    m_pEditRmStrayPointRatio->setText("0.2");
    m_pEditDiffFilterHalfW->setText("2");
    m_pEditDiffFilterSigma->setText("1");
    m_pCheckLinerity->setChecked(true);
    m_pEditPointMaxOffset->setText("100");
    m_pEditMinLinearity->setText("80");
    m_pEditCheckAngle->setChecked(true);
    m_pEditExpectedAngle->setText("0");
    m_pEditAngleDiffTolerance->setText("1");
}

void FindLineWidget::tryInsp()
{
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    Vision::PR_FIND_LINE_CMD stCmd;
    Vision::PR_FIND_LINE_RPY stRpy;

    stCmd.enAlgorithm = Vision::PR_FIND_LINE_ALGORITHM::CALIPER;
    stCmd.bFindPair = m_pCheckBoxFindPair->isChecked();
    stCmd.enDetectDir = static_cast<Vision::PR_CALIPER_DIR>(m_pComboBoxFindLineDirection->currentIndex());
    stCmd.nCaliperCount = m_pEditCaliperCount->text().toInt();
    stCmd.fCaliperWidth = m_pEditCaliperWidth->text().toFloat();
    stCmd.nEdgeThreshold = m_pEditEdgeThreshold->text().toInt();
    stCmd.enSelectEdge = static_cast<Vision::PR_CALIPER_SELECT_EDGE>(m_pComboBoxEdgeSelectMethod->currentIndex());
    stCmd.fRmStrayPointRatio = m_pEditRmStrayPointRatio->text().toFloat();
    stCmd.nDiffFilterHalfW = m_pEditDiffFilterHalfW->text().toInt();
    stCmd.fDiffFilterSigma = m_pEditDiffFilterSigma->text().toFloat();
    stCmd.bCheckLinearity = m_pCheckLinerity->isChecked();
    stCmd.fPointMaxOffset = m_pEditPointMaxOffset->text().toFloat() / dResolutionX;
    stCmd.fMinLinearity = m_pEditMinLinearity->text().toFloat() / ONE_HUNDRED_PERCENT;
    stCmd.bCheckAngle = m_pEditCheckAngle->isChecked();
    stCmd.fExpectedAngle = m_pEditExpectedAngle->text().toFloat();
    stCmd.fAngleDiffTolerance = m_pEditAngleDiffTolerance->text().toFloat();

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    stCmd.matInputImg = pUI->getImage();
    cv::Rect rectROI = pUI->getSelectedROI();
    if (rectROI.width <= 0 || rectROI.height <= 0) {
        QMessageBox::critical(this, QStringLiteral("Add Insp Hole Window"), QStringLiteral("Please select a ROI to do inspection."));
        return;
    }

    stCmd.rectRotatedROI.center = cv::Point2f ( rectROI.x + rectROI.width / 2.f, rectROI.y + rectROI.height / 2 );
    stCmd.rectRotatedROI.size = rectROI.size();
    Vision::PR_FindLine ( &stCmd, &stRpy );
    QString strMsg;
    strMsg.sprintf("Inspect Status %d, linearity %f pass %d, angle %f pass %d", Vision::ToInt32 ( stRpy.enStatus ), stRpy.fLinearity, stRpy.bLinearityCheckPass, stRpy.fAngle, stRpy.bAngleCheckPass );
    QMessageBox::information(this, "Find Line", strMsg);
}

void FindLineWidget::confirmWindow(OPERATION enOperation)
{
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    QJsonObject jsonValue;
    
    jsonValue["Algorithm"] = Vision::ToInt32(Vision::PR_FIND_LINE_ALGORITHM::CALIPER);
    jsonValue["FindPair"] = m_pCheckBoxFindPair->isChecked();
    jsonValue["DetectDir"] = m_pComboBoxFindLineDirection->currentIndex();
    jsonValue["CaliperCount"] = m_pEditCaliperCount->text().toInt();
    jsonValue["CaliperWidth"] = m_pEditCaliperWidth->text().toFloat();
    jsonValue["EdgeThreshold"] = m_pEditEdgeThreshold->text().toInt();
    jsonValue["SelectEdge"] = m_pComboBoxEdgeSelectMethod->currentIndex();
    jsonValue["RmStrayPointRatio"] = m_pEditRmStrayPointRatio->text().toFloat();
    jsonValue["DiffFilterHalfW"] = m_pEditDiffFilterHalfW->text().toInt();
    jsonValue["DiffFilterSigma"] = m_pEditDiffFilterSigma->text().toFloat();
    jsonValue["CheckLinerity"] = m_pCheckLinerity->isChecked();
    jsonValue["PointMaxOffset"] = m_pEditPointMaxOffset->text().toFloat();
    jsonValue["MinLinearity"] = m_pEditMinLinearity->text().toFloat() / ONE_HUNDRED_PERCENT;
    jsonValue["CheckAngle"] = m_pEditCheckAngle->isChecked();
    jsonValue["ExpectedAngle"] = m_pEditExpectedAngle->text().toFloat();
    jsonValue["AngleDiffTolerance"] = m_pEditAngleDiffTolerance->text().toFloat();

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
    window.usage = Engine::Window::Usage::FIND_LINE;
    window.inspParams = byteArray;
    window.x = (rectROI.x + rectROI.width / 2.f) * dResolutionX;
    window.y = (rectROI.y + rectROI.height / 2.f) * dResolutionY;
    window.width = rectROI.width  * dResolutionX;
    window.height = rectROI.height * dResolutionY;
    window.deviceId = pUI->getSelectedDevice().getId();
    window.angle = 0;
    int result = Engine::OK;
    if ( OPERATION::ADD == enOperation ) {
        window.deviceId = pUI->getSelectedDevice().getId();
        char windowName[100];
        _snprintf(windowName, sizeof(windowName), "FindLine [%d, %d] @ %s", Vision::ToInt32(window.x), Vision::ToInt32(window.y), pUI->getSelectedDevice().getName().c_str());
        window.name = windowName;
        result = Engine::CreateWindow ( window );
        if (result != Engine::OK) {
		    String errorType, errorMessage;
		    Engine::GetErrorDetail(errorType, errorMessage);
		    System->setTrackInfo(QString("Error at CreateWindow, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
		    return;
	    }else {
            System->setTrackInfo(QString("Success to Create Window: %1.").arg ( window.name.c_str() ) );
        }

        QDetectObj detectObj ( window.Id, window.name.c_str() );
        cv::Point2f ptCenter ( window.x / dResolutionX, window.y / dResolutionY );
        cv::Size2f szROI ( window.width / dResolutionX, window.height / dResolutionY );
        detectObj.setFrame ( cv::RotatedRect ( ptCenter, szROI, window.angle ) );
        auto vecDetectObjs = pUI->getDetectObjs();
        vecDetectObjs.push_back ( detectObj );
        pUI->setDetectObjs ( vecDetectObjs );        
    }else {
        window.Id = m_currentWindow.Id;
        window.name = m_currentWindow.name;
        result = Engine::UpdateWindow ( window );
        if (result != Engine::OK) {
		    String errorType, errorMessage;
		    Engine::GetErrorDetail(errorType, errorMessage);
		    System->setTrackInfo(QString("Error at UpdateWindow, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
		    return;
	    }else {
            System->setTrackInfo(QString("Success to update window: %1.").arg(window.name.c_str()));
        }
    }

    m_pParent->UpdateInspWindowList();
}

void FindLineWidget::setCurrentWindow(const Engine::Window &window)
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
    QString strCaliperCount("abcd");
    m_pCheckBoxFindPair->setChecked(jsonValue["FindPair"].toBool());
    m_pComboBoxFindLineDirection->setCurrentIndex(jsonValue["DetectDir"].toInt());
    m_pEditCaliperCount->setText(QString::number(jsonValue["CaliperCount"].toInt()));
    m_pEditCaliperWidth->setText(QString::number(jsonValue["CaliperWidth"].toDouble()));
    m_pEditEdgeThreshold->setText(QString::number(jsonValue["EdgeThreshold"].toInt()));
    m_pComboBoxEdgeSelectMethod->setCurrentIndex(jsonValue["SelectEdge"].toInt());
    m_pEditRmStrayPointRatio->setText(QString::number(jsonValue["RmStrayPointRatio"].toDouble()));
    m_pEditDiffFilterHalfW->setText(QString::number(jsonValue["DiffFilterHalfW"].toInt()));
    m_pEditDiffFilterSigma->setText(QString::number(jsonValue["DiffFilterSigma"].toDouble()));
    m_pCheckLinerity->setChecked(jsonValue["CheckLinerity"].toBool());
    m_pEditPointMaxOffset->setText(QString::number(jsonValue["PointMaxOffset"].toDouble()));
    m_pEditMinLinearity->setText(QString::number(jsonValue["MinLinearity"].toDouble() * ONE_HUNDRED_PERCENT));
    m_pEditCheckAngle->setChecked(jsonValue["CheckAngle"].toBool());
    m_pEditExpectedAngle->setText(QString::number(jsonValue["ExpectedAngle"].toDouble()));
    m_pEditAngleDiffTolerance->setText(QString::number(jsonValue["AngleDiffTolerance"].toDouble()));
}