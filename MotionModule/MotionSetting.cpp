#include "MotionSetting.h"
#include "../common/SystemData.h"
#include "../include/IdDefine.h"
#include <QVBoxLayout>

MotionSetting::MotionSetting(MotionControl* pCtrl, QWidget *parent)
	: m_pCtrl(pCtrl), QWidget(parent)
{
	ui.setupUi(this);

	m_pMotionIO = new MotionIO(m_pCtrl);
	m_pMotionMtr = new MotionMotor(m_pCtrl);

	initUI();	

	//m_timer = new QTimer(this);
	//connect(m_timer, SIGNAL(timeout()), this, SLOT(updataIO()));
	//m_timer->start(200);
}

MotionSetting::~MotionSetting()
{
	//if (m_timer)
	//{
	//	delete m_timer;
	//	m_timer = NULL;
	//}

	if (m_pMotionIO)
	{
		delete m_pMotionIO;
		m_pMotionIO = NULL;
	}

	if (m_pMotionMtr)
	{
		delete m_pMotionMtr;
		m_pMotionMtr = NULL;
	}
}

void MotionSetting::initUI()
{
	// IO	
	QVBoxLayout *pVLayout = new QVBoxLayout();
	pVLayout->addWidget(m_pMotionIO);
	ui.tab_2->setLayout(pVLayout);

	pVLayout = new QVBoxLayout();
	pVLayout->addWidget(m_pMotionMtr);
	ui.tab_1->setLayout(pVLayout);

	// Motion

	connect(ui.comboBox_triggerNum, SIGNAL(currentIndexChanged(int)), SLOT(onTriggerNumIndexChanged(int)));
	QStringList ls;
	ls << QStringLiteral("Ë«Í·DLP¼ì²â") << QStringLiteral("ËÄÍ·DLP¼ì²â");
	ui.comboBox_triggerNum->addItems(ls);
	int nDlpNumIndex = System->getParam("motion_trigger_dlp_num_index").toInt();
	ui.comboBox_triggerNum->setCurrentIndex(nDlpNumIndex);

	int nPatternNum = System->getParam("motion_trigger_pattern_num").toInt();
	double dPatternExposure = System->getParam("motion_trigger_pattern_exposure").toDouble();
	double dPatternPeriod = System->getParam("motion_trigger_pattern_period").toDouble();
	ui.lineEdit_trigger_caputured_num->setText(QString("%1").arg(nPatternNum));
	ui.lineEdit_trigger_pattern_exposure->setText(QString("%1").arg(dPatternExposure));
	ui.lineEdit_trigger_pattern_period->setText(QString("%1").arg(dPatternPeriod));

	connect(ui.pushButton_trigger_save, SIGNAL(clicked()), SLOT(onTriggerSave()));
}

void MotionSetting::onTriggerNumIndexChanged(int iIndex)
{
	int nDlpNumIndex = ui.comboBox_triggerNum->currentIndex();
}

void MotionSetting::onTriggerSave()
{
	int nDlpNumIndex = ui.comboBox_triggerNum->currentIndex();
	System->setParam("motion_trigger_dlp_num_index", nDlpNumIndex);

	int nPatternNum = ui.lineEdit_trigger_caputured_num->text().toInt();
	double dPatternExposure = ui.lineEdit_trigger_pattern_exposure->text().toDouble();
	double dPatternPeriod = ui.lineEdit_trigger_pattern_period->text().toDouble();

	System->setParam("motion_trigger_pattern_num", nPatternNum);
	System->setParam("motion_trigger_pattern_exposure", dPatternExposure);
	System->setParam("motion_trigger_pattern_period", dPatternPeriod);
}