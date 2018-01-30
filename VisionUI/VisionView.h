#pragma once

#include <QMainWindow>
#include "ui_VisionView.h"

#include <QToolBar>
#include <QAction>
#include <QDockWidget>
#include "VisionAPI.h"
#include "constants.h"
#include <QThread>

using namespace AOI::Vision;

class QDetectObj;
class VisionViewWidget;
class VisionView : public QMainWindow
{
	Q_OBJECT

public:
	VisionView(QWidget *parent = Q_NULLPTR);
	~VisionView();

public:
    void setViewState(VISION_VIEW_MODE state);
	void setImage(const cv::Mat& matImage, bool bDisplay);
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
    void setDeviceWindows(const QVector<cv::RotatedRect> &vecWindows);
    void setSelectedFM(const QVector<cv::RotatedRect> &vecWindows);
    void getSelectDeviceWindow(cv::RotatedRect &rrectCadWindow, cv::RotatedRect &rrectImageWindow) const;

	bool startUpCapture();
	bool endUpCapture();

	void setHeightData(cv::Mat& matHeight);

private slots:
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

	void showLight();

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

	QAction *showLightAct;

private:
	void setButtonsEnable(bool flag);
	void setLiveButtonEnable(bool flag);

private:
	Ui::VisionView ui;	
	VisionViewWidget* m_pViewWidget;

	QWidget* m_pLightWidget;
};
