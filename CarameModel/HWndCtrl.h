#pragma once
#include <QMessageBox>
#include <QGraphicsView>
#include "GraphicsContext.h"
#include "HObjectEntry.h"
#include "qlineedit.h"
#include <QLabel>

#include "opencv/cv.h"

using namespace cv;

/// <summary>
/// This class works as a wrapper class for the HALCON window
/// HWindow. HWndCtrl is in charge of the visualization.
/// You can move and zoom the visible image part by using GUI component 
/// inputs or with the mouse. The class HWndCtrl uses a graphics stack 
/// to manage the iconic objects for the display. Each object is linked 
/// to a graphical context, which determines how the object is to be drawn.
/// The context can be changed by calling changeGraphicSettings().
/// The graphical "modes" are defined by the class GraphicsContext and 
/// map most of the dev_set_* operators provided in HDevelop.
/// </summary>

// modeView
const int MODE_VIEW_NONE        = 10;
const int MODE_VIEW_ZOOM_IN     = 11;
const int MODE_VIEW_ZOOM_OUT    = 15;
const int MODE_VIEW_MOVE        = 12;
const int MODE_VIEW_ZOOMWINDOW	= 13;

// for ROI
const int MODE_INCLUDE_ROI     = 1;
const int MODE_EXCLUDE_ROI     = 2;

// max Num
const int MAXNUMOBJLIST       = 40;

class HWndCtrl :public QWidget
{

public:
	explicit HWndCtrl(QWidget *parent);
	~HWndCtrl(void);

private:
	QLabel* hv_windowHandle; // Halcon ����

public:
	/// <summary>Error message when an exception is thrown</summary>
	std::string  exceptionText;
	double  zoomWndFactor;
	QRect   rectWindow;
private:

	int    stateView;
	//bool   mousePressed = false;
	bool   mouseLeftPressed;
	bool   mouseRightPressed;
	double startX, startY;

	/* dispROI is a flag to know when to add the ROI models to the
	   paint routine and whether or not to respond to mouse events for
	   ROI objects */
	int           dispROI;

	/* Basic parameters, like dimension of window and displayed image part */
	int   windowWidth;
	int   windowHeight;
	int   imageWidth;
	int   imageHeight;

	// 		 int[] CompRangeX;
	// 		 int[] CompRangeY;


	int    prevCompX, prevCompY;
	double stepSizeX, stepSizeY;


	/* Image coordinates, which describe the image part that is displayed
	   in the HALCON window */
	double ImgRow1, ImgCol1, ImgRow2, ImgCol2;

	//public double  zoomWndFactor;
	double  zoomAddOn;
	int     zoomWndSize;

	/// <summary> 
	/// List of HALCON objects to be drawn into the HALCON window. 
	/// The list shouldn't contain more than MAXNUMOBJLIST objects, 
	/// otherwise the first entry is removed from the list.
	/// </summary>
	QList<HObjectEntry> HObjList;

	/// <summary>
	/// Instance that describes the graphical context for the
	/// HALCON window. According on the graphical settings
	/// attached to each HALCON object, this graphical context list 
	/// is updated constantly.
	/// </summary>
	GraphicsContext	 mGC; // ��ǰ����״̬��ÿ������µĶ�����ͬʱ��ӵ�ǰ����


private:
	void initial(QRect rect);

	QDialog   *m_pWidgetValue;  // ��ʾ����ֵ
	QLineEdit *m_pPosEdit;
	QLineEdit *m_pValueEdit;
	void showPixelValue(double posX, double posY);
	bool m_bShowFlag;  // �Ƿ���ʾ����ֵ��־λ

	QWidget * m_parent;

protected:
	void mouseMoveEvent(QMouseEvent * event);
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent * event);

	void resizeEvent(QResizeEvent *event);
	void paintEvent(QPaintEvent *event);

public:
	void addObj(cv::Mat &obj, bool bImage = false);
	void resetAll(int flag = 0);
	void clearList();
	void setImagePart(int r1, int c1, int r2, int c2);
	void repaint();
	QImage cvMat2QImage(const cv::Mat& mat);
	cv::Mat QImage2cvMat(QImage image);
	void A_Transform(Mat& src, Mat& dst, int dx, int dy);
	void changeGraphicSettings(QString mode, QString val);
	void setViewState(int state);
	void zoomImage(double x, double y, double scale);
	void moveImage(double motionX, double motionY);
	void setHWindowSize(QRect rect = QRect());
	void setShowFlag(bool bShowFlag);
	QLabel* getHWindowHandle();

	int getImageWidth();
	int getImageHeight();
};

