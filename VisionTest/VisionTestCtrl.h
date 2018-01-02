#pragma once

#include <QObject>
#include <BaseTestCase.h>

class VisionTestCtrl : public QObject
{
	Q_OBJECT

public:
	VisionTestCtrl(QObject *parent = NULL);
	~VisionTestCtrl();
};
