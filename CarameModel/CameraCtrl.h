#pragma once
#include "CameraDevice.h"
#include "QMap"

class CameraCtrl
{

public:
	CameraCtrl(void);
	~CameraCtrl(void);

	bool isBasler();
	bool isDalsa();
	bool isImage();

	bool initial();
	void unInit();

	int  getCameraCount();
	CameraDevice * getCamera( int n);	

	void setCrossEnable(bool bEnable);
	bool isCrossEnable();

	void setCrossGap(int nGapWidth);
	int getCrossGap();

private:
	CameraDeviceList	m_CameraList; 
	bool m_bCrossGap;
	int m_nCrossWidth;
};

