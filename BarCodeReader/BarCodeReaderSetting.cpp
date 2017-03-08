#include "BarCodeReaderSetting.h"
#include <QInputDialog>
#include "../Common/SystemData.h"
#include "../Common/translatormgr.h"
#include <QComboBox>
#include <qmessagebox.h>

//#define _TR(x) tr(QStringLiteral(x).toLocal8Bit().data())


QWidget * QBarSettingDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	if(index.column() == 1)
	{
		QComboBox * p = new QComboBox(parent);
		if(p)
		{
			QStringList ls;
			ls << "COM0"<<"COM1"<<"COM2"<<"COM3"<<"COM4"<<"COM5"<<"COM6"<<"COM7"<<"COM8"<<"COM9";
			p->addItems(ls);
			
			return p;
		}
	}
	else if(index.column() == 2)
	{
		QComboBox * p = new QComboBox(parent);
		if(p)
		{
			QStringList ls;
			ls << "1200"<<"2400"<<"9600"<<"19200"<<"115200";
			p->addItems(ls);

			return p;
		}
		
	}

	return NULL;
}

void QBarSettingDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
	if(index.column() == 1)
	{
		QComboBox * p = (QComboBox *)editor;
		if(p)
		{
			p->setCurrentText(index.data().toString());
		}
	}
	else if(index.column() == 2)
	{
		QComboBox * p = (QComboBox *)editor;
		if(p)
		{
			int bound = index.data().toInt();
			p->setCurrentText(QString::number(bound));
		}
	}
	else
	{
		QItemDelegate::setEditorData(editor,index);
	}
}

void QBarSettingDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
{
	if(index.column() == 1 )
	{
		QComboBox * p = (QComboBox *)editor;
		if(p)
		{
			QString port = p->currentText();
			model->setData(index,port);
		}
	}
	else if(index.column() == 2)
	{
		QComboBox * p = (QComboBox *)editor;
		if(p)
		{
			int bound = p->currentText().toInt();
			model->setData(index,bound);
		}
	}
	else
	{
		QItemDelegate::setModelData(editor, model,index);
	}
}

QBarCodeReaderSetting::QBarCodeReaderSetting(QBarCodeDevMgr * pDevMgr,QWidget *parent)
	: QWidget(parent),m_pBarCodeDevMgr(pDevMgr)
{
	ui.setupUi(this);

	ui.tableView->setModel(&m_model);
	ui.tableView->setItemDelegate(&m_delegate);

	this->loadData();

	connect(ui.pushButton,SIGNAL(clicked()),SLOT(addDevice()));
	connect(ui.pushButton_2,SIGNAL(clicked()),SLOT(removeDevice()));
	connect(ui.pushButton_3,SIGNAL(clicked()),SLOT(openDevice()));
	connect(ui.pushButton_4,SIGNAL(clicked()),SLOT(closeDevice()));
	connect(ui.pushButton_5,SIGNAL(clicked()),SLOT(readBarCode()));
	connect(ui.pushButton_7,SIGNAL(clicked()),SLOT(save()));

	QTranslatorMgr::initRetranslangSlot(this,SLOT(onChangeLange()));

	if(USER_LEVEL_TECH > System->getUserLevel())
	{
		ui.pushButton->setEnabled(false);
		ui.pushButton_2->setEnabled(false);		
	}

	QString barcodecounts = System->getParam(QStringLiteral("barcodecounts")).toString();
	ui.lineEdit_barcodecount->setText(barcodecounts);


}

QBarCodeReaderSetting::~QBarCodeReaderSetting()
{

}

void QBarCodeReaderSetting::addDevice()
{
	if(m_pBarCodeDevMgr)
	{
		//QString name = QInputDialog::getText(NULL,tr(QStringLiteral("输入").toLocal8Bit().data()),tr(QStringLiteral("请输入名称").toLocal8Bit().data()));
		QString name = QInputDialog::getText(NULL,QStringLiteral("输入"),QStringLiteral("请输入名称"));
		if(name.isEmpty())return;

		QBarCodeDevice * pDev = m_pBarCodeDevMgr->addDevice(name);
		if(pDev)
		{
			int n = m_model.rowCount();
			m_model.insertRow(n);
			m_model.setData(m_model.index(n,0),name);
			m_model.setData(m_model.index(n,1),pDev->getCommPort());
			m_model.setData(m_model.index(n,2),pDev->getBoundRate());
			m_model.setData(m_model.index(n,3),pDev->isOpen());
		}
	}
}

