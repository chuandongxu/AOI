#include "VisionDetect.h"
#include "../common/SystemData.h"
#include "VisionDetectSetting.h"
#include "VisionDetectRunView.h"
#include "SysCalibrationView.h"

VisionDetect::VisionDetect(int id, const QString &name)
	:QModuleBase(id, name)
{
	m_pDetectView = new VisionDetectRunView(&m_ctrl);
	m_pCaliView = new SysCalibrationView(&m_ctrl);
}

VisionDetect::~VisionDetect()
{

}

void VisionDetect::addSettingWiddget(QTabWidget * tabWidget)
{
	//if (tabWidget)
	//{
	//	tabWidget->addTab(new VisionDetectSetting(&m_ctrl), QStringLiteral("视觉检测"));
	//}

	//QString user;
	//int level = 0;
	//System->getUser(user, level);
	//if (USER_LEVEL_MANAGER > level)
	//{
	//	//tabWidget->setEnabled(false);
	//}	
}

QWidget* VisionDetect::getDetectView()
{
	return m_pDetectView;
}

QWidget* VisionDetect::getCellEditorView()
{
	return m_pDetectView->getCellEditorView();
}

QWidget* VisionDetect::getCaliView()
{
	return m_pCaliView;
}

bool VisionDetect::loadCmdData(int nStation)
{
	return m_ctrl.loadCmdData(nStation);
}

bool VisionDetect::setHeightData(cv::Mat& matHeight)
{
	m_ctrl.getProfileEditor()->setHeightData(matHeight);
	return true;
}

bool VisionDetect::setProfGrayImg(cv::Mat& imgGray)
{
	m_ctrl.getProfileEditor()->setProfGrayImg(imgGray);
	return true;
}

bool VisionDetect::setProf3DImg(QImage& img3D)
{
	m_ctrl.getProfileEditor()->setProf3DImg(img3D);
	return true;
}

bool VisionDetect::setProfData(QVector<cv::Point2d>& profDatas)
{
	m_ctrl.getProfileEditor()->setProfData(profDatas);
	return true;
}

bool VisionDetect::setProfRange(double x1, double y1, double x2, double y2)
{
	m_ctrl.getProfileEditor()->setProfRange(x1, y1, x2, y2);
	return true;
}

void VisionDetect::prepareNewProf()
{
	m_ctrl.getProfileEditor()->prepareNewProf();
}

bool VisionDetect::calculate3DHeight(int nStation, QVector<cv::Mat>& imageMats, cv::Mat& heightMat, cv::Mat& matHeightResultImg)
{
	return m_ctrl.calculate3DHeight(nStation, imageMats, heightMat, matHeightResultImg);
}

bool VisionDetect::generateGrayImage(QVector<cv::Mat>& imageMats, cv::Mat& grayMat)
{
	return m_ctrl.generateGrayImage(imageMats, grayMat);
}

bool VisionDetect::matchPosition(cv::Mat& matDisplay, QVector<QDetectObj*>& objTests)
{
	return m_ctrl.matchPosition(matDisplay, objTests);
}

bool VisionDetect::calculateDetectHeight(cv::Mat& matHeight, QVector<QDetectObj*>& objTests)
{
	return m_ctrl.calculateDetectHeight(matHeight, objTests);
}

bool VisionDetect::merge3DHeight(QVector<cv::Mat>& matHeights, cv::Mat& matHeight)
{
	return m_ctrl.merge3DHeight(matHeights, matHeight);
}

bool VisionDetect::matchAlignment(cv::Mat& matDisplay, QVector<QProfileObj*>& objProfTests)
{
	return m_ctrl.matchAlignment(matDisplay, objProfTests);
}

bool VisionDetect::calculateDetectProfile(cv::Mat& matHeight, QVector<QProfileObj*>& objProfTests)
{
	return m_ctrl.calculateDetectProfile(matHeight, objProfTests);
}

QMOUDLE_INSTANCE(VisionDetect)