#ifndef _TOWERLIGHT_H_
#define _TOWERLIGHT_H_

enum LightMode
{
    MODE_IDLE,
    MODE_RUNNING,
    MODE_WARNING,
    MODE_ERROR,
    MODE_NULL
};

class ITowerLight
{
public:	
	// 设置调试功能
	virtual bool triggerLight(bool bRead, bool bYellow, bool bGreen, bool bBuzzer) = 0;		//设置显示Tower Light的灯光和蜂鸣器
	virtual bool stopAllLight() = 0;	                                                    //关闭所有灯光和蜂鸣器	

    // 设置当前亮灯状态
    virtual bool setLightMode(LightMode mode = MODE_IDLE) = 0;                              //设置三色灯模式，不同的模式支持不同的灯光                     
};

#endif