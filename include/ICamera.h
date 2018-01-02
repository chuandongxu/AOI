#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "../lib//VisionLibrary/include/opencv/cv.h"

#include <QWidget>

#define  CAMERAMODULE_EVENT_IMAGEARRIVED  "Image Arrived"

class ViewCtrl;
class ICamera
{
public:
	// ��ʼ��
	// camerasNameAndID , ����������ƺ�ID 
    // nWindow ������ʾ���ڵĸ����� = 0 �����ô��ڸ���������������� �����������һ������
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

	virtual bool grabCamImage(int nCamera,    void *lpImage, bool bSyns = true) = 0 ;  // ץȡ���ͼ��
	virtual void setExposureTime(int nCamera, double exposureTime) = 0 ;               // ����

	// indPic = -1 ��ʾ��ȡ��ǰͼ�� �� ����������ֵ���򵱴��ļ���ʱ �� ���Ի�ȡ����ΪindPic��ͼ�� ͼ������ɻ�ȡ
	virtual void getExistImage(int nWindow, int nCamera,   void *lpImage , int indPic = -1 ) =  0;   // ��ȡͼ��
	virtual void getNumImage( int nCamera, int &nPic) =  0;
	
	// ������ʾͼ����������, ����Image������������ȵȣ� Ӧ��������ģʽ�½Ϻ�
	/************************************************************************/
	/*  addHObject ���ͼ������ʱ�� bImage ��־Ϊtrue ������ ͼ�� �� false ���������������ݣ�������������ȵ�   */
	/*  ��Ӷ������������ʾʱ�� ImageӦ���������                                                               */
	/************************************************************************/
	virtual void addHObject( int nWindowCtrl ,  void *pHObject , bool bImage = false )    = 0 ; // ���ͼ�����ݵ� ͼ�񴰿ڿ���, ������ӵ�ͼ�����ݿ��Խ������ŵ���������
	virtual void clearHObjects( int nWindowCtrl ) = 0 ;   // �����������
	virtual void repaintHWindow( int nWindowCtrl ) = 0;   // ��ʾ
	virtual void changeGraphicSettings( int nWindowCtrl , QString mode, QString val) = 0 ;   // ������ʾͼ�����ݵ����ԣ� �� mode = "Color" , val = "red" ��ʾ��ӵ�����Ϊ��ɫ�� 
	                                                                                         // ע���ȵ��øú����ٵ��� addHObject

	virtual void dispObject( int nWindowCtrl , void *pHObject )    = 0 ; 

	// ��ȡ����ָ��
	virtual  QWidget* getViewWidget(int indView) = 0;
	virtual ViewCtrl * getViewWindow(int indView) = 0; // ����ViewCtrl����
	virtual QWidget* getMainView() = 0;

	// ��ʼ��3D����
	virtual void load3DViewData(int indView, int nSizeX, int nSizeY, QVector<double>& xValues, QVector<double>& yValues, QVector<double>& zValues) = 0;
	virtual void show3DView(int indView) = 0;

	// ��ȡ�������
	virtual void getIndCamera( int &indCam) = 0 ;

	// �ر�HWindow ��Ӧ������
	virtual void resetHWindowState( int nWindowCtrl ) = 0 ;
};

#endif