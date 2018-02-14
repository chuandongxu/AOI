#pragma once

#include <QWidget>
#include "ui_VisionViewWidget.h"

#include <QToolBar>
#include <QAction>
#include <QDockWidget>
#include <QMutex>
#include "VisionAPI.h"
#include "constants.h"
#include <QThread>

using namespace AOI::Vision;

class VisionViewWidget;
class CameraOnLive :public QThread
{
	Q_OBJECT

public:
	CameraOnLive(VisionViewWidget* pView);
	~CameraOnLive(){};

public:
	void setQuitFlag();
	bool isRuning(){ return m_bRuning; };
private:
	void run();

	void drawCross(cv::Mat& image);
	void showImageToScreen(cv::Mat& image);
private:
	VisionViewWidget       *m_pView;
	bool                    m_bQuit;
	bool                    m_bRuning;
};

class QDetectObj;
class DViewUtility;
class VisionViewWidget : public QWidget
{
	Q_OBJECT

public:
	VisionViewWidget(QWidget *parent = Q_NULLPTR);
	~VisionViewWidget();

private:
	void init();

public:
	void setViewState(VISION_VIEW_MODE state);
	void setImage(const cv::Mat& matImage, bool bDisplay);
	cv::Mat getImage();
	void clearImage();
	void addImageText(QString szText);
	void displayImage(cv::Mat& image);

	void load3DViewData(int nSizeX, int nSizeY, QVector<double>& xValues, QVector<double>& yValues, QVector<double>& zValues);
	void show3DView();

	cv::Mat getSelectImage();
	void clearSelect();
	cv::Rect2f getSelectedROI();

	void displayObjs(QVector<QDetectObj*> objs, bool bShowNumber);
	void setDeviceWindows(const QVector<cv::RotatedRect> &vecWindows);
	void setSelectedFM(const QVector<cv::RotatedRect> &vecWindows);
	void getSelectDeviceWindow(cv::RotatedRect &rrectCadWindow, cv::RotatedRect &rrectImageWindow) const;

private slots:
	void onResultEvent(const QVariantList &data);

public:
	void openFile();
	void cameraFile();
	void saveAsFile();
	void zoomIn();
	void zoomOut();
	void fullScreen();
	void moveScreen();

	bool isLiving();
	bool onLive();
	void onStopLive();

	void show3D();
	void showSelectROI3D();

protected:
	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);

	void mouseMoveEvent(QMouseEvent * event);
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent * event);

private:
	void addNodeByDrag(int nType, int nObjID, QPoint ptPos);
	void displayAllObjs();

	void loadImage(QString& fileName);
	void repaintAll();
	void A_Transform(cv::Mat& src, cv::Mat& dst, int dx, int dy);

	cv::Point convertToImgPos(const cv::Point &ptMousePos);
	cv::Point convertToMousePos(const cv::Point &ptImgPos);

	void show3DView(cv::Rect& rectROI);

	void fullImage();
	void zoomImage(double scale);
	void moveImage(double motionX, double motionY);
	
	void _zoomImageForDisplay(const cv::Mat &matInputImg, cv::Mat &matOutput);
	void _cutImageForDisplay(const cv::Mat &matInputImg, cv::Mat &matOutput);
	void _drawDeviceWindows(cv::Mat &matImg);
	void _calcMoveRange();
	void _checkSelectedDevice(const cv::Point &ptMousePos);
	void setButtonsEnable(bool flag, bool bLiveVideo);

public:
	void setHeightData(cv::Mat& matHeight);

private:
	VISION_VIEW_MODE    m_stateView;
	bool                m_mouseLeftPressed;
	bool                m_mouseRightPressed;
	double              m_startX, m_startY;
	double              m_preMoveX, m_preMoveY;

	int   m_windowWidth;
	int   m_windowHeight;
	int   m_imageWidth;
	int   m_imageHeight;

	double m_dScale;
	double m_dMovedX;
	double m_dMovedY;

private:
	Ui::VisionViewWidget ui;

	QMutex m_mutex;
	CameraOnLive * m_pCameraOnLive;

	cv::Mat	m_hoImage;
	cv::Mat	m_dispImage;
	cv::Mat m_3DMatHeight;

private:
	DViewUtility   *m_pMainViewFull3D;
	DViewUtility   *m_pView3D;
	cv::Rect m_selectROI;
	QDockWidget *m_pSelectView;
	bool m_bShow3DInitial;
	bool m_bMainView3DInitial;
	QVector<cv::RotatedRect>    m_vecDeviceWindows;
	QVector<cv::RotatedRect>    m_vecSelectedFM;   //FM for fiducial mark
	cv::RotatedRect             m_selectedDevice;
	cv::Size m_szCadOffset;
	cv::Size _szMoveRange;

	static const cv::Scalar _constRedScalar;
	static const cv::Scalar _constBlueScalar;
	static const cv::Scalar _constCyanScalar;
	static const cv::Scalar _constGreenScalar;
	static const cv::Scalar _constYellowScalar;
	const float             _constMaxZoomScale = 4.f;
	const float             _constMinZoomScale = 0.25;
	const float             _constZoomInStep = 2.0;
	const float             _constZoomOutStep = 0.5;
	const int               _constDeviceWindowLineWidth = 5;
};
