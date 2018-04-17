#ifndef _WORKHOLDER_H_
#define _WORKHOLDER_H_

class IWorkHolder
{
public:	
	// General Functions:
	virtual bool IsBoardArrived() = 0;		//检测前方是否有板子到达
	virtual bool IsBoardDelivered() = 0;	//检测板子是否发送完成
	virtual bool IsBoardReady() = 0;		//检测板子是否在检测位置

	virtual bool pullBoard(bool bWait) = 0;	//获取当前板子，并运动到检测位置
	virtual bool pushBoard(bool bWait) = 0;	//板子检测完成，清出
	//virtual void clearBorad() = 0;			//清空当前轨道
};

#endif