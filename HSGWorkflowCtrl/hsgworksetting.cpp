#include "hsgworksetting.h"
#include <qfiledialog.h>
#include "../Common/SystemData.h"
#include "../include/IdDefine.h"
#include "../Common/eos.h"
#include <QMessagebox>
#include <future>

#include "VisionAPI.h"
using namespace AOI;

//定义系统Vision LOG
#define VISION_LOG_ALL		2
#define VISION_LOG_FAIL		1

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

	
	m_pValidatorDouble = new QDoubleValidator(-1000.0, 1000.0, 3, this);
	//ui.lineEditBinA->setValidator(m_pValidatorDouble);	

	connect(ui.comboBox,SIGNAL(currentIndexChanged(int)),SLOT(onTypeIndexChanged(int)));
	QStringList ls;	
	ls << QStringLiteral("类型1") << QStringLiteral("类型2");
	ui.comboBox->addItems(ls);	

	connect(ui.comboBoxSelectStation,SIGNAL(currentIndexChanged(int)),SLOT(onStationIndexChanged(int)));
	ls.clear();
	int nStationNum = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
	for (int i = 0; i< nStationNum; i++)
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
    connect(ui.checkBox_safe_grating,SIGNAL(stateChanged(int)),SLOT(onCheckSafeGrating(int)));	

	initUI();

	if (USER_LEVEL_MANAGER > System->getUserLevel())
	{
		ui.tabWidget->setEnabled(false);
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

void QWorkSetting::initUI()
{
	QStringList ls;

	connect(ui.comboBox_dlpMode, SIGNAL(currentIndexChanged(int)), SLOT(onDlpModeIndexChanged(int)));
	ls.clear();
    ls << QStringLiteral("DLP触发模式") << QStringLiteral("控制卡触发模式") << QStringLiteral("Trigger版触发模式");
	ui.comboBox_dlpMode->addItems(ls);
    int nDlpMode = System->getSysRunMode();
	ui.comboBox_dlpMode->setCurrentIndex(nDlpMode);

    connect(ui.comboBox_triggerMode, SIGNAL(currentIndexChanged(int)), SLOT(onTriggerModeIndexChanged(int)));
    ls.clear();
    ls << QStringLiteral("Lighting卡硬触发") << QStringLiteral("Lighting卡软触发");
    ui.comboBox_triggerMode->addItems(ls);
    int nTriggerMode = System->getParam("lighting_trigger_mode").toInt();
    ui.comboBox_triggerMode->setCurrentIndex(nTriggerMode);

	connect(ui.pushButton_DlpSave, SIGNAL(clicked()), SLOT(onDlpModeSave()));

	// Motion
	connect(ui.comboBox_triggerNum, SIGNAL(currentIndexChanged(int)), SLOT(onTriggerNumIndexChanged(int)));
	ls.clear();
	ls << QStringLiteral("双头DLP检测") << QStringLiteral("四头DLP检测");
	ui.comboBox_triggerNum->addItems(ls);
	int nDlpNumIndex = System->getParam("motion_trigger_dlp_num_index").toInt();
	ui.comboBox_triggerNum->setCurrentIndex(nDlpNumIndex);

	int nPatternNum = System->getParam("motion_trigger_pattern_num").toInt();
	double dPatternExposure = System->getParam("motion_trigger_pattern_exposure").toDouble();
	double dPatternPeriod = System->getParam("motion_trigger_pattern_period").toDouble();
	double dLightExposure = System->getParam("motion_trigger_light_exposure").toDouble();
	double dLightPeriod = System->getParam("motion_trigger_light_period").toDouble();
	ui.lineEdit_trigger_caputured_num->setText(QString("%1").arg(nPatternNum));
	ui.lineEdit_trigger_pattern_exposure->setText(QString("%1").arg(dPatternExposure));
	ui.lineEdit_trigger_pattern_period->setText(QString("%1").arg(dPatternPeriod));
	ui.lineEdit_trigger_light_exposure->setText(QString("%1").arg(dLightExposure));
	ui.lineEdit_trigger_light_period->setText(QString("%1").arg(dLightPeriod));

	connect(ui.pushButton_trigger_save, SIGNAL(clicked()), SLOT(onTriggerSave()));

	//Vision System Setting
	bool bLogAllCase = System->getParam("camera_log_allcase_enable").toBool();
	ui.checkBox_LogAllCase->setChecked(bLogAllCase);

	int nLogType = System->getParam("vision_log_type").toInt();
	if (VISION_LOG_ALL == nLogType)
	{
		ui.radioButton_logFailCase->setChecked(false);
		ui.radioButton_logAllCase->setChecked(true);
	}
	else if (VISION_LOG_FAIL == nLogType)
	{
		ui.radioButton_logFailCase->setChecked(true);
		ui.radioButton_logAllCase->setChecked(false);
	}
	ui.groupBox_LogType->setEnabled(bLogAllCase);

	if (bLogAllCase)
	{
		if (VISION_LOG_ALL == nLogType)
		{
			Vision::PR_SetDebugMode(Vision::PR_DEBUG_MODE::LOG_ALL_CASE);
		}
		else if (VISION_LOG_FAIL == nLogType)
		{
			Vision::PR_SetDebugMode(Vision::PR_DEBUG_MODE::LOG_FAIL_CASE);
		}
	}
	else
	{
		Vision::PR_SetDebugMode(Vision::PR_DEBUG_MODE::DISABLED);
	}

	bool bAutoClearRecord = System->getParam("vision_record_auto_clear").toBool();
	ui.checkBox_AutoClearRecord->setChecked(bAutoClearRecord);

	connect(ui.checkBox_LogAllCase, SIGNAL(stateChanged(int)), SLOT(onLogAllCase(int)));
	connect(ui.radioButton_logFailCase, SIGNAL(toggled(bool)), SLOT(onClickLogFailCase(bool)));
	connect(ui.radioButton_logAllCase, SIGNAL(toggled(bool)), SLOT(onClickLogAllCase(bool)));
	connect(ui.pushButton_PRInit, SIGNAL(clicked()), SLOT(onInitPRSystem()));
	connect(ui.pushButton_PRRelease, SIGNAL(clicked()), SLOT(onUninitPRSystem()));
	connect(ui.pushButton_PRClearRecord, SIGNAL(clicked()), SLOT(onClearAllRecords()));
    connect(ui.pushButton_PRDumpTimeLog, SIGNAL(clicked()), SLOT(onDumpVisionTimeLog()));
	connect(ui.checkBox_AutoClearRecord, SIGNAL(stateChanged(int)), SLOT(onAutoClearRecord(int)));

	bool bStartUpHomeEnable = System->getParam("auto_startup_home_enable").toBool();
	ui.checkBox_startUpHome->setChecked(bStartUpHomeEnable);

	bool bStartUpZReadyEnable = System->getParam("auto_startup_zready_enable").toBool();
	ui.checkBox_startUpZReady->setChecked(bStartUpZReadyEnable);

	bool bStartUpLoadDBEnable = System->getParam("auto_startup_loaddb_enable").toBool();
	ui.checkBox_startUpLoadDB->setChecked(bStartUpLoadDBEnable);

	bool bStartUpDLPEnable = System->getParam("auto_startup_dlp_enable").toBool();
	ui.checkBox_startUpDLPInit->setChecked(bStartUpDLPEnable);

	int nZReadyID = System->getParam("auto_startup_zready_id").toInt();
	ui.lineEdit_zReady->setText(QString("%1").arg(nZReadyID));

	QString szDBPath = System->getParam("auto_startup_db_path").toString();
	ui.lineEdit_DBPath->setText(QString("%1").arg(szDBPath));

	connect(ui.checkBox_startUpHome, SIGNAL(stateChanged(int)), SLOT(onCheckStartUpHome(int)));
	connect(ui.checkBox_startUpZReady, SIGNAL(stateChanged(int)), SLOT(onCheckStartZReady(int)));
	connect(ui.checkBox_startUpLoadDB, SIGNAL(stateChanged(int)), SLOT(onCheckStartLoadDB(int)));
	connect(ui.checkBox_startUpDLPInit, SIGNAL(stateChanged(int)), SLOT(onCheckStartDLPInit(int)));
	connect(ui.pushButton_saveZReady, SIGNAL(clicked()), SLOT(onSaveZReady()));
	connect(ui.pushButton_selectDBPath, SIGNAL(clicked()), SLOT(onSelectDBPath()));
	connect(ui.pushButton_saveDBPath, SIGNAL(clicked()), SLOT(onSaveDBPath()));

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

void QWorkSetting::onDlpModeIndexChanged(int index)
{
	int nIndex = index;
}

void QWorkSetting::onTriggerModeIndexChanged(int index)
{
    int nIndex = index;
}

void QWorkSetting::onDlpModeSave()
{
	int nDlpMode = ui.comboBox_dlpMode->currentIndex();
    System->setSysRunMode(nDlpMode);

    int nTriggerMode = ui.comboBox_triggerMode->currentIndex();
    System->setParam("lighting_trigger_mode", nTriggerMode);

	if (QMessageBox::Ok == QMessageBox::warning(NULL, QStringLiteral("提示"),
		QStringLiteral("系统配置已修改，请务必关闭软件并重新启动!"), QMessageBox::Ok))
	{

	}
}

void QWorkSetting::onTriggerNumIndexChanged(int iIndex)
{
	int nDlpNumIndex = ui.comboBox_triggerNum->currentIndex();
}

void QWorkSetting::onTriggerSave()
{
	int nDlpNumIndex = ui.comboBox_triggerNum->currentIndex();
	System->setParam("motion_trigger_dlp_num_index", nDlpNumIndex);

	int nPatternNum = ui.lineEdit_trigger_caputured_num->text().toInt();
	double dPatternExposure = ui.lineEdit_trigger_pattern_exposure->text().toDouble();
	double dPatternPeriod = ui.lineEdit_trigger_pattern_period->text().toDouble();
	double dLightExposure = ui.lineEdit_trigger_light_exposure->text().toDouble();
	double dLightPeriod = ui.lineEdit_trigger_light_period->text().toDouble();

	System->setParam("motion_trigger_pattern_num", nPatternNum);
	System->setParam("motion_trigger_pattern_exposure", dPatternExposure);
	System->setParam("motion_trigger_pattern_period", dPatternPeriod);
	System->setParam("motion_trigger_light_exposure", dLightExposure);
	System->setParam("motion_trigger_light_period", dLightPeriod);
}

void QWorkSetting::onLogAllCase(int iState)
{
	int data = 0;
	if (Qt::Checked == iState)data = 1;

	System->setParam("camera_log_allcase_enable", (bool)data);
	ui.groupBox_LogType->setEnabled((bool)data);

	if ((bool)data)
	{
		int nLogType = System->getParam("vision_log_type").toInt();
		if (VISION_LOG_ALL == nLogType)
		{
			Vision::PR_SetDebugMode(Vision::PR_DEBUG_MODE::LOG_ALL_CASE);
		}
		else if (VISION_LOG_FAIL == nLogType)
		{
			Vision::PR_SetDebugMode(Vision::PR_DEBUG_MODE::LOG_FAIL_CASE);
		}
	}
	else
	{
		Vision::PR_SetDebugMode(Vision::PR_DEBUG_MODE::DISABLED);
	}
}

void QWorkSetting::onClickLogFailCase(bool s)
{
	if (s)
	{
		System->setParam("vision_log_type", (int)VISION_LOG_FAIL);
		Vision::PR_SetDebugMode(Vision::PR_DEBUG_MODE::LOG_FAIL_CASE);
	}
}

void QWorkSetting::onClickLogAllCase(bool s)
{
	if (s)
	{
		System->setParam("vision_log_type", (int)VISION_LOG_ALL);
		Vision::PR_SetDebugMode(Vision::PR_DEBUG_MODE::LOG_ALL_CASE);
	}
}

void QWorkSetting::onInitPRSystem()
{
	Vision::VisionStatus retStatus = Vision::PR_Init();
	if (retStatus == Vision::VisionStatus::OK)
	{
		QMessageBox::information(this, QStringLiteral("信息"), QStringLiteral("初始化成功"));
	}
	else
	{
		System->setTrackInfo(QString("Error at PR_Init, error code = %1").arg((int)retStatus));
	}
}

void QWorkSetting::onUninitPRSystem()
{
	/*Vision::VisionStatus retStatus = Vision::PR_Init();
	if (retStatus == Vision::VisionStatus::OK)
	{
	QMessageBox::information(this, QStringLiteral("信息"), QStringLiteral("初始化成功"));
	}
	else
	{
	System->setTrackInfo(QString("Error at PR_Init, error code = %1").arg((int)retStatus));
	}*/
}

void QWorkSetting::onClearAllRecords()
{
	if (QMessageBox::Ok == QMessageBox::question(NULL, QStringLiteral("信息提示"),
		QStringLiteral("全部清除 Records ？"), QMessageBox::Ok, QMessageBox::Cancel))
	{
		Vision::VisionStatus retStatus = Vision::PR_FreeAllRecord();
		if (retStatus == Vision::VisionStatus::OK)
		{
			QMessageBox::information(this, QStringLiteral("信息"), QStringLiteral("清除Records成功"));
		}
		else
		{
			System->setTrackInfo(QString("Error at PR_FreeAllRecord, error code = %1").arg((int)retStatus));
		}
	}
}

void QWorkSetting::onDumpVisionTimeLog()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter(tr("Log Files (*.log *.txt)"));
    dialog.setViewMode(QFileDialog::Detail);
    if ( ! dialog.exec() )
        return;

    QStringList fileNames = dialog.selectedFiles();
    std::string filePath = fileNames[0].toStdString();

    std::async( std::launch::async, 
        [filePath] ()
        { Vision::PR_DumpTimeLog(filePath); }
    );
}

void QWorkSetting::onAutoClearRecord(int iState)
{
	int data = 0;
	if (Qt::Checked == iState)data = 1;

	System->setParam("vision_record_auto_clear", (bool)data);
}

void QWorkSetting::onCheckStartUpHome(int iState)
{
	int data = 0;
	if (Qt::Checked == iState)data = 1;

	System->setParam("auto_startup_home_enable", (bool)data);
}

void QWorkSetting::onCheckStartZReady(int iState)
{
	int data = 0;
	if (Qt::Checked == iState)data = 1;

	System->setParam("auto_startup_zready_enable", (bool)data);
}

void QWorkSetting::onCheckStartLoadDB(int iState)
{
	int data = 0;
	if (Qt::Checked == iState)data = 1;

	System->setParam("auto_startup_loaddb_enable", (bool)data);
}

void QWorkSetting::onCheckStartDLPInit(int iState)
{
	int data = 0;
	if (Qt::Checked == iState)data = 1;

	System->setParam("auto_startup_dlp_enable", (bool)data);
}

void QWorkSetting::onSaveZReady()
{
	int nZReadyID = ui.lineEdit_zReady->text().toInt();
	System->setParam("auto_startup_zready_id", nZReadyID);
}

void QWorkSetting::onSelectDBPath()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFile);
	dialog.setAcceptMode(QFileDialog::AcceptOpen);
	dialog.setNameFilter(tr("Project Files (*.aoi)"));
	dialog.setViewMode(QFileDialog::Detail);
	QStringList fileNames;
	if (dialog.exec())  {
		fileNames = dialog.selectedFiles();
	}
	else
		return;

	QString str = fileNames[0];
	if (!str.isEmpty())ui.lineEdit_DBPath->setText(str);
}

void QWorkSetting::onSaveDBPath()
{
	QString str = ui.lineEdit_DBPath->text();
	if (!str.isEmpty()) System->setParam("auto_startup_db_path", str);
}