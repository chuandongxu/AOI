#include "windows.h"
#include "tlhelp32.h"

#include "QSmartDetect.h"
#include <qthreadpool.h>
#include <QtWidgets/QApplication>
#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
//#include "../include/IVision.h"
#include "StartupWidget.h"
#include <QDebug>
#include <qtextcodec.h>
#include "../Common/translatormgr.h"
#include <QDateTime.h>
#include "../Common/CVSFile.h"

//IVision * InitGenVision()
//{
//	qDebug() << "InitGenVision";
//
//	QMoudleMgr::loadPlugins();
//	IVision * pUI = getModule<IVision>(VISION_MODEL);
//
//	return pUI;
//}

int KillProcess(PCZZWSTR processName, int processId)
{
	PROCESSENTRY32 pe;
	DWORD id = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hSnapshot, &pe))
	{
		return 0;
	}

	while (1)
	{
		pe.dwSize = sizeof(PROCESSENTRY32);
		if (Process32Next(hSnapshot, &pe) == FALSE)
		{
			break;
		}
		//find processName
		if (wcsicmp(pe.szExeFile, processName) == 0)
		{
			id = pe.th32ProcessID;
			break;
		}
	}
	CloseHandle(hSnapshot);
	//if(id == 0)
	//  return ;

	if (processId != id)
	{
		//Kill The Process
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, id);
		if (hProcess != NULL)
		{
			TerminateProcess(hProcess, 0);
			CloseHandle(hProcess);
		}
	}
	
	return 1;
}

int main(int argc, char *argv[])
{
	DWORD processId = GetCurrentProcessId();
	KillProcess(__TEXT("NovoDetect.exe"), processId);

    QApplication a(argc, argv);

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
	if (QDialog::Rejected == widget.exec())
	{
		return -1;
	}
	QThreadPool * pIns = QThreadPool::globalInstance();
	if (pIns)pIns->setMaxThreadCount(5);

	//IVision * pUI = InitGenVision();
	//if (!pUI) return -1;

	QMoudleMgr::loadPlugins();

    QSmartDetect w;
	w.setWindowFlags(w.windowFlags()& ~Qt::WindowMaximizeButtonHint);
	//w.showMaximized();
    w.show();

	QSystem::closeMessage();
	int iRet = a.exec();

	QMoudleMgr::removePlugins();

	return iRet;
}
