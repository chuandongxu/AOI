#include "CameraDevice.h"
#include "../Common/SystemData.h"
#include "QDebug.h"
#include <QDateTime>

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "CryptLib.h"
#include <QFile>
#include <time.h>

typedef Pylon::CBaslerGigEInstantCamera Camera_t;

const int g_ImageBufferSize = 10;
const int g_ImageFrameTriggerNum = 1000;

CameraDevice::CameraDevice(QString cameraName, QString  cameraID)
{	
	m_bOpen  = false;	
	m_bSaveAsEncrypt = false;
	m_bStopFlag = true;
	//openDevice(cameraName, cameraID);

	QString user;
	int level = 0;
	System->getUser(user, level);
	if (USER_LEVEL_MANAGER > level)
	{
		m_bSaveAsEncrypt = true;
	}
}

CameraDevice::~CameraDevice(void)
{
	//closeDevice();
}

void CameraDevice::saveImage(cv::Mat& image, QString name, QString path)
{
	QString capturePath = System->getParam("camera_cap_image_path").toString();
	
	QString fileName = QString("");
	if (!name.isEmpty() && name.contains(".bmp"))
	{
		fileName = name;		
	}
	else
	{
		QDateTime dtm = QDateTime::currentDateTime();
		fileName = dtm.toString("MMddhhmmssms") + QStringLiteral(".bmp");
	}

	QString strSave = path + fileName;

	if (path.isEmpty())
	{
		strSave = capturePath + "/" + fileName;
	}

	if (!strSave.isEmpty())
	{
		IplImage frameImg = IplImage(image);
		cvSaveImage(strSave.toStdString().c_str(), &frameImg);	

		if (m_bSaveAsEncrypt)
		{
			QString nameEncrypt = strSave.left(strSave.indexOf('.')) + ".ent";
			AOI::Crypt::EncryptFileNfg(strSave.toStdString(), nameEncrypt.toStdString());

			QFile::remove(strSave);
		}
	}
}

bool CameraDevice::getStatus()
{
	return m_bOpen;
}

BaslerCameraDevice::BaslerCameraDevice(IPylonDevice* dev, QString cameraName, QString  cameraID)
	:CameraDevice(cameraName, cameraID)
{
	m_hv_AcqHandle = dev;
	openDevice(cameraName, cameraID);
}

BaslerCameraDevice::~BaslerCameraDevice(void)
{
	closeDevice();
	delete m_camera;
	delete m_hv_AcqHandle;
}

void BaslerCameraDevice::openDevice(QString cameraName, QString cameraID, bool bHWTrigger)
{
	if (m_hv_AcqHandle && m_hv_AcqHandle->IsOpen())
		closeDevice();

	// Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
	// is initialized during the lifetime of this object.
	//Pylon::PylonAutoInitTerm autoInitTerm;
	try
	{
		m_camera = new Camera_t(m_hv_AcqHandle);

		m_camera->Open();

		m_camera->UserSetSelector.SetValue(Basler_GigECamera::UserSetSelector_UserSet1);
		m_camera->UserSetLoad.Execute();

		m_bOpen = true;

		setHardwareTrigger(bHWTrigger);	
	}
	catch (GenICam::GenericException &e)
	{
		// Error handling.
		qDebug() << "An exception occurred." << endl
			<< e.GetDescription() << endl;

		m_bOpen = false;
	}
}

void BaslerCameraDevice::closeDevice()
{
	if (m_hv_AcqHandle && m_bOpen)
	{
		try
		{
			m_camera->DetachDevice();
			m_hv_AcqHandle->Close();

			m_bOpen = false;
		}
		catch (GenICam::GenericException &e)
		{
			// Error handling.
			qDebug() << "An exception occurred." << endl
				<< e.GetDescription() << endl;
		}
	}
}


void BaslerCameraDevice::getExposureTime(double *exposureTime)
{
	if (m_bOpen)
	{
		//Pylon::PylonAutoInitTerm autoInitTerm;
		try
		{
			// Only look for cameras supported by Camera_t.
			CDeviceInfo info;
			info.SetDeviceClass(Camera_t::DeviceClass());

			// Create an instant camera object with the first found camera device that matches the specified device class.
			Camera_t camera(m_hv_AcqHandle);

			*exposureTime = camera.ExposureTimeAbs.GetValue();
			m_hv_expouserTime = *exposureTime;
		}
		catch (GenICam::GenericException &e)
		{
			// Error handling.
			qDebug() << "An exception occurred." << endl
				<< e.GetDescription() << endl;
		}
	}
	else
	{
		*exposureTime = 0;
	}
}

