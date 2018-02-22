#pragma once

#include <QMainWindow>
#include "ui_QProfileView.h"

#include <QToolBar>
#include <QAction>

#include "opencv/cv.h"

class QProfileView : public QMainWindow
{
	Q_OBJECT

public:
	QProfileView(QWidget *parent = Q_NULLPTR);
	~QProfileView();

protected:
	void closeEvent(QCloseEvent *e);

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
	cv::Rect2f getSelectedROI();
	void clearSelect();

	QVector<cv::Rect>& getMeasureRange() { return m_measRanges; }
	void clearMeasure();

	int getWindowWidth() { return m_windowWidth; }
	int getWindowHeight() { return m_imageHeight; }

private slots:
	void zoomIn();
	void zoomOut();
	void fullScreen();
	void moveScreen();

	void onStart();
	void onReset();
	void onDone();

private:
	void init();
	void createActions();
	void createToolBars();
	void createStatusBar();

	QMenu *editMenu;

	QToolBar *editToolBar;

	QAction *zoomInAct;
	QAction *zoomOutAct;
	QAction *fullScreenAct;
	QAction *moveAct;

protected:
	void mouseMoveEvent(QMouseEvent * event);
	void mouseDoubleClickEvent(QMouseEvent * event);
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent * event);

private:
	void loadImage(QString& fileName);
	void repaintAll();
	void A_Transform(cv::Mat& src, cv::Mat& dst, int dx, int dy);
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
	Ui::QProfileView ui;
	cv::Mat	m_hoImage;
	cv::Mat	m_dispImage;

	cv::Rect m_selectROI;	
	bool m_bSelected;

	QVector<cv::Rect> m_measRanges;
	cv::Rect m_measRangeCurr;
};
