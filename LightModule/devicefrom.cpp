#include "devicefrom.h"
#include "LightDefine.h"
#include "SystemData.h"

#include "lightmodule_global.h"

QDeviceFrom::QDeviceFrom(QLightDevice * device,QWidget *parent)
	: m_device(device),QWidget(parent)
{
	ui.setupUi(this);
	//ui.pushButton_3->setDisabled(true);
	QStringList ls =  QLightDevice::enumCommPort();
	ui.comboBox->addItems(ls);

	ls = QLightDevice::enumCommBound();
	ui.comboBox_2->addItems(ls);

	if(m_device->isOpenCommPort())ui.pushButton->setText(QStringLiteral("关闭"));
	else ui.pushButton->setText(QStringLiteral("打开"));

	QString key = QString("%0-%1").arg(m_device->getDeviceName()).arg(COMM_PORT);
	QVariant data = System->getParam(key);
	ui.comboBox->setCurrentText(data.toString());
	
	key = QString("%0-%1").arg((m_device->getDeviceName())).arg(COMM_BOUND);
	data = System->getParam(key);
	ui.comboBox_2->setCurrentText(data.toString());

    QLineEdit * editCtrls[_MAX_CHDATA_NUM] = { ui.lineEdit_1, ui.lineEdit_2, ui.lineEdit_3, ui.lineEdit_4, ui.lineEdit_5, ui.lineEdit_6, ui.lineEdit_7, ui.lineEdit_8 };
    QSlider * sliderCtrls[_MAX_CHDATA_NUM] = { ui.horizontalSlider_1, ui.horizontalSlider_2, ui.horizontalSlider_3, ui.horizontalSlider_4, ui.horizontalSlider_5, ui.horizontalSlider_6, ui.horizontalSlider_7, ui.horizontalSlider_8 };
    QLineEdit * editLums[_MAX_CHDATA_NUM] = { ui.lineEdit_1_value, ui.lineEdit_2_value, ui.lineEdit_3_value, ui.lineEdit_4_value, ui.lineEdit_5_value, ui.lineEdit_6_value, ui.lineEdit_7_value, ui.lineEdit_8_value };
    for (int i = 0; i < _MAX_CHDATA_NUM; i++)
	{
		key = QString("%0-%1%2").arg(m_device->getDeviceName()).arg(NAMED_CH).arg(i);
		data = System->getParam(key);
		editCtrls[i]->setText(data.toString());
		m_device->setChName(i,data.toString());
		key = QString("%0-%1%2").arg(m_device->getDeviceName()).arg(LUM_CH).arg(i);
		data = System->getParam(key);
		sliderCtrls[i]->setValue(data.toInt());
		m_device->setChLuminance(i,data.toInt());
		editLums[i]->setText(data.toString());
	}
	//connect(m_device,SIGNAL(dataReady()),SLOT(onDataReady()));
	connect(ui.pushButton, SIGNAL(clicked()), SLOT(onOpenCommPort()));
	connect(ui.pushButton_2, SIGNAL(clicked()), SLOT(onSendTestData()));
	connect(ui.pushButton_3, SIGNAL(clicked()), SLOT(onSaveData()));

	connect(ui.horizontalSlider_1,SIGNAL(valueChanged(int)),SLOT(onSliderChanged1(int)));
	connect(ui.horizontalSlider_2,SIGNAL(valueChanged(int)),SLOT(onSliderChanged2(int)));
	connect(ui.horizontalSlider_3,SIGNAL(valueChanged(int)),SLOT(onSliderChanged3(int)));
	connect(ui.horizontalSlider_4,SIGNAL(valueChanged(int)),SLOT(onSliderChanged4(int)));
    connect(ui.horizontalSlider_5, SIGNAL(valueChanged(int)), SLOT(onSliderChanged5(int)));
    connect(ui.horizontalSlider_6, SIGNAL(valueChanged(int)), SLOT(onSliderChanged6(int)));
    connect(ui.horizontalSlider_7, SIGNAL(valueChanged(int)), SLOT(onSliderChanged7(int)));
    connect(ui.horizontalSlider_8, SIGNAL(valueChanged(int)), SLOT(onSliderChanged8(int)));

	connect(ui.lineEdit_1,SIGNAL(textChanged(const QString &)),SLOT(onTextChanged1(const QString &)));
	connect(ui.lineEdit_2,SIGNAL(textChanged(const QString &)),SLOT(onTextChanged2(const QString &)));
	connect(ui.lineEdit_3,SIGNAL(textChanged(const QString &)),SLOT(onTextChanged3(const QString &)));
	connect(ui.lineEdit_4,SIGNAL(textChanged(const QString &)),SLOT(onTextChanged4(const QString &)));
    connect(ui.lineEdit_5, SIGNAL(textChanged(const QString &)), SLOT(onTextChanged5(const QString &)));
    connect(ui.lineEdit_6, SIGNAL(textChanged(const QString &)), SLOT(onTextChanged6(const QString &)));
    connect(ui.lineEdit_7, SIGNAL(textChanged(const QString &)), SLOT(onTextChanged7(const QString &)));
    connect(ui.lineEdit_8, SIGNAL(textChanged(const QString &)), SLOT(onTextChanged8(const QString &)));

    for (int i = m_device->getChnNum(); i < _MAX_CHDATA_NUM; i++)
    {
        editCtrls[i]->setEnabled(false);
        sliderCtrls[i]->setEnabled(false);
    }
}

