#include "cameramodule.h"
#include "CameraCtrl.h"
#include "viewctrl.h"
#include "../Common/SystemData.h"
#include "AttchWidget.h"
#include "qlist.h"
#include "CameraSetting.h"

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// camera control number
const int CAMERA_CTR_NUM = 1;

CameraModule::CameraModule(int id, const QString &name)
	:QModuleBase(id, name)
{
	initial();
}

void CameraModule::setErrorMap()
{
	System->addErrorMap(ERROR_GRABIMAGE, MSG_ERROR_GRABIMAGE);
}


CameraModule::~CameraModule()
{
	unInit();
}

void CameraModule::initial(int nWindow)
{
	// 相机设备初始化
	m_pCameraCtrl = NULL;
	m_pCameraCtrl = new CameraCtrl();
	m_pCameraCtrl->initial();

	m_pMainView = NULL;
	m_pMainView = new QMainView(m_pCameraCtrl);

	// 窗口
	int nWid = CAMERA_CTR_NUM;
	for (int nTmp = 0; nTmp < nWid; nTmp++)
	{
		m_pListViewCtrl.append(new ViewCtrl());
		m_pListViewCtrl[nTmp]->initial();
		m_pListViewCtrl[nTmp]->connectCameraCtrl(m_pCameraCtrl);
	}

	// 错误码
	setErrorMap();

	System->setParam("camera_hw_tri_enable", true);
}

void CameraModule::unInit()
{
	//  Release all by system already
	for (int nTmp = 1; nTmp < m_pListViewCtrl.size(); nTmp++)
	{
		delete m_pListViewCtrl[nTmp];
	}
	m_pListViewCtrl.clear();

	if (m_pCameraCtrl)
	{
		m_pCameraCtrl->unInit();
		delete m_pCameraCtrl;
		m_pCameraCtrl = NULL;
	}

	/*if (m_pMainView)
	{
		delete m_pMainView;
		m_pMainView = NULL;
	}*/
}

void CameraModule::openCamera()
{
	return;
}

void CameraModule::closeCamera()
{
	return;
}

int CameraModule::getCameraNum()
{
	if (m_pCameraCtrl)
	{
		return m_pCameraCtrl->getCameraCount();
	}

	return 0;
}

bool CameraModule::startUpCapture()
{
	for (int i = 0; i < m_pListViewCtrl.size(); i++)
	{
		ViewCtrl* pViewCtl = getViewWindow(i);
		if (pViewCtl)
		{
			pViewCtl->endUpCapture();
		}
	}
	
	if (m_pMainView)
	{
		return m_pMainView->startUpCapture();
	}

	return false;
}

bool CameraModule::endUpCapture()
{	
	if (m_pMainView)
	{
		return m_pMainView->endUpCapture();
	}
	return false;
}

const QVector<cv::Mat>& CameraModule::getImageBuffer()
{
	return m_pMainView->getImageBuffer();
}

const cv::Mat& CameraModule::getImageItemBuffer(int nIndex)
{
	return m_pMainView->getImageItemBuffer(nIndex);
}

int CameraModule::getImageBufferNum()
{	
	return m_pMainView->getImageBufferNum();
}

int CameraModule::getImageBufferCaptureNum()
{
	return m_pMainView->getImageBufferCaptureNum();
}

bool CameraModule::startCapturing()
{
	return m_pMainView->startCapturing();
}

void CameraModule::clearImageBuffer()
{
	m_pMainView->clearImageBuffer();
}

bool CameraModule::isCaptureImageBufferDone()
{
	return m_pMainView->isCaptureImageBufferDone();
}

bool CameraModule::lockCameraCapture(int iStation)
{
	return m_pMainView->lockCameraCapture(iStation);
}

void CameraModule::unlockCameraCapture()
{
	m_pMainView->unlockCameraCapture();
}

bool CameraModule::isCameraCaptureAvaiable()
{
	return m_pMainView->isCameraCaptureAvaiable();
}

void CameraModule::addImageText(QString& szText)
{
	m_pMainView->addImageText(szText);
}

void CameraModule::clearImage()
{
	m_pMainView->clearImage();
}

void CameraModule::setImage(cv::Mat& image, bool bDisplay)
{
	m_pMainView->setImage(image, bDisplay);
}

void CameraModule::setHeightData(cv::Mat& matHeight)
{
	m_pMainView->setHeightData(matHeight);
}

bool CameraModule::grabCamImage(int nCamera, void *lpImage, bool bSync)
{
	/****/
	if (!lpImage)
	{
		return false;
	}

	((Mat*)lpImage)->release();
	// GenEmptyObj( (HObject*)lpImage );
	CameraCtrl * ctrlTmp = (CameraCtrl*)m_pCameraCtrl;
	if (ctrlTmp)
	{
		CameraDevice * tmpDevice = ctrlTmp->getCamera(nCamera);
		if (tmpDevice)
		{
			return tmpDevice->captureImage(*(Mat*)lpImage);
		}
	}

	return false;

	/****/
}

void CameraModule::setExposureTime(int nCamera, double exposureTime)
{
	/*****/
	CameraCtrl * ctrlTmp = (CameraCtrl*)m_pCameraCtrl;
	if (ctrlTmp)
	{
		CameraDevice * tmpDevice = ctrlTmp->getCamera(nCamera);
		if (tmpDevice)
		{
			tmpDevice->setExposureTime(exposureTime);
		}
	}

	/*****/

}

void CameraModule::addSettingWiddget(QTabWidget *tabWidget)
{
	if (tabWidget)
	{
		tabWidget->addTab(new CameraSetting(m_pCameraCtrl, getViewWindow(0)), QStringLiteral("相机设定"));
	}

	QString user;
	int level = 0;
	System->getUser(user, level);
	if (USER_LEVEL_MANAGER > level)
	{
		//tabWidget->setEnabled(false);
	}
		
}

