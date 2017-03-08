#ifndef _VISION_UI_H_
#define _VISION_UI_H_

#include <qwidget.h>
#include <qstring.h>

class IVisionUI 
{
public:
	virtual void init() = 0;
	virtual void Show() = 0;
	virtual void unInit() = 0;

	virtual void setStateWidget(QWidget * stateWidget) = 0;
	virtual void setCneterwidget(QWidget * centWidget) = 0;
	virtual void setTitle(const QString & str,const QString &ver) = 0;
};

#endif