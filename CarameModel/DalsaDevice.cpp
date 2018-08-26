#include "DalsaDevice.h"
#include "caramemodel_global.h"
#include "../Common/SystemData.h"
#include "QDebug.h"
#include <QDateTime>
#include <qapplication.h>
#include <qthread.h>
#include <time.h>

typedef Pylon::CBaslerCameraLinkInstantCamera Camera_t;

const QString g_szDefaultDalsaHWConfigFile = "N_AOI_Default_Default_HW_TRIGGER.ccf";
const QString g_szDefaultDalsaSWConfigFile = "N_AOI_Default_Default_SW_TRIGGER.ccf";
const int g_nBufferSize = 54;

int DalsaCameraDevice::_sXferIndex = 0;
DalsaCameraDevice::DalsaCameraDevice(SapLocation* loc, QString cameraName, QString  cameraID)
    :CameraDevice(cameraName, cameraID)
{
    m_loc = loc;

    m_Acq = NULL;
    //m_AcqDevice = NULL;
    m_Buffers = NULL;
    m_Xfer = NULL;
    m_Feature = NULL;
    //m_View = NULL;

    m_nGrabNum = 0;
    m_nGrabCount = 0;

    m_bCapturedImage = false;

    m_hv_AcqHandle = NULL;
    m_camera = NULL;

    m_cameraName = cameraName;
    m_cameraID = cameraID;

    openDevice(cameraName, cameraID);
}

DalsaCameraDevice::~DalsaCameraDevice(void)
{
    closeDevice();
    delete m_loc;

    closeCamera();
}

void DalsaCameraDevice::openDevice(QString cameraName, QString cameraID, bool bHWTrigger)
{    
    try
    {
        QString path = QApplication::applicationDirPath();
        path += "/config/";
        QString fileName = path + (bHWTrigger ? g_szDefaultDalsaHWConfigFile : g_szDefaultDalsaSWConfigFile);

        m_Acq = new SapAcquisition(*m_loc, fileName.toStdString().c_str());
        //m_AcqDevice = new SapAcqDevice(*m_loc, FALSE);
        //m_Feature = new SapFeature(m_loc->GetServerName());
        //int nDlpNum = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
        //int nCaptureNum = DLP_SEQ_PATTERN_IMG_NUM * nDlpNum;
        m_Buffers = new SapBuffer(g_nBufferSize, m_Acq);
        //m_View = new SapView(m_Buffers, SapHwndAutomatic);
        m_Xfer = new SapAcqToBuf(m_Acq, m_Buffers, XferCallback, this);

        // Create resources for all objects  
        BOOL success = m_Acq->Create();
        //success = m_AcqDevice->Create();
        success = m_Buffers->Create();
        //success = m_View->Create();
        success = m_Xfer->Create();
        //success = m_Feature->Create();

        m_bOpen = true;
    }
    catch (...)
    {
        // Error handling.
        qDebug() << "An exception occurred." << endl;
        m_bOpen = false;
    }
}

void DalsaCameraDevice::closeDevice()
{
    if (m_bOpen)
    {
        try
        {    
            // Release resources for all objects  
            BOOL success = m_Xfer->Destroy();
            //success = m_View->Destroy();
            //success = m_AcqDevice->Destroy();
            success = m_Buffers->Destroy();
            success = m_Acq->Destroy();
            //success = m_Feature->Destroy();
        
            // Free all objects  
            delete m_Xfer;
            //delete m_View;
            //delete m_AcqDevice;
            delete m_Buffers;
            delete m_Acq;
            //delete m_Feature;

            m_bOpen = false;
        }
        catch (...)
        {
            // Error handling.
            qDebug() << "An exception occurred." << endl;
        }
    }
}

