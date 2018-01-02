#pragma once

#include <QMainWindow>
#include "ui_VisionView.h"
#include <QToolBar>
#include <QAction>
#include "VisionCtrl.h"

#include "opencv/cv.h"

using namespace cv;

class VisionView : public QMainWindow
{
	Q_OBJECT

public:
	VisionView(VisionCtrl* pCtrl, QWidget *parent = Q_NULLPTR);
	~VisionView();

public:
	void setImage(cv::Mat& matImage);
	cv::Mat getImage();
	void clearImage();
	void addImageText(QString szText);
	void displayImage(cv::Mat& image);

private slots:	
	void openFile();
	void openFileFolder();
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

	QToolBar *fileToolBar;
	QToolBar *editToolBar;
	QAction *openAct;
	QAction *openFolderAct;
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
	double convertToImgX(double dMouseValue);
	double convertToImgY(double dMouseValue);

private:
	bool convertToGrayImage(QString& szFilePath, cv::Mat &matGray);

private:
	void fullImage();
	void zoomImage(double scale);
	void moveImage(double motionX, double motionY);

private:
	int    m_stateView;
	bool   m_mouseLeftPressed;
	bool   m_mouseRightPressed;
	double m_startX, m_startY;

	int   m_windowWidth;
	int   m_windowHeight;
	int   m_imageWidth;
	int   m_imageHeight;

	double m_dScale;
	double m_dMovedX;
	double m_dMovedY;

private:
	Ui::VisionView ui;
	cv::Mat	m_hoImage;
	cv::Mat	m_dispImage;
	VisionCtrl* m_pCtrl;
};
