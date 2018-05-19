#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>

#include "InspContourWidget.h"
#include "../Common/SystemData.h"
#include "DataStoreAPI.h"
#include "VisionAPI.h"
#include "../include/IVisionUI.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../Common/CommonFunc.h"
#include "../DataModule/QDetectObj.h"
#include "InspWindowWidget.h"
#include "../DataModule/CalcUtils.hpp"

using namespace NFG::AOI;
using namespace AOI;

enum BASIC_PARAM {
    DEFECT_THRESHOLD,
    MIN_DEFECT_AREA,
    DEFECT_INNER_LENGTH_TOL,
    DEFECT_OUTER_LENGTH_TOL,
    INNER_MASK_DEPTH,
    OUTER_MASK_DEPTH,
};

InspContourWidget::InspContourWidget(InspWindowWidget *parent)
    : EditInspWindowBaseWidget(parent)
{
    ui.setupUi(this);

    m_pEditDefectThreshold = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditDefectThreshold->setValidator(new QIntValidator(1, 255, m_pEditDefectThreshold.get()));
    m_pEditDefectThreshold->setText("30");
    ui.tableWidget->setCellWidget(DEFECT_THRESHOLD, DATA_COLUMN, m_pEditDefectThreshold.get());

    m_pEditMinDefectArea = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditMinDefectArea->setValidator(new QIntValidator(100, 10000000, m_pEditMinDefectArea.get()));
    m_pEditMinDefectArea->setText("5000");
    ui.tableWidget->setCellWidget(MIN_DEFECT_AREA, DATA_COLUMN, m_pEditMinDefectArea.get());

    m_pEditDefectInnerLengthTol = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditDefectInnerLengthTol->setValidator(new QIntValidator(10, 10000, m_pEditDefectInnerLengthTol.get()));
    m_pEditDefectInnerLengthTol->setText("100");
    ui.tableWidget->setCellWidget(DEFECT_INNER_LENGTH_TOL, DATA_COLUMN, m_pEditDefectInnerLengthTol.get());

    m_pEditDefectOuterLengthTol = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditDefectOuterLengthTol->setValidator(new QIntValidator(10, 10000, m_pEditDefectOuterLengthTol.get()));
    m_pEditDefectOuterLengthTol->setText("100");
    ui.tableWidget->setCellWidget(DEFECT_OUTER_LENGTH_TOL, DATA_COLUMN, m_pEditDefectOuterLengthTol.get());

    m_pEditInnerMaskDepth = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditInnerMaskDepth->setValidator(new QIntValidator(10, 10000, m_pEditInnerMaskDepth.get()));
    m_pEditInnerMaskDepth->setText("50");
    ui.tableWidget->setCellWidget(INNER_MASK_DEPTH, DATA_COLUMN, m_pEditInnerMaskDepth.get());

    m_pEditOuterMaskDepth = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pEditOuterMaskDepth->setValidator(new QIntValidator(10, 10000, m_pEditOuterMaskDepth.get()));
    m_pEditOuterMaskDepth->setText("50");
    ui.tableWidget->setCellWidget(OUTER_MASK_DEPTH, DATA_COLUMN, m_pEditOuterMaskDepth.get());
}

InspContourWidget::~InspContourWidget() {
}

void InspContourWidget::setDefaultValue() {
    m_pEditDefectThreshold->setText("30");
    m_pEditMinDefectArea->setText("5000");
    m_pEditDefectInnerLengthTol->setText("100");
    m_pEditDefectOuterLengthTol->setText("100");
    m_pEditInnerMaskDepth->setText("50");
    m_pEditOuterMaskDepth->setText("50");

    m_bIsTryInspected = false;
    m_currentWindow.recordId = 0;
}

void InspContourWidget::setCurrentWindow(const Engine::Window &window) {
    m_currentWindow = window;

    QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
	if (json_error.error != QJsonParseError::NoError) {
        System->setTrackInfo(QString("Invalid inspection parameter encounted."));
        return;
    }

    if (!parse_doucment.isObject()) {
        System->setTrackInfo(QString("Invalid inspection parameter encounted."));
        return;
    }

    QJsonObject jsonValue = parse_doucment.object();

    m_pEditDefectThreshold->setText(QString::number(jsonValue["DefectThreshold"].toInt()));
    m_pEditMinDefectArea->setText(QString::number(jsonValue["MinDefectArea"].toDouble()));
    m_pEditDefectInnerLengthTol->setText(QString::number(jsonValue["DefectInnerLengthTol"].toDouble()));
    m_pEditDefectOuterLengthTol->setText(QString::number(jsonValue["DefectOuterLengthTol"].toDouble()));
    m_pEditInnerMaskDepth->setText(QString::number(jsonValue["InnerMaskDepth"].toDouble()));
    m_pEditOuterMaskDepth->setText(QString::number(jsonValue["OuterMaskDepth"].toDouble()));
}

