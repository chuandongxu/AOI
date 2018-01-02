#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "../lib//VisionLibrary/include/opencv/cv.h"

#include <QWidget>

#define  CAMERAMODULE_EVENT_IMAGEARRIVED  "Image Arrived"

class ViewCtrl;
class ICamera
{
public:
	// 初始化
	// camerasNameAndID , 设置相机名称和ID 
    // nWindow 设置显示窗口的个数， = 0 则设置窗口个数等于相机个数， 如果无相机则打开一个窗口
	virtual void initial(int nWindow = 0 )=0;
	virtual void unInit() = 0;

	//------------HSG Main Flow---------------------------------------
	virtual bool startUpCapture() = 0;
	virtual bool endUpCapture() = 0;
	
	virtual const QVector<cv::Mat>& getImageBuffer() = 0;
	virtual const cv::Mat& getImageItemBuffer(int nIndex) = 0;
	virtual int getImageBufferNum() = 0;
	virtual int getImageBufferCaptureNum() = 0;
	virtual bool startCapturing() = 0;
	virtual void clearImageBuffer() = 0;
	virtual bool isCaptureImageBufferDone() = 0;

	virtual bool lockCameraCapture(int iStation) = 0;
	virtual void unlockCameraCapture() = 0;
	virtual bool isCameraCaptureAvaiable() = 0;

	virtual void addImageText(QString& szText) = 0;
	virtual void clearImage() = 0;
	virtual void setImage(cv::Mat& image, bool bDisplay = true) = 0;

	virtual void setHeightData(cv::Mat& matHeight) = 0;

	//-----------------------------------------------------------------
	virtual void openCamera() = 0;
	virtual void closeCamera() = 0;
	virtual int getCameraNum() = 0;	

	virtual bool grabCamImage(int nCamera,    void *lpImage, bool bSyns = true) = 0 ;  // 抓取相机图像
	virtual void setExposureTime(int nCamera, double exposureTime) = 0 ;               // 设置

	// indPic = -1 表示获取当前图像 ， 若设置其他值，则当打开文件夹时 ， 可以获取索引为indPic的图像， 图像个数由获取
	virtual void getExistImage(int nWindow, int nCamera,   void *lpImage , int indPic = -1 ) =  0;   // 获取图像
	virtual void getNumImage( int nCamera, int &nPic) =  0;
	
	// 用于显示图像类型数据, 包括Image、轮廓、区域等等， 应用在离线模式下较好
	/************************************************************************/
	/*  addHObject 添加图像数据时， bImage 标志为true 表明是 图像 ， false 则是其他类型数据，如轮廓、区域等等   */
	/*  添加多个数据用于显示时， Image应该最先添加                                                               */
	/************************************************************************/
	virtual void addHObject( int nWindowCtrl ,  void *pHObject , bool bImage = false )    = 0 ; // 添加图像数据到 图像窗口控制, 这样添加的图像数据可以进行缩放等其他操作
	virtual void clearHObjects( int nWindowCtrl ) = 0 ;   // 清除所有数据
	virtual void repaintHWindow( int nWindowCtrl ) = 0;   // 显示
	virtual void changeGraphicSettings( int nWindowCtrl , QString mode, QString val) = 0 ;   // 设置显示图像数据的属性， 如 mode = "Color" , val = "red" 显示添加的轮廓为红色， 
	                                                                                         // 注意先调用该函数再调用 addHObject

	virtual void dispObject( int nWindowCtrl , void *pHObject )    = 0 ; 

	// 获取窗口指针
	virtual  QWidget* getViewWidget(int indView) = 0;
	virtual ViewCtrl * getViewWindow(int indView) = 0; // 返回ViewCtrl窗口
	virtual QWidget* getMainView() = 0;

	// 初始化3D数据
	virtual void load3DViewData(int indView, int nSizeX, int nSizeY, QVector<double>& xValues, QVector<double>& yValues, QVector<double>& zValues) = 0;
	virtual void show3DView(int indView) = 0;

	// 获取相机索引
	virtual void getIndCamera( int &indCam) = 0 ;

	// 关闭HWindow 响应鼠标键盘
	virtual void resetHWindowState( int nWindowCtrl ) = 0 ;
};

#endif