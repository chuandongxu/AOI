#pragma once

#include "workholder_global.h"

#include "../Common/modulebase.h"
#include "../include/IWorkholder.h"
#include "WorkCtrl.h"

class Workholder : public QModuleBase, public IWorkHolder
{
public:
    Workholder(int id, const QString &name);
    ~Workholder();

    virtual void addSettingWiddget(QTabWidget * tabWidget);

public:
    virtual bool IsBoardArrived() override;		//检测前方是否有板子到达
    virtual bool IsBoardDelivered() override;	//检测板子是否发送完成
    virtual bool IsBoardReady() override;		//检测板子是否在检测位置

    virtual bool pullBoard(bool bWait) override;	//获取当前板子，并运动到检测位置
    virtual bool pushBoard(bool bWait) override;	//板子检测完成，清出

private:
    WorkCtrl m_ctrl;
};
