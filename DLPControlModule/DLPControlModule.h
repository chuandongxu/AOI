#pragma once

#include "../Common/modulebase.h"
#include "../include/IDlp.h"
#include "DLPControl.h"

class DLPControlModule : public QModuleBase, public IDlp
{
public:
	DLPControlModule(int id, const QString &name);
	~DLPControlModule();
	
	virtual void addSettingWiddget(QTabWidget * tabWidget);

public:
	int getDLPNum();
	bool startUpCapture(int indDLP);
	bool endUpCapture(int indDLP);

	bool isConnected(int indDLP);
	bool trigger(int indDLP);

private:
	DLPControl m_ctrl;	
};
