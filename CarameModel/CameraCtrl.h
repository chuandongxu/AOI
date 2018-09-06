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
    CameraDevice *getCamera(int n);

private:
    CameraDeviceList    m_CameraList; 
};
