#ifndef CARAMEMODEL_H
#define CARAMEMODEL_H

#include "caramemodel_global.h"
#include "../include/ICamera.h"
#include "../common/modulebase.h"
#include "QMainView.h"
#include "CameraCtrl.h"
#include "viewctrl.h"

#include "opencv/cv.h"

class CameraModule : public QModuleBase, public ICamera
{
public:
	CameraModule(int id,const QString &name);
	~CameraModule();

     void initial(int nWindow = 0 );
	 void unInit();

public:
	bool startUpCapture();
	bool endUpCapture();

	const QVector<cv::Mat>& getImageBuffer();
	const cv::Mat& getImageItemBuffer(int nIndex);
	int getImageBufferNum();
	int getImageBufferCaptureNum();
	bool startCapturing();
	void clearImageBuffer();
	bool isCaptureImageBufferDone();

	bool lockCameraCapture(int iStation);
	void unlockCameraCapture();
	bool isCameraCaptureAvaiable();

	void addImageText(QString& szText);
	void clearImage();
	void setImage(cv::Mat& image, bool bDisplay);

	void setHeightData(cv::Mat& matHeight);

private:
	 void openCamera();
	 void closeCamera();
	 int getCameraNum();	

	 bool grabCamImage(int nCamera,       void *lpImage , bool bSync = true);
	 void setExposureTime(int nCamera, double exposureTime);
	
	 void getExistImage(int nWindow, int nCamera,   void *lpImage , int indPic = -1 ); // 获取图像， 默认获取当前图像， 否则获取索引为indPic的图像
	 void getNumImage( int nCamera, int &nPic) ;               // 获取图像个数

	 // 显示图像数据
	 void addHObject( int nWindowCtrl ,  void *pHObject , bool bImage = false ) ; // 添加图像数据到 图像窗口控制, 这样添加的图像数据可以进行缩放等其他操作
	 void clearHObjects( int nWindowCtrl )  ;
	 void repaintHWindow( int nWindowCtrl ) ;
	 void changeGraphicSettings( int nWindowCtrl , QString mode, QString val);   // 设置显示图像数据的属性， 如 显示添加的轮廓为红色， 注意先调用该函数再调用 addHObject
	 void dispObject(  int nWindowCtrl , void *pHObject ) ; 

	 //获取图像窗口
	 QWidget * getViewWidget(int indView)  ; // 返回ViewCtrl窗口
	 ViewCtrl * getViewWindow(int indView); // 返回ViewCtrl窗口
	 virtual QWidget* getMainView();

	 void load3DViewData(int indView, int nSizeX, int nSizeY, QVector<double>& xValues, QVector<double>& yValues, QVector<double>& zValues);
	 void show3DView(int indView);

	 void getIndCamera( int &indCam);
	 void resetHWindowState( int nWindowCtrl )  ;

private:
	virtual void addSettingWiddget(QTabWidget *tabWidget);
    virtual void showSettingWidget();
	void setErrorMap();

private: 
	CameraCtrl*      m_pCameraCtrl;
	QList<ViewCtrl*>  m_pListViewCtrl;
	QMainView* m_pMainView;
};

#endif // CARAMEMODEL_H
