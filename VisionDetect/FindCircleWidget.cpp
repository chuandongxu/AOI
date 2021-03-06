#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>

#include "FindCircleWidget.h"
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
    FIND_INNER_ATTRI,
    FIND_PAIR,
    SEARCH_START_ANGLE,
    SEARCH_END_ANGLE,
    CALIPER_COUNT,
    CALIPER_WIDTH,
    EDGE_THRESHOLD,
    EDGE_SELECT_METHOD,
    REMOVE_STRAY_POINT_RATIO,
    GAUSSIAN_DIFF_HALF_WIDTH,
    GAUSSIAN_DIFF_SIGMA,
};

FindCircleWidget::FindCircleWidget(InspWindowWidget *parent)
: EditInspWindowBaseWidget(parent) {
    ui.setupUi(this);

    m_bSupportMask = true;

    m_pComboBoxInnerAttribute = std::make_unique<QComboBox>(ui.tableWidget);
    m_pComboBoxInnerAttribute->addItem("Bright");
    m_pComboBoxInnerAttribute->addItem("Dark");
    ui.tableWidget->setCellWidget(FIND_INNER_ATTRI, DATA_COLUMN, m_pComboBoxInnerAttribute.get());

    m_pCheckBoxFindPair = std::make_unique<QCheckBox>(ui.tableWidget);
    ui.tableWidget->setCellWidget(FIND_PAIR, DATA_COLUMN, m_pCheckBoxFindPair.get());

    m_pEditStartSrchAngle = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditStartSrchAngle->setValidator(new QDoubleValidator(0, 360, 2, m_pEditStartSrchAngle.get()));
    ui.tableWidget->setCellWidget(SEARCH_START_ANGLE, DATA_COLUMN, m_pEditStartSrchAngle.get());

    m_pEditEndSrchAngle = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditEndSrchAngle->setValidator(new QDoubleValidator(0, 360, 2, m_pEditEndSrchAngle.get()));
    ui.tableWidget->setCellWidget(SEARCH_END_ANGLE, DATA_COLUMN, m_pEditEndSrchAngle.get());

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
}

FindCircleWidget::~FindCircleWidget() {
}

void FindCircleWidget::setDefaultValue() {
    m_pCheckBoxFindPair->setChecked(false);
    m_pComboBoxInnerAttribute->setCurrentIndex(0);
    m_pEditStartSrchAngle->setText("0");
    m_pEditEndSrchAngle->setText("360");
    m_pEditCaliperCount->setText("20");
    m_pEditCaliperWidth->setText("10");
    m_pEditEdgeThreshold->setText("50");
    m_pComboBoxEdgeSelectMethod->setCurrentIndex(0);
    m_pEditRmStrayPointRatio->setText("0.2");
    m_pEditDiffFilterHalfW->setText("2");
    m_pEditDiffFilterSigma->setText("1");
}

void FindCircleWidget::tryInsp() {
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    Vision::PR_FIND_CIRCLE_CMD stCmd;
    Vision::PR_FIND_CIRCLE_RPY stRpy;

    stCmd.enInnerAttribute = static_cast<Vision::PR_OBJECT_ATTRIBUTE>(m_pComboBoxInnerAttribute->currentIndex());
    stCmd.bFindCirclePair = m_pCheckBoxFindPair->isChecked();
    stCmd.fStartSrchAngle = m_pEditStartSrchAngle->text().toFloat();
    stCmd.fEndSrchAngle = m_pEditEndSrchAngle->text().toFloat();
    stCmd.nCaliperCount = m_pEditCaliperCount->text().toInt();
    stCmd.fCaliperWidth = m_pEditCaliperWidth->text().toFloat();
    stCmd.nEdgeThreshold = m_pEditEdgeThreshold->text().toInt();
    stCmd.enSelectEdge = static_cast<Vision::PR_CALIPER_SELECT_EDGE>(m_pComboBoxEdgeSelectMethod->currentIndex());
    stCmd.fRmStrayPointRatio = m_pEditRmStrayPointRatio->text().toFloat();
    stCmd.nDiffFilterHalfW = m_pEditDiffFilterHalfW->text().toInt();
    stCmd.fDiffFilterSigma = m_pEditDiffFilterSigma->text().toFloat();

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

    stCmd.ptExpectedCircleCtr = cv::Point2f(rectROI.x + rectROI.width / 2.f, rectROI.y + rectROI.height / 2);
    stCmd.fMaxSrchRadius = qMin(rectROI.width / 2.0f, rectROI.height / 2.0f);
    stCmd.fMinSrchRadius = qMin(rectROI.width / 2.0f, rectROI.height / 2.0f) / 3.0f;
    Vision::PR_FindCircle(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK == stRpy.enStatus)
        pUI->displayImage(stRpy.matResultImg);
    QString strMsg;
    strMsg.sprintf("Inspect Status %d, center(%f, %f) radius1# %f, radius2# %f", Vision::ToInt32(stRpy.enStatus), stRpy.ptCircleCtr.x, stRpy.ptCircleCtr.y, stRpy.fRadius, stRpy.fRadius2);
    QMessageBox::information(this, "Find Circle", strMsg);
}

