#include "AlignmentWidget.h"
#include <QMessageBox>
#include <qjsonobject.h>
#include <qjsondocument.h>

#include "../Common/SystemData.h"
#include "DataStoreAPI.h"
#include "VisionAPI.h"
#include "../include/IVisionUI.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../Common/CommonFunc.h"
#include "../DataModule/QDetectObj.h"
#include "InspWindowWidget.h"

using namespace NFG::AOI;
using namespace AOI;

enum BASIC_PARAM {
	ALGORITHM_ATTRI,
	RECORDID_ATTRI,
	SUBPIXEL_ATTRI,
	MOTION_ATTRI,
	MINSCORE_ATTRI,
};

AlignmentWidget::AlignmentWidget(InspWindowWidget *parent)
	:EditInspWindowBaseWidget(parent)
{
	ui.setupUi(this);

	m_pComboBoxAlgorithm = std::make_unique<QComboBox>(ui.tableWidget);
	m_pComboBoxAlgorithm->addItem("SQUARE DIFF");
	m_pComboBoxAlgorithm->addItem("HIERARCHICAL EDGE");
	m_pComboBoxAlgorithm->addItem("HIERARCHICAL AREA");
	ui.tableWidget->setCellWidget(ALGORITHM_ATTRI, DATA_COLUMN, m_pComboBoxAlgorithm.get());

	m_pEditRecordID = std::make_unique<QLineEdit>(ui.tableWidget);
	m_pEditRecordID->setValidator(new QIntValidator(0, 1000, m_pEditRecordID.get()));
	ui.tableWidget->setCellWidget(RECORDID_ATTRI, DATA_COLUMN, m_pEditRecordID.get());
	m_pEditRecordID->setEnabled(false);

	m_pCheckBoxSubPixel = std::make_unique<QCheckBox>(ui.tableWidget);
	ui.tableWidget->setCellWidget(SUBPIXEL_ATTRI, DATA_COLUMN, m_pCheckBoxSubPixel.get());

	m_pComboBoxMotion = std::make_unique<QComboBox>(ui.tableWidget);
	m_pComboBoxMotion->addItem("TRANSLATION");
	m_pComboBoxMotion->addItem("EUCLIDEAN");
	m_pComboBoxMotion->addItem("AFFINE");
	m_pComboBoxMotion->addItem("HOMOGRAPHY");
	ui.tableWidget->setCellWidget(MOTION_ATTRI, DATA_COLUMN, m_pComboBoxMotion.get());

	m_pEditMinScore = std::make_unique<QLineEdit>(ui.tableWidget);
	m_pEditMinScore->setValidator(new QIntValidator(1, 100, m_pEditMinScore.get()));
	ui.tableWidget->setCellWidget(MINSCORE_ATTRI, DATA_COLUMN, m_pEditMinScore.get());
}

AlignmentWidget::~AlignmentWidget()
{
}

void AlignmentWidget::setDefaultValue()
{
	m_pComboBoxAlgorithm->setCurrentIndex(0);
	m_pEditRecordID->setText("0");

	m_pCheckBoxSubPixel->setChecked(false);
	m_pComboBoxMotion->setCurrentIndex(0);
	m_pEditMinScore->setText("60");
}

void AlignmentWidget::tryInsp()
{
	auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
	auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

	int nRecordID = m_pEditRecordID->text().toInt();

	if (nRecordID <= 0)
	{
		Vision::PR_LRN_TEMPLATE_CMD stCmd;
		Vision::PR_LRN_TEMPLATE_RPY stRpy;

		stCmd.enAlgorithm = static_cast<Vision::PR_MATCH_TMPL_ALGORITHM>(m_pComboBoxAlgorithm->currentIndex());

		auto pUI = getModule<IVisionUI>(UI_MODEL);
		stCmd.matInputImg = pUI->getImage();
		cv::Rect rectROI = pUI->getSelectedROI();
		if (rectROI.width <= 0 || rectROI.height <= 0) {
			QMessageBox::critical(this, QStringLiteral("Add Alignment Window"), QStringLiteral("Please select a ROI to do inspection."));
			return;
		}

		stCmd.rectROI = rectROI;

		Vision::PR_LrnTmpl(&stCmd, &stRpy);
		QString strMsg;
		strMsg.sprintf("Inspect Status %d, recordID(%d)", Vision::ToInt32(stRpy.enStatus), stRpy.nRecordId);
		m_pEditRecordID->setText(QString::number(stRpy.nRecordId));
		QMessageBox::information(this, "Alignment", strMsg);
	}
	else
	{
		Vision::PR_MATCH_TEMPLATE_CMD stCmd;
		Vision::PR_MATCH_TEMPLATE_RPY stRpy;

		stCmd.enAlgorithm = static_cast<Vision::PR_MATCH_TMPL_ALGORITHM>(m_pComboBoxAlgorithm->currentIndex());
		stCmd.bSubPixelRefine = m_pCheckBoxSubPixel->isChecked();
		stCmd.enMotion = static_cast<Vision::PR_OBJECT_MOTION>(m_pComboBoxMotion->currentIndex());
		stCmd.fMinMatchScore = m_pEditMinScore->text().toFloat();
		stCmd.nRecordId = nRecordID;	

		auto pUI = getModule<IVisionUI>(UI_MODEL);
		stCmd.matInputImg = pUI->getImage();
		cv::Rect rectROI = pUI->getSelectedROI();
		if (rectROI.width <= 0 || rectROI.height <= 0) {
			QMessageBox::critical(this, QStringLiteral("Add Alignment Window"), QStringLiteral("Please select a ROI to do inspection."));
			return;
		}

		stCmd.rectSrchWindow = rectROI;
		stCmd.rectSrchWindow.x -= stCmd.rectSrchWindow.width  * 0.1;
		stCmd.rectSrchWindow.y -= stCmd.rectSrchWindow.height * 0.1;
		stCmd.rectSrchWindow.width  *= 1.2;
		stCmd.rectSrchWindow.height *= 1.2;

		Vision::PR_MatchTmpl(&stCmd, &stRpy);
		QString strMsg;
		strMsg.sprintf("Inspect Status %d, center(%f, %f), rotation(%f), score(%f)", Vision::ToInt32(stRpy.enStatus), stRpy.ptObjPos.x, stRpy.ptObjPos.y, stRpy.fRotation, stRpy.fMatchScore);
		QMessageBox::information(this, "Alignment", strMsg);
	}
}