void DalsaCameraDevice::XferCallback(SapXferCallbackInfo *pInfo)
{
    double dtime_start = double(clock());
    DalsaCameraDevice* pDalsaCam = (DalsaCameraDevice*)(pInfo->GetContext());
    if (pDalsaCam)
    {
        //int pitch = pDalsaCam->m_Buffers->GetPitch();

        // Get the buffer data address
        BYTE pData;
        void* pDataAddr = &pData;
        bool success = pDalsaCam->m_Buffers->GetAddress(pDalsaCam->_sXferIndex, &pDataAddr);

        int width  = pDalsaCam->m_Buffers->GetWidth();
        int height = pDalsaCam->m_Buffers->GetHeight();

        cv::Mat imageNew = cv::Mat::zeros(cv::Size(width, height), CV_8U);
        memcpy(imageNew.data, pDataAddr, width * height);

        pDalsaCam->updateGrabCount(imageNew);

        ++ pDalsaCam->_sXferIndex;
        pDalsaCam->_sXferIndex %= g_nBufferSize;

        success = pDalsaCam->m_Buffers->ReleaseAddress(pDataAddr);

        pDalsaCam->m_bCapturedImage = true;
    }
    double dtime_movePos = double(clock());
    //qDebug() << "Transfer Image Time: " << dtime_movePos - dtime_start << " ms";
}

void DalsaCameraDevice::updateGrabCount(cv::Mat& imgMat)
{
    m_waitMutex.lock();
    m_imageMats.push_back(imgMat);
    m_nGrabCount += 1;
    if (m_nGrabCount >= m_nGrabNum)
    {
        if (m_nGrabNum < 2)
            QThread::msleep(50); // To avoid the capture image done before start to wait.

        m_waitCon.wakeAll();
    }
    m_waitMutex.unlock();
    qDebug() << "Grab Image Count: " << m_nGrabCount;
}

void DalsaCameraDevice::setCamera(IPylonDevice* dev)
{
    m_hv_AcqHandle = dev;
    openCamera();
}

void DalsaCameraDevice::openCamera()
{
    if (m_hv_AcqHandle && m_hv_AcqHandle->IsOpen())
        closeCamera();

    // Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
    // is initialized during the lifetime of this object.
    //Pylon::PylonAutoInitTerm autoInitTerm;
    try
    {
        m_camera = new Camera_t(m_hv_AcqHandle);
        m_camera->Open();
    
        m_camera->UserSetSelector.SetValue(Basler_CLCameraParams::UserSetSelector_UserSet1);
        m_camera->UserSetLoad.Execute();    
    }
    catch (GenICam::GenericException &e)
    {
        // Error handling.
        qDebug() << "An exception occurred." << endl
            << e.GetDescription() << endl;        
    }
}

void DalsaCameraDevice::closeCamera()
{
    if (m_camera)
    {
        try
        {
            m_camera->DetachDevice();
            delete m_camera;
        }
        catch (GenICam::GenericException &e)
        {
            // Error handling.
            qDebug() << "An exception occurred." << endl
                << e.GetDescription() << endl;
        }
    }

    if (m_hv_AcqHandle)
    {
        try
        {        
            m_hv_AcqHandle->Close();
            
        }
        catch (GenICam::GenericException &e)
        {
            // Error handling.
            qDebug() << "An exception occurred." << endl
                << e.GetDescription() << endl;
        }
    }
}

void DalsaCameraDevice::getExposureTime(double *exposureTime)
{
    if (m_bOpen)
    {        
        if (m_camera && m_camera->IsOpen())
        {
            try
            {
                // Only look for cameras supported by Camera_t.
                CDeviceInfo info;
                info.SetDeviceClass(Camera_t::DeviceClass());

                *exposureTime = m_camera->ExposureTimeAbs.GetValue();
            }
            catch (GenICam::GenericException &e)
            {
                // Error handling.
                qDebug() << "An exception occurred." << endl
                    << e.GetDescription() << endl;
            }
        }
    }
    else
    {
        *exposureTime = 0;
    }
}

void DalsaCameraDevice::setExposureTime(double exposureTime)
{
    if (m_bOpen)
    {
        if (m_camera && m_camera->IsOpen())
        {
            try
            {
                // Only look for cameras supported by Camera_t.
                CDeviceInfo info;
                info.SetDeviceClass(Camera_t::DeviceClass());

                m_camera->ExposureTimeAbs.SetValue(exposureTime);
            }
            catch (GenICam::GenericException &e)
            {
                // Error handling.
                qDebug() << "An exception occurred." << endl
                    << e.GetDescription() << endl;
            }
        }
    }
}

