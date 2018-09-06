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

enum BASIC_PARAM
{
    FIND_PAIR,
    FIND_DIRECTION,
    CALIPER_COUNT,
    CALIPER_WIDTH,
    EDGE_THRESHOLD,
    EDGE_SELECT_METHOD,
    REMOVE_STRAY_POINT_RATIO,
    GAUSSIAN_DIFF_HALF_WIDTH,
    GAUSSIAN_DIFF_SIGMA,
    POINT_MAX_OFFSET,
    CHECK_LINERITY,    
    MIN_LINERITY,
    CHECK_ANGLE,
    EXPECTED_ANGLE,
    ANGLE_DIFF_TOL,
};

FindLineWidget::FindLineWidget(InspWindowWidget *parent)
: EditInspWindowBaseWidget(parent) {
    ui.setupUi(this);

    m_bSupportMask = true;

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

    m_pEditPointMaxOffset = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditPointMaxOffset->setValidator(new QDoubleValidator(1, 5000, 2, m_pEditPointMaxOffset.get()));
    ui.tableWidget->setCellWidget(POINT_MAX_OFFSET, DATA_COLUMN, m_pEditPointMaxOffset.get());

    m_pCheckLinerity = std::make_unique<QCheckBox>(ui.tableWidget);
    ui.tableWidget->setCellWidget(CHECK_LINERITY, DATA_COLUMN, m_pCheckLinerity.get());

    m_pSpecAndResultMinLinearity = std::make_unique<SpecAndResultWidget>(ui.tableWidget, 0, 100);
    ui.tableWidget->setCellWidget(MIN_LINERITY, DATA_COLUMN, m_pSpecAndResultMinLinearity.get());

    m_pEditCheckAngle = std::make_unique<QCheckBox>(ui.tableWidget);
    ui.tableWidget->setCellWidget(CHECK_ANGLE, DATA_COLUMN, m_pEditCheckAngle.get());

    m_pSpecAndResultAngle = std::make_unique<SpecAndResultWidget>(ui.tableWidget, -360, 360);
    ui.tableWidget->setCellWidget(EXPECTED_ANGLE, DATA_COLUMN, m_pSpecAndResultAngle.get());

    m_pSpecAndResultAngleDiffTol = std::make_unique<SpecAndResultWidget>(ui.tableWidget, -100, 100);
    ui.tableWidget->setCellWidget(ANGLE_DIFF_TOL, DATA_COLUMN, m_pSpecAndResultAngleDiffTol.get());
}

FindLineWidget::~FindLineWidget() {
}

void FindLineWidget::setDefaultValue() {
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
    m_pSpecAndResultMinLinearity->setSpec(80);
    m_pEditCheckAngle->setChecked(true);
    m_pSpecAndResultAngle->setSpec(0);
    m_pSpecAndResultAngleDiffTol->setSpec(1);
}

void FindLineWidget::tryInsp() {
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
    stCmd.fMinLinearity = m_pSpecAndResultMinLinearity->getSpec() / ONE_HUNDRED_PERCENT;
    stCmd.bCheckAngle = m_pEditCheckAngle->isChecked();
    stCmd.fExpectedAngle = m_pSpecAndResultAngle->getSpec();
    stCmd.fAngleDiffTolerance = m_pSpecAndResultAngleDiffTol->getSpec();

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    stCmd.matInputImg = pUI->getImage();
    cv::Rect rectROI = pUI->getSelectedROI();
    if (rectROI.width <= 0 || rectROI.height <= 0) {
        QMessageBox::critical(this, QStringLiteral("Add Insp Hole Window"), QStringLiteral("Please select a ROI to do inspection."));
        return;
    }

    cv::Mat matMask = getMask();
    cv::Mat matBigMask = cv::Mat::ones(pUI->getImage().size(), CV_8UC1);
    matBigMask *= Vision::PR_MAX_GRAY_LEVEL;
    cv::Mat matMaskROI(matBigMask, cv::Rect(pUI->getSelectedROI()));
    matMask.copyTo(matMaskROI);
    stCmd.matMask = matBigMask;

    stCmd.rectRotatedROI.center = cv::Point2f(rectROI.x + rectROI.width / 2.f, rectROI.y + rectROI.height / 2);
    stCmd.rectRotatedROI.size = rectROI.size();
    Vision::PR_FindLine(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK == stRpy.enStatus)
        pUI->displayImage(stRpy.matResultImg);
    m_pSpecAndResultMinLinearity->setResult(stRpy.fLinearity * ONE_HUNDRED_PERCENT);
    m_pSpecAndResultAngle->setResult(stRpy.fAngle);
    m_pSpecAndResultAngleDiffTol->setResult(stRpy.fAngle - stCmd.fExpectedAngle);
    QString strMsg;
    strMsg.sprintf("Inspect Status %d, linearity %.2f %, angle %f", Vision::ToInt32(stRpy.enStatus), stRpy.fLinearity * ONE_HUNDRED_PERCENT, stRpy.fAngle);
    QMessageBox::information(this, "Find Line", strMsg);    
}

void FindLineWidget::confirmWindow(OPERATION enOperation) {
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();
    auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();

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
    jsonValue["MinLinearity"] = m_pSpecAndResultMinLinearity->getSpec() / ONE_HUNDRED_PERCENT;
    jsonValue["CheckAngle"] = m_pEditCheckAngle->isChecked();
    jsonValue["ExpectedAngle"] = m_pSpecAndResultAngle->getSpec();
    jsonValue["AngleDiffTolerance"] = m_pSpecAndResultAngleDiffTol->getSpec();

    QJsonDocument document;
    document.setObject(jsonValue);
    QByteArray byteArray = document.toJson(QJsonDocument::Compact);

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    auto rectROI = pUI->getSelectedROI();
    if (rectROI.width <= 0 || rectROI.height <= 0) {
        QMessageBox::critical(this, QStringLiteral("Add Insp Hole Window"), QStringLiteral("Please select a ROI first to add inspection window."));
        return;
    }
    Engine::Window window;
    window.lightId = m_pParent->getSelectedLighting() + 1;
    window.usage = Engine::Window::Usage::FIND_LINE;
    window.inspParams = byteArray;
    window.mask = this->convertMaskMat2Bny(getMask());

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
    window.width = rectROI.width  * dResolutionX;
    window.height = rectROI.height * dResolutionY;
    window.deviceId = pUI->getSelectedDevice().getId();
    window.angle = 0;
    int result = Engine::OK;
    if (OPERATION::ADD == enOperation) {
        window.deviceId = pUI->getSelectedDevice().getId();
        char windowName[100];
        _snprintf(windowName, sizeof(windowName), "FindLine [%d, %d] @ %s", Vision::ToInt32(window.x), Vision::ToInt32(window.y), pUI->getSelectedDevice().getName().c_str());
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

void FindLineWidget::setCurrentWindow(const Engine::Window &window) {
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
    m_pSpecAndResultMinLinearity->setSpec(jsonValue["MinLinearity"].toDouble() * ONE_HUNDRED_PERCENT);
    m_pEditCheckAngle->setChecked(jsonValue["CheckAngle"].toBool());
    m_pSpecAndResultAngle->setSpec(jsonValue["ExpectedAngle"].toDouble());
    m_pSpecAndResultAngle->clearResult();
    m_pSpecAndResultAngleDiffTol->setSpec(jsonValue["AngleDiffTolerance"].toDouble());
    m_pSpecAndResultAngleDiffTol->clearResult();

    this->setMask(convertMaskBny2Mat(window.mask));
}