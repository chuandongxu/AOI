#pragma once

#include <QMainWindow>
#include "ui_QCellView.h"

#include <QToolBar>
#include <QAction>

#include "opencv/cv.h"

using namespace cv;

class QDetectObj;
class QCellView : public QMainWindow
{
	Q_OBJECT

public:
	QCellView(QWidget *parent = Q_NULLPTR);
	~QCellView();

public:
	void setImage(cv::Mat& matImage);
	cv::Mat getImage();
	int getImageWidth();
	int getImageHeight();
	void clearImage();
	void addImageText(QString szText);
	void displayImage(cv::Mat& image);

	void setSelect();
	bool isSelect();
	cv::Mat getSelectImage();
	cv::Rect getSelectRect();
	Rect2f getSelectScale();
	void clearSelect();

	void displayObjs(QVector<QDetectObj*> objs, bool bShowNumber);

private slots:
	void openConfigFile();
	void saveAsConfigFile();
	void openFile();
	void cameraFile();
	void saveAsFile();
	void zoomIn();
	void zoomOut();
	void fullScreen();
	void moveScreen();

private:
	void init();
	void createActions();
	void createToolBars();
	void createStatusBar();

	QMenu *fileMenu;
	QMenu *editMenu;

	QToolBar *configToolBar;
	QToolBar *fileToolBar;
	QToolBar *editToolBar;
	QAction *openConfigAct;
	QAction *saveAsConfigAct;
	QAction *openAct;
	QAction *cameraAct;
	QAction *saveAsAct;
	QAction *zoomInAct;
	QAction *zoomOutAct;
	QAction *fullScreenAct;
	QAction *moveAct;

protected:
	void mouseMoveEvent(QMouseEvent * event);
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent * event);

private:
	void loadImage(QString& fileName);
	void repaintAll();
	void A_Transform(Mat& src, Mat& dst, int dx, int dy);
	void setViewState(int state);
	double convertToImgX(double dMouseValue, bool bLen = false);
	double convertToImgY(double dMouseValue, bool bLen = false);

private:
	void fullImage();
	void zoomImage(double scale);
	void moveImage(double motionX, double motionY);

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
	Ui::QCellView ui;
	cv::Mat	m_hoImage;
	cv::Mat	m_dispImage;

	cv::Rect m_selectROI;
	bool m_bSelected;
};
