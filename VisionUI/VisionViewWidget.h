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
#include "../DataModule/QDetectObj.h"
#include "VisionViewStruct.hpp"

using namespace AOI::Vision;

class VisionViewWidget;
class CameraOnLive :public QThread
{
	Q_OBJECT

public:
	CameraOnLive(VisionViewWidget* pView, bool bHWTrigger);
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
	bool					m_bHWTrigger;
	bool                    m_bQuit;
	bool                    m_bRuning;
};

class DViewUtility;
class VisionViewWidget : public QWidget
{
	Q_OBJECT

public:
	VisionViewWidget(QWidget *parent = Q_NULLPTR);
	~VisionViewWidget();

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
    cv::Rect getSrchWindow() const { return m_rectSrchWindow; }
    void setSrchWindow(const cv::Rect &rectSrchWindow);

	void displayObjs(QVector<QDetectObj*> objs, bool bShowNumber);
    void setDetectObjs(const QVector<QDetectObj> &vecDetectObjs);
    void setCurrentDetectObj(const QDetectObj &detectObj);
    QVector<QDetectObj> getDetectObjs() const;
	void setDeviceWindows(const VisionViewDeviceVector &vecWindows);
	void getSelectDeviceWindow(cv::RotatedRect &rrectCadWindow, cv::RotatedRect &rrectImageWindow) const;
    VisionViewDevice getSelectedDevice() const;
    void setConfirmedFM(const VisionViewFMVector &vecFM);
    void setCurrentFM(const VisionViewFM &fm);
    VisionViewFM getCurrentFM() const;
    void setHeightData(const cv::Mat& matHeight);
	cv::Mat getHeightData() const;

	void openFile();
	void cameraFile();
	void saveAsFile();
	void zoomIn();
	void zoomOut();
	void fullScreen();
	void moveScreen();

	bool isLiving();
	bool onLive(bool bPromptSelect);
	void onStopLive();

	void show3D();
	void showSelectROI3D();
    void showInspectROI();

    void copyDevice();
    void pasteDevice();

protected:
	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);

	void mouseMoveEvent(QMouseEvent * event);
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent * event);

private slots:
	void onResultEvent(const QVariantList &data);
    void onSearchDeviceState(const QVariantList &data);

private:
	void init();	
	void addNodeByDrag(int nType, int nObjID, QPoint ptPos);
	void displayAllObjs();

	void loadImage(QString& fileName);
	void repaintAll();
	void A_Transform(cv::Mat& src, cv::Mat& dst, int dx, int dy);

	cv::Point convertToImgPos(const cv::Point &ptMousePos);
	cv::Point convertToMousePos(const cv::Point &ptImgPos);

	void show3DView(cv::Rect& rectROI);
    void showInspectView(cv::Rect& rectROI);

	void fullImage();
	void zoomImage(double scale);
	void moveImage(double motionX, double motionY);
	
    void _drawSelectedROI(cv::Mat &matImage);
	void _zoomImageForDisplay(const cv::Mat &matInputImg, cv::Mat &matOutput);
	void _cutImageForDisplay(const cv::Mat &matInputImg, cv::Mat &matOutput);
	void _drawDeviceWindows(cv::Mat &matImg);
    void _drawDetectObjs();
	void _calcMoveRange();
	bool _checkSelectedDevice(const cv::Point &ptMousePos);

    void _moveToSelectDevice(const QString& name);
    bool _pasteSelectedDevice();    

private:
    Ui::VisionViewWidget    ui;

	VISION_VIEW_MODE        m_stateView;
    VISION_VIEW_MODE        m_enPreviousState;
	bool                    m_mouseLeftPressed;
	bool                    m_mouseRightPressed;
	double                  m_startX, m_startY;
	double                  m_preMoveX, m_preMoveY;

	int                     m_windowWidth;
	int                     m_windowHeight;
	int                     m_imageWidth;
	int                     m_imageHeight;

	double                  m_dScale;
	double                  m_dMovedX;
	double                  m_dMovedY;	

	QMutex                  m_mutex;
	CameraOnLive           *m_pCameraOnLive;

	cv::Mat	                m_hoImage;
	cv::Mat	                m_dispImage;
	cv::Mat                 m_3DMatHeight;

	DViewUtility           *m_pMainViewFull3D;
	DViewUtility           *m_pView3D;
	cv::Rect                m_selectROI;
    cv::Rect                m_rectSrchWindow;
	QDockWidget            *m_pSelectView;
	bool                    m_bShow3DInitial;
	bool                    m_bMainView3DInitial;
	VisionViewDeviceVector  m_vecDevices;
	VisionViewDevice        m_selectedDevice;
    VisionViewDevice        m_selectedCopyDevice;
    VisionViewFMVector      m_vecConfirmedFM;
    VisionViewFM            m_currentFM;
	cv::Size                m_szCadOffset;
	cv::Size                m_szMoveRange;
    bool                    m_bDisplayDetectObjs;
    QVector<QDetectObj>     m_vecDetectObjs;
    QDetectObj              m_currentDetectObj;

	static const cv::Scalar _constRedScalar;
	static const cv::Scalar _constBlueScalar;
	static const cv::Scalar _constCyanScalar;
	static const cv::Scalar _constGreenScalar;
	static const cv::Scalar _constYellowScalar;
    static const cv::Scalar _constOrchidScalar;
	const float             _constMaxZoomScale = 4.f;
	const float             _constMinZoomScale = 0.25;
	const float             _constZoomInStep = 2.0;
	const float             _constZoomOutStep = 0.5;
	const int               _constDeviceWindowLineWidth = 2;

    QWidget* m_pInspectWidget;
};
