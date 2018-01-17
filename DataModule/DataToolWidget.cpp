#include "DataToolWidget.h"

#include <QGridLayout>
#include <QLabel>
#include <QIcon>
#include <qdir.h>
#include <QScrollArea>
#include <QDebug>

QPropertyWidget::QPropertyWidget(QWidget *parent)
	: QWidget(parent)
{
	boolManager = new QtBoolPropertyManager(this);
	intManager = new QtIntPropertyManager(this);
	doubleManager = new QtDoublePropertyManager(this);
	stringManager = new QtStringPropertyManager(this);
	sizeManager = new QtSizePropertyManager(this);
	rectManager = new QtRectPropertyManager(this);
	sizePolicyManager = new QtSizePolicyPropertyManager(this);
	enumManager = new QtEnumPropertyManager(this);
	groupManager = new QtGroupPropertyManager(this);

	QtProperty *item0 = groupManager->addProperty(QStringLiteral("基本属性"));

	QtProperty *item1 = stringManager->addProperty("objectName");
	item0->addSubProperty(item1);

	QtProperty *item2 = boolManager->addProperty("enabled");
	item0->addSubProperty(item2);
	//m_mapProperties.insert("enabled", item2);

	QtProperty *item3 = rectManager->addProperty("geometry");
	item0->addSubProperty(item3);

	QtProperty *item4 = sizePolicyManager->addProperty("sizePolicy");
	item0->addSubProperty(item4);

	QtProperty *item5 = sizeManager->addProperty("sizeIncrement");
	item0->addSubProperty(item5);

	QtProperty *item7 = boolManager->addProperty("mouseTracking");
	item0->addSubProperty(item7);

	QtProperty *item8 = enumManager->addProperty("direction");
	QStringList enumNames;
	enumNames << "Up" << "Right" << "Down" << "Left";
	enumManager->setEnumNames(item8, enumNames);
	QMap<int, QIcon> enumIcons;
	enumIcons[0] = QIcon(":/demo/images/up.png");
	enumIcons[1] = QIcon(":/demo/images/right.png");
	enumIcons[2] = QIcon(":/demo/images/down.png");
	enumIcons[3] = QIcon(":/demo/images/left.png");
	enumManager->setEnumIcons(item8, enumIcons);
	item0->addSubProperty(item8);		

	QtCheckBoxFactory *checkBoxFactory = new QtCheckBoxFactory(this);
	QtSpinBoxFactory *spinBoxFactory = new QtSpinBoxFactory(this);
	QtDoubleSpinBoxFactory *doublespinBoxFactory = new QtDoubleSpinBoxFactory(this);
	QtSliderFactory *sliderFactory = new QtSliderFactory(this);
	QtScrollBarFactory *scrollBarFactory = new QtScrollBarFactory(this);
	QtLineEditFactory *lineEditFactory = new QtLineEditFactory(this);
	QtEnumEditorFactory *comboBoxFactory = new QtEnumEditorFactory(this);

	editor1 = new QtTreePropertyBrowser();
	editor1->setFactoryForManager(boolManager, checkBoxFactory);
	editor1->setFactoryForManager(intManager, spinBoxFactory);
	editor1->setFactoryForManager(doubleManager, doublespinBoxFactory);
	editor1->setFactoryForManager(stringManager, lineEditFactory);
	editor1->setFactoryForManager(sizeManager->subIntPropertyManager(), spinBoxFactory);
	editor1->setFactoryForManager(rectManager->subIntPropertyManager(), spinBoxFactory);
	editor1->setFactoryForManager(sizePolicyManager->subIntPropertyManager(), spinBoxFactory);
	editor1->setFactoryForManager(sizePolicyManager->subEnumPropertyManager(), comboBoxFactory);
	editor1->setFactoryForManager(enumManager, comboBoxFactory);

	editor1->addProperty(item0);

	QScrollArea *scroll1 = new QScrollArea();
	scroll1->setWidgetResizable(true);
	scroll1->setWidget(editor1);

	QGridLayout *layout = new QGridLayout(this);
	//QLabel *label1 = new QLabel("Property Browser");	
	//label1->setWordWrap(true);	
	//label1->setFrameShadow(QFrame::Sunken);	
	//label1->setFrameShape(QFrame::Panel);
	//label1->setAlignment(Qt::AlignCenter);
	//layout->addWidget(label1, 0, 0);
	layout->addWidget(scroll1, 1, 0);

	connect(boolManager, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(boolPropertyValueChanged(QtProperty*)));
	connect(intManager, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(intPropertyValueChanged(QtProperty*)));
	connect(doubleManager, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(doublePropertyValueChanged(QtProperty*)));
	connect(rectManager, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(rectPropertyValueChanged(QtProperty*)));
	connect(enumManager, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(enumPropertyValueChanged(QtProperty*)));
}

