#ifndef DEVICEFROM_H
#define DEVICEFROM_H

#include <QWidget>
#include "lightdevice.h"
#include "ui_DeviceForm.h"

class DeviceForm : public QWidget
{
	Q_OBJECT

public:
	DeviceForm(QLightDevice * device, QWidget *parent = 0);
	~DeviceForm();
	
public slots:
	void onOpenCommPort();
	void onSendTestData();

	void onSliderChanged1(int i);
	void onSliderChanged2(int i);
	void onSliderChanged3(int i);
	void onSliderChanged4(int i);
    void onSliderChanged5(int i);
    void onSliderChanged6(int i);
    void onSliderChanged7(int i);
    void onSliderChanged8(int i);

	void onTextChanged1(const QString &text);
	void onTextChanged2(const QString &text);
	void onTextChanged3(const QString &text);
	void onTextChanged4(const QString &text);
    void onTextChanged5(const QString &text);
    void onTextChanged6(const QString &text);
    void onTextChanged7(const QString &text);
    void onTextChanged8(const QString &text);

	//void onDataReady();
	void onSaveData();

private:
	Ui::DeviceForm ui;
	QLightDevice *m_device;
};

#endif // DEVICEFROM_H
