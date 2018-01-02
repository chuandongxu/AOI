#ifndef VIEWCTRL_H
#define VIEWCTRL_H

#include <QWidget>
#include "ui_viewctrl.h"
#include "CameraCtrl.h"
#include "HWndCtrl.h"
#include <QThread>
#include <QVBoxLayout>
#include "qevent.h"
#include "qaction.h"
#include <QLabel>

#include "opencv/cv.h"

using namespace cv;

class ViewCtrl;
class CameraOnLive :public QThread
{
	Q_OBJECT

public:
	CameraOnLive(ViewCtrl* pCtrl, CameraDevice *pCameraTmp, QLabel* hv_windowHandle);
	~CameraOnLive(){};

public:
	void setQuitFlag();
	bool isRuning(){return m_bRuning;};
private:
	void run();

	void drawCross(cv::Mat& image);
	void showImageToScreen(cv::Mat& image);
private:
	ViewCtrl*		m_pCtrl;
	CameraDevice *m_pCameraTmp ;
	QLabel*        m_hv_windowHandle;
	bool          m_bQuit;
	bool          m_bRuning;
};


//////////////////////////////////////////////////////////////////////////
// 图像GUI窗口 ， 通过setCamera与相机关联
class DViewUtility;
class ViewCtrl : public QWidget
{
	Q_OBJECT

public:
	ViewCtrl(   QWidget *parent = 0);
	~ViewCtrl();
	void initial( int nWindow = 0 );
	void setMyLayout();
	void connectCameraCtrl( CameraCtrl* pCameralCtrl );

private:
	Ui::ViewCtrl ui;
	QHBoxLayout *m_pLayoutToolsMain;
	QHBoxLayout *m_pLayoutTools ;      // tools 布局
	QHBoxLayout *m_pLayoutToolsAnd ;   // tools 其他布局 , 如导航条
	QHBoxLayout *m_pLayoutToolsOther;   // tools 其他布局 , 如导航条
	QHBoxLayout *m_pLayoutHWindow;     // HWindow 布局
	QVBoxLayout *m_pLayoutMain;        // 整体布局

private:
	// 参数List
	CameraCtrl      *m_pCameralCtrl;      // 相机
	cv::Mat          m_hoImage;
	HWndCtrl         m_hwndCtrl;          // Halcon 窗口控制
 	QLabel*          m_hv_windowHandle;   // Halcon 窗口句柄

	// 离线图片
	QList<QString> m_picPath;       // 文件夹下所有 图片的路径 ， QList的长度即 m_hoImages 的个数
	int            m_picIndCur;     // 当前图片序号
	int           m_picNum;        // 图片数量
	
	// 绘图数据
	cv::Mat  m_hoRegionDrawObject;  // 通过 draw 获取到的 region ， QList的长度即 m_indShow
	cv::Mat  m_hoImageDrawObject;   // region 对应的图像
	cv::Mat  m_hvDrawObject;        // 表示该region 的 htuple 数据

	// 当前参数
	CameraDevice  *m_pCameraCur;           // 当前相机
public:
	void setCurDevice( int indCam);

public:
	HWndCtrl   * getHwndCtrl( );
	CameraCtrl * getCameraCtrl();
	cv::Mat      getImage( int indPic =  -1 );
	void		 setImageBuffer(QVector<cv::Mat>& matImages);
	const QVector<cv::Mat>& getImageBuffer();
	int			 getImageBufferNum();
	void		 clearImageBuffer();
	void         setButtonsEnable( bool flag);
	void         freshCur();   // 当前主要使用变量值获取
	void         freshNavigation();
	void         getDrawObjects(  int nCamera,   void *pHo_Region , void * pHv_ofRegion = NULL , void *pHo_Image = NULL ) ;

	CameraDevice* getCurrentDevice(){
		return m_pCameraCur;
	}

	void addImageText(QString szText);	
	void clearImage();
	void setImage(cv::Mat image);

	void load3DViewData(int nSizeX, int nSizeY, QVector<double>& xValues, QVector<double>& yValues, QVector<double>& zValues);
	void show3D();

	bool startUpCapture();
	bool endUpCapture();

private:
	void resizeEvent(QResizeEvent *event);
	void setHWindowSize();
public:
	QRect getHWindowRect();

private slots:
	void onClickOpenFile();
	void onToolButton_openImages(); 
	void onClickSnapImage();
	void onClickZoom();
	void onClickSaveImage();
	void onClickPushbutton_onLive();
	void onClickPushbutton_stopLive();
	void onClickAdaptWindow();
	void onToolButton_adaptImage();
	void onToolButton_moveImage();
	void onComboBox_cameras();
	void onToolButton_firstPic() ;
	void onToolButton_forwardPic() ;
	void onToolButton_nextPic() ;
	void onToolButton_LastPic() ;
	void onToolButton_3DView();
	void onMenu_Draw_Triggered(QAction *); // 绘图
	
	// 绘图
private:
	QAction *m_ActionMenuDrawRect ;
	QAction *m_ActionMenuDrawRect2;
	QAction *m_ActionMenuDrawCircle;
	QAction *m_ActionMenuDrawPoly;

	// 相机实时显示线程
private:
	CameraOnLive * m_pCameraOnLive;
	QVector<cv::Mat> m_bufferImages;
	int m_nCaptureNum;

private:
	DViewUtility   *m_pView3D;
};

#endif // VIEWCTRL_H