void AlignmentWidget::confirmWindow(OPERATION enOperation)
{
	auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
	auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

	QJsonObject json;
	json.insert("Algorithm", m_pComboBoxAlgorithm->currentIndex());
	json.insert("SubPixel", m_pCheckBoxSubPixel->isChecked());
	json.insert("Motion", m_pComboBoxMotion->currentIndex());
	json.insert("MinScore", m_pEditMinScore->text().toFloat());

	QJsonDocument document;
	document.setObject(json);
	QByteArray byte_array = document.toJson(QJsonDocument::Compact);

	auto pUI = getModule<IVisionUI>(UI_MODEL);
	auto rectROI = pUI->getSelectedROI();
	if (rectROI.width <= 0 || rectROI.height <= 0) {
		QMessageBox::critical(this, QStringLiteral("Add Alignment Window"), QStringLiteral("Please select a ROI to do inspection."));
		return;
	}

	Engine::Window window;
	window.lightId = m_pParent->getSelectedLighting() + 1;
	window.usage = Engine::Window::Usage::ALIGNMENT;
	window.inspParams = byte_array;
	window.x = (rectROI.x + rectROI.width  / 2.f) * dResolutionX;
	window.y = (rectROI.y + rectROI.height / 2.f) * dResolutionY;
	window.width = rectROI.width  * dResolutionX;
	window.height = rectROI.height * dResolutionY;
	window.deviceId = pUI->getSelectedDevice().getId();
	window.angle = 0;
	window.recordId = m_pEditRecordID->text().toInt();

    if (ReadBinaryFile(FormatRecordName(window.recordId), window.recordData) != 0) {
        QMessageBox::critical(this, QStringLiteral("Add Alignment Window"), QStringLiteral("Failed to read record data."));
	    return;
    }

	int result = Engine::OK;
	if (OPERATION::ADD == enOperation) {
		window.deviceId = pUI->getSelectedDevice().getId();
		char windowName[100];
		_snprintf(windowName, sizeof(windowName), "Alignment [%d, %d] @ %s", Vision::ToInt32(window.x), Vision::ToInt32(window.y), pUI->getSelectedDevice().getName().c_str());
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

		QDetectObj detectObj(window.Id, window.name.c_str());
		cv::Point2f ptCenter(window.x / dResolutionX, window.y / dResolutionY);
		cv::Size2f szROI(window.width / dResolutionX, window.height / dResolutionY);
		detectObj.setFrame(cv::RotatedRect(ptCenter, szROI, window.angle));
		auto vecDetectObjs = pUI->getDetectObjs();
		vecDetectObjs.push_back(detectObj);
		pUI->setDetectObjs(vecDetectObjs);
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

	m_pParent->UpdateInspWindowList();
}

void AlignmentWidget::setCurrentWindow(const Engine::Window &window)
{
	m_currentWindow = window;

	auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
	auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();

			m_pComboBoxAlgorithm->setCurrentIndex(obj.take("Algorithm").toInt());
			
			m_pCheckBoxSubPixel->setChecked(obj.take("SubPixel").toBool());
			m_pComboBoxMotion->setCurrentIndex(obj.take("Motion").toInt());
			m_pEditMinScore->setText(QString::number(obj.take("MinScore").toDouble()));
		}
	}

	m_pEditRecordID->setText(QString::number(window.recordId));
}
