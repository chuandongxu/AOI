#include "DataCtrl.h"
#include "../Common/ThreadPrioc.h"
#include "datamodule_global.h"
#include "QDetectObj.h"
#include "QProfileObj.h"

#include <QDir>

#include <QApplication>
#include <QDebug>
#include "../Common/SystemData.h"

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "../lib/DataStoreAPI/include/DataStoreAPI.h"

#include "dog_api_cpp.h"
#include "dog_vcode.h"
#include "errorprinter.h"

#define DEFAULT_DATABASE_TMP_NAME "DeviceTmpDataBase.aoi"
#define DEFAULT_DATABASE_OBJ_NAME "DeviceDataBase.aoi"
#define DEFAULT_DATABASE_USER "XSG"

using namespace NFG::AOI;

//template<int id>
//class CCompFindDevice {
//public:
//	bool operator()(const Engine::Device& lhs) {
//		return (lhs.Id == id);
//	}
//};
//
//Engine::DeviceVector::iterator rstFind = std::find_if(vecDevice.begin(), vecDevice.end(), CCompFindDevice<nDeviceID>());
//if (rstFind != vecDevice.end())
//{
//}

bool getDeviceGroup(Int64 devieId, const std::string groupNameUsage, std::vector<Int64>& winGroupIds)
{
	Int64Vector vecGroupIds;
	auto result = Engine::GetAllWindowGroups(vecGroupIds);
	if (result != Engine::OK) {
		String errorType, errorMessage;
		Engine::GetErrorDetail(errorType, errorMessage);
		System->setTrackInfo(QString("Error at GetAllWindowGroups, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
		return false;
	}

	for (auto groupId : vecGroupIds)
	{
		Engine::WindowGroup windowGroup;
		result = Engine::GetGroupWindows(groupId, windowGroup);
		if (result != Engine::OK) {
			String errorType, errorMessage;
			Engine::GetErrorDetail(errorType, errorMessage);
			System->setTrackInfo(QString("Error at GetGroupWindows, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
			return false;
		}

		if (windowGroup.deviceId == devieId && QString(windowGroup.name.c_str()).contains(groupNameUsage.c_str()))
		{
			winGroupIds.push_back(groupId);
		}		
	}

	return true;
}

bool getGroupWindow(Int64 groupId, Engine::Window::Usage usage, Engine::WindowVector &vecWindow)
{
	Engine::WindowGroup windowGroup;
	auto result = Engine::GetGroupWindows(groupId, windowGroup);
	if (result != Engine::OK) {
		String errorType, errorMessage;
		Engine::GetErrorDetail(errorType, errorMessage);
		System->setTrackInfo(QString("Error at GetGroupWindows, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
		return false;
	}

	for (auto win : windowGroup.vecWindows)
	{
		if (win.usage == usage)
		{
			vecWindow.push_back(win);
		}
	}

	return true;
}

//bool getDeviceWindow(Int64 devieId, Engine::WindowVector &vecWindow)
//{	
//	auto result = Engine::GetAllWindows(vecWindow);
//	if (result != Engine::OK) {
//		String errorType, errorMessage;
//		Engine::GetErrorDetail(errorType, errorMessage);
//
//		System->setTrackInfo(QString("Error at GetAllWindows, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
//		return false;
//	}
//
//	return true;
//}
//
//bool getDeviceWindow(Int64 devieId, Engine::Window::Usage usage,Engine::WindowVector &vecWindow)
//{
//	Engine::WindowVector vecWindowTmp;
//	if (!getDeviceWindow(devieId, vecWindowTmp))
//	{
//		return false;
//	}
//
//	if (!vecWindowTmp.empty())
//	{
//		for (auto win : vecWindowTmp)
//		{
//			if (win.usage == usage)
//			{
//				vecWindow.push_back(win);
//			}
//		}
//
//		return true;
//	}
//
//	return false;
//}
//
//bool getGropuWindow(Int64 winId, Int64& winGroupId)
//{
//	Int64Vector vecGroupIds;
//	auto result = Engine::GetAllWindowGroups(vecGroupIds);
//	if (result != Engine::OK) {
//		String errorType, errorMessage;
//		Engine::GetErrorDetail(errorType, errorMessage);
//		System->setTrackInfo(QString("Error at GetAllWindowGroups, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
//		return false;
//	}
//
//	for (auto groupId : vecGroupIds) 
//	{
//		Engine::WindowGroup windowGroup;
//		result = Engine::GetGroupWindows(groupId, windowGroup);
//		if (result != Engine::OK) {
//			String errorType, errorMessage;
//			Engine::GetErrorDetail(errorType, errorMessage);
//			System->setTrackInfo(QString("Error at GetGroupWindows, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
//			return false;
//		}
//
//		for (auto win : windowGroup.vecWindows)
//		{
//			if (win.Id == winId && (win.usage == Engine::Window::Usage::HEIGHT_MEASURE))
//			{
//				winGroupId = windowGroup.Id;
//				break;
//			}
//		}
//	}
//
//	return true;
//}

DataCtrl::DataCtrl(QObject *parent)
	: QObject(parent)
{
	m_nCycleTestNum = 0;

	m_bInfiniteCycles = false;

	m_nProfileIndex = 0;

	m_boardObj = new QBoardObj(0, "TestBoard");

	String info;
	Engine::GetVersion(info);

	System->setTrackInfo(QString("Database Version:%1").arg(info.c_str()));
}

DataCtrl::~DataCtrl()
{
	for (int i = 0; i < m_cellTmpObjs.size(); i++)
	{
		delete m_cellTmpObjs[i];
	}
	m_cellTmpObjs.clear();

	for (int i = 0; i < m_cellTestObjs.size(); i++)
	{
		delete m_cellTestObjs[i];
	}
	m_cellTestObjs.clear();

	if (m_boardObj)
	{
		delete m_boardObj;
		m_boardObj = NULL;
	}
}

void DataCtrl::setInfiniteCycles(bool bInfinite)
{
	m_bInfiniteCycles = bInfinite;
}

void DataCtrl::incrementCycleTests()
{
	QAutoLocker loacker(&m_mutex);
	m_nCycleTestNum += 1;
}

void DataCtrl::decrementCycleTests()
{
	QAutoLocker loacker(&m_mutex);
	if (m_nCycleTestNum > 0)
	{
		m_nCycleTestNum -= 1;
	}
	else
	{
		m_nCycleTestNum = 0;
	}
}

int	DataCtrl::getCycleTests()
{
	QAutoLocker loacker(&m_mutex);

	if (m_bInfiniteCycles) return 1;

	return m_nCycleTestNum;
}

QBoardObj* DataCtrl::getBoardObj()
{
	return m_boardObj;
}

int DataCtrl::getObjNum(DataTypeEnum emDataType)
{
	return getCellType(emDataType).size();
}

QDetectObj* DataCtrl::getObj(int nIndex, DataTypeEnum emDataType)
{
	if (nIndex >= 0 && nIndex < getCellType(emDataType).size())
	{
		return getCellType(emDataType)[nIndex];
	}

	return NULL;
}

void DataCtrl::pushObj(QDetectObj* pObj, DataTypeEnum emDataType)
{
	getCellType(emDataType).push_back(pObj);
}

void DataCtrl::deleteObj(int nIndex, DataTypeEnum emDataType)
{
	if (nIndex >= 0 && nIndex < getCellType(emDataType).size())
	{
		delete getCellType(emDataType)[nIndex];
		getCellType(emDataType).removeAt(nIndex);
	}
}

void DataCtrl::clearObjs(DataTypeEnum emDataType)
{
	for (int i = 0; i < getCellType(emDataType).size(); i++)
	{
		delete getCellType(emDataType)[i];
	}
	getCellType(emDataType).clear();
}


bool DataCtrl::saveDataBase(QString& szFilePath, DataTypeEnum emDataType)
{
	QString path = QApplication::applicationDirPath();
	path += "/3D/data/";

	QString szDataPath = szFilePath;
	if (szDataPath.isEmpty() || !szDataPath.contains(".aoi") || !szDataPath.contains(".AOI"))
	{
		switch (emDataType)
		{
		case EM_DATA_TYPE_TMP:
			szDataPath = path + DEFAULT_DATABASE_TMP_NAME;
			break;
		case EM_DATA_TYPE_OBJ:
			szDataPath = path + DEFAULT_DATABASE_OBJ_NAME;
			break;
		default:
			break;
		}		
	}

	auto result = Engine::OpenProject(szDataPath.toStdString(), DEFAULT_DATABASE_USER);
	if (result != Engine::OK) {
		String errorType, errorMessage;
		Engine::GetErrorDetail(errorType, errorMessage);
		System->setTrackInfo(QString("Error at OpenProject, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));

		result = Engine::CreateProject(szDataPath.toStdString(), DEFAULT_DATABASE_USER);
		if (result != Engine::OK) {
			String errorType, errorMessage;
			Engine::GetErrorDetail(errorType, errorMessage);
			System->setTrackInfo(QString("Error at CreateProject, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
			return false;
		}
	}

	clearDataBase();

	Engine::Board board;
	board.name = m_boardObj->getName().toStdString();
	board.abbr = "TB";
	board.angle = 0;
	result = Engine::CreateBoard(board);
	if (result != Engine::OK) {
		String errorType, errorMessage;
		Engine::GetErrorDetail(errorType, errorMessage);
		System->setTrackInfo(QString("Error at CreateBoard, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
		return false;
	}

	for (int i = 0; i < m_boardObj->getBoardAlignNum(); i++)
	{
		cv::RotatedRect rect = m_boardObj->getBoardAlign(i);
		int nRecordID = m_boardObj->getRecordID(i);

		Engine::Alignment alignment;		
		alignment.recordID = nRecordID;
		alignment.tmplPosX = rect.center.x;
		alignment.tmplPosY = rect.center.y;
		alignment.tmplWidth = rect.size.width;
		alignment.tmplHeight = rect.size.height;
		alignment.vecLight.push_back(0);
	
		result = Engine::CreateAlignment(alignment);
		if (result != Engine::OK) {
			String errorType, errorMessage;
			Engine::GetErrorDetail(errorType, errorMessage);

			System->setTrackInfo(QString("Error at CreateAlignment, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
			return false;
		}
	}

	for (int i = 0; i < getObjNum(emDataType); i++)
	{
		QDetectObj* pObj = getObj(i, emDataType);
		if (pObj)
		{
			Engine::Device device;
			device.name = (pObj->getName() + "#" +pObj->getType()).toStdString();
			device.schematicName = pObj->getBitmap().toStdString();
			device.x = pObj->getX();
			device.y = pObj->getY();
			device.width = pObj->getWidth();
			device.height = pObj->getHeight();
			device.angle = pObj->getAngle();

			// create new device			
			result = Engine::CreateDevice(board.Id, device);
			if (result != Engine::OK) {
				String errorType, errorMessage;
				Engine::GetErrorDetail(errorType, errorMessage);
				System->setTrackInfo(QString("Error at CreateDevice, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
				return false;
			}			

			//result = Engine::UpdateDevice(device);
			//if (result != Engine::OK) {
			//	String errorType, errorMessage;
			//	Engine::GetErrorDetail(errorType, errorMessage);
			//	System->setTrackInfo(QString("Error at UpdateDevice, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
			//	break;
			//}

			if (pObj->isLocCreated())
			{
				Engine::Window window;
				window.name = QString("Win Alignment %1").arg(device.Id).toStdString();
				window.x = pObj->getLoc().center.x;
				window.y = pObj->getLoc().center.y;
				window.width = pObj->getLoc().size.width;
				window.height = pObj->getLoc().size.height;
				window.angle = pObj->getLoc().angle;
				window.recordID = pObj->getRecordID();

				window.usage = Engine::Window::Usage::ALIGNMENT;
				result = Engine::CreateWindow(window);
				if (result != Engine::OK) {
					String errorType, errorMessage;
					Engine::GetErrorDetail(errorType, errorMessage);

					System->setTrackInfo(QString("Error at CreateWindow, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
					return false;
				}

				Engine::WindowGroup windowGroup;
				windowGroup.name = QString("Alignment Group %1").arg(device.Id).toStdString();
				windowGroup.vecWindows.push_back(window);	
				windowGroup.deviceId = device.Id;

				result = Engine::CreateWindowGroup(windowGroup);
				if (result != Engine::OK) {
					String errorType, errorMessage;
					Engine::GetErrorDetail(errorType, errorMessage);

					System->setTrackInfo(QString("Error at CreateWindowGroup, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
					return false;
				}
			}	

			Engine::WindowVector vecBaseWindow;
			QList<int> baseAllIDs;
			if (pObj->isHgtBaseCreated())
			{				
				for (int j = 0; j < pObj->getHeightBaseNum(); j++)
				{
					Engine::Window window;
					window.name = QString("Height Base %1 %2").arg(device.Id).arg(j+1).toStdString();
					window.x = pObj->getHeightBase(j).center.x;
					window.y = pObj->getHeightBase(j).center.y;
					window.width = pObj->getHeightBase(j).size.width;
					window.height = pObj->getHeightBase(j).size.height;
					window.angle = pObj->getHeightBase(j).angle;

					window.usage = Engine::Window::Usage::HEIGHT_BASE;
					result = Engine::CreateWindow(window);
					if (result != Engine::OK) {
						String errorType, errorMessage;
						Engine::GetErrorDetail(errorType, errorMessage);

						System->setTrackInfo(QString("Error at CreateWindow, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
						return false;
					}

					baseAllIDs.push_back(window.Id);
					vecBaseWindow.push_back(window);
				}
			}

			if (pObj->isHgtDetectCreated())
			{
				for (int j = 0; j < pObj->getHeightDetectNum(); j++)
				{
					Engine::Window window;
					window.name = QString("Height Measure %1 %2").arg(device.Id).arg(j + 1).toStdString();
					window.x = pObj->getHeightDetect(j).center.x;
					window.y = pObj->getHeightDetect(j).center.y;
					window.width = pObj->getHeightDetect(j).size.width;
					window.height = pObj->getHeightDetect(j).size.height;
					window.angle = pObj->getHeightDetect(j).angle;

					window.usage = Engine::Window::Usage::HEIGHT_MEASURE;
					result = Engine::CreateWindow(window);
					if (result != Engine::OK) {
						String errorType, errorMessage;
						Engine::GetErrorDetail(errorType, errorMessage);

						System->setTrackInfo(QString("Error at CreateWindow, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
						return false;
					}

					// add windows relations 
					Engine::WindowGroup windowGroup;
					windowGroup.name = QString("Measure Group %1 %2").arg(device.Id).arg(j).toStdString();
					windowGroup.vecWindows.push_back(window);
					windowGroup.deviceId = device.Id;

					QList<int> baseIndexes = pObj->getDBRelationBase(j);
					QList<int> baseRelIds;
					for (auto index : baseIndexes)
					{
						baseRelIds.push_back(baseAllIDs[index]);
					}

					//Engine::WindowVector vecWindow;
					//result = Engine::GetAllWindows(vecWindow);
					//if (result != Engine::OK) {
					//	String errorType, errorMessage;
					//	Engine::GetErrorDetail(errorType, errorMessage);

					//	System->setTrackInfo(QString("Error at GetAllWindows, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
					//	return false;
					//}
					for (auto win : vecBaseWindow)
					{
						if (baseRelIds.contains(win.Id))
						{
							windowGroup.vecWindows.push_back(win);
						}						
					}

					result = Engine::CreateWindowGroup(windowGroup);
					if (result != Engine::OK) {
						String errorType, errorMessage;
						Engine::GetErrorDetail(errorType, errorMessage);

						System->setTrackInfo(QString("Error at CreateWindowGroup, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
						return false;
					}
				}
			}
		}
	}

	return true;
}

bool DataCtrl::loadDataBase(QString& szFilePath, DataTypeEnum emDataType)
{
	QString path = QApplication::applicationDirPath();
	path += "/3D/data/";

	QString szDataPath = szFilePath;
	if (szDataPath.isEmpty() || !szDataPath.contains(".aoi") || !szDataPath.contains(".AOI"))
	{
		switch (emDataType)
		{
		case EM_DATA_TYPE_TMP:
			szDataPath = path + DEFAULT_DATABASE_TMP_NAME;
			break;
		case EM_DATA_TYPE_OBJ:
			szDataPath = path + DEFAULT_DATABASE_OBJ_NAME;
			break;
		default:
			break;
		}
	}

	auto result = Engine::OpenProject(szDataPath.toStdString(), DEFAULT_DATABASE_USER);
	if (result != Engine::OK) {
		String errorType, errorMessage;
		Engine::GetErrorDetail(errorType, errorMessage);
		System->setTrackInfo(QString("Error at OpenProject, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));		
		return false;
	}

	Engine::BoardVector vecBoard;
	result = Engine::GetAllBoards(vecBoard);
	if (result != Engine::OK) {
		String errorType, errorMessage;
		Engine::GetErrorDetail(errorType, errorMessage);
		System->setTrackInfo(QString("Error at GetAllBoards, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
		return false;
	}

	if (!vecBoard.empty())
	{
		m_boardObj->setID(vecBoard[0].Id);
		QString boardName = vecBoard[0].name.c_str();
		m_boardObj->setName(boardName);

		m_boardObj->clearBoardAlignments();
		Engine::AlignmentVector vecAlignment;
		result = Engine::GetAllAlignments(vecAlignment);
		if (result != Engine::OK) {
			String errorType, errorMessage;
			Engine::GetErrorDetail(errorType, errorMessage);
			System->setTrackInfo(QString("Error at GetAlignmentList, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
		}
		for (Engine::Alignment &alignment : vecAlignment)
		{
			cv::RotatedRect rect;
			rect.center.x = alignment.tmplPosX;
			rect.center.y = alignment.tmplPosY;
			rect.size.width = alignment.tmplWidth;
			rect.size.height = alignment.tmplHeight;
			rect.angle = 0;

			int recordID = alignment.recordID;

			m_boardObj->addBoardAlign(rect, recordID);
		}

		Engine::DeviceVector vecDevice;
		result = Engine::GetBoardDevice(vecBoard[0].Id, vecDevice);
		if (result != Engine::OK) {
			String errorType, errorMessage;
			Engine::GetErrorDetail(errorType, errorMessage);
		}

		for (Engine::Device &device : vecDevice)
		{
			QString nameAndType = device.name.c_str();
			QString name = nameAndType.left(nameAndType.indexOf('#'));
			QString type = nameAndType.right(nameAndType.size() - nameAndType.indexOf('#') - 1);

			QDetectObj *pObj = new QDetectObj(device.Id, name);
			pObj->setType(type);

			QString szBitmapPath = device.schematicName.c_str();
			pObj->setBitmap(szBitmapPath);

			cv::RotatedRect rtFrame;
			rtFrame.center.x = device.x;
			rtFrame.center.y = device.y;
			rtFrame.size.width = device.width;
			rtFrame.size.height = device.height;
			rtFrame.angle = device.angle;
			pObj->setFrame(rtFrame);

			std::vector<Int64> groupIDs;
			if (getDeviceGroup(device.Id, "Alignment", groupIDs))
			{
				if (!groupIDs.empty())
				{
					Engine::WindowVector vecWindow;
					if (getGroupWindow(groupIDs[0], Engine::Window::Usage::ALIGNMENT, vecWindow))
					{
						if (!vecWindow.empty())
						{
							cv::RotatedRect rtLoc;
							rtLoc.center.x = vecWindow[0].x;
							rtLoc.center.y = vecWindow[0].y;
							rtLoc.size.width = vecWindow[0].width;
							rtLoc.size.height = vecWindow[0].height;
							rtLoc.angle = vecWindow[0].angle;
							pObj->setLoc(rtLoc);
							pObj->setRecordID(vecWindow[0].recordID);
						}
					}
				}
			}

			QMap<int, int> heightBaseIndexAndID;
			QMap<int, int> heightMeasureIndexAndID;
			int nBaseIndex = 0, nMeasureIndex = 0;

			groupIDs.clear();
			if (getDeviceGroup(device.Id, "Measure", groupIDs))
			{
				for (auto groupID : groupIDs)
				{
					int nMeasureWinID = 0;
					Engine::WindowVector vecWindow;
					if (getGroupWindow(groupID, Engine::Window::Usage::HEIGHT_MEASURE, vecWindow))
					{						
						for (auto win : vecWindow)
						{
							cv::RotatedRect rtDetect;
							rtDetect.center.x = win.x;
							rtDetect.center.y = win.y;
							rtDetect.size.width = win.width;
							rtDetect.size.height = win.height;
							rtDetect.angle = win.angle;
							pObj->addHeightDetect(rtDetect);
							heightMeasureIndexAndID.insert(win.Id, nMeasureIndex++);
							nMeasureWinID = win.Id;
						}
					}

					vecWindow.clear();
					if (getGroupWindow(groupID, Engine::Window::Usage::HEIGHT_BASE, vecWindow))
					{
						for (auto win : vecWindow)
						{
							cv::RotatedRect rtBase;
							rtBase.center.x = win.x;
							rtBase.center.y = win.y;
							rtBase.size.width = win.width;
							rtBase.size.height = win.height;
							rtBase.angle = win.angle;
							pObj->addHeightBase(rtBase);
							heightBaseIndexAndID.insert(win.Id, nBaseIndex++);

							pObj->addDBRelation(heightMeasureIndexAndID.value(nMeasureWinID), heightBaseIndexAndID.value(win.Id));
						}
					}					
				}
			}	

			pushObj(pObj, emDataType);
		}
	}

	return true;
}

void DataCtrl::clearDataBase()
{
	// Clear Boards
	Engine::BoardVector vecBoard;
	auto result = Engine::GetAllBoards(vecBoard);
	if (result != Engine::OK) {
		String errorType, errorMessage;
		Engine::GetErrorDetail(errorType, errorMessage);
		System->setTrackInfo(QString("Error at GetAllBoards, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
		return;
	}

	if (!vecBoard.empty())
	{
		for (int i = 0; i < vecBoard.size(); i++)
		{
			result = Engine::DeleteBoard(vecBoard[i].Id);
			if (result != Engine::OK) {
				String errorType, errorMessage;
				Engine::GetErrorDetail(errorType, errorMessage);
				System->setTrackInfo(QString("Error at DeleteBoard, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
				return;
			}
		}
	}

	Engine::AlignmentVector vecAlignment;
	result = Engine::GetAllAlignments(vecAlignment);
	if (result != Engine::OK) {
		String errorType, errorMessage;
		Engine::GetErrorDetail(errorType, errorMessage);
		System->setTrackInfo(QString("Error at GetAlignmentList, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
	}
	for (Engine::Alignment &alignment : vecAlignment)
	{
		result = Engine::DeleteAlignment(alignment.Id);
		if (result != Engine::OK) {
			String errorType, errorMessage;
			Engine::GetErrorDetail(errorType, errorMessage);
			System->setTrackInfo(QString("Error at DeleteAlignment, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
			return;
		}
	}

	// Clear all windows
	Engine::WindowVector vecWindow;
	result = Engine::GetAllWindows(vecWindow);
	if (result != Engine::OK) {
		String errorType, errorMessage;
		Engine::GetErrorDetail(errorType, errorMessage);
		System->setTrackInfo(QString("Error at GetAllWindows, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
		return;
	}

	if (vecWindow.size() > 1)
	{
		for (int i = 0; i < vecWindow.size(); i++)
		{
			result = Engine::DeleteWindow(vecWindow[i].Id);
			if (result != Engine::OK) {
				String errorType, errorMessage;
				Engine::GetErrorDetail(errorType, errorMessage);
				System->setTrackInfo(QString("Error at DeleteWindow, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
				return;
			}
		}
	}

	// Clear window group
	Int64Vector vecGroupIds;
	result = Engine::GetAllWindowGroups(vecGroupIds);
	if (result != Engine::OK) 
	{
		String errorType, errorMessage;
		Engine::GetErrorDetail(errorType, errorMessage);
		System->setTrackInfo(QString("Error at GetAllWindowGroups, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
		return;
	}

	for (auto groupId : vecGroupIds) 
	{
		//Engine::WindowGroup windowGroup;
		//result = Engine::GetGroupWindows(groupId, windowGroup);
		//if (result != Engine::OK) {
		//	String errorType, errorMessage;
		//	Engine::GetErrorDetail(errorType, errorMessage);
		//	System->setTrackInfo(QString("Error at GetGroupWindows, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
		//	return;
		//}
		
		result = Engine::DeleteWindowGroup(groupId);
		if (result != Engine::OK) {
			String errorType, errorMessage;
			Engine::GetErrorDetail(errorType, errorMessage);
			System->setTrackInfo(QString("Error at DeleteWindowGroup, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
			return;
		}
	}
}

QVector<QDetectObj*>& DataCtrl::getCellType(DataTypeEnum emDataType)
{
	switch (emDataType)
	{
	case EM_DATA_TYPE_TMP:
		return m_cellTmpObjs;
		break;
	case EM_DATA_TYPE_OBJ:
		return m_cellTestObjs;
		break;
	default:
		break;
	}

	return QVector<QDetectObj*>();
}

unsigned int DataCtrl::getCoreData(int nIndex)
{
	const int numBytesToShow = 10;
	if (nIndex < 0 || nIndex >= numBytesToShow) return 0;

	//Prints the error messages for the return values of the functions
	ErrorPrinter errorPrinter;

	//Used to hold the return value of the called functions
	dogStatus status;

	CDog dog1(CDogFeature::fromFeature(1));
	status = dog1.login(vendor_code);
	errorPrinter.printError(status);

	if (!DOG_SUCCEEDED(status))
	{
		status = dog1.logout();
		errorPrinter.printError(status);
		return 0;
	}	

	dog_size_t size = 0;
	CDogFile defaultFile = dog1.getFile(1);

	status = defaultFile.getFileSize(size);
	errorPrinter.printError(status);
	if (!DOG_SUCCEEDED(status))
	{
		status = dog1.logout();
		errorPrinter.printError(status);
		return 0;
	}

	unsigned int unRetValue = 0;
	if (size)
	{		
		unsigned int fsize = (size < numBytesToShow) ? size : numBytesToShow;

		unsigned char *filedata = new unsigned char[fsize];

		status = defaultFile.read(filedata, fsize);
		errorPrinter.printError(status);
		if (!DOG_SUCCEEDED(status))
		{
			delete[] filedata;
			status = dog1.logout();
			errorPrinter.printError(status);
			return 0;
		}

		unRetValue = filedata[nIndex];

		delete[] filedata;
	}
	else
	{
		unRetValue = 0;
	}
	
	status = dog1.logout();
	errorPrinter.printError(status);

	return unRetValue;
}

int DataCtrl::getProfObjNum()
{
	return m_profileObjs.size();
}

int DataCtrl::increaseProfObjIndex()
{
	return ++m_nProfileIndex;
}

int DataCtrl::getProfObjIndex()
{
	return m_nProfileIndex;
}

QProfileObj* DataCtrl::getProfObj(int nIndex)
{
	if (nIndex >= 0 && nIndex < m_profileObjs.size())
	{
		return m_profileObjs[nIndex];
	}

	return NULL;
}

void DataCtrl::pushProfObj(QProfileObj* pObj)
{
	m_profileObjs.push_back(pObj);
}

void DataCtrl::deleteProfObj(int nIndex)
{
	if (nIndex >= 0 && nIndex < m_profileObjs.size())
	{
		delete m_profileObjs[nIndex];
		m_profileObjs.removeAt(nIndex);
	}
}

void DataCtrl::clearProfObjs()
{
	for (int i = 0; i < m_profileObjs.size(); i++)
	{
		delete m_profileObjs[i];
	}
	m_profileObjs.clear();
}

bool DataCtrl::saveProfDataBase(QString& szFilePath)
{
	QString path = QApplication::applicationDirPath();
	path += "/3D/data/";

	std::string strResultMatPath = QString(path + "profileData.yml").toStdString();
	if (!szFilePath.isEmpty())
	{
		strResultMatPath = szFilePath.toStdString();
	}

	cv::FileStorage fs(strResultMatPath, cv::FileStorage::WRITE);
	if (!fs.isOpened())
		return false;

	clearFiles(path + "prof/");

	write(fs, QString("obj_index").toStdString(), m_nProfileIndex);
	write(fs, QString("obj_num").toStdString(), getProfObjNum());

	for (int i = 0; i < getProfObjNum(); i++)
	{
		QProfileObj* pObj = getProfObj(i);
		if (pObj)
		{
			write(fs, QString("obj_ID_%1").arg(i + 1).toStdString(), pObj->getID());
			write(fs, QString("obj_name_%1").arg(i + 1).toStdString(), pObj->getName().toStdString());

			double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
			pObj->getCutingPosn(x1, y1, x2, y2);

			write(fs, QString("pos_x1_%1").arg(i + 1).toStdString(), x1);
			write(fs, QString("pos_y1_%1").arg(i + 1).toStdString(), y1);
			write(fs, QString("pos_x2_%1").arg(i + 1).toStdString(), x2);
			write(fs, QString("pos_y2_%1").arg(i + 1).toStdString(), y2);

			QImage img3D = pObj->get3dImage();
			if (!img3D.isNull())
			{
				QPixmap pixmap3D = QPixmap::fromImage(img3D);
				pixmap3D.save(path + "prof/" + QString("prof_3D_%1").arg(i + 1) + ".bmp", "bmp");
			}		

			//QImage imgGray = pObj->getGrayImage();
			//QPixmap pixmapGray = QPixmap::fromImage(imgGray);
			//pixmapGray.save(path + "prof/" + QString("prof_gray_%1").arg(i + 1) + ".bmp", "bmp");

			cv::Mat grayMat = pObj->getGrayImage();
			if (!grayMat.empty())
			{
				IplImage imgGray = IplImage(grayMat);
				cvSaveImage((path + "prof/" + QString("prof_gray_%1").arg(i + 1) + ".bmp").toStdString().c_str(), &imgGray);
			}		

			QVector<cv::Point2d> profDatas = pObj->getProfData();
			std::vector<cv::KeyPoint> profDatasTmp;
			for (int j = 0; j < profDatas.size(); j++)
			{
				profDatasTmp.push_back(cv::KeyPoint(profDatas[j], 0));					
			}
			write(fs, QString("prof_datas_%1").arg(i + 1).toStdString(), profDatasTmp);	


			write(fs, QString("prof_meas_num_%1").arg(i + 1).toStdString(), pObj->getMeasObjNum());
			for (int j = 0; j < pObj->getMeasObjNum(); j++)
			{
				QProfMeasureObj* pMeasObj = pObj->getMeasObj(j);
				if (pMeasObj)
				{
					write(fs, QString("prof_meas_type_%1_%2").arg(i + 1).arg(j + 1).toStdString(), pMeasObj->getType());
					//write(fs, QString("prof_meas_name_%1_%2").arg(i + 1).arg(j + 1).toStdString(), pMeasObj->getName().toStdString());
				
					std::vector<cv::KeyPoint> profDatasTmp;
					for (int m = 0; m < pMeasObj->getRangeNum(); m++)
					{
						profDatasTmp.push_back(cv::KeyPoint(pMeasObj->getRange(m), pMeasObj->getRangeValue(m).x, pMeasObj->getRangeValue(m).y, pMeasObj->getRangeType(m)));
						qDebug() << "Write: " << pMeasObj->getRangeValue(m).x << " " << pMeasObj->getRangeValue(m).y << " " << pMeasObj->getRangeType(m);
					}
					write(fs, QString("prof_meas_range_%1_%2").arg(i + 1).arg(j + 1).toStdString(), profDatasTmp);

					write(fs, QString("prof_meas_value_%1_%2").arg(i + 1).arg(j + 1).toStdString(), pMeasObj->getValue());
				}
			}
		}
	}

	fs.release();

	

	return true;
}

bool DataCtrl::loadProfDataBase(QString& szFilePath)
{
	clearProfObjs();

	QString path = QApplication::applicationDirPath();
	path += "/3D/data/";

	std::string strResultMatPath = QString(path + "profileData.yml").toStdString();
	if (!szFilePath.isEmpty())
	{
		strResultMatPath = szFilePath.toStdString();
	}

	cv::FileStorage fs(strResultMatPath, cv::FileStorage::READ);
	if (!fs.isOpened())
		return false;	

	cv::FileNode fileNode = fs["obj_index"];
	cv::read(fileNode, m_nProfileIndex, 0);

	int  nObjNum = 0;
	fileNode = fs["obj_num"];
	cv::read(fileNode, nObjNum, 0);

	for (int i = 0; i < nObjNum; i++)
	{
		int nID = 0;
		fileNode = fs[QString("obj_ID_%1").arg(i + 1).toStdString()];
		cv::read(fileNode, nID, 0);	

		cv::String objName = "";
		fileNode = fs[QString("obj_name_%1").arg(i + 1).toStdString()];
		cv::read(fileNode, objName, "");

		QProfileObj *pObj = new QProfileObj(nID, objName.c_str());

		double x1 = 0.0;
		fileNode = fs[QString("pos_x1_%1").arg(i + 1).toStdString()];
		cv::read(fileNode, x1, 0.0);
		double y1 = 0.0;
		fileNode = fs[QString("pos_y1_%1").arg(i + 1).toStdString()];
		cv::read(fileNode, y1, 0.0);
		double x2 = 0.0;
		fileNode = fs[QString("pos_x2_%1").arg(i + 1).toStdString()];
		cv::read(fileNode, x2, 0.0);
		double y2 = 0.0;
		fileNode = fs[QString("pos_y2_%1").arg(i + 1).toStdString()];
		cv::read(fileNode, y2, 0.0);
		pObj->setCutingPosn(x1, y1, x2, y2);

		QString sz3DFilePath = path + "prof/" + QString("prof_3D_%1").arg(i + 1) + ".bmp";
		QFile file3D;
		if (file3D.exists(sz3DFilePath))
		{
			QPixmap img3D(sz3DFilePath);
			if (!img3D.isNull())
			{
				pObj->set3DImage(img3D.toImage());
			}
		}
		
		QString szGrayFielPath = path + "prof/" + QString("prof_gray_%1").arg(i + 1) + ".bmp";
		QFile fileGray;
		if (fileGray.exists(szGrayFielPath))
		{
			cv::Mat matGray = cv::imread(szGrayFielPath.toStdString(), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_COLOR);
			//QPixmap imgGray(path + "prof/" + QString("prof_gray_%1").arg(i + 1) + ".bmp");
			//if (!imgGray.isNull())
			if (!matGray.empty())
			{
				pObj->setGrayImage(matGray);
			}
		}					
		
		std::vector<cv::KeyPoint> profDatasTmp;
		fileNode = fs[QString("prof_datas_%1").arg(i + 1).toStdString()];
		cv::read(fileNode, profDatasTmp, std::vector<cv::KeyPoint>());
		QVector<cv::Point2d> profDatas;
		for (int j = 0; j < profDatasTmp.size(); j++)
		{
			profDatas.push_back(profDatasTmp.at(j).pt);
		}		
		pObj->setProfData(profDatas);

		int nMeasNum = 0;
		fileNode = fs[QString("prof_meas_num_%1").arg(i + 1).toStdString()];
		cv::read(fileNode, nMeasNum, 0);
		for (int j = 0; j < nMeasNum; j++)
		{
			int nMeasType = 0;
			fileNode = fs[QString("prof_meas_type_%1_%2").arg(i + 1).arg(j + 1).toStdString()];
			cv::read(fileNode, nMeasType, 0);

			MeasureType measType = (MeasureType)nMeasType;
			QProfMeasureObj* pMeasObj = NULL;
			switch (measType)
			{
			case MEASURE_TYPE_LNTOLNANGLE:
				pMeasObj = new QProfMeasLnToLnAngle();
				break;
			case MEASURE_TYPE_LNTOLN_DIST:
				pMeasObj = new QProfMeasLnToLnDist();
				break;
			case MEASURE_TYPE_PTTOLN_DIST:
				pMeasObj = new QProfMeasPtToLnDist();
				break;
			case MEASURE_TYPE_PTTOPT_DIST:
				pMeasObj = new QProfMeasPtToPtDist();
				break;
			case MEASURE_TYPE_PT_HEIGHT:
				pMeasObj = new QProfMeasPtHeight();
				break;
			case MEASURE_TYPE_CRTOCR_DIST:
				pMeasObj = new QProfMeasCirToCir();
				break;
			case MEASURE_TYPE_CRTOLN_DIST:
				pMeasObj = new QProfMeasCirToLn();
				break;
			case MEASURE_TYPE_CRTOPT_DIST:
				pMeasObj = new QProfMeasCirToPt();
				break;
			case MEASURE_TYPE_CR_RADIUS:
				pMeasObj = new QProfMeasCirRadius();
				break;
			case MEASURE_TYPE_AREA:
				pMeasObj = new QProfMeasProfArea();
				break;
			case MEASURE_TYPE_PRF_LENGTH:
				pMeasObj = new QProfMeasProfLength();
				break;
			case MEASURE_TYPE_NULL:
			default:
				break;
			}

			if (pMeasObj)
			{
				std::vector<cv::KeyPoint> profDatasTmp;
				fileNode = fs[QString("prof_meas_range_%1_%2").arg(i + 1).arg(j + 1).toStdString()];
				cv::read(fileNode, profDatasTmp, std::vector<cv::KeyPoint>());				
				for (int m = 0; m < profDatasTmp.size(); m++)
				{
					cv::Point ptValue;
					ptValue.x = profDatasTmp.at(m).size;
					ptValue.y = profDatasTmp.at(m).angle;
					pMeasObj->addRange(profDatasTmp.at(m).pt, (MeasureDataType)((int)profDatasTmp.at(m).response), ptValue);

					qDebug() << "Read: " << ptValue.x << " " << ptValue.y << " " << (int)profDatasTmp.at(m).response;
				}				

				double dValue = 0.0;
				fileNode = fs[QString("prof_meas_value_%1_%2").arg(i + 1).arg(j + 1).toStdString()];
				cv::read(fileNode, dValue, 0.0);
				pMeasObj->setValue(dValue);

				pObj->addMeasObj(pMeasObj);
			}
		}

		this->pushProfObj(pObj);
	}

	fs.release();

	return true;
}

void DataCtrl::clearProfDataBase()
{

}

void DataCtrl::clearFiles(const QString &folderFullPath)
{
	QDir dir(folderFullPath);
	dir.setFilter(QDir::Files);
	int fileCount = dir.count();
	for (int i = 0; i < fileCount; i++)
		dir.remove(dir[i]);
}


