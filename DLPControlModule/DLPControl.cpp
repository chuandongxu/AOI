#include "DLPControl.h"
#include "dlpcontrolmodule_global.h"

DLPControl::DLPControl(QObject *parent)
	: QObject(parent)
{	
	for (int i = 0; i < STATION_COUNT; i++)
	{
		MainWindow* mainWin = new MainWindow();
		mainWin->setDLP(i);
		p_mainWins.push_back(mainWin);
	}	
}

DLPControl::~DLPControl()
{	
	//delete p_mainWin;
	//for (int i = 0; i < STATION_COUNT; i++)
	//{		
	//	delete p_mainWins[i];
	//}
}

void DLPControl::applyDefaultSolution(int indDLP)
{
	if (indDLP >= 0 && indDLP < p_mainWins.size())
	{
		p_mainWins[indDLP]->ApplyDefaultSolution();
	}
}


QWidget* DLPControl::getMainWin(int indDLP)
{	
	if (indDLP >= 0 && indDLP < p_mainWins.size())
	{
		return p_mainWins[indDLP];
	}
	return NULL;
}

int DLPControl::getDLPNum()
{
	return p_mainWins.size();
}

void DLPControl::setDLPSerialNumber(int indDLP, QString& szSerialNumber)
{
	if (indDLP >= 0 && indDLP < p_mainWins.size())
	{		
		p_mainWins[indDLP]->setUSBSerialNB(szSerialNumber.toStdString());
	}
}

QString DLPControl::getDLPSerialNumber(int indDLP)
{
	if (indDLP >= 0 && indDLP < p_mainWins.size())
	{
		return p_mainWins[indDLP]->getUSBSerialNB().c_str();
	}

	return "";
}

bool DLPControl::startUpCapture(int indDLP)
{
	if (indDLP >= 0 && indDLP < p_mainWins.size())
	{
		return p_mainWins[indDLP]->startUpCapture();
	}

	return false;
}

bool DLPControl::endUpCapture(int indDLP)
{
	if (indDLP >= 0 && indDLP < p_mainWins.size())
	{
		p_mainWins[indDLP]->endUpCapture();
	}

	return false;
}

bool DLPControl::isConnected(int indDLP)
{
	if (indDLP >= 0 && indDLP < p_mainWins.size())
	{
		return p_mainWins[indDLP]->isConnected();
	}

	return false;
}

bool DLPControl::trigger(int indDLP)
{
	if (indDLP >= 0 && indDLP < p_mainWins.size())
	{
		return p_mainWins[indDLP]->trigger();
	}

	return false;
}
