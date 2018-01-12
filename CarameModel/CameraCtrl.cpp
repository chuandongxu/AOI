#include "CameraCtrl.h"
#include <QMessageBox>
#include "../Common/SystemData.h"
#include "QDebug.h"
#include "DalsaDevice.h"
#include "sapclassbasic.h"
#include "ImageDevice.h"

// Limits the amount of cameras used for grabbing.
// It is important to manage the available bandwidth when grabbing with multiple cameras.
// This applies, for instance, if two GigE cameras are connected to the same network adapter via a switch.
// To manage the bandwidth, the GevSCPD interpacket delay parameter and the GevSCFTD transmission delay
// parameter can be set for each GigE camera device.
// The "Controlling Packet Transmission Timing with the Interpacket and Frame Transmission Delays on Basler GigE Vision Cameras"
// Application Notes (AW000649xx000)
// provide more information about this topic.
// The bandwidth used by a FireWire camera device can be limited by adjusting the packet size.
static const size_t c_maxCamerasToUse = 4;

CameraCtrl::CameraCtrl(void)
{	
	//Pylon::PylonAutoInitTerm autoInitTerm;
	PylonInitialize();
}


CameraCtrl::~CameraCtrl(void)
{
	PylonTerminate();
	unInit();
}

bool CameraCtrl::isBasler()
{
	int nCaptureMode = System->getParam("camera_capture_mode").toInt();

	return 0 == nCaptureMode;
}

bool CameraCtrl::isDalsa()
{
	int nCaptureMode = System->getParam("camera_capture_mode").toInt();

	return 1 == nCaptureMode;
}

bool CameraCtrl::isImage()
{
	int nCaptureMode = System->getParam("camera_capture_mode").toInt();

	return 2 == nCaptureMode;
}


