#include "HeightDetectWidget.h"
#include <QMessageBox>
#include <qjsonobject.h>
#include <qjsondocument.h>

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
	MEASURE_TYPE_ATTRI,
	RANGE_MIN_ATTRI,
	RANGE_MAX_ATTRI,	
};

HeightDetectWidget::HeightDetectWidget(InspWindowWidget *parent)
	:EditInspWindowBaseWidget(parent)
{
	ui.setupUi(this);

	m_pCheckBoxMeasure = std::make_unique<QCheckBox>(ui.tableWidget);
	ui.tableWidget->setCellWidget(MEASURE_TYPE_ATTRI, DATA_COLUMN, m_pCheckBoxMeasure.get());

	m_pEditMinRange = std::make_unique<QLineEdit>(ui.tableWidget);
	m_pEditMinRange->setValidator(new QDoubleValidator(0, 100, 2, m_pEditMinRange.get()));
	ui.tableWidget->setCellWidget(RANGE_MIN_ATTRI, DATA_COLUMN, m_pEditMinRange.get());

	m_pEditMaxRange = std::make_unique<QLineEdit>(ui.tableWidget);
	m_pEditMaxRange->setValidator(new QDoubleValidator(0, 100, 2, m_pEditMaxRange.get()));
	ui.tableWidget->setCellWidget(RANGE_MAX_ATTRI, DATA_COLUMN, m_pEditMaxRange.get());

	m_pStdItmModelRel = std::make_unique<QStandardItemModel>(ui.tableWidget);
	ui.listView->setModel(m_pStdItmModelRel.get());	
}

HeightDetectWidget::~HeightDetectWidget()
{
}

void HeightDetectWidget::setDefaultValue()
{
	m_pCheckBoxMeasure->setChecked(true);
	m_pEditMinRange->setText("30");
	m_pEditMaxRange->setText("70");
	m_pStdItmModelRel->clear();
}

void HeightDetectWidget::tryInsp()
{
	if (!m_pCheckBoxMeasure->isChecked())
	{
		QString strMsg;
		strMsg.sprintf("Select Measure Window to measure height!");
		QMessageBox::information(this, "Measure Height", strMsg);
		return;
	}

	auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
	auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

	Vision::PR_CALC_3D_HEIGHT_DIFF_CMD stCmd;
	Vision::PR_CALC_3D_HEIGHT_DIFF_RPY stRpy;

	stCmd.fEffectHRatioStart = m_pEditMinRange->text().toFloat();
	stCmd.fEffectHRatioEnd = m_pEditMaxRange->text().toFloat();

	auto pUI = getModule<IVisionUI>(UI_MODEL);
	stCmd.matHeight = pUI->getHeightData();
	cv::Rect rectROI = pUI->getSelectedROI();
	if (rectROI.width <= 0 || rectROI.height <= 0) {
		QMessageBox::critical(this, QStringLiteral("Add Insp Hole Window"), QStringLiteral("Please select a ROI to do inspection."));
		return;
	}
	stCmd.rectROI = rectROI;

	for (int i = 0; i < m_pStdItmModelRel->rowCount(); i++)
	{
		QStandardItem* pItem = m_pStdItmModelRel->item(i);
		if (pItem && pItem->checkState() == Qt::Checked)
		{
			int nWinID = pItem->text().toInt();	
			stCmd.vecRectBases.push_back(getWindowRect(nWinID));
		}
	}

	Vision::PR_Calc3DHeightDiff(&stCmd, &stRpy);
	QString strMsg;
	strMsg.sprintf("Inspect Status %d, height(%f)", Vision::ToInt32(stRpy.enStatus), stRpy.fHeightDiff);
	QMessageBox::information(this, "Height Detect", strMsg);
}

