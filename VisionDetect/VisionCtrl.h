#pragma once

#include <QObject>
#include <QVector>

#include "opencv/cv.h"
//using namespace cv;

#include "../DataModule/QProfileObj.h"

#include "../lib/VisionLibrary/include/VisionStruct.h"
using namespace AOI;

//---------------------------------------------------------------
typedef std::vector <std::vector<float>> QResultDataVec;
//---------------------------------------------------------------

class QVLCellEditor;
class QVLProfileEditor;
class QDetectObj;
Q_DECL_EXPORT class VisionCtrl : public QObject
{
	Q_OBJECT

public:
	VisionCtrl(QObject *parent=NULL);
	~VisionCtrl();

public:
	//QVLCellEditor* getCellTmpEditor();
	QVLCellEditor* getCellObjEditor();
	QVLProfileEditor* getProfileEditor();

	bool loadCmdData(int nStation);
	bool calculate3DHeight(int nStation, QVector<cv::Mat>& imageMats, cv::Mat& heightMat, cv::Mat& matHeightResultImg);
	bool generateAverageImage(const QVector<cv::Mat>& imageMats, cv::Mat& grayMat);
	bool matchPosition(cv::Mat& matDisplay, QVector<QDetectObj*>& objTests);
	bool calculateDetectHeight(cv::Mat& matHeight, QVector<QDetectObj*>& objTests);
    bool merge3DHeight(QVector<cv::Mat>& matHeights, cv::Mat& matHeight, cv::Point2f& ptFramePos);
	bool mergeImage(QVector<cv::Mat>& matInputImages, QVector<cv::Mat>& matOutputImages);

	bool matchAlignment(cv::Mat& matDisplay, QVector<QProfileObj*>& objProfTests);
	bool calculateDetectProfile(cv::Mat& matHeight, QVector<QProfileObj*>& objProfTests);

	bool calcPtToPtDist(cv::Point2d& pt1, cv::Point2d& pt2, QProfMeasureObj* pProfObj);
	bool calcPtToLineDist(cv::Point2d& ptOrg, bool bReversedFit, double fSlope, double fIntercept, QProfMeasureObj* pProfObj);
	bool calcTwoLineDist(Vision::PR_Line2f& stLine1, Vision::PR_Line2f& stLine2, QProfMeasLnToLnDist* pProfObj);
	bool calcTwoLineAngle(Vision::PR_Line2f& stLine1, Vision::PR_Line2f& stLine2, QProfMeasLnToLnAngle* pProfObj);
	bool fitLine(QVector<cv::Point2d>& profDatas, int nProfDataIndex1, int nProfDataIndex2, Vision::PR_Line2f& stLine);
	bool fitLine(QVector<cv::Point2d>& profDatas, int nProfDataIndex1, int nProfDataIndex2, bool& bReversedFit, double& fSlope, double& fIntercept);
	bool fitCircle(QVector<cv::Point2d>& profDatas, int nProfDataIndex1, int nProfDataIndex2, cv::Point2f& ptCirCenter, double& dRadius);
	bool CrossSectionArea(QVector<cv::Point2d>& profDatas, int nProfDataIndex1, int nProfDataIndex2, double& dArea);
	bool CrossSectionProfLength(QVector<cv::Point2d>& profDatas, int nProfDataIndex1, int nProfDataIndex2, double& dLength);
    QString getVisionLibraryVersion() const { return m_strVisionLibraryVersion; }

private:
	cv::Mat drawHeightGray(const cv::Mat &matHeight);

	void addImageText(cv::Mat image, cv::Point ptPos, QString szText);
    bool calculateDetectProfileMeas(QProfileObj* pObj, QProfMeasureObj*  pMeasObj);
	void cutingUsingBilinearInterpolation(cv::Mat& matHeight, double posX1, double posY1, double posX2, double posY2, QVector<cv::Point2d>& profDatas);

	QVLCellEditor *m_pVLCellObjEditor;
	//QVLCellEditor *m_pVLCellTmpEditor;
	QVLProfileEditor *m_pVLProflieEditor;	
	//QVector<cv::Mat> m_matBaseSurfaces;
	QVector<AOI::Vision::PR_CALC_3D_HEIGHT_CMD> m_stCalcHeightCmds;
    QString         m_strVisionLibraryVersion;
};
