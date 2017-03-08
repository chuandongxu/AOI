#ifndef VISIONUI_H
#define VISIONUI_H

#include "visionui_global.h"
#include "../include/VisionUI.h"
#include "../Common/modulebase.h"

class QVisionUIModule : public IVisionUI,public QModuleBase
{
public:
	QVisionUIModule(int id,const QString &name);
	~QVisionUIModule();

public:
	virtual void init();
	virtual void Show();
	virtual void unInit();

	void setStateWidget(QWidget * stateWidget);
	void setCneterwidget(QWidget * centWidget);
	void setTitle(const QString & str,const QString &ver);

private:
	void * m_mainWidget;
};

#endif // VISIONUI_H