bool DalsaCameraDevice::getCameraScreenSize(int& nWidth, int& nHeight)
{
    if (m_bOpen)
    {
        if (m_camera && m_camera->IsOpen())
        {
            try
            {
                // Only look for cameras supported by Camera_t.
                CDeviceInfo info;
                info.SetDeviceClass(Camera_t::DeviceClass());            

                nWidth = m_camera->Width.GetValue();
                nHeight = m_camera->Height.GetValue();
                return true;
            }
            catch (GenICam::GenericException &e)
            {
                // Error handling.
                qDebug() << "An exception occurred." << endl
                    << e.GetDescription() << endl;
            }
        }
    }
    nWidth = 0;
    nHeight = 0;
    return false;
}

void DalsaCameraDevice::setHardwareTrigger(bool bOn)
{
    if (m_bOpen)
    {
        if (m_camera && m_camera->IsOpen())
        {
            //Pylon::PylonAutoInitTerm autoInitTerm;
            try
            {
                // Only look for cameras supported by Camera_t.
                CDeviceInfo info;
                info.SetDeviceClass(Camera_t::DeviceClass());

                m_camera->TriggerSelector.SetValue(Basler_CLCameraParams::TriggerSelector_FrameStart);
                m_camera->TriggerMode.SetValue(bOn ? Basler_CLCameraParams::TriggerMode_On : Basler_CLCameraParams::TriggerMode_Off);
                //camera.AcquisitionMode.SetValue(AcquisitionMode_SingleFrame);
                //m_camera->TriggerSource.SetValue(Basler_CLCameraParams::TriggerSource_CC1);
                //m_camera->TriggerActivation.SetIntValue(Basler_CLCameraParams::TriggerActivation_RisingEdge);
                if (bOn)
                {
                    m_camera->ExposureMode.SetValue(Basler_CLCameraParams::ExposureMode_TriggerWidth);
                }
                else
                {
                    m_camera->ExposureMode.SetValue(Basler_CLCameraParams::ExposureMode_Timed);
                    setExposureTime(10000);
                }
            }
            catch (GenICam::GenericException &e)
            {
                // Error handling.
                qDebug() << "An exception occurred." << endl
                    << e.GetDescription() << endl;
            }
        }

        if (bOn)
        {
            closeDevice();
            QThread::msleep(200);
            openDevice(m_cameraName, m_cameraID, bOn);
        }
        else
        {
            m_Acq->SetParameter(CORACQ_PRM_EXT_TRIGGER_ENABLE, bOn ? TRUE : FALSE);
        }        

        //closeDevice();
        //QThread::msleep(200);
        //openDevice(m_cameraName, m_cameraID, bOn);            
    }
}

void DalsaCameraDevice::setTriggerActive(bool bActiveHigh)
{
    if (m_bOpen)
    {    
        m_Acq->SetParameter(CORACQ_PRM_EXT_TRIGGER_DETECTION, bActiveHigh ? CORACQ_VAL_RISING_EDGE : CORACQ_VAL_FALLING_EDGE);        
    }
}

bool DalsaCameraDevice::captureImage(cv::Mat &imageMat)
{
    if (!m_bOpen) return false;

    m_waitMutex.lock();
    m_imageMats.clear();
    m_nGrabNum = 1;
    m_nGrabCount = 0;
    m_waitMutex.unlock();

    m_bCapturedImage = false;
    BOOL success = m_Xfer->Snap();    
    
    if (m_camera && (m_camera->TriggerMode.GetValue() == Basler_CLCameraParams::TriggerMode_On))
    {
        m_Acq->SoftwareTrigger(SapAcquisition::SoftwareTriggerExtFrame);
    }

    if (m_Xfer->Wait(5000))
    {
        int nWaitTime = 50;
        while ((m_Xfer->IsGrabbing() || !m_bCapturedImage) && nWaitTime-- > 0)
        {
            ::Sleep(100);
        }

        if (m_imageMats.size() > 0)
        {
            imageMat = m_imageMats[0];
            return true;
        }
        else
        {
            return false;
        }
    }

    return false;    
}

void DalsaCameraDevice::softwareTrigger()
{
    if (!m_bOpen) return;

    m_Acq->SoftwareTrigger(SapAcquisition::SoftwareTriggerExtFrame);
}

bool DalsaCameraDevice::startGrabing(int nNum)
{
    if (!m_bOpen) return false;
    
    m_bCapturedImage = false;

    m_waitMutex.lock();
    m_imageMats.clear();
    m_nGrabNum = nNum;
    m_nGrabCount = 0;
    m_waitMutex.unlock();    

    BOOL success = m_Xfer->Grab();
    if (success)
    {
        m_bStopFlag = false;
    }
    return success;
}

