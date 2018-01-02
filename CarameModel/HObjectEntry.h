#pragma once
#include <QMap>
#include <QString>

#include "opencv/cv.h"

#define ToInt(value)                (static_cast<int>(value))

class HObjectEntry
{
public:
	HObjectEntry(cv::Mat obj, QMap<QString, QString> gc);
	~HObjectEntry(void);

public:
	/// <summary>Hashlist defining the graphical context for HObj</summary>
	QMap<QString, QString>	gContext;
	/// <summary>HALCON object</summary>
	cv::Mat _Obj;

public:
	void clear();
};

