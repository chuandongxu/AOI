#pragma once

#include <QMap>
#include <QVariant>
#include <QVector>

#include "opencv/cv.h"

const int g_nImageStructDataNum = 5;
struct QImageStruct
{
public:
	QImageStruct()
	{
	}

	cv::Mat _img[g_nImageStructDataNum];
};

struct Q3DStructData
{
	Q3DStructData()
	{
		_bStartCapturing = false;
		_bCapturedDone = false;
	}

	void clear()
	{
		_bStartCapturing = false;
		_bCapturedDone = false;
		_3DMatHeight.release();
		_matImage.release();
		_srcImageMats.clear();
	}

	bool _bStartCapturing;
	bool _bCapturedDone;
	cv::Mat _3DMatHeight;
	cv::Mat _matImage;
	QVector<cv::Mat> _srcImageMats;
};

//---------------------------------------------------------------
typedef QMap<QString, QVariant> QCheckerParamMap;
typedef QList<Q3DStructData> QCheckerParamDataList;
typedef QMap<int, int> QCheckerPositionMap;