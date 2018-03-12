#pragma once

#include <QWidget>
#include "ui_QColorWeight.h"

#include "opencv/cv.h"
#include <memory>

enum GrayWeightMethodEm
{
	EM_MODE_PT_THRESHOLD = 0,
	EM_MODE_ONE_THRESHOLD,
	EM_MODE_TWO_THRESHOLD
};

struct stGrayWeightParams
{
	GrayWeightMethodEm _emMode;
	int _nRScale;
	int _nGScale;
	int _nBScale;
	int _nThreshold1;
	int _nThreshold2;
};

struct stColorSpaceParams
{
	int _nRThreshold;
	int _nTThreshold;
};


class QCustomPlot;
class QCPBars;
class QColorWeight : public QWidget
{
	Q_OBJECT

public:
	QColorWeight(QWidget *parent = Q_NULLPTR);
	~QColorWeight();

protected:
	//void closeEvent(QCloseEvent *e);

public:
	void setImage(cv::Mat& img);

	// Gray Weight
	void setGrayParams(const stGrayWeightParams& grayParams);
	stGrayWeightParams getGrayParams();
	cv::Mat generateGrayImage(cv::Point ptPos);

	// Color Space
	void setColorParams(const stColorSpaceParams& colorParams);
	stColorSpaceParams getColorParams();
	cv::Mat generateColorImage(cv::Point ptPos);

private:
	void initUI();
	void initData();

	void loadConfig();
	void saveConfig();

	void setupDateDemo(std::shared_ptr<QCustomPlot> customPlot);
	int calcGrayValue(cv::Scalar& pixel);
	void generateGrayPlot();

	void setupDateColor(std::shared_ptr<QCustomPlot> customPlot, int nColorIndex);
	void generateColorPlot();

private:
	void clearGrayData();
	int getGrayValue(int nGrayLevel);	
	int incrementGrayValue(int nGrayLevel);

	void clearColorData(int nColorIndex);
	int getColorValue(int nColorIndex, int nGrayLevel);
	int incrementColorValue(int nColorIndex, int nGrayLevel);

	void displaySourceImg();
	void displayGrayImg();
	void displayColorImg();

private slots:
	void onGrayModeIndexChanged(int iIndex);

	void onCheckBoxEnableR(int iState);
	void onCheckBoxEnableG(int iState);
	void onCheckBoxEnableB(int iState);

	void onGrayLeftSliderChanged(int i);
	void onGrayRightSliderChanged(int i);

	void onGrayRSliderChanged(int i);
	void onGrayGSliderChanged(int i);
	void onGrayBSliderChanged(int i);

	void onColorRnSliderChanged(int i);
	void onColorTnSliderChanged(int i);

	void onLoadParams();
	void onSaveParams();

private:
	cv::Mat generateColorRange(int nRn, int nTn, cv::Mat& matImage);
	cv::Mat generateColorTrig(int nWidth, int nHeight, int nMinR, int nMaxR, int nMinG, int nMaxG, int nMinB, int nMaxB);
	bool calcTwoLineIntersect(cv::Point2f pt1, cv::Point2f pt2, cv::Point2f pt3, cv::Point2f pt4, cv::Point2f& ptIntersect);
	bool calcTwoLineIntersect(double k1, double b1, double k2, double b2, cv::Point2f& ptIntersect);

private:
	Ui::QColorWeight ui;

	// Gray Weight
	std::shared_ptr<QCustomPlot> m_customPlot;
	cv::Mat m_imageMat;
	cv::Mat m_maskMat;
	QMap<int, int> m_grayHitDatas;

	int m_nGrayLevelThreshold1;
	int m_nGrayLevelThreshold2;

	QGraphicsScene * m_grayScene;
	QGraphicsScene * m_grayColorScene;

	cv::Point m_grayGenPt;

	// Color Space
	cv::Point m_colorGenPt;
	std::shared_ptr<QCustomPlot> m_customPlotR;
	std::shared_ptr<QCustomPlot> m_customPlotG;
	std::shared_ptr<QCustomPlot> m_customPlotB;
	std::shared_ptr<QCustomPlot> m_customPlotGray;
	QMap<int, int> m_colorRHitDatas;
	QMap<int, int> m_colorGHitDatas;
	QMap<int, int> m_colorBHitDatas;
	QMap<int, int> m_colorGrayHitDatas;
	uchar m_maxR;
	uchar m_minR;
	uchar m_maxG;
	uchar m_minG;
	uchar m_maxB;
	uchar m_minB;
	uchar m_maxT;
	uchar m_minT;

	// Display
	QGraphicsScene * m_sourceImgScene;
	QGraphicsScene * m_grayImgScene;
	QGraphicsScene * m_colorImgScene;
};