QPropertyWidget::~QPropertyWidget()
{

}

void QPropertyWidget::boolPropertyValueChanged(QtProperty * property)
{
	qDebug() << property->propertyName() << " " << property->valueText();

	/*if (m_pNode)
	{
		if (property->propertyName() == "enabled")
		{
			if (m_pNode->isEnabled() != boolManager->value(property))
			{
				m_pNode->setEnable(boolManager->value(property));				
			}
		}
	}*/
}

void QPropertyWidget::intPropertyValueChanged(QtProperty * property)
{
	qDebug() << property->propertyName() << " " << property->valueText();	
}

void QPropertyWidget::doublePropertyValueChanged(QtProperty * property)
{
	qDebug() << property->propertyName() << " " << property->valueText();	
}

void QPropertyWidget::rectPropertyValueChanged(QtProperty * property)
{
	qDebug() << property->propertyName() << " " << property->valueText();
}

void QPropertyWidget::enumPropertyValueChanged(QtProperty * property)
{
	qDebug() << property->propertyName() << " " << property->valueText();
}


void QPropertyWidget::setValue()
{
	//intManager->setValue(m_mapProperties["id"], pLine->_ID);
	//stringManager->setValue(m_mapProperties["type"], "line");
}

DataToolWidget::DataToolWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	QGridLayout* pGridLayout = new QGridLayout(ui.page);
	pGridLayout->setAlignment(Qt::AlignTop);

	QString path = QApplication::applicationDirPath();
	path += "/image/DataImage/";

	QDir dir(path);
	if (dir.exists())
	{
		dir.setFilter(QDir::Files | QDir::NoSymLinks);
		QFileInfoList list = dir.entryInfoList();

		int nImageWidth = 60, nImageHeight = 60;
		for (int i = 0; i < list.size(); i++)
		{
			QFileInfo file_info = list.at(i);
			QString suffix = file_info.suffix();
			if (QString::compare(suffix, QString("png"), Qt::CaseInsensitive) == 0)
			{
				QString filePath = file_info.absoluteFilePath();

				QLabel *boatIcon = new QLabel();
				QPixmap pixmap = QPixmap(filePath);
				if (pixmap.width() > nImageWidth)
				{
					pixmap = pixmap.scaled(QSize(nImageWidth, nImageHeight));
				}
				boatIcon->setPixmap(pixmap);
				boatIcon->move(0, 0);
				//boatIcon->show();
				boatIcon->setAttribute(Qt::WA_DeleteOnClose);
				//boatIcon->setObjectName(QString("%1-%2").arg(i).arg(j));

				pGridLayout->addWidget(boatIcon, i / 4, i % 4, 1, 1);
			}
		}
	}

	m_propertyWidget = new QPropertyWidget(this);
	ui.dockWidget->setWidget(m_propertyWidget);
	ui.dockWidget->setFixedHeight(330);
}

DataToolWidget::~DataToolWidget()
{
}