void CameraModule::getExistImage(int nWindow, int nCamera, void *lpImage, int indPic)
{
	if (nWindow < m_pListViewCtrl.count())
	{
		if (!lpImage)
		{
			return;
		}

		Mat * pHo_tmp = (Mat *)lpImage;
		pHo_tmp->release();

		*pHo_tmp = m_pListViewCtrl[nWindow]->getImage(indPic);
	}
}


void CameraModule::getNumImage(int nCamera, int &nPic) // 获取图像个数
{

}


void CameraModule::addHObject(int nWindowCtrl, void *pHObject, bool bImage)  // 添加图像数据到 图像窗口控制, 这样添加的图像数据可以进行缩放等其他操作
{
	/*****/

	if (nWindowCtrl >= m_pListViewCtrl.count())
	{
		return;
	}

	HWndCtrl *pHwndCtrlTmp = m_pListViewCtrl[nWindowCtrl]->getHwndCtrl();
	if (pHwndCtrlTmp)
	{
		Mat *pHObjectTmp = (Mat *)pHObject;
		if (pHObjectTmp)
		{
			pHwndCtrlTmp->addObj(*pHObjectTmp, bImage);
		}

	}
	/*****/

}
void CameraModule::dispObject(int nWindowCtrl, void *pHObject)
{
	if (nWindowCtrl >= m_pListViewCtrl.count())
	{
		return;
	}

	HWndCtrl *pHwndCtrlTmp = m_pListViewCtrl[nWindowCtrl]->getHwndCtrl();
	if (pHwndCtrlTmp)
	{
		Mat *pHObjectTmp = (Mat *)pHObject;
		if (pHObjectTmp)
		{
			cvtColor(*pHObjectTmp, *pHObjectTmp, CV_BGR2RGB);
			if (pHObjectTmp->type() == CV_8UC3)
			{
				cvtColor(*pHObjectTmp, *pHObjectTmp, CV_BGR2RGB);
			}
			else if (pHObjectTmp->type() == CV_8UC1)
			{
				cvtColor(*pHObjectTmp, *pHObjectTmp, CV_GRAY2RGB);	
			}
			QImage image = QImage((uchar*)(*pHObjectTmp).data, (*pHObjectTmp).cols, (*pHObjectTmp).rows, ToInt((*pHObjectTmp).step), QImage::Format_RGB888);

			pHwndCtrlTmp->getHWindowHandle()->setPixmap(QPixmap::fromImage(image));
		}
	}
}

void CameraModule::clearHObjects(int nWindowCtrl)
{
	/*****/
	HWndCtrl *pHwndCtrlTmp = m_pListViewCtrl[nWindowCtrl]->getHwndCtrl();
	if (pHwndCtrlTmp)
	{
		pHwndCtrlTmp->clearList();
	}
	/*****/
}
void CameraModule::repaintHWindow(int nWindowCtrl)
{
	HWndCtrl *pHwndCtrlTmp = m_pListViewCtrl[nWindowCtrl]->getHwndCtrl();
	if (pHwndCtrlTmp)
	{
		pHwndCtrlTmp->repaint();
	}
}

void CameraModule::changeGraphicSettings(int nWindowCtrl, QString mode, QString val)
{
	/****/
	HWndCtrl *pHwndCtrlTmp = m_pListViewCtrl[nWindowCtrl]->getHwndCtrl();
	if (pHwndCtrlTmp)
	{
		pHwndCtrlTmp->changeGraphicSettings(mode, val);
	}
	/****/
}



QWidget* CameraModule::getViewWidget(int indView)
{
	int nCount = m_pListViewCtrl.count();
	if (indView < nCount)
	{
		return new QAttchWidget(m_pListViewCtrl[indView]);
	}

	return NULL;
}

ViewCtrl* CameraModule::getViewWindow(int indView)
{
	int nCount = m_pListViewCtrl.count();
	if (indView < nCount)
	{
		return  m_pListViewCtrl[indView];
	}

	return NULL;
}

QWidget* CameraModule::getMainView()
{
	return m_pMainView;
}

void CameraModule::load3DViewData(int indView, int nSizeX, int nSizeY, QVector<double>& xValues, QVector<double>& yValues, QVector<double>& zValues)
{
	int nCount = m_pListViewCtrl.count();
	if (indView < nCount)
	{
		ViewCtrl* pViewCtrl = m_pListViewCtrl[indView];
		if (pViewCtrl)
		{
			pViewCtrl->load3DViewData(nSizeX, nSizeY, xValues, yValues, zValues);
		}
	}
}

void CameraModule::show3DView(int indView)
{
	int nCount = m_pListViewCtrl.count();
	if (indView < nCount)
	{
		ViewCtrl* pViewCtrl = m_pListViewCtrl[indView];
		if (pViewCtrl)
		{
			pViewCtrl->show3D();
		}
	}
}

void CameraModule::getIndCamera(int &indCam)
{
	//if ( m_pViewCtrl )
	{
		// indCam = m_pViewCtrl->getIndCamera();
	}
}

void  CameraModule::resetHWindowState(int nWindowCtrl)
{
	if (nWindowCtrl >= m_pListViewCtrl.count())
	{
		return;
	}

	HWndCtrl *pHwndCtrlTmp = m_pListViewCtrl[nWindowCtrl]->getHwndCtrl();
	if (pHwndCtrlTmp)
	{
		pHwndCtrlTmp->setViewState(MODE_VIEW_NONE);
	}

}

void CameraModule::showSettingWidget()
{


}

QMOUDLE_INSTANCE(CameraModule)