QDeviceFrom::~QDeviceFrom()
{
	//disconnect(m_device,SIGNAL(dataReady()),this,SLOT(onDataReady()));
}

void QDeviceFrom::onOpenCommPort()
{
	QString name = ui.comboBox->currentText();
	int bound = ui.comboBox_2->currentText().toInt();

	if(m_device->isOpenCommPort())m_device->closeCommPort();	
	else m_device->openCommPort(name,bound);
	

	if(m_device->isOpenCommPort())ui.pushButton->setText(QStringLiteral("关闭"));
	else ui.pushButton->setText(QStringLiteral("打开"));
}

void QDeviceFrom::onSendTestData()
{
	//QString str = ui.lineEdit_5->text();
	//m_device->sendTestData(str);
}

//void QDeviceFrom::onDataReady()
//{
//	//QString str;
//	//m_device->recvHexData(str);
//	//ui.textEdit->setText(str);
//}

void QDeviceFrom::onSaveData()
{
	QString data = ui.comboBox->currentText();
	QString key = QString("%0-%1").arg(m_device->getDeviceName()).arg(COMM_PORT);
	System->setParam(key,data);
	data = ui.comboBox_2->currentText();
	key = QString("%0-%1").arg(m_device->getDeviceName()).arg(COMM_BOUND);
	System->setParam(key,data);

    QLineEdit * editCtrls[_MAX_CHDATA_NUM] = { ui.lineEdit_1, ui.lineEdit_2, ui.lineEdit_3, ui.lineEdit_4, ui.lineEdit_5, ui.lineEdit_6, ui.lineEdit_7, ui.lineEdit_8 };
    QSlider * sliderCtrls[_MAX_CHDATA_NUM] = { ui.horizontalSlider_1, ui.horizontalSlider_2, ui.horizontalSlider_3, ui.horizontalSlider_4, ui.horizontalSlider_5, ui.horizontalSlider_6, ui.horizontalSlider_7, ui.horizontalSlider_8 };
    QLineEdit * editLums[_MAX_CHDATA_NUM] = { ui.lineEdit_1_value, ui.lineEdit_2_value, ui.lineEdit_3_value, ui.lineEdit_4_value, ui.lineEdit_5_value, ui.lineEdit_6_value, ui.lineEdit_7_value, ui.lineEdit_8_value };

    for (int i = 0; i < _MAX_CHDATA_NUM; i++)
	{
		key = QString("%0-%1%2").arg(m_device->getDeviceName()).arg(NAMED_CH).arg(i);
		data = editCtrls[i]->text();
		System->setParam(key, data);

		key = QString("%0-%1%2").arg(m_device->getDeviceName()).arg(LUM_CH).arg(i);
		data = QString("%1").arg(sliderCtrls[i]->value());
		System->setParam(key, data);
	}
}

void QDeviceFrom::onSliderChanged1(int lum)
{
	QString str = QString::number(lum);
    ui.lineEdit_1_value->setText(str);
	m_device->openLight(0);
	m_device->setChLuminance(0,lum);
}

void QDeviceFrom::onSliderChanged2(int lum)
{
	QString str = QString::number(lum);
    ui.lineEdit_2_value->setText(str);
	m_device->openLight(1);
	m_device->setChLuminance(1,lum);
}

void QDeviceFrom::onSliderChanged3(int lum)
{
	QString str = QString::number(lum);
    ui.lineEdit_3_value->setText(str);
	m_device->openLight(2);
	m_device->setChLuminance(2,lum);
}

void QDeviceFrom::onSliderChanged4(int lum)
{
	QString str = QString::number(lum);
    ui.lineEdit_4_value->setText(str);
	m_device->openLight(3);
	m_device->setChLuminance(3,lum);
}

void QDeviceFrom::onSliderChanged5(int lum)
{
    QString str = QString::number(lum);
    ui.lineEdit_5_value->setText(str);
    m_device->openLight(4);
    m_device->setChLuminance(4, lum);
}

void QDeviceFrom::onSliderChanged6(int lum)
{
    QString str = QString::number(lum);
    ui.lineEdit_6_value->setText(str);
    m_device->openLight(5);
    m_device->setChLuminance(5, lum);
}

void QDeviceFrom::onSliderChanged7(int lum)
{
    QString str = QString::number(lum);
    ui.lineEdit_7_value->setText(str);
    m_device->openLight(6);
    m_device->setChLuminance(6, lum);
}

void QDeviceFrom::onSliderChanged8(int lum)
{
    QString str = QString::number(lum);
    ui.lineEdit_8_value->setText(str);
    m_device->openLight(7);
    m_device->setChLuminance(7, lum);
}

void QDeviceFrom::onTextChanged1(const QString &text)
{
	m_device->setChName(0,text);
}
	
void QDeviceFrom::onTextChanged2(const QString &text)
{
	m_device->setChName(1,text);
}
	
void QDeviceFrom::onTextChanged3(const QString &text)
{
	m_device->setChName(2,text);
}
	
void QDeviceFrom::onTextChanged4(const QString &text)
{
	m_device->setChName(3,text);
}

void QDeviceFrom::onTextChanged5(const QString &text)
{
    m_device->setChName(4, text);
}

void QDeviceFrom::onTextChanged6(const QString &text)
{
    m_device->setChName(5, text);
}

void QDeviceFrom::onTextChanged7(const QString &text)
{
    m_device->setChName(6, text);
}

void QDeviceFrom::onTextChanged8(const QString &text)
{
    m_device->setChName(7, text);
}