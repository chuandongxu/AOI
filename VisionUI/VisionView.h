#pragma once

#include <QMainWindow>
#include "ui_VisionView.h"

#include <QToolBar>
#include <QAction>
#include <QDockWidget>
#include <QThread>
#include <memory>

#include "VisionAPI.h"
#include "constants.h"

#include "../DataModule/QDetectObj.h"
#include "VisionViewStruct.hpp"
#include "VisionViewWidget.h"

using namespace AOI::Vision;

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

	cv::Mat getSelectImage();
	void clearSelect();
	cv::Rect2f getSelectedROI();

	void displayObjs(QVector<QDetectObj*> objs, bool bShowNumber);
    void setDetectObjs(const QVector<QDetectObj> &vecDetectObjs);
    void setCurrentDetectObj(const QDetectObj &detectObj);
    QVector<QDetectObj> getDetectObjs() const;
    void setDeviceWindows(const VisionViewDeviceVector &vecWindows);
    void getSelectDeviceWindow(cv::RotatedRect &rrectCadWindow, cv::RotatedRect &rrectImageWindow) const;
    virtual VisionViewDevice getSelectedDevice() const;
    void setConfirmedFM(const VisionViewFMVector &vecFM);
    void setCurrentFM(const VisionViewFM &fm);
    VisionViewFM getCurrentFM() const;

	bool startUpCapture();
	bool endUpCapture();

	void setHeightData(cv::Mat& matHeight);
	cv::Mat getHeightData();

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
    void selectROI();
	void showSelectROI3D();

	void showLight();
	void showColorSpace();

	void showJoystickWidget();

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
	std::unique_ptr<QAction> m_pZoomInAct;
	std::unique_ptr<QAction> m_pZoomOutAct;
	std::unique_ptr<QAction> m_pFullScreenAct;
	std::unique_ptr<QAction> m_pMoveAct;

	QAction *onLiveAct;
	QAction *onStopAct;

	QAction *show3DAct;
    std::unique_ptr<QAction> m_pSelectROI;
	std::unique_ptr<QAction> m_pSelect3DROI;

	QAction *showLightAct;
	QAction *showColorSpaceAct;

	QAction *showJoystick;

private:
	void setButtonsEnable(bool flag);
	void setLiveButtonEnable(bool flag);

private:
	Ui::VisionView ui;	
	VisionViewWidget* m_pViewWidget;

	QWidget* m_pLightWidget;
	QWidget* m_pColorWidget;
};
