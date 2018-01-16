#pragma once

#include <QMainWindow>
#include "ui_VisionView.h"

#include <QToolBar>
#include <QAction>
#include <QDockWidget>
#include <QMutex>
#include <QThread>

#include "opencv/cv.h"

class VisionView;
class CameraOnLive :public QThread
{
	Q_OBJECT

public:
	CameraOnLive(VisionView* pView);
	~CameraOnLive(){};

public:
	void setQuitFlag();
	bool isRuning(){ return m_bRuning; };
private:
	void run();

	void drawCross(cv::Mat& image);
	void showImageToScreen(cv::Mat& image);
private:
	VisionView*		m_pView;	
	bool          m_bQuit;
	bool          m_bRuning;
};

class QDetectObj;
class DViewUtility;
class VisionView : public QMainWindow
{
	Q_OBJECT

public:
	VisionView(QWidget *parent = Q_NULLPTR);
	~VisionView();

public:
	void setImage(cv::Mat& matImage, bool bDisplay);
	cv::Mat getImage();
	void clearImage();
	void addImageText(QString szText);
	void displayImage(cv::Mat& image);

	void load3DViewData(int nSizeX, int nSizeY, QVector<double>& xValues, QVector<double>& yValues, QVector<double>& zValues);
	void show3DView();

	void setSelect();
	cv::Mat getSelectImage();
	void clearSelect();
	cv::Rect2f getSelectScale();

	void displayObjs(QVector<QDetectObj*> objs, bool bShowNumber);

private slots:
	void onResultEvent(const QVariantList &data);

	void openFile();
	void cameraFile();
	void saveAsFile();
	void zoomIn();
	void zoomOut();
	void fullScreen();
	void moveScreen();

	void onClickPushbutton_onLive();
	void onClickPushbutton_stopLive();

	void show3D();
	void showSelectROI3D();

private:
	void init();
	void createActions();
	void createToolBars();
	void createStatusBar();

	QMenu *fileMenu;
	QMenu *editMenu;
	QMenu *detectMenu;

	QToolBar *fileToolBar;
	QToolBar *editToolBar;
	QToolBar *videoToolBar;
	QToolBar *detectToolBar;
	QAction *openAct;
	QAction *cameraAct;
	QAction *saveAsAct;
	QAction *zoomInAct;
	QAction *zoomOutAct;
	QAction *fullScreenAct;
	QAction *moveAct;

	QAction *onLiveAct;
	QAction *onStopAct;

	QAction *show3DAct;
	QAction *selectROI;

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
	void setViewState(int state);
	double convertToImgX(double dMouseValue, bool bLen = false);
	double convertToImgY(double dMouseValue, bool bLen = false);
	double convertToMouseX(double dImgValue, bool bLen = false);
	double convertToMouseY(double dImgValue, bool bLen = false);

	void show3DView(cv::Rect& rectROI);
private:
	void fullImage();
	void zoomImage(double scale);
	void moveImage(double motionX, double motionY);

	void setButtonsEnable(bool flag, bool bLiveVideo);

public:
	bool startUpCapture();
	bool endUpCapture();

	void setHeightData(cv::Mat& matHeight);
private:
	int    m_stateView;
	bool   m_mouseLeftPressed;
	bool   m_mouseRightPressed;
	double m_startX, m_startY;
	double m_preMoveX, m_preMoveY;

	int   m_windowWidth;
	int   m_windowHeight;
	int   m_imageWidth;
	int   m_imageHeight;

	double m_dScale;
	double m_dMovedX;
	double m_dMovedY;

private:
	Ui::VisionView ui;
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
};
