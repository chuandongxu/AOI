#include "statewidget.h"
#include "ui_statewidget.h"
#include "../Common//SystemData.h"
#include <qmessagebox>
#include "../Common/eos.h"
#include "../include/IdDefine.h"

QStateWidget::QStateWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QStateWidget)
{
    ui->setupUi(this);
	this->startTimer(100);

	int n = System->getSysParam("OK_COUNT_STATION1").toInt();
	int m = System->getSysParam("NG_COUNT_STATION1").toInt();
	ui->lineEdit_9->setText(QString::number(n));
	ui->lineEdit_10->setText(QString::number(m));

	n = System->getSysParam("OK_COUNT_STATION2").toInt();
    m = System->getSysParam("NG_COUNT_STATION2").toInt();
	ui->lineEdit_11->setText(QString::number(n));
	ui->lineEdit_12->setText(QString::number(m));	

	double ok_precent = System->getParam("SYS_OK_PRECENT").toDouble();
	QString ok_precent_str= QString::number(ok_precent,'f',5);
	ui->lineEdit_22->setText(ok_precent_str);
	int factory_sum =  System->getParam("SYS_FACTORY_SUM").toInt();
	ui->lineEdit_21->setText(QString::number(factory_sum));
	connect(ui->pushButton_2,SIGNAL( clicked()),SLOT(onClearProductCount()));

	QEos::Attach(EVENT_GOHOME_STATE,this,SLOT(onGoHomeState(const QVariantList &)));
	QEos::Attach(EVENT_GOHOMEING_STATE,this,SLOT(onIsGoHomeIng(const QVariantList &)));
	QEos::Attach(EVENT_RUN_STATE,this,SLOT(onRunState(const QVariantList &)));
	QEos::Attach(EVENT_CHECK_STATE,this,SLOT(onResoultEvent(const QVariantList &)));
}

QStateWidget::~QStateWidget()
{
    delete ui;
}

void QStateWidget::onClearProductCount()
{
	System->setSysParam("OK_COUNT_STATION1",0);
	System->setSysParam("NG_COUNT_STATION1",0);
	System->setSysParam("OK_COUNT_STATION2",0);
	System->setSysParam("NG_COUNT_STATION2",0);	

	ui->lineEdit_9->setText("0");
	ui->lineEdit_10->setText("0");
	ui->lineEdit_11->setText("0");
	ui->lineEdit_12->setText("0");	
	ui->lineEdit_21->setText("0");
	ui->lineEdit_22->setText("0");

}

void QStateWidget::onGoHomeState(const QVariantList &data)
{
	
	if(data.size() == 1)
	{
		int iState = data[0].toInt();
		if(GOHOME_STATE_OK == iState)
		{
			ui->label->setStyleSheet("background-color: rgb(50, 255, 50);");
			ui->label->setText(QStringLiteral("回零成功"));
		}
		else 
		{
			ui->label->setStyleSheet("background-color: rgb(255, 50, 50);");
			ui->label->setText(QStringLiteral("回零失败"));
		}
	}
}

void QStateWidget::onIsGoHomeIng(const QVariantList &data)
{
	if(data.size() == 1)
	{
		int iState = data[0].toInt();
		if(GOHOMEING_STATE_OK == iState)
		{
			ui->label->setStyleSheet("background-color: rgb(250, 250,0);");
			ui->label->setText(QStringLiteral("回零中..."));
		}
		else 
		{
			ui->label->setStyleSheet("background-color: rgb(255, 50, 50);");
			ui->label->setText(QStringLiteral("回零失败"));
		}
	}
}
void QStateWidget::onRunState(const QVariantList &data)
{
	if(data.size() == 1)
	{
		int iState = data[0].toInt();
		if(RUN_STATE_RUNING == iState)
		{
			ui->label_2->setStyleSheet("background-color: rgb(50, 255, 50);");
			ui->label_2->setText(QStringLiteral("正在运行"));
		}
		else 
		{
			ui->label_2->setStyleSheet("background-color: rgb(255, 170, 127);");
			ui->label_2->setText(QStringLiteral("已停止运行"));
		}
	}
}

void QStateWidget::timerEvent(QTimerEvent * event)
{
	updataStates();
	showErrorMessage();
}

