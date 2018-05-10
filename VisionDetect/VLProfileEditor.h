#pragma once

#include <QMainWindow>
#include "ui_VLProfileEditor.h"

#include "opencv/cv.h"
#include <qstandarditemmodel>

//using namespace cv;

//#include "../lib/VisionLibrary/include/VisionAPI.h"
//using namespace AOI;

class VisionCtrl;
class QProfileObj;
class QCustomPlot;
class QProfileView;
class QVLProfileEditor : public QMainWindow
{
	Q_OBJECT

public:
	QVLProfileEditor(VisionCtrl* pCtrl, QWidget *parent = Q_NULLPTR);
	~QVLProfileEditor();

protected:
	void closeEvent(QCloseEvent *e);

private slots:
	void loadFile();
	void saveFile();

	void onLn2LnDist();
	void onPt2LnDist();
	void onPt2PtDist();
	void onPtHeight();
	void onCir2CirDist();
	void onCir2LnDist();
	void onCir2PtDist();
	void onCrRadious();
	void onProfArea();
	void onProfLength();

	void onLn2LnAngle();

	void onProfileIndexChanged(int iIndex);
	void onAddProfile();
	void onDeleteProfile();

	void onMeasureIndexChanged(int iIndex);
	void onAddMeasure();
	void onDeleteMeasure();

private:
	//bool fitLine(int nProfDataIndex1, int nProfDataIndex2, Vision::PR_Line2f& stLine);

	void captureProfDataImage(cv::Mat& matImg);

	void updateMeasure(int iIndex);

	void loadProfileIndex(int nIndex);
	void loadProfileMeasIndex(int iIndex);
	void displayObj();	
	void displayObjMeasure();

private:
	void init();
	void createActions();
	void createToolBars();
	void createStatusBar();

	QMenu *fileMenu;
	QMenu *editMenu;

	QToolBar *fileToolBar;
	QToolBar *editToolBar;

	QAction *loadAct;
	QAction *saveAct;
	QAction *line2lineAct;
	QAction *point2lineAct;
	QAction *point2pointAct;
	QAction *pointHeightAct;
	QAction *circle2circleAct;
	QAction *circle2lineAct;
	QAction *circle2pointAct;
	QAction *arcRadiusAct;
	QAction *line2lineAngleAct;
	//QAction *fitlineAngleAct;
	QAction *squareAreaAct;
	QAction *profileLengthAct;

public:
	void setHeightData(const cv::Mat& matHeight);
	void setProfGrayImg(cv::Mat& imgGray);

	void setProf3DImg(QImage& img3D);
	void setProfData(QVector<cv::Point2d>& profDatas);
	void setProfRange(double x1, double y1, double x2, double y2);

	void prepareNewProf();

private:
	int convetToProfileDataIndex(int mousePos);

	void setImagGray(cv::Mat& imgGray);
	void setImag3D(QImage& image);
	void setCutLine();
	void setProfileData();

private:
	void setupLineChartDemo(QCustomPlot *customPlot);
	void setupSincScatterDemo(QCustomPlot *customPlot);

private:
	void saveDataBase();
	void loadDataBase();

	void updateProfileList();
	void updateProfMeasList();

private:
	Ui::QVLProfileEditor ui;
	VisionCtrl* m_pCtrl;

	QCustomPlot* m_pPlotProfile;
	QVector<cv::Point> m_plotSelects;
	QVector<int> m_plotSelectTypes;
	QVector<cv::Point> m_plotSelectValues;
	QProfileView* m_pProfView;

	QStandardItemModel m_modelProf;
	QStandardItemModel m_modelMeasure;

	QGraphicsScene * m_imgGrayScene;
	QGraphicsScene * m_img3DScene;

	cv::Mat m_3DMatHeight;
	cv::Mat m_imgGray;

	cv::Mat m_imgGrayCutting;
	QImage m_img3D;
	QVector<cv::Point2d> m_profDatas;
	cv::Point2d m_startPt;
	cv::Point2d m_endPt;

	QProfileObj* m_curObj;
};