void BaslerCameraDevice::setExposureTime(double exposureTime)
{
	if (m_bOpen)
	{
		//Pylon::PylonAutoInitTerm autoInitTerm;
		try
		{
			// Only look for cameras supported by Camera_t.
			CDeviceInfo info;
			info.SetDeviceClass(Camera_t::DeviceClass());

			// Create an instant camera object with the first found camera device that matches the specified device class.
			Camera_t camera(m_hv_AcqHandle);

			camera.ExposureTimeAbs.SetValue(exposureTime);
			m_hv_expouserTime = exposureTime;
		}
		catch (GenICam::GenericException &e)
		{
			// Error handling.
			qDebug() << "An exception occurred." << endl
				<< e.GetDescription() << endl;
		}
	}
}

bool BaslerCameraDevice::getCameraScreenSize(int& nWidth, int& nHeight)
{
	if (m_bOpen)
	{
		//Pylon::PylonAutoInitTerm autoInitTerm;
		try
		{
			// Only look for cameras supported by Camera_t.
			CDeviceInfo info;
			info.SetDeviceClass(Camera_t::DeviceClass());

			// Create an instant camera object with the first found camera device that matches the specified device class.
			Camera_t camera(m_hv_AcqHandle);

			nWidth  = camera.SensorWidth.GetValue();
			nHeight = camera.SensorHeight.GetValue();

			return true;
		}
		catch (GenICam::GenericException &e)
		{
			// Error handling.
			qDebug() << "An exception occurred." << endl
				<< e.GetDescription() << endl;
		}
	}

	nWidth = 0;
	nHeight = 0;
	return false;
}

void BaslerCameraDevice::setHardwareTrigger(bool bOn)
{
	if (m_bOpen)
	{
		//Pylon::PylonAutoInitTerm autoInitTerm;
		try
		{
			// Only look for cameras supported by Camera_t.
			CDeviceInfo info;
			info.SetDeviceClass(Camera_t::DeviceClass());

			m_camera->TriggerSelector.SetValue(TriggerSelector_AcquisitionStart);
			m_camera->TriggerMode.SetValue(TriggerMode_Off);
			m_camera->AcquisitionFrameRateEnable.SetValue(false);

			m_camera->TriggerSelector.SetValue(TriggerSelector_FrameStart);
			m_camera->TriggerMode.SetValue(bOn ? TriggerMode_On : TriggerMode_Off);
			//camera.AcquisitionMode.SetValue(AcquisitionMode_SingleFrame);
			m_camera->TriggerSource.SetValue(TriggerSource_Line1);
			m_camera->TriggerActivation.SetIntValue(TriggerActivation_RisingEdge);

			//double TiggerDelay_us = 0;
			//m_camera->TriggerDelayAbs.SetValue(TiggerDelay_us);
			//m_camera->ExposureMode.SetValue(ExposureMode_Timed);
			//m_camera->ExposureTimeAbs.SetValue(35000);

			m_camera->AcquisitionStatusSelector.SetValue(AcquisitionStatusSelector_FrameTriggerWait);

			bool IsWaitingForFrameTrigger = m_camera->AcquisitionStatus.GetValue();
		}
		catch (GenICam::GenericException &e)
		{
			// Error handling.
			qDebug() << "An exception occurred." << endl
				<< e.GetDescription() << endl;
		}
	}
}

