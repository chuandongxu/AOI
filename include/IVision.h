#ifndef _VISION_DETECT_INTERFACE_H_
#define _VISION_DETECT_INTERFACE_H_

#include "../lib/VisionLibrary/include/opencv/cv.h"


class QDetectObj;
class QProfileObj;
class IVision
{
public:
	virtual QWidget* getDetectView() = 0;
	virtual QWidget* getCellEditorView() = 0;
	virtual QWidget* getCaliView() = 0;
    virtual QWidget* getInspWindowView() = 0;
	virtual QWidget* getColorWeightView() = 0;
    virtual QWidget* getInspect3DProfileView() = 0;

	virtual bool loadCmdData(int nStation) = 0;

	virtual bool setHeightData(const cv::Mat& matHeight) = 0;
	virtual bool setProfGrayImg(cv::Mat& imgGray) = 0;
	virtual bool setProf3DImg(QImage& img3D) = 0;
	virtual bool setProfData(QVector<cv::Point2d>& profDatas) = 0;
	virtual bool setProfRange(double x1, double y1, double x2, double y2) = 0;
	virtual void prepareNewProf() = 0;

	virtual bool calculate3DHeight(int nStation, QVector<cv::Mat>& imageMats, cv::Mat& heightMat, cv::Mat& matHeightResultImg) = 0;
	virtual bool generateAverageImage(const QVector<cv::Mat>& imageMats, cv::Mat& grayMat) = 0;
	virtual bool matchPosition(cv::Mat& matDisplay, QVector<QDetectObj*>& objTests) = 0;
	virtual bool calculateDetectHeight(cv::Mat& matHeight, QVector<QDetectObj*>& objTests) = 0;
    virtual bool merge3DHeight(QVector<cv::Mat>& matHeights, cv::Mat& matHeight, cv::Point2f& ptFramePos) = 0;
	virtual bool mergeImage(QVector<cv::Mat>& matInputImages, QVector<cv::Mat>& matOutputImages) = 0;

	virtual bool matchAlignment(cv::Mat& matDisplay, QVector<QProfileObj*>& objProfTests) = 0;
	virtual bool calculateDetectProfile(cv::Mat& matHeight, QVector<QProfileObj*>& objProfTests) = 0;

	virtual cv::Mat generateGrayImage(cv::Mat& img, cv::Point ptPos) = 0;
	virtual cv::Mat generateColorImage(cv::Mat& img, cv::Point ptPos) = 0;
    virtual void setColorWidgetImage(const cv::Mat &matImg) = 0;
    virtual cv::Mat getColorWidgetProcessedImage() = 0;
    virtual QString getVisionLibraryVersion() const = 0;


    virtual bool setInspect3DHeight(QVector<cv::Mat>& matHeights, int nRow, int nCol, int nTotalRow, int nTotalCol) = 0;
    virtual void inspect3DProfile(cv::Rect& rectROI) = 0;
};

#endif