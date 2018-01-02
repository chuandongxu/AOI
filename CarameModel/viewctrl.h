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
// ͼ��GUI���� �� ͨ��setCamera���������
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
	QHBoxLayout *m_pLayoutTools ;      // tools ����
	QHBoxLayout *m_pLayoutToolsAnd ;   // tools �������� , �絼����
	QHBoxLayout *m_pLayoutToolsOther;   // tools �������� , �絼����
	QHBoxLayout *m_pLayoutHWindow;     // HWindow ����
	QVBoxLayout *m_pLayoutMain;        // ���岼��

private:
	// ����List
	CameraCtrl      *m_pCameralCtrl;      // ���
	cv::Mat          m_hoImage;
	HWndCtrl         m_hwndCtrl;          // Halcon ���ڿ���
 	QLabel*          m_hv_windowHandle;   // Halcon ���ھ��

	// ����ͼƬ
	QList<QString> m_picPath;       // �ļ��������� ͼƬ��·�� �� QList�ĳ��ȼ� m_hoImages �ĸ���
	int            m_picIndCur;     // ��ǰͼƬ���
	int           m_picNum;        // ͼƬ����
	
	// ��ͼ����
	cv::Mat  m_hoRegionDrawObject;  // ͨ�� draw ��ȡ���� region �� QList�ĳ��ȼ� m_indShow
	cv::Mat  m_hoImageDrawObject;   // region ��Ӧ��ͼ��
	cv::Mat  m_hvDrawObject;        // ��ʾ��region �� htuple ����

	// ��ǰ����
	CameraDevice  *m_pCameraCur;           // ��ǰ���
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
	void         freshCur();   // ��ǰ��Ҫʹ�ñ���ֵ��ȡ
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
	void onMenu_Draw_Triggered(QAction *); // ��ͼ
	
	// ��ͼ
private:
	QAction *m_ActionMenuDrawRect ;
	QAction *m_ActionMenuDrawRect2;
	QAction *m_ActionMenuDrawCircle;
	QAction *m_ActionMenuDrawPoly;

	// ���ʵʱ��ʾ�߳�
private:
	CameraOnLive * m_pCameraOnLive;
	QVector<cv::Mat> m_bufferImages;
	int m_nCaptureNum;

private:
	DViewUtility   *m_pView3D;
};

#endif // VIEWCTRL_H
