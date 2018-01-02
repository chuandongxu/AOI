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
	
	 void getExistImage(int nWindow, int nCamera,   void *lpImage , int indPic = -1 ); // ��ȡͼ�� Ĭ�ϻ�ȡ��ǰͼ�� �����ȡ����ΪindPic��ͼ��
	 void getNumImage( int nCamera, int &nPic) ;               // ��ȡͼ�����

	 // ��ʾͼ������
	 void addHObject( int nWindowCtrl ,  void *pHObject , bool bImage = false ) ; // ���ͼ�����ݵ� ͼ�񴰿ڿ���, ������ӵ�ͼ�����ݿ��Խ������ŵ���������
	 void clearHObjects( int nWindowCtrl )  ;
	 void repaintHWindow( int nWindowCtrl ) ;
	 void changeGraphicSettings( int nWindowCtrl , QString mode, QString val);   // ������ʾͼ�����ݵ����ԣ� �� ��ʾ��ӵ�����Ϊ��ɫ�� ע���ȵ��øú����ٵ��� addHObject
	 void dispObject(  int nWindowCtrl , void *pHObject ) ; 

	 //��ȡͼ�񴰿�
	 QWidget * getViewWidget(int indView)  ; // ����ViewCtrl����
	 ViewCtrl * getViewWindow(int indView); // ����ViewCtrl����
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