void QBarCodeReaderSetting::removeDevice()
{
	if(m_pBarCodeDevMgr)
	{
		int n = ui.tableView->currentIndex().row();
		m_pBarCodeDevMgr->removeDevice(n);

		

		QString name = m_model.data(m_model.index(n,0)).toString();
		QString port = m_model.data(m_model.index(n,1)).toString();
		int bound = m_model.data(m_model.index(n,2)).toInt();

		QString key = QString("barDev-name-%0").arg(name);
		System->delSysParam(key);

		key = QString("barDev-port-%0").arg(name);
		System->delSysParam(key);

		key = QString("barDev-boundRate-%0").arg(name);
		System->delSysParam(key);

		m_model.removeRow(n);
	}
}

void QBarCodeReaderSetting::openDevice()
{
	if(m_pBarCodeDevMgr)
	{
		int n = ui.tableView->currentIndex().row();
		if( n > -1)
		{
			QBarCodeDevice * pDev = m_pBarCodeDevMgr->getDevice(n);
			if(pDev)pDev->open();

			if(pDev->isOpen())m_model.setData(m_model.index(n,3),true);
			else m_model.setData(m_model.index(n,3),false);
		}
		else 
		{
			QMessageBox::warning(this,QStringLiteral("提示"),QStringLiteral("请选择条码设备"));
		}
	}
}

void QBarCodeReaderSetting::closeDevice()
{
	if(m_pBarCodeDevMgr)
	{
		int n = ui.tableView->currentIndex().row();
		if(n > -1)
		{
			QBarCodeDevice * pDev = m_pBarCodeDevMgr->getDevice(n);
			if(pDev)pDev->close();

			if(pDev->isOpen())m_model.setData(m_model.index(n,3),true);
			else m_model.setData(m_model.index(n,3),false);
		}
		else 
		{
			QMessageBox::warning(this,QStringLiteral("提示"),QStringLiteral("请选择条码设备"));
		}
	}
}

void QBarCodeReaderSetting::readBarCode()
{
	if(m_pBarCodeDevMgr)
	{
		int n = ui.tableView->currentIndex().row();
		if( n > -1)
		{
			QBarCodeDevice * pDev = m_pBarCodeDevMgr->getDevice(n);
			if(pDev)
			{
				QString str = pDev->readBarCode();
				int n = m_model.setData(m_model.index(0,4),str);
			}
		}
		else 
		{
			QMessageBox::warning(this,QStringLiteral("提示"),QStringLiteral("请选择条码设备"));
		}
	}
}

void QBarCodeReaderSetting::save()
{
	int n = m_model.rowCount();
	for(int i=0; i<n; i++)
	{
		QString name = m_model.data(m_model.index(i,0)).toString();
		QString port = m_model.data(m_model.index(i,1)).toString();
		int bound = m_model.data(m_model.index(i,2)).toInt();

		QString key = QString("barDev-name-%0").arg(name);
		System->setSysParam(key,name);

		key = QString("barDev-port-%0").arg(name);
		System->setSysParam(key,port);

		key = QString("barDev-boundRate-%0").arg(name);
		System->setSysParam(key,bound);

		QBarCodeDevice * pDev = m_pBarCodeDevMgr->getDevice(i);
		if(pDev)
		{
			pDev->setName(name);
			pDev->setCommPort(port);
			pDev->setBoundRate(bound);
		}
		QString barcodecounts = ui.lineEdit_barcodecount->text();
		System->setParam(QStringLiteral("barcodecounts"),QVariant(barcodecounts).toInt());
	}
}

void QBarCodeReaderSetting::onChangeLange()
{
	this->loadData();
}

void QBarCodeReaderSetting::loadData()
{
	if(m_pBarCodeDevMgr)
	{
		m_model.clear();
		QStringList ls;
		ls << QStringLiteral("名称") << QStringLiteral("端口号") << QStringLiteral("波特率")
			<< QStringLiteral("打卡状态") << QStringLiteral("条码");
		//ls << tr("name") << tr("port") << tr("bound")
		//	<< tr("state") << tr("barcode");
		m_model.setHorizontalHeaderLabels(ls);

		int n = m_pBarCodeDevMgr->getDevcount();
		for(int i=0; i<n; i++)
		{
			QBarCodeDevice * pDev = m_pBarCodeDevMgr->getDevice(i);
			if(pDev)
			{
				QString name = pDev->getDevName();
				QString port = pDev->getCommPort();
				int bound = pDev->getBoundRate();
				bool isOpen = pDev->isOpen();

				m_model.insertRow(i);
				m_model.setData(m_model.index(i,0),name);
				m_model.setData(m_model.index(i,1),port);
				m_model.setData(m_model.index(i,2),bound);
				m_model.setData(m_model.index(i,3),isOpen);
			}
		}
	}
}

//QString QBarCodeReaderSetting::tr(const QString & str)
//{
//	return QObject::tr(str.toLocal8Bit().data());
//}