bool InspContourWidget::_learnContour(int &recordId) {
    Vision::PR_LRN_CONTOUR_CMD stCmd;
    Vision::PR_LRN_CONTOUR_RPY stRpy;

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    stCmd.matInputImg = pUI->getImage();
    cv::Rect rectROI = pUI->getSelectedROI();
    if (rectROI.width <= 0 || rectROI.height <= 0) {
        QMessageBox::critical(this, QStringLiteral("边界检测框"), QStringLiteral("Please select a ROI to do inspection."));
        return false;
    }

    stCmd.rectROI = pUI->getSelectedROI();

    Vision::PR_LrnContour(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK != stRpy.enStatus) {
        System->showMessage(QStringLiteral("边界检测框"), QStringLiteral("学习边界失败"));
        recordId = 0;
        return false;
    }

    recordId = stRpy.nRecordId;
    return true;
}

bool InspContourWidget::_inspContour(int recordId, bool bShowResult /*= true*/) {
    QString strTitle(QStringLiteral("边界检测框"));
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
	auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    Vision::PR_INSP_CONTOUR_CMD stCmd;
    Vision::PR_INSP_CONTOUR_RPY stRpy;

    stCmd.nDefectThreshold = m_pEditDefectThreshold->text().toInt();
    stCmd.fMinDefectArea = m_pEditMinDefectArea->text().toFloat() / dResolutionX / dResolutionY;
    stCmd.fDefectInnerLengthTol = m_pEditDefectInnerLengthTol->text().toFloat() / dResolutionX;
    stCmd.fDefectOuterLengthTol = m_pEditDefectOuterLengthTol->text().toFloat() / dResolutionX;
    stCmd.fInnerMaskDepth = m_pEditInnerMaskDepth->text().toFloat() / dResolutionX;
    stCmd.fOuterMaskDepth = m_pEditOuterMaskDepth->text().toFloat() / dResolutionX;
    stCmd.nRecordId = recordId;

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    stCmd.matInputImg = pUI->getImage();
    cv::Rect rectROI = pUI->getSelectedROI();
    if (rectROI.width <= 0 || rectROI.height <= 0) {
        QMessageBox::critical(this, strTitle, QStringLiteral("Please select a ROI to do inspection."));
        return false;
    }
    stCmd.rectROI = rectROI;

    Vision::PR_InspContour(&stCmd, &stRpy);
    if (bShowResult) {
        QString strMsg;
        strMsg.sprintf("Inspect contour status %d, defect count %d.", Vision::ToInt32(stRpy.enStatus), stRpy.vecDefectContour.size());
        QMessageBox::information(this, strTitle, strMsg);
    }

    return Vision::VisionStatus::OK == stRpy.enStatus;
}

void InspContourWidget::tryInsp() {
	auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
	auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

	int nRecordId = 0;
    bool bNewRecord = false;
    if (m_currentWindow.recordId > 0)
        nRecordId = m_currentWindow.recordId;
    else {
        if (!_learnContour(nRecordId))
            return;
        else
            bNewRecord = true;
    }

    if (_inspContour(nRecordId))
        m_bIsTryInspected = true;
    else
        m_bIsTryInspected = false;
	
    if (bNewRecord)
	    Vision::PR_FreeRecord(nRecordId);
}

void InspContourWidget::confirmWindow(OPERATION enOperation) {
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
	auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();
    auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();

    int nRecordId = 0;
    if (!_learnContour(nRecordId)) {
        return;
    }

    if (!_inspContour(nRecordId, false)) {
        System->showInteractMessage(QStringLiteral("边界检测框"), QStringLiteral("检测失败!"));
        return;
    }

	QJsonObject json;
	json.insert("DefectThreshold", m_pEditDefectThreshold->text().toInt());
	json.insert("MinDefectArea", m_pEditMinDefectArea->text().toFloat());
    json.insert("DefectInnerLengthTol", m_pEditDefectInnerLengthTol->text().toFloat());
    json.insert("DefectOuterLengthTol", m_pEditDefectOuterLengthTol->text().toInt());
    json.insert("InnerMaskDepth", m_pEditInnerMaskDepth->text().toFloat());
    json.insert("OuterMaskDepth", m_pEditOuterMaskDepth->text().toFloat());

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
	window.usage = Engine::Window::Usage::INSP_CONTOUR;
	window.inspParams = byte_array;
	
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
	window.width = rectROI.width   * dResolutionX;
	window.height = rectROI.height * dResolutionY;
	window.deviceId = pUI->getSelectedDevice().getId();
	window.angle = 0;

    window.recordId = nRecordId;
    if (ReadBinaryFile(FormatRecordName(window.recordId), window.recordData) != 0) {
        QMessageBox::critical(this, QStringLiteral("Add Alignment Window"), QStringLiteral("Failed to read record data."));
	    return;
    }

	int result = Engine::OK;
	if (OPERATION::ADD == enOperation) {
		window.deviceId = pUI->getSelectedDevice().getId();
		char windowName[100];
		_snprintf(windowName, sizeof(windowName), "Inspect Contour [%d, %d] @ %s", Vision::ToInt32(window.x), Vision::ToInt32(window.y), pUI->getSelectedDevice().getName().c_str());

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
		}
		else {
			System->setTrackInfo(QString("Success to update window: %1.").arg(window.name.c_str()));
		}
	}

	m_pParent->UpdateInspWindowList();
}