void FindCircleWidget::confirmWindow(OPERATION enOperation) {
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    Int32 bBoardRotated = 0; Engine::GetParameter("BOARD_ROTATED", bBoardRotated, false);
    float fCombinedImageScale = 1.f; Engine::GetParameter("ScanImageZoomFactor", fCombinedImageScale, 1.f);

    QJsonObject json;
    json.insert("InnerAttri", m_pComboBoxInnerAttribute->currentIndex());
    json.insert("FindPair", m_pCheckBoxFindPair->isChecked());
    json.insert("SearchStartAngle", m_pEditStartSrchAngle->text().toFloat());
    json.insert("SearchEndAngle", m_pEditEndSrchAngle->text().toFloat());
    json.insert("CaliperCount", m_pEditCaliperCount->text().toInt());
    json.insert("CaliperWidth", m_pEditCaliperWidth->text().toFloat());
    json.insert("EdgeThreshold", m_pEditEdgeThreshold->text().toInt());
    json.insert("SelectEdge", m_pComboBoxEdgeSelectMethod->currentIndex());
    json.insert("RmStrayPointRatio", m_pEditRmStrayPointRatio->text().toFloat());
    json.insert("DiffFilterHalfW", m_pEditDiffFilterHalfW->text().toInt());
    json.insert("DiffFilterSigma", m_pEditDiffFilterSigma->text().toFloat());

    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    auto rectROI = pUI->getSelectedROI();
    if (rectROI.width <= 0 || rectROI.height <= 0) {
        QMessageBox::critical(this, QStringLiteral("Add Insp Circle Window"), QStringLiteral("Please select a ROI to do inspection."));
        return;
    }

    Engine::Window window;
    window.lightId = m_pParent->getSelectedLighting() + 1;
    window.usage = Engine::Window::Usage::FIND_CIRCLE;
    window.inspParams = byte_array;
    window.mask = this->convertMaskMat2Bny(getMask());

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
    window.width = rectROI.width  * dResolutionX;
    window.height = rectROI.height * dResolutionY;
    window.deviceId = pUI->getSelectedDevice().getId();
    window.angle = 0;

    int result = Engine::OK;
    if (OPERATION::ADD == enOperation) {
        window.deviceId = pUI->getSelectedDevice().getId();
        char windowName[100];
        _snprintf(windowName, sizeof(windowName), "FindCircle [%d, %d] @ %s", Vision::ToInt32(window.x), Vision::ToInt32(window.y), pUI->getSelectedDevice().getName().c_str());
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

    m_currentWindow = window;
    updateWindowToUI(window, enOperation);
    m_pParent->updateInspWindowList();
}

void FindCircleWidget::setCurrentWindow(const Engine::Window &window) {
    m_currentWindow = window;

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError)
        return;

    if (! parse_doucment.isObject())
        return;

    QJsonObject obj = parse_doucment.object();

    m_pComboBoxInnerAttribute->setCurrentIndex(obj.take("InnerAttri").toInt());
    m_pCheckBoxFindPair->setChecked(obj.take("FindPair").toBool());
    m_pEditStartSrchAngle->setText(QString::number(obj.take("SearchStartAngle").toDouble()));
    m_pEditEndSrchAngle->setText(QString::number(obj.take("SearchEndAngle").toDouble()));
    m_pEditCaliperCount->setText(QString::number(obj.take("CaliperCount").toInt()));
    m_pEditCaliperWidth->setText(QString::number(obj.take("CaliperWidth").toDouble()));
    m_pEditEdgeThreshold->setText(QString::number(obj.take("EdgeThreshold").toInt()));
    m_pComboBoxEdgeSelectMethod->setCurrentIndex(obj.take("SelectEdge").toInt());
    m_pEditRmStrayPointRatio->setText(QString::number(obj.take("RmStrayPointRatio").toDouble()));
    m_pEditDiffFilterHalfW->setText(QString::number(obj.take("DiffFilterHalfW").toInt()));
    m_pEditDiffFilterSigma->setText(QString::number(obj.take("DiffFilterSigma").toDouble()));

    this->setMask(convertMaskBny2Mat(window.mask));
}
