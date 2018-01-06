#include <QtWidgets/QApplication>
#include <qthreadpool.h>
#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/VisionUI.h"
#include "../include/IdDefine.h"
#include "StartupWidget.h"
#include <QDebug>
#include <qtextcodec.h>
#include "../Common/translatormgr.h"
#include <QDateTime.h>
#include "../Common/CVSFile.h"

IVisionUI * InitGenVision()
{
	qDebug() << "InitGenVision";

	QMoudleMgr::loadPlugins();
	IVisionUI * pUI = getModule<IVisionUI>(UI_MODEL);
	
	//IVisionUI * pUI = getMainModule<IVisionUI>();

	return pUI;
}

void freeGenVision()
{
	//QMoudleMgr::removePlugin(UI_MODEL);
	//QMoudleMgr::removePlugin(CTRL_MODEL);
	//QMoudleMgr::removePlugin(VISION_MODEL);
	//QMoudleMgr::removePlugin(CTS_MODEL);
	//QMoudleMgr::removePlugin(CAN_MODEL);
	//QMoudleMgr::removePlugin(MOTION_MODEL);
	//QMoudleMgr::removePlugin(LIGHT_MODEL);
	//QMoudleMgr::removePlugin(ABBROBOT_MODEL);
	//QMoudleMgr::removePlugin(ROBOT_MODEL);
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	//QList<QByteArray> bbr = QTextCodec::availableCodecs();
	QTextCodec *code = QTextCodec::codecForName("utf-8");
	QTextCodec::setCodecForLocale(code);
	QString langPostfix = System->getLangConfig();
	QTranslatorMgr::changeLange(langPostfix);

	QSystem::loadQss("project.qss");	

	QFont font;
	font.setFamily("MS Shell Dlg 2");
	font.setPixelSize(12);
	a.setFont(font);

	QStartupWidget widget;
	if(QDialog::Rejected == widget.exec())
	{
		return -1;
	}

	QThreadPool * pIns = QThreadPool::globalInstance();
	if(pIns)pIns->setMaxThreadCount(5);

	//QString path = QApplication::applicationDirPath();
	//QDateTime dtm =  QDateTime::currentDateTime();
	//QString strdate = dtm.toString("yyyy-MM-dd-hh");
	//strdate += ".csv";
	//qUserRecord(path + "/record/" +strdate);
	
	int iRet = -1;
	IVisionUI * pUI = InitGenVision();
	if(pUI)
	{
		pUI->init();
		pUI->Show();		

		QSystem::closeMessage();
		iRet = a.exec();

		pUI->unInit();
	}
	QMoudleMgr::removePlugins();
	return iRet;
}
