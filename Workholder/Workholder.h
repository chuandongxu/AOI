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
    virtual bool IsBoardArrived() override;		//���ǰ���Ƿ��а��ӵ���
    virtual bool IsBoardDelivered() override;	//�������Ƿ������
    virtual bool IsBoardReady() override;		//�������Ƿ��ڼ��λ��

    virtual bool pullBoard(bool bWait) override;	//��ȡ��ǰ���ӣ����˶������λ��
    virtual bool pushBoard(bool bWait) override;	//���Ӽ����ɣ����

private:
    WorkCtrl m_ctrl;
};
