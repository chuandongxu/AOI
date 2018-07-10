#pragma once

#include <QWidget>
#include "ui_QColorWeight.h"

#include "opencv/cv.h"
#include <memory>
#include <string>
#include "../include/constants.h"

struct GrayWeightParams
{
	GRAY_WEIGHT_METHOD enMethod;
    bool bEnableR;
    bool bEnableG;
    bool bEnableB;
	int nRScale;
	int nGScale;
	int nBScale;
	int nThreshold1;
	int nThreshold2;
};

struct ColorSpaceParams
{
	int nRThreshold;
	int nTThreshold;
};

class QCustomPlot;
class QCPBars;
class ColorScene;
class QColorWeight : public QWidget
{
	Q_OBJECT

public:
	QColorWeight(QWidget *parent = Q_NULLPTR);
	~QColorWeight();

protected:
	//void closeEvent(QCloseEvent *e);

public:
	void setImage(const cv::Mat& img);
    cv::Mat getProcessedImage() const { return m_maskMat; }

	// Gray Weight
	void setGrayParams(const GrayWeightParams& grayParams);
	GrayWeightParams getGrayParams() const;
	cv::Mat generateGrayImage(cv::Point ptPos);

	// Color Space
	void setColorParams(const ColorSpaceParams& colorParams);
	ColorSpaceParams getColorParams() const;
	cv::Mat generateColorImage(cv::Point ptPos);
	void setColorImagePos(cv::Point ptMousePos);

    std::string getJsonFormattedParams() const;
    void setJsonFormattedParams(const std::string &jsonParams);

private:
	void initUI();
	void initData();

    void setupDateDemo(std::shared_ptr<QCustomPlot> customPlot, bool bCreate = false);
	int calcGrayValue(cv::Scalar& pixel);
    cv::Mat _transformColorImage(const cv::Mat &matInput);
    cv::Mat _convertToGrayImage();
	void generateGrayPlot();

    void setupDateColor(std::shared_ptr<QCustomPlot> customPlot, QCPBars *regen, int nColorIndex, bool bCreate = false);
	void generateColorPlot();
	void clearGrayData();
	int getGrayValue(int nGrayLevel);	
	int incrementGrayValue(int nGrayLevel);

	void clearColorData(int nColorIndex);
	int getColorValue(int nColorIndex, int nGrayLevel);
	int incrementColorValue(int nColorIndex, int nGrayLevel);

	void displaySourceImg();
	void displayGrayImg();
	void displayColorImg();

	cv::Mat generateColorRange(int nRn, int nTn, cv::Mat& matImage);
	cv::Mat generateColorTrig(int nWidth, int nHeight, int nMinR, int nMaxR, int nMinG, int nMaxG, int nMinB, int nMaxB);
	bool calcTwoLineIntersect(cv::Point2f pt1, cv::Point2f pt2, cv::Point2f pt3, cv::Point2f pt4, cv::Point2f& ptIntersect);
	bool calcTwoLineIntersect(double k1, double b1, double k2, double b2, cv::Point2f& ptIntersect);

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

    void onColorWidgetState(const QVariantList &data);

private:
	Ui::QColorWeight ui;

	// Gray Weight
    std::shared_ptr<QGraphicsScene> m_plotSeneGrayWeight;
	std::shared_ptr<QCustomPlot> m_customPlot;
	cv::Mat m_matSrcImage;
    cv::Mat m_matSrcTransform;
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
    QCPBars *m_regenR;
    QCPBars *m_regenG;
    QCPBars *m_regenB;
    QCPBars *m_regenGray;
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
	ColorScene * m_colorImgScene;

    std::shared_ptr<QGraphicsScene> m_plotSeneR;
    std::shared_ptr<QGraphicsScene> m_plotSeneG;
    std::shared_ptr<QGraphicsScene> m_plotSeneB;
    std::shared_ptr<QGraphicsScene> m_plotSeneGray;
};
