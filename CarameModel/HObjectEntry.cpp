#include "HObjectEntry.h"

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

HObjectEntry::HObjectEntry(cv::Mat obj, QMap<QString, QString> gc)
{
	_Obj = obj;
	gContext = gc;
}

HObjectEntry::~HObjectEntry(void)
{
}

void HObjectEntry::clear()
{
	_Obj.release();
	gContext.clear();
}