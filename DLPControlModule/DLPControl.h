#pragma once

#include <QObject>
#include "mainwindow.h"
#include <QVector>

class DLPControl : public QObject
{
	Q_OBJECT

public:
	DLPControl(QObject *parent=NULL);
	~DLPControl();	

	void applyDefaultSolution(int indDLP);

	QWidget* getMainWin(int indDLP);

	int getDLPNum();
	void setDLPSerialNumber(int indDLP, QString& szSerialNumber);
	QString getDLPSerialNumber(int indDLP);

	bool startUpCapture(int indDLP);
	bool endUpCapture(int indDLP);

	bool isConnected(int indDLP);
	bool trigger(int indDLP);

private:
	QVector<MainWindow*> p_mainWins;
};
