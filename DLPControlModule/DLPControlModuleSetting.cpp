#include "DLPControlModuleSetting.h"
#include "../include/hidapi.h"

#include "../Common/SystemData.h"

#define MY_VID 0x0451
#define MY_PID 0x6401

DLPControlModuleSetting::DLPControlModuleSetting(DLPControl* pCtrl, QWidget *parent)
	: m_pCtrl(pCtrl), QWidget(parent)
{
	ui.setupUi(this);

	m_pTabWidget = new QTabWidget(this);

	for (int i = 0; i < m_pCtrl->getDLPNum(); i++)
	{
		m_pTabWidget->addTab(m_pCtrl->getMainWin(i), QStringLiteral("DLP%1").arg(i+1));		
	}

	ui.verticalLayout->addWidget(m_pTabWidget);

	connect(ui.comboBox_selectDLP, SIGNAL(currentIndexChanged(int)), SLOT(onDLPIndexChanged(int)));
	for (int i = 0; i < m_pCtrl->getDLPNum(); i++)
	{
		ui.comboBox_selectDLP->addItem(QString("%1").arg(QStringLiteral("DLP%1").arg(i+1)));
	}
	ui.comboBox_selectDLP->setCurrentIndex(0);

	connect(ui.pushButton_inqureInfo, SIGNAL(clicked()), SLOT(onInqureInfo()));
	connect(ui.pushButton_bindDLPSerialNB, SIGNAL(clicked()), SLOT(onBindDLPSNB()));

	ui.tableView_dlpInfoList->setModel(&m_model);	

	for (int i = 0; i < m_pCtrl->getDLPNum(); i++)
	{
		QString serialNB = System->getParam(QString("dlp_param_serial_number_%1").arg(i + 1)).toString();
		m_pCtrl->setDLPSerialNumber(i, serialNB);
	}	
}

DLPControlModuleSetting::~DLPControlModuleSetting()
{
	//m_pVBox->removeWidget(m_pCtrl->getMainWin());
	//delete m_pVBox;
}

void DLPControlModuleSetting::onDLPIndexChanged(int iState)
{
	int nDLPIndex = ui.comboBox_selectDLP->currentIndex();

	QString serialNB = m_pCtrl->getDLPSerialNumber(nDLPIndex);
	ui.lineEdit_dlpInfo->setText(serialNB);
}

void DLPControlModuleSetting::onInqureInfo()
{
	refreshHWInfo();
}

void DLPControlModuleSetting::onBindDLPSNB()
{
	int nDLPIndex = ui.comboBox_selectDLP->currentIndex();

	int n = ui.tableView_dlpInfoList->currentIndex().row();
	QString serialNB = m_model.data(m_model.index(n, 1)).toString();
	int nIndexStart = serialNB.indexOf("#8&");
	int nIndexEnd = serialNB.indexOf('&', nIndexStart + 3);
	serialNB = serialNB.mid(nIndexStart + 3, nIndexEnd - nIndexStart - 3);

	System->setParam(QString("dlp_param_serial_number_%1").arg(nDLPIndex + 1), serialNB);
	m_pCtrl->setDLPSerialNumber(nDLPIndex, serialNB);
	ui.lineEdit_dlpInfo->setText(serialNB);
}

void DLPControlModuleSetting::refreshHWInfo()
{
	m_model.clear();
	QStringList ls;
	ls << QStringLiteral("产品名称") << QStringLiteral("序列号");
	m_model.setHorizontalHeaderLabels(ls);
	ui.tableView_dlpInfoList->setColumnWidth(0, 100);
	ui.tableView_dlpInfoList->setColumnWidth(1, 350);

	hid_device_info* deviceInfoList = hid_enumerate(MY_VID, MY_PID);

	hid_device_info* deviceInfo = deviceInfoList;

	while (deviceInfo != NULL)
	{
		//DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, deviceInfo->serial_number, -1, NULL, 0, NULL, FALSE);// WideCharToMultiByte的运用  
		//char *psText; // psText为char*的临时数组，作为赋值给std::string的中间变量  
		//psText = new char[dwNum];
		//WideCharToMultiByte(CP_OEMCP, NULL, deviceInfo->serial_number, -1, psText, dwNum, NULL, FALSE);// WideCharToMultiByte的再次运用  
		//std::string szDst = psText;// std::string赋值  
		//delete[]psText;// psText的清除

		DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, deviceInfo->product_string, -1, NULL, 0, NULL, FALSE);// WideCharToMultiByte的运用  
		char *psText; // psText为char*的临时数组，作为赋值给std::string的中间变量  
		psText = new char[dwNum];
		WideCharToMultiByte(CP_OEMCP, NULL, deviceInfo->product_string, -1, psText, dwNum, NULL, FALSE);// WideCharToMultiByte的再次运用  
		std::string szDst = psText;// std::string赋值  
		delete[]psText;// psText的清除

		if (szDst == "DLPC350") 
		{
			int nr = m_model.rowCount();
			m_model.insertRow(nr);
			m_model.setData(m_model.index(nr, 0), QString(szDst.c_str()));
			m_model.setData(m_model.index(nr, 1), QString(deviceInfo->path));
		}
	

		deviceInfo = deviceInfo->next;
	}

	hid_free_enumeration(deviceInfoList);
}