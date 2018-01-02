#pragma once

#include <QWidget>
#include "ui_DataEditor.h"

class DataEditor : public QWidget
{
	Q_OBJECT

public:
	DataEditor(QWidget *parent = Q_NULLPTR);
	~DataEditor();

private:
	Ui::DataEditor ui;
};
