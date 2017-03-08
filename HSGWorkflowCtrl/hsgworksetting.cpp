#include "hsgworksetting.h"
#include <qfiledialog.h>
#include "../Common/SystemData.h"
#include "../include/IdDefine.h"
#include "../Common/eos.h"
#include <QMessagebox>

#define BACKUP_DIR_DEFAULT_PATH "D://backup"

QWorkSetting::QWorkSetting(QFlowCtrl *p, QWidget *parent)
	: QWidget(parent),m_ctrl(p)
{
	ui.setupUi(this);

	int runSpeed = System->getRunSpeed();
	if(RUN_FULL_SPEED == runSpeed)
	{
		ui.radioButton->setChecked(false);
		ui.radioButton_2->setChecked(false);
		ui.radioButton_3->setChecked(false);
		ui.radioButton_4->setChecked(true);
	}
	else if(RUN_HIG_SPEED == runSpeed)
	{
		ui.radioButton->setChecked(false);
		ui.radioButton_2->setChecked(false);
		ui.radioButton_3->setChecked(true);
		ui.radioButton_4->setChecked(false);
	}
	else if(RUN_MID_SPEED == runSpeed)
	{
		ui.radioButton->setChecked(false);
		ui.radioButton_2->setChecked(true);
		ui.radioButton_3->setChecked(false);
		ui.radioButton_4->setChecked(false);
	}
	else if(RUN_LOW_SPEED == runSpeed)
	{
		ui.radioButton->setChecked(true);
		ui.radioButton_2->setChecked(false);
		ui.radioButton_3->setChecked(false);
		ui.radioButton_4->setChecked(false);
	}

	QString recordPath = System->getRecordPath();
	QString recordDetailPath = System->getRecordDetailPath();
	QString backupPath = System->getBackupDataPath();

	if(recordPath.isEmpty())recordPath = QApplication::applicationDirPath() + "/record";
	ui.lineEdit->setText(recordPath);

	if(recordDetailPath.isEmpty())recordDetailPath =  QApplication::applicationDirPath() + "/details";
	ui.lineEdit_2->setText(recordDetailPath);

	if(backupPath.isEmpty())
	{	
		QDir dir;
		if(!dir.exists(BACKUP_DIR_DEFAULT_PATH))
		{
			dir.mkdir(BACKUP_DIR_DEFAULT_PATH);
		}
		backupPath =  BACKUP_DIR_DEFAULT_PATH;
		System->setBackupDataPath(backupPath);
	}
	ui.lineEdit_3->setText(backupPath);	

	bool s = System->isEnableRecord();
	ui.checkBox->setChecked(s);
	s = System->isEnableRecordDetails();
	ui.checkBox_2->setChecked(s);
	s = System->isEnableBackupData();
	ui.checkBox_4->setChecked(s);
	
	bool bAutoCycle = System->getParam("sys_run_auto_cycle").toBool();
	ui.checkBoxAutoCycle->setChecked(bAutoCycle);

	bool bCheckBarcode = System->getParam("sys_run_check_barcode").toBool();
	ui.checkBoxCheckBarcode->setChecked(bCheckBarcode);
	
	m_pValidatorDouble = new QDoubleValidator(-1000.0, 1000.0, 3, this);
	//ui.lineEditBinA->setValidator(m_pValidatorDouble);	

	connect(ui.comboBox,SIGNAL(currentIndexChanged(int)),SLOT(onTypeIndexChanged(int)));
	QStringList ls;	
	ls << QStringLiteral("类型1") << QStringLiteral("类型2");
	ui.comboBox->addItems(ls);	

	connect(ui.comboBoxSelectStation,SIGNAL(currentIndexChanged(int)),SLOT(onStationIndexChanged(int)));
	ls.clear();
	for (int i = 0; i< STATION_COUNT; i++)
	{
		ls << QStringLiteral("工位") + QString("%0").arg(i+1);

	}
	ui.comboBoxSelectStation->addItems(ls);		
	ui.comboBoxSelectStation->setCurrentIndex(0);

	int isOpen = System->getSysParam(SAFE_DOOR_KEY).toInt();
	if(isOpen)ui.checkBox_3->setChecked(true);
	else ui.checkBox_3->setChecked(false);

    bool bsafeGrating = System->getParam("sys_run_safe_garting").toBool();
    ui.checkBox_safe_grating->setChecked(bsafeGrating);	
	

	connect(ui.radioButton,SIGNAL(toggled(bool)),SLOT(onclickLowSpeed(bool)));
	connect(ui.radioButton_2,SIGNAL(toggled(bool)),SLOT(onClickMidSpeed(bool)));
	connect(ui.radioButton_3,SIGNAL(toggled(bool)),SLOT(onClickHightSpeed(bool)));
	connect(ui.radioButton_4,SIGNAL(toggled(bool)),SLOT(onClickFullSpeed(bool)));

	connect(ui.checkBox,SIGNAL(stateChanged(int)),SLOT(onComBoxChange1(int)));
	connect(ui.checkBox_2,SIGNAL(stateChanged(int)),SLOT(onComBoxChange2(int)));
	connect(ui.checkBox_4,SIGNAL(stateChanged(int)),SLOT(onComBoxChange4(int)));

	connect(ui.pushButton,SIGNAL(clicked()),SLOT(onSelRecord()));
	connect(ui.pushButton_2,SIGNAL(clicked()),SLOT(onSaveRecord()));
	connect(ui.pushButton_3,SIGNAL(clicked()),SLOT(onSaveRecordDetail()));
	connect(ui.pushButton_4,SIGNAL(clicked()),SLOT(onSelRecordDetail()));
	connect(ui.pushButton_7,SIGNAL(clicked()),SLOT(onSelBackup()));
	connect(ui.pushButton_6,SIGNAL(clicked()),SLOT(onSaveBackup()));

	connect(ui.pushButton_5,SIGNAL(clicked()),SLOT(onSaveHsgType()));
	connect(ui.checkBox_3,SIGNAL(stateChanged(int)),SLOT(onEnableSafeDoor(int)));
	connect(ui.checkBoxAutoCycle,SIGNAL(stateChanged(int)),SLOT(onEnableAutoCycle(int)));
	connect(ui.checkBoxCheckBarcode,SIGNAL(stateChanged(int)),SLOT(onEnableCheckBarcode(int)));
    connect(ui.checkBox_safe_grating,SIGNAL(stateChanged(int)),SLOT(onCheckSafeGrating(int)));

	if(USER_LEVEL_TECH > System->getUserLevel())
	{
		ui.comboBox->setEnabled(false);
		ui.pushButton_5->setEnabled(false);
		ui.comboBoxSelectStation->setEnabled(false);	
		ui.checkBox_safe_grating->setEnabled(false);
		ui.checkBox_3->setEnabled(false);
		ui.checkBoxAutoCycle->setEnabled(false);
		ui.checkBoxCheckBarcode->setEnabled(false);

		ui.checkBox->setEnabled(false);

		ui.lineEdit->setEnabled(false);
		ui.pushButton->setEnabled(false);
		ui.pushButton_2->setEnabled(false);
		
		ui.checkBox_2->setEnabled(false);
		ui.lineEdit_2->setEnabled(false);
		ui.pushButton_4->setEnabled(false);
		ui.pushButton_3->setEnabled(false);

		ui.checkBox_4->setEnabled(false);
		ui.lineEdit_3->setEnabled(false);
		ui.pushButton_7->setEnabled(false);
		ui.pushButton_6->setEnabled(false);

		ui.groupBox->setEnabled(false);	
		ui.groupBox_3->setEnabled(false);
		ui.groupBox_5->setEnabled(false);
		ui.groupBox_6->setEnabled(false);		
	}
}