void HeightDetectWidget::confirmWindow(OPERATION enOperation)
{
	auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
	auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();
    auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();

	QJsonObject json;
	json.insert("MinRange", m_pEditMinRange->text().toFloat());
	json.insert("MaxRange", m_pEditMaxRange->text().toFloat());

	if (m_pCheckBoxMeasure->isChecked())
	{
		QString szBaseIDs;
		for (int i = 0; i < m_pStdItmModelRel->rowCount(); i++)
		{
			QStandardItem* pItem = m_pStdItmModelRel->item(i);
			if (pItem && pItem->checkState() == Qt::Checked)
			{
				szBaseIDs += pItem->text();
				szBaseIDs += ',';
			}
		}
		json.insert("BaseIDs", szBaseIDs);
	}

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
	window.usage = m_pCheckBoxMeasure->isChecked() ? Engine::Window::Usage::HEIGHT_MEASURE : Engine::Window::Usage::HEIGHT_BASE;
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
	window.width = rectROI.width  * dResolutionX;
	window.height = rectROI.height * dResolutionY;
	window.deviceId = pUI->getSelectedDevice().getId();
	window.angle = 0;

	int result = Engine::OK;
	if (OPERATION::ADD == enOperation) {
		window.deviceId = pUI->getSelectedDevice().getId();
		char windowName[100];
        if (Engine::Window::Usage::HEIGHT_MEASURE == window.usage)
		    _snprintf(windowName, sizeof(windowName), "Height Detect [%d, %d] @ %s", Vision::ToInt32(window.x), Vision::ToInt32(window.y), pUI->getSelectedDevice().getName().c_str());
        else
            _snprintf(windowName, sizeof(windowName), "Height Detect Base [%d, %d] @ %s", Vision::ToInt32(window.x), Vision::ToInt32(window.y), pUI->getSelectedDevice().getName().c_str());

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
        if (bBoardRotated)
            ptCenter.x = nBigImgWidth  - ptCenter.x;
        else
            ptCenter.y = nBigImgHeight - ptCenter.y; //In cad, up is positive, but in image, down is positive.
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

void HeightDetectWidget::setCurrentWindow(const Engine::Window &window)
{
	m_currentWindow = window;

	auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
	auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

	m_pCheckBoxMeasure->setChecked(window.usage == Engine::Window::Usage::HEIGHT_MEASURE);

	QString szBaseIDs = "";

	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(window.inspParams.c_str(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();

			m_pEditMinRange->setText(QString::number(obj.take("MinRange").toDouble()));
			m_pEditMaxRange->setText(QString::number(obj.take("MaxRange").toDouble()));

			szBaseIDs = obj.take("BaseIDs").toString();
		}
	}

	m_pStdItmModelRel->clear();
	if (window.usage == Engine::Window::Usage::HEIGHT_MEASURE)
	{
		QStringList baseIDs = szBaseIDs.split(',');

		//Get all base windows and bind the relation ones
		Engine::WindowVector vecCurrentDeviceWindows;
		getDeviceBaseWindows(vecCurrentDeviceWindows);
		
		int nItemCount = 0;
		for each (Engine::Window win in vecCurrentDeviceWindows)
		{
			//std::unique_ptr<QStandardItem> poListItem = std::make_unique<QStandardItem>();
			QStandardItem *poListItem = new QStandardItem;

			// Checkable item
			poListItem->setCheckable(true);
			// Uncheck the item
			poListItem->setCheckState(Qt::Unchecked);
			// Save checke state
			poListItem->setData(baseIDs.contains(QString::number(win.Id)) ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);

			poListItem->setText(QString::number(win.Id));

			m_pStdItmModelRel->setItem(nItemCount++, poListItem);
		}
	}	
}

void HeightDetectWidget::getDeviceBaseWindows(Engine::WindowVector& vecCurrentDeviceWindows)
{
	IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);

	Engine::WindowVector vecCurrentDeviceWindowsAll;
	auto result = Engine::GetDeviceWindows(pUI->getSelectedDevice().getId(), vecCurrentDeviceWindowsAll);
	if (result != Engine::OK) {
		String errorType, errorMessage;
		Engine::GetErrorDetail(errorType, errorMessage);
		errorMessage = "Failed to get inspect windows from database, error message " + errorMessage;
		QMessageBox::critical(nullptr, QStringLiteral("Inspect Window"), errorMessage.c_str(), QStringLiteral("Quit"));
	}
	
	for each (Engine::Window win in vecCurrentDeviceWindowsAll)
	{
		if (win.usage == Engine::Window::Usage::HEIGHT_BASE)
		{
			vecCurrentDeviceWindows.push_back(win);
		}
	}
}

cv::Rect HeightDetectWidget::getWindowRect(int nWinID)
{
	Engine::WindowVector vecCurrentDeviceWindows;
	getDeviceBaseWindows(vecCurrentDeviceWindows);

	for each (Engine::Window win in vecCurrentDeviceWindows)
	{
		if (win.Id == nWinID)
		{
			return cv::Rect(win.x, win.y, win.width, win.height);
		}
	}

	return cv::Rect();
}
