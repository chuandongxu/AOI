#include <qdatetime.h>
#include "bottonwidget.h"
#include <qheaderview.h>
#include "../Common/SystemData.h"
#include "../include/IdDefine.h"
#include "../Common/eos.h"
#include "../include/IFlowCtrl.h"
#include "../Common/ModuleMgr.h"
#include "../include/IData.h"
#include <QVBoxLayout>

//----------------------------------------------------------------
/*
QBottomModel QBottonWidget::m_model;

void QBottonWidget::initBottomModel()
	QStringList ls;
	ls << "" << QStringLiteral("时间")  << QStringLiteral("运行信息");

	m_model.setHorizontalHeaderLabels(ls);

	QSystem * p = QSystem::instance();
	if(p)
	{
		connect(p,SIGNAL(errorInfo(const QString &,const QString&,unsigned int)),
			&m_model,SLOT(onErrorInfo(const QString &,const QString&,unsigned int)));
	}
}
*/
//----------------------------------------------------------------
QBottonWidget::QBottonWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	//this->setFixedHeight(400);
	QErrorModel * model = System->getErrorModel();
	if(!model)return;

	ui.treeView->setModel(model);
	ui.treeView_2->setModel(&m_checkModel);

	IData * pData = getModule<IData>(DATA_MODEL);
	if (pData)
	{
		m_widgetEdit = pData->getDataEditor();		
		ui.scrollArea->setWidget(m_widgetEdit);
		ui.treeView_2->setVisible(false);		
	}

	QHeaderView * header = ui.treeView->header();
	header->resizeSection(0,50);
	//header->resizeSection(1,130);
	header->setSectionResizeMode(QHeaderView::ResizeToContents);
	header->setStretchLastSection(false);

	IFlowCtrl * p = getModule<IFlowCtrl>(CTRL_MODEL);
	if(!p)return;

	p->InitResoultItemModel(&m_checkModel);

	ui.treeView_2->setEditTriggers(QAbstractItemView::NoEditTriggers);
	header = ui.treeView_2->header();
	QList<int> wdls = p->getResoultItemHeaderWidth();
	for(int i=0; i<wdls.size(); i++)
	{
		if(-1 == wdls[0])header->setStretchLastSection(true);
		else header->resizeSection(i,wdls[i]);
	}
	
	connect(model,SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)),
		this,SLOT(onDataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)));
	
	QEos::Attach(EVENT_CHECK_STATE,this,SLOT(onResoultEvent(const QVariantList &)));
	QEos::Attach(EVENT_RUN_STATE, this, SLOT(onRunState(const QVariantList &)));
	//QEos::Attach(EVENT_HSG_TYPE,this,SLOT(onChangeModuleType(const QVariantList &)));
}

QBottonWidget::~QBottonWidget()
{

}

void QBottonWidget::checkInitState()
{
	
}

void QBottonWidget::resizeEvent(QResizeEvent * event)
{
	QSize size = event->size();
	int width = (size.width() - 20)/2;
	int height = size.height() - 5;

	ui.treeView->setGeometry(5,5,width,height);
	ui.treeView_2->setGeometry(15+width,5,width,height);

	ui.scrollArea->setGeometry(ui.treeView_2->geometry());
}

void QBottonWidget::onDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> & roles)
{
	QAbstractItemModel * model = ui.treeView->model();
	if(!model)return;

	int row = model->rowCount();
	ui.treeView->scrollTo(model->index(row-1,1),QAbstractItemView::PositionAtBottom);

}

/*
void QBottomModel::onErrorInfo(const QString &data,const QString &msg,unsigned int level)
{
	int row = rowCount();
	if(row > 30)
	{
		removeRow(0);
		row = rowCount();
	}
	
	unsigned int warringFlsg = (level & 0xf0000000) >> 28;
	QString path = QApplication::applicationDirPath();
	//if(level >= 0x40000000)path += "/image/tr_info.png";
	//else if(level >= 0x30000000)path += "/image/tr_warring.png";
	//else path += "/image/tr_error.png";

	if(0 == warringFlsg || 1 == warringFlsg) path += "/image/tr_error.png";
	else if(3 == warringFlsg || 5 == warringFlsg) path += "/image/tr_warring.png";
	else path += "/image/tr_info.png";
	
	QPixmap pixmap;
	pixmap.load(path);

	insertRow(row);
	setData(index(row,0),pixmap,Qt::DecorationRole);
	setData(index(row,1),data);
	setData(index(row,2),msg);
	//ui.treeView->scrollTo(m_model.index(row,0),QAbstractItemView::PositionAtBottom);
}
*/

void QBottonWidget::onRunState(const QVariantList &data)
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData)return;

	if (data.size() == 1)
	{
		int iState = data[0].toInt();
		if (RUN_STATE_RUNING == iState)
		{
			ui.scrollArea->setVisible(false);
			ui.treeView_2->setVisible(true);
		}
		else
		{
			ui.scrollArea->setVisible(true);
			ui.treeView_2->setVisible(false);			
		}
	}
}

void QBottonWidget::onResoultEvent(const QVariantList &data)
{
	if(data.size() < 7)return;

	int iStation = data[0].toInt();
	int iEvent = data[1].toInt();
	/*if(iEvent != STATION_STATE_RESOULT_SPLITS && iEvent != STATION_STATE_RESOULT_IO
		&& iEvent != STATION_STATE_RESOULT_ALL && iEvent != STATION_STATE_RESOULT_CAMERA
		&& iEvent != STATION_STATE_RESOULT_LED && iEvent != STATION_STATE_RESOULT_LOGO)return;*/

	int iState = data[2].toInt();
	
	QString strState;
	if (0 == iState)strState = "OK";
	else strState = "NG";

	if (iStation >= 0 && iStation < 6)
	{
		m_checkModel.setData(m_checkModel.index(iStation, 2), strState);
		//m_checkModel.setData(m_checkModel.index(iStation, 3), PressureValue);
		//m_checkModel.setData(m_checkModel.index(iStation, 4), LeakValue);
	}
}

void QBottonWidget::onChangeModuleType(const QVariantList &data)
{
	IFlowCtrl * p = getModule<IFlowCtrl>(CTRL_MODEL);
	if(!p)return;

	m_checkModel.clear();

	QHeaderView * header = ui.treeView->header();
	header->resizeSection(0,50);
	header->resizeSection(1,130);
	header->setStretchLastSection(true);

	p->InitResoultItemModel(&m_checkModel);

	header = ui.treeView_2->header();
	QList<int> wdls = p->getResoultItemHeaderWidth();
	for(int i=0; i<wdls.size(); i++)
	{
		if(-1 == wdls[0])header->setStretchLastSection(true);
		else header->resizeSection(i,wdls[i]);
	}
}