QWorkSetting::~QWorkSetting()
{
	if(m_pValidatorDouble)
	{
		delete m_pValidatorDouble;
		m_pValidatorDouble= NULL;
	}
}

void QWorkSetting::onClickFullSpeed(bool s)
{
	if(s)System->setRunSpeed(RUN_FULL_SPEED);
}

void QWorkSetting::onClickHightSpeed(bool s)
{
	if(s)System->setRunSpeed(RUN_HIG_SPEED);
}

void QWorkSetting::onClickMidSpeed(bool s)
{
	if(s)System->setRunSpeed(RUN_MID_SPEED);
}

void QWorkSetting::onclickLowSpeed(bool s)
{
	if(s)System->setRunSpeed(RUN_LOW_SPEED);
}

void QWorkSetting::onComBoxChange1(int iState)
{
	if(iState == Qt::Checked)
	{
		System->enableRecord(true);
	}
	else
	{
		System->enableRecord(false);
	}
}

void QWorkSetting::onComBoxChange2(int iState)
{
	if(iState == Qt::Checked)
	{
		System->enableRecordDetails(true);
	}
	else
	{
		System->enableRecordDetails(false);
	}
}

void QWorkSetting::onComBoxChange4(int iState)
{
	if(iState == Qt::Checked)
	{
		System->enableBackupData(true);
	}
	else
	{
		System->enableBackupData(false);
	}
}

