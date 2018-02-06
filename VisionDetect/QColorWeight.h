#pragma once

#include <QWidget>
#include "ui_QColorWeight.h"

#include "opencv/cv.h"

class QCustomPlot;
class QColorWeight : public QWidget
{
	Q_OBJECT

public:
	QColorWeight(QWidget *parent = Q_NULLPTR);
	~QColorWeight();

private:
	void initUI();
	void initData();

	void setupDateDemo(QCustomPlot *customPlot);
	int calcGrayValue(cv::Scalar& pixel);
	void generateGrayPlot();

private:
	void clearData();
	int getGrayValue(int nGrayLevel);
	void setGrayValue(int nGrayLevel, int nValue);
	int incrementGrayValue(int nGrayLevel);

public:
	void setImage(cv::Mat& img);
	cv::Mat generateGrayImage(cv::Point2f ptPos);
	cv::Mat generateColorImage(cv::Point2f ptPos);

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

private:
	cv::Mat generateColorTrig(int nWidth, int nHeight, int nMinR, int nMaxR, int nMinG, int nMaxG, int nMinB, int nMaxB);

private:
	Ui::QColorWeight ui;
	// Gray Weight
	QCustomPlot* m_customPlot;
	cv::Mat m_imageMat;
	QMap<int, int> m_grayHitDatas;

	int m_nGrayLevelThreshold1;
	int m_nGrayLevelThreshold2;

	QGraphicsScene * m_grayScene;

	// Color
	//QCustomPlot* m_customPlot;
	QGraphicsScene * m_grayColorScene;
};
