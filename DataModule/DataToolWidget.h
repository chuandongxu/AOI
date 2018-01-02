#pragma once

#include <QWidget>
#include "ui_DataToolWidget.h"

#include "..\lib\qtbrowersproperty\qtpropertymanager.h"
#include "..\lib\qtbrowersproperty\qteditorfactory.h"
#include "..\lib\qtbrowersproperty\qttreepropertybrowser.h"
#include "..\lib\qtbrowersproperty\qtbuttonpropertybrowser.h"
#include "..\lib\qtbrowersproperty\qtgroupboxpropertybrowser.h"

class QPropertyWidget : public QWidget
{
	Q_OBJECT

public:
	QPropertyWidget(QWidget *parent = 0);
	~QPropertyWidget();
	
	virtual void setValue();

	protected slots:
	virtual void boolPropertyValueChanged(QtProperty* property);
	virtual void intPropertyValueChanged(QtProperty* property);
	virtual void doublePropertyValueChanged(QtProperty* property);
	virtual void rectPropertyValueChanged(QtProperty* property);
	virtual void enumPropertyValueChanged(QtProperty* property);

protected:	
	QMap<QString, QtProperty*> m_mapProperties;

	QtBoolPropertyManager *boolManager;
	QtIntPropertyManager *intManager;
	QtDoublePropertyManager *doubleManager;
	QtStringPropertyManager *stringManager;
	QtSizePropertyManager *sizeManager;
	QtRectPropertyManager *rectManager;
	QtSizePolicyPropertyManager *sizePolicyManager;
	QtEnumPropertyManager *enumManager;
	QtGroupPropertyManager *groupManager;

	QtAbstractPropertyBrowser *editor1;
};

class DataToolWidget : public QWidget
{
	Q_OBJECT

public:
	DataToolWidget(QWidget *parent = Q_NULLPTR);
	~DataToolWidget();

private:
	Ui::DataToolWidget ui;
	QPropertyWidget* m_propertyWidget;
};