void QWorkSetting::onEnableSafeDoor(int iState)
{
	if(iState == Qt::Checked)
	{
		System->setSysParam(SAFE_DOOR_KEY,1);
	}
	else
	{
		System->setSysParam(SAFE_DOOR_KEY,0);
	}
}

void QWorkSetting::onEnableAutoCycle(int iState)
{
	int data = 0;
	if(Qt::Checked == iState)data = 1;

	System->setParam("sys_run_auto_cycle", (bool)data);
}

void QWorkSetting::onEnableCheckBarcode(int iState)
{
	int data = 0;
	if(Qt::Checked == iState)data = 1;

	System->setParam("sys_run_check_barcode", (bool)data);
}

void QWorkSetting::onCheckSafeGrating(int iState)
{
	int data = 0;
	if(Qt::Checked == iState)data = 1;
	
	System->setParam("sys_run_safe_garting", (bool)data);
}

void QWorkSetting::onSelRecord()
{
	QString str = QFileDialog::getExistingDirectory();
	if(!str.isEmpty())ui.lineEdit->setText(str);
}

void QWorkSetting::onSaveRecord()
{
	QString str = ui.lineEdit->text();
	if(!str.isEmpty())System->setRecordPath(str);
}

void QWorkSetting::onSelRecordDetail()
{
	QString str = QFileDialog::getExistingDirectory();
	if(!str.isEmpty())ui.lineEdit_2->setText(str);
}

void QWorkSetting::onSaveRecordDetail()
{
	QString str = ui.lineEdit_2->text();
	if(!str.isEmpty())System->setRecordDetailsPath(str);
}

void QWorkSetting::onSelBackup()
{
	QString str = QFileDialog::getExistingDirectory(0,"Data Backup Path", System->getBackupDataPath());
	if(!str.isEmpty())ui.lineEdit_3->setText(str);
}

void QWorkSetting::onSaveBackup()
{
	QString str = ui.lineEdit_3->text();
	if(!str.isEmpty())System->setBackupDataPath(str);
}

void QWorkSetting::onStationIndexChanged(int index)
{
	int nStation = index;
}

void QWorkSetting::onTypeIndexChanged(int index)
{
	int nStation = ui.comboBoxSelectStation->currentIndex();
	QString hsgType = ui.comboBox->currentText();	
}

void QWorkSetting::onSaveHsgType()
{
	int nStation = ui.comboBoxSelectStation->currentIndex();

	QString str = ui.comboBox->currentText();
	if(!str.isEmpty())
	{
	/*	QString after = System->getSysParam(QString(HSG_TYPE_SAVEID).arg(nStation)).toString();
		System->setSysParam(QString(HSG_TYPE_SAVEID).arg(nStation),str);

		QEos::Notify(EVENT_HSG_TYPE,after);*/
	}	
	
	if(QMessageBox::Ok == QMessageBox::warning(NULL,QStringLiteral("提示"),
		QStringLiteral("系统配置已修改，请务必关闭软件并重新启动!"),QMessageBox::Ok))
	{
		
	}
}