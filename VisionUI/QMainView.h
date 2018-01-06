#pragma once

#include <QMainWindow>
#include "ui_QMainView.h"

#include <QToolBar>
#include <QAction>
#include <QDockWidget>
#include <QMutex>

#include "opencv/cv.h"

class DViewUtility;
class QMainView : public QMainWindow
{
	Q_OBJECT

public:
	QMainView(QWidget *parent = Q_NULLPTR);
	~QMainView();

public:
	void setImage(cv::Mat& matImage, bool bDisplay);
	cv::Mat getImage();
	void clearImage();
	void addImageText(QString szText);
	void displayImage(cv::Mat& image);

private slots:
	void onResultEvent(const QVariantList &data);

	void openFile();
	void cameraFile();
	void saveAsFile();
	void zoomIn();
	void zoomOut();
	void fullScreen();
	void moveScreen();

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
	QToolBar *detectToolBar;
	QAction *openAct;
	QAction *cameraAct;
	QAction *saveAsAct;
	QAction *zoomInAct;
	QAction *zoomOutAct;
	QAction *fullScreenAct;
	QAction *moveAct;

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

	void setButtonsEnable(bool flag);	

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
	Ui::QMainView ui;

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