bool DalsaCameraDevice::captureImageByFrameTrig(QVector<cv::Mat>& imageMats)
{
    if (!m_bOpen) return false;

    m_waitMutex.lock();
    bool bWaitDone = m_waitCon.wait(&m_waitMutex);
    if (!bWaitDone)
    {
        m_waitMutex.unlock();
        return false;
    }
    m_waitMutex.unlock();

    bool bCaptureImageAsMatlab = System->getParam("camera_cap_image_matlab").toBool();
    int nDlpNum = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
    bool bTriggerBoard = System->isTriggerBoard();

    int nTotalImageNum = m_imageMats.size();

    imageMats.clear();
    for (int i = 0; i < nTotalImageNum; ++ i)
    {
        int nIndex = i;
        if (bCaptureImageAsMatlab && nTotalImageNum >= DLP_SEQ_PATTERN_IMG_NUM)
        {
            if (bTriggerBoard)
            {
                if (nTotalImageNum <= DLP_SEQ_PATTERN_IMG_NUM * nDlpNum)
                {
                    int nImgIndex = nIndex % DLP_SEQ_PATTERN_IMG_NUM;
                    if (5 == nImgIndex)// 5 Pattern Sequence Special Index
                    {
                        nIndex += 1;
                    }
                    else if (6 == nImgIndex)// 6 Pattern Sequence Special Index
                    {
                        nIndex -= 1;
                    }
                }
                else // 48 + 6 pics
                {
                    int nLightImgNum = nTotalImageNum - DLP_SEQ_PATTERN_IMG_NUM * nDlpNum;
                    if (nIndex < DLP_SEQ_PATTERN_IMG_NUM * nDlpNum)
                    {
                        int nImgIndex = nIndex % DLP_SEQ_PATTERN_IMG_NUM;
                        if (5 == nImgIndex)// 5 Pattern Sequence Special Index
                        {
                            nIndex += 1;
                        }
                        else if (6 == nImgIndex)// 6 Pattern Sequence Special Index
                        {
                            nIndex -= 1;
                        }
                    }
                }
            }
            else
            {
                if (nIndex <= DLP_SEQ_PATTERN_IMG_NUM * nDlpNum)
                {
                    int nImgIndex = nIndex % DLP_SEQ_PATTERN_IMG_NUM;
                    if (5 == nImgIndex)// 5 Pattern Sequence Special Index
                    {
                        nIndex += 1;
                    }
                    else if (6 == nImgIndex)// 6 Pattern Sequence Special Index
                    {
                        nIndex -= 1;
                    }
                }
            }            
        }

        if (bTriggerBoard && (nTotalImageNum > DLP_SEQ_PATTERN_IMG_NUM * nDlpNum))
        {
            int nLightImgNum = nTotalImageNum - DLP_SEQ_PATTERN_IMG_NUM * nDlpNum;
            if (nIndex < DLP_SEQ_PATTERN_IMG_NUM * nDlpNum)
            {
                imageMats.push_back(m_imageMats[nIndex + nLightImgNum]);
            }
            else
            {
                imageMats.push_back(m_imageMats[nIndex - DLP_SEQ_PATTERN_IMG_NUM * nDlpNum]);
            }
        }
        else
        {
            imageMats.push_back(m_imageMats[nIndex]);
        }        
    }

    return m_bStopFlag ? false : true;
}

void DalsaCameraDevice::stopGrabing()
{
    if (!m_bOpen) return;

    BOOL success = m_Xfer->Freeze();    
    success = m_Xfer->Wait(1000);

    m_waitMutex.lock();
    m_bStopFlag = true;
    m_waitCon.wakeAll();
    m_waitMutex.unlock();
}

void DalsaCameraDevice::clearGrabing()
{
    m_waitMutex.lock();
    m_imageMats.clear();
    m_nGrabNum = 0;
    m_nGrabCount = 0;
    m_waitMutex.unlock();

    m_Buffers->Clear();
    m_Buffers->ResetIndex();
    _sXferIndex = 0;
}

bool DalsaCameraDevice::isGrabing()
{
    if (!m_bOpen) return false;

    return m_Xfer->IsGrabbing() && !m_bStopFlag;
}