bool BaslerCameraDevice::captureImage(cv::Mat &imageMat)
{
	if (!m_hv_AcqHandle || !m_bOpen) return false;

	// Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
	// is initialized during the lifetime of this object.
	//Pylon::PylonAutoInitTerm autoInitTerm;
	try
	{
		if (!m_camera->IsPylonDeviceAttached()) return false;

		// The parameter MaxNumBuffer can be used to control the count of buffers
		// allocated for grabbing. The default value of this parameter is 10.
		m_camera->MaxNumBuffer = 5;

		// This smart pointer will receive the grab result data.
		CGrabResultPtr ptrGrabResult;

		// Camera.StopGrabbing() is called automatically by the RetrieveResult() method
		// when c_countOfImagesToGrab images have been retrieved.
		//while (m_camera->IsGrabbing())
		if (m_camera->GrabOne(5000, ptrGrabResult, TimeoutHandling_ThrowException))
		{
			// Wait for an image and then retrieve it. A timeout of 5000 ms is used.
			//m_camera->RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);

			// Image grabbed successfully?
			if (ptrGrabResult->GrabSucceeded())
			{
				// Access the image data.
				qDebug() << "SizeX: " << ptrGrabResult->GetWidth() << endl;
				qDebug() << "SizeY: " << ptrGrabResult->GetHeight() << endl;

#ifdef PYLON_WIN_BUILD
				// Display the grabbed image.
				//Pylon::DisplayImage(1, ptrGrabResult);
#endif			

				uint32_t width = ptrGrabResult->GetWidth();
				uint32_t height = ptrGrabResult->GetHeight();


				CImageFormatConverter fc;
				fc.OutputPixelFormat = PixelType_BGR8packed;
				fc.OutputBitAlignment = OutputBitAlignment_MsbAligned;
				CPylonImage image;
				fc.Convert(image, ptrGrabResult);
				//imageMat = cv::Mat(height, width, CV_8UC3, image.GetBuffer(), Mat::AUTO_STEP).clone();

				cv::Mat imageNew(height, width, CV_8UC3);
				memcpy(imageNew.ptr(), image.GetBuffer(), 3 * width*height);

				imageMat = imageNew;

				bool bCaptureImage = System->getParam("camera_cap_image_enable").toBool();
				if (bCaptureImage)
				{
					saveImage(imageMat);
				}
			}
			else
			{
				qDebug() << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
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

	return true;
}

void BaslerCameraDevice::softwareTrigger()
{
	if (!m_bOpen) return;	

	m_camera->ExecuteSoftwareTrigger();
}

bool BaslerCameraDevice::startGrabing(int nNum)
{
	if (!m_hv_AcqHandle || !m_bOpen) return false;

	try
	{
		if (!m_camera->IsPylonDeviceAttached()) return false;

		// The parameter MaxNumBuffer can be used to control the count of buffers
		// allocated for grabbing. The default value of this parameter is 10.
		m_camera->MaxNumBuffer = nNum;

		// Start the grabbing of c_countOfImagesToGrab images.
		// The camera device is parameterized with a default configuration which
		// sets up free-running continuous acquisition.
		m_camera->StartGrabbing(nNum);

		m_bStopFlag = false;
	}
	catch (GenICam::GenericException &e)
	{
		// Error handling.
		qDebug() << "An exception occurred." << endl
			<< e.GetDescription() << endl;

		return false;
	}

	return true;
}

bool BaslerCameraDevice::captureImageByFrameTrig(QVector<cv::Mat>& imageMats)
{
	if (!m_hv_AcqHandle || !m_bOpen) return false;

	try
	{
		if (!m_camera->IsPylonDeviceAttached()) return false;

		// This smart pointer will receive the grab result data.
		CGrabResultPtr ptrGrabResult;

		// Camera.StopGrabbing() is called automatically by the RetrieveResult() method
		while (m_camera->IsGrabbing() && !m_bStopFlag)
		{			
			try
			{
				// Wait for an image and then retrieve it. A timeout of 5000 ms is used.
				if (!m_camera->RetrieveResult(60 * 1000, ptrGrabResult, TimeoutHandling_Return))
				{
					qDebug() << "RetrieveResult exception occurred. Return Directly!" << endl;
					return false;
				}
			}
			catch (GenICam::GenericException &e)
			{
				// Error handling.
				qDebug() << "RetrieveResult exception occurred." << endl
					<< e.GetDescription() << endl;
				continue;
			}

			// Image grabbed successfully?
			if (ptrGrabResult->GrabSucceeded())
			{
				// Access the image data.
				//qDebug() << "SizeX: " << ptrGrabResult->GetWidth() << endl;
				//qDebug() << "SizeY: " << ptrGrabResult->GetHeight() << endl;

				uint32_t width = ptrGrabResult->GetWidth();
				uint32_t height = ptrGrabResult->GetHeight();

				CImageFormatConverter fc;
				fc.OutputPixelFormat = PixelType_BGR8packed;
				fc.OutputBitAlignment = OutputBitAlignment_MsbAligned;
				CPylonImage image;
				fc.Convert(image, ptrGrabResult);

				cv::Mat imageNew(height, width, CV_8UC3);
				memcpy(imageNew.ptr(), image.GetBuffer(), 3 * width*height);

				image.Release();

				imageMats.push_back(imageNew);
			}
			else
			{
				qDebug() << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
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

	return true;
}

void BaslerCameraDevice::stopGrabing()
{
	if (!m_hv_AcqHandle || !m_bOpen) return;

	try
	{
		if (!m_camera->IsPylonDeviceAttached()) return;

		m_camera->StopGrabbing();

		m_bStopFlag = true;
	}
	catch (GenICam::GenericException &e)
	{
		// Error handling.
		qDebug() << "An exception occurred." << endl
			<< e.GetDescription() << endl;
	}
}

bool BaslerCameraDevice::isGrabing()
{
	if (!m_bOpen) return false;

	return m_camera->IsGrabbing() && !m_bStopFlag;
}