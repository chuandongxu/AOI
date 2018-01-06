#pragma once
#include <QMap>
#include <QString>

#include "opencv/cv.h"

#define ToInt(value)                (static_cast<int>(value))

class HObjectEntry
{
public:
	HObjectEntry(cv::Mat obj);
	~HObjectEntry(void);

public:
	cv::Mat _Obj;

public:
	void clear();
};

