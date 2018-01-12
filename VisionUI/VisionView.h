#pragma once

#include <QMainWindow>
#include "ui_VisionView.h"

#include <QToolBar>
#include <QAction>
#include <QDockWidget>
#include <QMutex>
#include "VisionAPI.h"
#include "constants.h"

using namespace AOI::Vision;

class QDetectObj;
class DViewUtility;
class VisionView : public QMainWindow
{
	Q_OBJECT

public:
	VisionView(QWidget *parent = Q_NULLPTR);
	~VisionView();

public:
    void setViewState(VISION_VIEW_MODE state);
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
    void setDeviceWindows(const QVector<cv::RotatedRect> &vecWindows);
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
	double convertToImgX(double dMouseValue, bool bLen = false);
	double convertToImgY(double dMouseValue, bool bLen = false);
	double convertToMouseX(double dImgValue, bool bLen = false);
	double convertToMouseY(double dImgValue, bool bLen = false);

	void show3DView(cv::Rect& rectROI);

	void fullImage();
	void zoomImage(double scale);
	void moveImage(double motionX, double motionY);

	void setButtonsEnable(bool flag);
    void _zoomImageForDisplay(const cv::Mat &matInputImg, cv::Mat &matOutput);
    void _cutImageForDisplay(const cv::Mat &matInputImg, cv::Mat &matOutput);
    void _drawDeviceWindows(cv::Mat &matImg);
    void _calcMoveRange();
public:
	bool startUpCapture();
	bool endUpCapture();

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
	Ui::VisionView ui;

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
    QVector<cv::RotatedRect> m_vecDeviceWindows;
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
};