bool CameraCtrl::initial()
{
	// 相机设备模块初始化
	if ( !m_CameraList.isEmpty() )
	{
		for ( int nC = 0 ; nC < m_CameraList.length(); nC++)
		{
			m_CameraList[nC]->closeDevice();
		}
		m_CameraList.clear();
	}

	if (isBasler())
	{
		// Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
		// is initialized during the lifetime of this object.	
		try
		{
			// Get the transport layer factory.
			CTlFactory& tlFactory = CTlFactory::GetInstance();

			// Get all attached devices and exit application if no device is found.
			DeviceInfoList_t devices;
			if (tlFactory.EnumerateDevices(devices) == 0)
			{
				throw RUNTIME_EXCEPTION("No camera present.");
			}

			// Create an array of instant cameras for the found devices and avoid exceeding a maximum number of devices.
			CInstantCameraArray cameras(min(devices.size(), c_maxCamerasToUse));

			// Create and attach all Pylon Devices.
			for (size_t i = 0; i < cameras.GetSize(); ++i)
			{
				IPylonDevice* dev = tlFactory.CreateDevice(devices[i]);
				if (NULL != dev)
				{
					cameras[i].Attach(dev);

					// Print the model name of the camera.
					qDebug() << "Using device " << cameras[i].GetDeviceInfo().GetModelName() << endl;

					// The parameter MaxNumBuffer can be used to control the count of buffers
					// allocated for grabbing. The default value of this parameter is 10.
					cameras[i].MaxNumBuffer = 5;

					QString szModuleName = cameras[i].GetDeviceInfo().GetModelName();
					cameras[i].DetachDevice();
					m_CameraList.push_back(new BaslerCameraDevice(dev, szModuleName, QString("%1").arg(i+1)));
				}
			}

		}
		catch (GenICam::GenericException &e)
		{
			// Error handling.
			qDebug() << "An exception occurred." << endl
				<< e.GetDescription() << endl;

			return false;
		}
	}
	else if (isDalsa())
	{
		try
		{
			// Get total number of boards in the system
			int serverCount = SapManager::GetServerCount();
			if (serverCount == 0)
			{
				qDebug() << ("No device found!\n");
				return FALSE;
			}

			int nDeviceCount = 0;
			for (int serverIndex = 0; serverIndex < serverCount; serverIndex++)
			{
				if (SapManager::GetResourceCount(serverIndex, SapManager::ResourceAcq) != 0)
				{
					char serverName[CORSERVER_MAX_STRLEN];
					SapManager::GetServerName(serverIndex, serverName, sizeof(serverName));
					qDebug() << ("Dalsa Server Index %d: %s\n", serverIndex, serverName);

					// Scan all the acquisition devices on that server and show menu to user
					int deviceCount = SapManager::GetResourceCount(serverName, SapManager::ResourceAcq);

					for (int deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++)
					{
						char deviceName[CORPRM_GETSIZE(CORACQ_PRM_LABEL)];
						SapManager::GetResourceName(serverName, SapManager::ResourceAcq, deviceIndex, deviceName, sizeof(deviceName));
						qDebug() << ("Dalsa Device Index %d: %s\n", deviceIndex + 1, deviceName);

						if (!QString(deviceName).contains("not available"))
						{
							SapLocation* loc = new SapLocation(serverName, deviceIndex);
							if (loc->IsUnknown() || loc->IsNull())
							{
								delete loc;
							}
							else
							{
								m_CameraList.push_back(new DalsaCameraDevice(loc, deviceName, QString("%1").arg(++nDeviceCount)));
							}
						}
					}
				}
			}
		}
		catch (...)
		{
			qDebug() << "An exception occurred." << endl;
			return false;
		}

		// Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
		// is initialized during the lifetime of this object.	
		try
		{
			// Get the transport layer factory.
			CTlFactory& tlFactory = CTlFactory::GetInstance();

			// Get all attached devices and exit application if no device is found.
			DeviceInfoList_t devices;
			if (tlFactory.EnumerateDevices(devices) == 0)
			{
				throw RUNTIME_EXCEPTION("No camera present.");
			}

			// Create an array of instant cameras for the found devices and avoid exceeding a maximum number of devices.
			CInstantCameraArray cameras(min(devices.size(), c_maxCamerasToUse));

			// Create and attach all Pylon Devices.
			if (m_CameraList.size() == cameras.GetSize())
			{
				for (size_t i = 0; i < cameras.GetSize(); ++i)
				{
					IPylonDevice* dev = tlFactory.CreateDevice(devices[i]);
					if (NULL != dev)
					{
						cameras[i].Attach(dev);
						// Print the model name of the camera.
						qDebug() << "Using device " << cameras[i].GetDeviceInfo().GetModelName() << endl;
						// The parameter MaxNumBuffer can be used to control the count of buffers
						// allocated for grabbing. The default value of this parameter is 10.
						//cameras[i].MaxNumBuffer = 5;
						QString szModuleName = cameras[i].GetDeviceInfo().GetModelName();
						cameras[i].DetachDevice();

						((DalsaCameraDevice*)(m_CameraList[i]))->setCamera(dev);

						//bool bHardwareTrigger = System->getParam("camera_hw_tri_enable").toBool();
						//if (!bHardwareTrigger)
						//{
						//	((DalsaCameraDevice*)(m_CameraList[i]))->setHardwareTrigger(bHardwareTrigger);
						//}
					}
				}
			}
			else
			{
				qDebug() << "Acq and Camera_t Number is wrong!";
			}

		}
		catch (GenICam::GenericException &e)
		{
			// Error handling.
			qDebug() << "An exception occurred." << endl
				<< e.GetDescription() << endl;

			return false;
		}
	}
	else if (isImage())
	{
		m_CameraList.push_back(new ImageDevice("Image Device", QString("%1").arg(1)));
	}

	return true;
}

void CameraCtrl::unInit()
{
	for(int i = 0; i<m_CameraList.size(); i++)
	{
		m_CameraList[i]->closeDevice();
		delete m_CameraList[i];
	}
	m_CameraList.clear();
}

int  CameraCtrl::getCameraCount()
{
	return m_CameraList.length();
}

CameraDevice * CameraCtrl::getCamera( int n)
{
	if ( n < m_CameraList.length() )
	{
		return m_CameraList[n];
	}
	
	return NULL;
}