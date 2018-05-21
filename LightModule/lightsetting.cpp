#include "lightsetting.h"
#include "../Common/SystemData.h"
#include "LightDefine.h"

QLightSetting::QLightSetting(QLightCtrl * pCtrl,QWidget *parent)
	: m_lightCtrl(pCtrl), QWidget(parent)
{
	ui.setupUi(this);

	int n = System->getParam(LIGHT_COUNT).toInt();
	for(int i = 0; i<n; i++)
	{
		QString str = QString(QStringLiteral("光源控制器%0")).arg(i+1);
		ui.tabWidget->addTab(new QDeviceFrom(m_lightCtrl->getLightDevice(i),this),str);
	}

	connect(ui.pushButton,SIGNAL(clicked()),SLOT(onAddLightCtrl()));
	connect(ui.pushButton_2,SIGNAL(clicked()),SLOT(onDelLightCtrl()));
	//ui.pushButton->setVisible(false);
	//ui.pushButton_2->setVisible(false);
}

QLightSetting::~QLightSetting()
{

}

void QLightSetting::onAddLightCtrl()
{
	int n = ui.tabWidget->count();
	QString str = QString(QStringLiteral("光源控制器%0")).arg(n+1);
	QString name = QString("light%0").arg(n+1);

    QLightDevice * pDevice = NULL;
    if (System->isTriggerBoard())
    {
        pDevice = new QLightCardDevice(name, _CHN_NUM, NULL);
    }
    else
    {
        pDevice = new QLightDerivedDevice(name, _CHN_NUM, NULL);
    }

	m_lightCtrl->addDevice(pDevice);
	ui.tabWidget->addTab(new QDeviceFrom(pDevice,this),str);

	System->setParam(LIGHT_COUNT,n+1);
}

void QLightSetting::onDelLightCtrl()
{
	int n = ui.tabWidget->currentIndex();
	int count = ui.tabWidget->count();
	if(n >= 0)
	{
		ui.tabWidget->removeTab(n);
		m_lightCtrl->delDevice(n);
		System->setParam(LIGHT_COUNT,count-1);
	}
	
}