void QStateWidget::updataStates()
{
	//检测驱动器状态
}

QString QStateWidget::getWorkState()
{
	return "";
}

QString QStateWidget::getRobtState()
{
	return "";
}

QString QStateWidget::getLeftStationState()
{

	return "";
}

QString QStateWidget::getRigthStationState()
{
	
	return "";
}

QString QStateWidget::getWorkMod()
{
	
	return "";
}

void QStateWidget::showErrorMessage()
{
	int code = System->getErrorCode();
	if(ERROR_NO_ERROR != code)
	{
		QString msg = System->getErrorString();

		QString str;
		if(msg.isEmpty())str = QString(QStringLiteral("未知确切含义错误 %0")).arg(code);
		else str = QString(QStringLiteral(" %0")).arg(msg);
		if(code < 0x30000000)
		{
			QSystem::showMessage(QStringLiteral("错误"),str,0);
		}
		else if(code < 0x40000000)
		{
			QSystem::showMessage(QStringLiteral("告警"),str);
		}
	}
}


void QStateWidget::onResoultEvent(const QVariantList &data)
{
	int iStation = data[0].toInt();
	int iEvent = data[1].toInt();
	if(iEvent != STATION_STATE_RESOULT)return;

	int iState = data[2].toInt();
	double PressureValue = data[3].toDouble();
	double LeakValue = data[4].toDouble();

	if(iStation >= 0 && iStation < 6)
	{
		QString strState;
		
		switch (iStation)
		{
		case  0:
			if(0 == iState)
			{
				int n = System->getSysParam("OK_COUNT_STATION1").toInt();
				n++;
				System->setSysParam("OK_COUNT_STATION1",n);
				ui->lineEdit_9->setText(QString::number(n));
			}
			else 
			{
				int n = System->getSysParam("NG_COUNT_STATION1").toInt();
				n++;
				System->setSysParam("NG_COUNT_STATION1",n);
				ui->lineEdit_10->setText(QString::number(n));
			}
			break;
		case  1:
			if(0 == iState)
			{
				int n = System->getSysParam("OK_COUNT_STATION2").toInt();
				n++;
				System->setSysParam("OK_COUNT_STATION2",n);
				ui->lineEdit_11->setText(QString::number(n));
			}
			else 
			{
				int n = System->getSysParam("NG_COUNT_STATION2").toInt();
				n++;
				System->setSysParam("NG_COUNT_STATION2",n);
				ui->lineEdit_12->setText(QString::number(n));
			}
			break;		
		default:
			break;
		}

		int sum = calSum();
		double ok_precent = calOkPrecent();
		ui->lineEdit_21->setText(QVariant(sum).toString());
		QString ok_precent_str= QString::number(ok_precent,'f',5);
		ui->lineEdit_22->setText(ok_precent_str);

		//int n = ui->lineEdit_11->text().toInt();
		//ui->lineEdit_11->setText(QString::number(++n));
	}
}

int  QStateWidget::calSum()
{
     int sum = 0;
	 for(int i = 1 ; i<=6 ;i++)
	 {
		 int n = System->getSysParam(QStringLiteral("NG_COUNT_STATION%0").arg((i))).toInt();
		 sum = sum+n;
		 int m = System->getSysParam(QStringLiteral("OK_COUNT_STATION%0").arg((i))).toInt();
		 sum = sum+m;
	 }
	 System->setParam("SYS_FACTORY_SUM",QVariant(sum));
	 return sum;
}
double  QStateWidget::calOkPrecent()
{
	double okPrecent = 0.0;
	double sum = 0;
	double ok_sum = 0;
	for(int i = 1 ; i<=6 ;i++)
	{
		int n = System->getSysParam(QStringLiteral("NG_COUNT_STATION%0").arg((i))).toDouble();
		sum = sum+n;
		int m = System->getSysParam(QStringLiteral("OK_COUNT_STATION%0").arg((i))).toDouble();
		sum = sum+m;
		ok_sum = ok_sum+m;
	}
	if(sum != 0)
	{
		okPrecent = ok_sum/sum;
	}
	System->setParam("SYS_OK_PRECENT",QVariant(okPrecent));
	return okPrecent;
}