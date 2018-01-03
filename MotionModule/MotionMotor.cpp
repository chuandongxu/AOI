#include "MotionMotor.h"
#include "../Common/SystemData.h"
#include "../include/IdDefine.h"

#include "opencv2/opencv.hpp"

///////////////////////////////////////
MotionMotorOnLive::MotionMotorOnLive(MotionMotor* pMotor)
	: m_pMotor(pMotor)
{
	m_bQuit = false;
	m_bRuning = false;
}

void MotionMotorOnLive::run()
{
	m_bRuning = true;

	while (!m_bQuit)
	{
		m_pMotor->updataStatus();

		if (m_bQuit)break;

		QThread::msleep(200);
	}

	m_bRuning = false;
}
////////////////////////////////////////////

MotionMotor::MotionMotor(MotionControl* pCtrl, QWidget *parent)
	: m_pCtrl(pCtrl), QWidget(parent)
{
	ui.setupUi(this);

	m_pThreadOnLive = NULL;	

	initUI();

	loadConfig();	
	updateUI();
}

MotionMotor::~MotionMotor()
{
	if (m_pThreadOnLive)
	{
		delete m_pThreadOnLive;
		m_pThreadOnLive = NULL;
	}
}

void MotionMotor::initUI()
{
	// Init UI
	QStringList ls;
	ls << QStringLiteral("正方向") << QStringLiteral("负方向");
	ui.comboBox_home_dir0->addItems(ls);	
	ui.comboBox_home_dir1->addItems(ls);
	ui.comboBox_home_dir2->addItems(ls);
	ui.comboBox_home_dir3->addItems(ls);
	ui.comboBox_home_dir4->addItems(ls);
	ui.comboBox_home_dir5->addItems(ls);
	ui.comboBox_home_dir6->addItems(ls);
	ui.comboBox_home_dir7->addItems(ls);

	ls.clear();
	ls << QStringLiteral("回零") << QStringLiteral("回零并找索引");
	ui.comboBox_home_mode0->addItems(ls);
	ui.comboBox_home_mode1->addItems(ls);
	ui.comboBox_home_mode2->addItems(ls);
	ui.comboBox_home_mode3->addItems(ls);
	ui.comboBox_home_mode4->addItems(ls);
	ui.comboBox_home_mode5->addItems(ls);
	ui.comboBox_home_mode6->addItems(ls);
	ui.comboBox_home_mode7->addItems(ls);

	connect(ui.pushButton_enable, SIGNAL(clicked()), SLOT(onEnable()));
	connect(ui.pushButton_home, SIGNAL(clicked()), SLOT(onHome()));
	connect(ui.pushButton_disable, SIGNAL(clicked()), SLOT(onDisable()));
	connect(ui.pushButton_im_stop, SIGNAL(clicked()), SLOT(onImStop()));
	connect(ui.pushButton_clearError, SIGNAL(clicked()), SLOT(onClearError()));
	connect(ui.pushButton_moveRel, SIGNAL(clicked()), SLOT(onMoveRel()));
	connect(ui.pushButton_moveAbs, SIGNAL(clicked()), SLOT(onMoveAbs()));
	connect(ui.pushButton_stop, SIGNAL(clicked()), SLOT(onStop()));
	connect(ui.pushButton_stop_2, SIGNAL(clicked()), SLOT(onStop()));
	connect(ui.pushButton_saveMtr, SIGNAL(clicked()), SLOT(onParamSave()));


	connect(ui.pushButton_onLive, SIGNAL(clicked()), SLOT(onAutoLive()));
	connect(ui.pushButton_onStop, SIGNAL(clicked()), SLOT(onAutoStop()));
	ui.pushButton_onLive->setEnabled(true);
	ui.pushButton_onStop->setEnabled(false);

	ui.tableView_prof->setModel(&m_moveProfModel);
	ui.tableView_prof->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	
	connect(ui.pushButton_prof_add, SIGNAL(clicked()), SLOT(onAddProf()));
	connect(ui.pushButton_prof_del, SIGNAL(clicked()), SLOT(onDelProf()));
	connect(ui.pushButton_prof_save, SIGNAL(clicked()), SLOT(onProfSave()));

	ui.lineEdit_move_vel->setText(QString("%1").arg(20));
	ui.lineEdit_move_acc->setText(QString("%1").arg(10));
	ui.lineEdit_move_dec->setText(QString("%1").arg(10));

	ui.tableView_pt->setModel(&m_movePointModel);
	ui.tableView_pt->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	connect(ui.pushButton_pt_add, SIGNAL(clicked()), SLOT(onAddPoint()));
	connect(ui.pushButton_pt_del, SIGNAL(clicked()), SLOT(onDelPoint()));
	connect(ui.pushButton_pt_moveTo, SIGNAL(clicked()), SLOT(onPointMove()));
	connect(ui.pushButton_pt_save, SIGNAL(clicked()), SLOT(onPointSave()));

	ui.lineEdit_pt_posn->setText(QString("%1").arg(20));

	for (int i = 0; i < m_pCtrl->getMotorAxisNum(); i++)
	{
		ui.comboBox_pt_mtr->addItem(QString("%1").arg(m_pCtrl->getMotorAxisID(i)));
	}
}

void MotionMotor::loadConfig()
{
	loadMotorConfig();
	loadMtrProfConfig();
	loadMtrPointConfig();
}

void MotionMotor::updateUI()
{
	updateMotorUI();
	updateMtrProfData();
	updateMtrPointData();
}

//void MotionMotor::saveConfig()
//{
//
//}

void MotionMotor::loadMotorConfig()
{
	m_pCtrl->clearMotorParams();
	
	QString path = QApplication::applicationDirPath();
	path += "/data/";
	std::string  szMotorParamFile = QString(path + "motorParamData.yml").toStdString();
	
	cv::FileStorage fs(szMotorParamFile, cv::FileStorage::READ);

	if (!fs.isOpened())
	{
		for (int i = 0; i < AXIS_MOTOR_NUM; i++)
		{
			QMotorParam mtrParam;
			mtrParam._ID = i + 1;
			m_pCtrl->addMotorParam(mtrParam);
		}
		return;
	}		

	for (int i = 0; i < AXIS_MOTOR_NUM; i++)
	{
		QMotorParam mtrParam;

		cv::FileNode fileNode = fs[QString("ID_%1").arg(i).toStdString()];
		cv::read(fileNode, mtrParam._ID, 0);

		fileNode = fs[QString("name_%1").arg(i).toStdString()];
		cv::String name;
		cv::read(fileNode, name, "");
		mtrParam._name = name.c_str();

		fileNode = fs[QString("res_%1").arg(i).toStdString()];
		cv::read(fileNode, mtrParam._res, 0);

		fileNode = fs[QString("dir_%1").arg(i).toStdString()];
		int nDir = 0;
		cv::read(fileNode, nDir, 0);
		mtrParam._homeProf._dir = (QMtrHomeProfile::HomeDir)nDir;

		fileNode = fs[QString("mode_%1").arg(i).toStdString()];
		int nMode = 0;
		cv::read(fileNode, nMode, 0);
		mtrParam._homeProf._mode = (QMtrHomeProfile::HomeMode)nMode;

		fileNode = fs[QString("vel_%1").arg(i).toStdString()];
		cv::read(fileNode, mtrParam._homeProf._velPf._vel, 0);
		fileNode = fs[QString("acc_%1").arg(i).toStdString()];
		cv::read(fileNode, mtrParam._homeProf._velPf._acc, 0);
		fileNode = fs[QString("dec_%1").arg(i).toStdString()];
		cv::read(fileNode, mtrParam._homeProf._velPf._dec, 0);		

		m_pCtrl->addMotorParam(mtrParam);
	}

	fs.release();	
}

void MotionMotor::updateMotorUI()
{
	for (int i = 0; i < m_pCtrl->getMotorParamsNum(); i++)
	{
		QMotorParam mtrParam = m_pCtrl->getMotorParamByIndex(i);

		switch (i)
		{
		case 0:
			ui.lineEdit_name0->setText(mtrParam._name);
			ui.doubleSpinBox_resn0->setValue(mtrParam._res);
			ui.lineEdit_home_vec0->setText(QString("%1").arg(mtrParam._homeProf._velPf._vel));
			ui.lineEdit_home_acc0->setText(QString("%1").arg(mtrParam._homeProf._velPf._acc));
			ui.comboBox_home_dir0->setCurrentIndex(mtrParam._homeProf._dir);
			ui.comboBox_home_mode0->setCurrentIndex(mtrParam._homeProf._mode);
			break;
		case 1:
			ui.lineEdit_name1->setText(mtrParam._name);
			ui.doubleSpinBox_resn1->setValue(mtrParam._res);
			ui.lineEdit_home_vec1->setText(QString("%1").arg(mtrParam._homeProf._velPf._vel));
			ui.lineEdit_home_acc1->setText(QString("%1").arg(mtrParam._homeProf._velPf._acc));
			ui.comboBox_home_dir1->setCurrentIndex(mtrParam._homeProf._dir);
			ui.comboBox_home_mode1->setCurrentIndex(mtrParam._homeProf._mode);
			break;
		case 2:
			ui.lineEdit_name2->setText(mtrParam._name);
			ui.doubleSpinBox_resn2->setValue(mtrParam._res);
			ui.lineEdit_home_vec2->setText(QString("%1").arg(mtrParam._homeProf._velPf._vel));
			ui.lineEdit_home_acc2->setText(QString("%1").arg(mtrParam._homeProf._velPf._acc));
			ui.comboBox_home_dir2->setCurrentIndex(mtrParam._homeProf._dir);
			ui.comboBox_home_mode2->setCurrentIndex(mtrParam._homeProf._mode);
			break;
		case 3:
			ui.lineEdit_name3->setText(mtrParam._name);
			ui.doubleSpinBox_resn3->setValue(mtrParam._res);
			ui.lineEdit_home_vec3->setText(QString("%1").arg(mtrParam._homeProf._velPf._vel));
			ui.lineEdit_home_acc3->setText(QString("%1").arg(mtrParam._homeProf._velPf._acc));
			ui.comboBox_home_dir3->setCurrentIndex(mtrParam._homeProf._dir);
			ui.comboBox_home_mode3->setCurrentIndex(mtrParam._homeProf._mode);
			break;
		case 4:
			ui.lineEdit_name4->setText(mtrParam._name);
			ui.doubleSpinBox_resn4->setValue(mtrParam._res);
			ui.lineEdit_home_vec4->setText(QString("%1").arg(mtrParam._homeProf._velPf._vel));
			ui.lineEdit_home_acc4->setText(QString("%1").arg(mtrParam._homeProf._velPf._acc));
			ui.comboBox_home_dir4->setCurrentIndex(mtrParam._homeProf._dir);
			ui.comboBox_home_mode4->setCurrentIndex(mtrParam._homeProf._mode);
			break;
		case 5:
			ui.lineEdit_name5->setText(mtrParam._name);
			ui.doubleSpinBox_resn5->setValue(mtrParam._res);
			ui.lineEdit_home_vec5->setText(QString("%1").arg(mtrParam._homeProf._velPf._vel));
			ui.lineEdit_home_acc5->setText(QString("%1").arg(mtrParam._homeProf._velPf._acc));
			ui.comboBox_home_dir5->setCurrentIndex(mtrParam._homeProf._dir);
			ui.comboBox_home_mode5->setCurrentIndex(mtrParam._homeProf._mode);
			break;
		case 6:
			ui.lineEdit_name6->setText(mtrParam._name);
			ui.doubleSpinBox_resn6->setValue(mtrParam._res);
			ui.lineEdit_home_vec6->setText(QString("%1").arg(mtrParam._homeProf._velPf._vel));
			ui.lineEdit_home_acc6->setText(QString("%1").arg(mtrParam._homeProf._velPf._acc));
			ui.comboBox_home_dir6->setCurrentIndex(mtrParam._homeProf._dir);
			ui.comboBox_home_mode6->setCurrentIndex(mtrParam._homeProf._mode);
			break;
		case 7:
			ui.lineEdit_name7->setText(mtrParam._name);
			ui.doubleSpinBox_resn7->setValue(mtrParam._res);
			ui.lineEdit_home_vec7->setText(QString("%1").arg(mtrParam._homeProf._velPf._vel));
			ui.lineEdit_home_acc7->setText(QString("%1").arg(mtrParam._homeProf._velPf._acc));
			ui.comboBox_home_dir7->setCurrentIndex(mtrParam._homeProf._dir);
			ui.comboBox_home_mode7->setCurrentIndex(mtrParam._homeProf._mode);
			break;
		default:
			break;
		}		
	}
}

void MotionMotor::saveMotorConfig()
{
	for (int i = 0; i < m_pCtrl->getMotorParamsNum(); i++)
	{
		QMotorParam mtrParam = m_pCtrl->getMotorParamByIndex(i);

		switch (i)
		{
		case 0:
			mtrParam._name = ui.lineEdit_name0->text();
			mtrParam._res = ui.doubleSpinBox_resn0->value();
			mtrParam._homeProf._velPf._vel = ui.lineEdit_home_vec0->text().toDouble();
			mtrParam._homeProf._velPf._acc = ui.lineEdit_home_acc0->text().toDouble();
			mtrParam._homeProf._velPf._dec = mtrParam._homeProf._velPf._acc;
			mtrParam._homeProf._dir = (QMtrHomeProfile::HomeDir)ui.comboBox_home_dir0->currentIndex();
			mtrParam._homeProf._mode = (QMtrHomeProfile::HomeMode)ui.comboBox_home_mode0->currentIndex();
			break;
		case 1:
			mtrParam._name = ui.lineEdit_name1->text();
			mtrParam._res = ui.doubleSpinBox_resn1->value();
			mtrParam._homeProf._velPf._vel = ui.lineEdit_home_vec1->text().toDouble();
			mtrParam._homeProf._velPf._acc = ui.lineEdit_home_acc1->text().toDouble();
			mtrParam._homeProf._velPf._dec = mtrParam._homeProf._velPf._acc;
			mtrParam._homeProf._dir = (QMtrHomeProfile::HomeDir)ui.comboBox_home_dir1->currentIndex();
			mtrParam._homeProf._mode = (QMtrHomeProfile::HomeMode)ui.comboBox_home_mode1->currentIndex();
			break;
		case 2:
			mtrParam._name = ui.lineEdit_name2->text();
			mtrParam._res = ui.doubleSpinBox_resn2->value();
			mtrParam._homeProf._velPf._vel = ui.lineEdit_home_vec2->text().toDouble();
			mtrParam._homeProf._velPf._acc = ui.lineEdit_home_acc2->text().toDouble();
			mtrParam._homeProf._velPf._dec = mtrParam._homeProf._velPf._acc;
			mtrParam._homeProf._dir = (QMtrHomeProfile::HomeDir)ui.comboBox_home_dir2->currentIndex();
			mtrParam._homeProf._mode = (QMtrHomeProfile::HomeMode)ui.comboBox_home_mode2->currentIndex();
			break;
		case 3:
			mtrParam._name = ui.lineEdit_name3->text();
			mtrParam._res = ui.doubleSpinBox_resn3->value();
			mtrParam._homeProf._velPf._vel = ui.lineEdit_home_vec3->text().toDouble();
			mtrParam._homeProf._velPf._acc = ui.lineEdit_home_acc3->text().toDouble();
			mtrParam._homeProf._velPf._dec = mtrParam._homeProf._velPf._acc;
			mtrParam._homeProf._dir = (QMtrHomeProfile::HomeDir)ui.comboBox_home_dir3->currentIndex();
			mtrParam._homeProf._mode = (QMtrHomeProfile::HomeMode)ui.comboBox_home_mode3->currentIndex();
			break;
		case 4:
			mtrParam._name = ui.lineEdit_name4->text();
			mtrParam._res = ui.doubleSpinBox_resn4->value();
			mtrParam._homeProf._velPf._vel = ui.lineEdit_home_vec4->text().toDouble();
			mtrParam._homeProf._velPf._acc = ui.lineEdit_home_acc4->text().toDouble();
			mtrParam._homeProf._velPf._dec = mtrParam._homeProf._velPf._acc;
			mtrParam._homeProf._dir = (QMtrHomeProfile::HomeDir)ui.comboBox_home_dir4->currentIndex();
			mtrParam._homeProf._mode = (QMtrHomeProfile::HomeMode)ui.comboBox_home_mode4->currentIndex();
			break;
		case 5:
			mtrParam._name = ui.lineEdit_name5->text();
			mtrParam._res = ui.doubleSpinBox_resn5->value();
			mtrParam._homeProf._velPf._vel = ui.lineEdit_home_vec5->text().toDouble();
			mtrParam._homeProf._velPf._acc = ui.lineEdit_home_acc5->text().toDouble();
			mtrParam._homeProf._velPf._dec = mtrParam._homeProf._velPf._acc;
			mtrParam._homeProf._dir = (QMtrHomeProfile::HomeDir)ui.comboBox_home_dir5->currentIndex();
			mtrParam._homeProf._mode = (QMtrHomeProfile::HomeMode)ui.comboBox_home_mode5->currentIndex();
			break;
		case 6:
			mtrParam._name = ui.lineEdit_name6->text();
			mtrParam._res = ui.doubleSpinBox_resn6->value();
			mtrParam._homeProf._velPf._vel = ui.lineEdit_home_vec6->text().toDouble();
			mtrParam._homeProf._velPf._acc = ui.lineEdit_home_acc6->text().toDouble();
			mtrParam._homeProf._velPf._dec = mtrParam._homeProf._velPf._acc;
			mtrParam._homeProf._dir = (QMtrHomeProfile::HomeDir)ui.comboBox_home_dir6->currentIndex();
			mtrParam._homeProf._mode = (QMtrHomeProfile::HomeMode)ui.comboBox_home_mode6->currentIndex();
			break;
		case 7:
			mtrParam._name = ui.lineEdit_name7->text();
			mtrParam._res = ui.doubleSpinBox_resn7->value();
			mtrParam._homeProf._velPf._vel = ui.lineEdit_home_vec7->text().toDouble();
			mtrParam._homeProf._velPf._acc = ui.lineEdit_home_acc7->text().toDouble();
			mtrParam._homeProf._velPf._dec = mtrParam._homeProf._velPf._acc;
			mtrParam._homeProf._dir = (QMtrHomeProfile::HomeDir)ui.comboBox_home_dir7->currentIndex();
			mtrParam._homeProf._mode = (QMtrHomeProfile::HomeMode)ui.comboBox_home_mode7->currentIndex();
			break;
		default:
			break;
		}

		m_pCtrl->updateMotorParam(mtrParam._ID, mtrParam);
	}

	QString path = QApplication::applicationDirPath();
	path += "/data/";

	std::string szMotorParamFile = QString(path + "motorParamData.yml").toStdString();
	cv::FileStorage fs(szMotorParamFile, cv::FileStorage::WRITE);
	if (!fs.isOpened())
		return;

	for (int i = 0; i < AXIS_MOTOR_NUM; i++)
	{
		QMotorParam mtrParam = m_pCtrl->getMotorParamByIndex(i);

		write(fs, QString("ID_%1").arg(i).toStdString(), mtrParam._ID);
		write(fs, QString("name_%1").arg(i).toStdString(), mtrParam._name.toStdString());	
		write(fs, QString("res_%1").arg(i).toStdString(), mtrParam._res);
		write(fs, QString("dir_%1").arg(i).toStdString(), mtrParam._homeProf._dir);	
		write(fs, QString("mode_%1").arg(i).toStdString(), mtrParam._homeProf._mode);
		write(fs, QString("vel_%1").arg(i).toStdString(), mtrParam._homeProf._velPf._vel);
		write(fs, QString("acc_%1").arg(i).toStdString(), mtrParam._homeProf._velPf._acc);
		write(fs, QString("dec_%1").arg(i).toStdString(), mtrParam._homeProf._velPf._dec);
	}

	fs.release();
}

void MotionMotor::loadMtrProfConfig()
{
	m_pCtrl->clearMotorProfiles();

	QString path = QApplication::applicationDirPath();
	path += "/data/";
	std::string  szMotorFile = QString(path + "motorProfileData.yml").toStdString();

	cv::FileStorage fs(szMotorFile, cv::FileStorage::READ);

	if (!fs.isOpened())
	{
		return;
	}

	int dataNum = 0;
	cv::FileNode fileNode = fs[QString("prof_num").toStdString()];
	cv::read(fileNode, dataNum, 0);

	int nIncID = 0;
	fileNode = fs[QString("inc_ID").toStdString()];
	cv::read(fileNode, nIncID, 0);
	m_pCtrl->setMotorProfileID(nIncID);

	for (int i = 0; i < dataNum; i++)
	{
		QMtrMoveProfile mtrParam;

		fileNode = fs[QString("ID_%1").arg(i).toStdString()];
		cv::read(fileNode, mtrParam._ID, 0);

		fileNode = fs[QString("name_%1").arg(i).toStdString()];
		cv::String name;
		cv::read(fileNode, name, "");
		mtrParam._name = name.c_str();

		fileNode = fs[QString("vel_%1").arg(i).toStdString()];
		cv::read(fileNode, mtrParam._velPf._vel, 0);
		fileNode = fs[QString("acc_%1").arg(i).toStdString()];
		cv::read(fileNode, mtrParam._velPf._acc, 0);
		fileNode = fs[QString("dec_%1").arg(i).toStdString()];
		cv::read(fileNode, mtrParam._velPf._dec, 0);

		fileNode = fs[QString("smooth_%1").arg(i).toStdString()];
		cv::read(fileNode, mtrParam._smooth, 0);

		m_pCtrl->addMotorProfile(mtrParam);
	}

	fs.release();
}

void MotionMotor::updateMtrProfData()
{
	m_moveProfModel.clear();

	QStringList ls;
	ls << QStringLiteral("名称") << QStringLiteral("ID") << QStringLiteral("运行速度")
		<< QStringLiteral("加速度") << QStringLiteral("减速度") << QStringLiteral("平滑时间");
	m_moveProfModel.setHorizontalHeaderLabels(ls);

	for (int i = 0; i < m_pCtrl->getMotorProfilesNum(); i++)
	{
		int nr = m_moveProfModel.rowCount();
		m_moveProfModel.insertRow(nr);

		QMtrMoveProfile mtrProf = m_pCtrl->getMotorProfileByIndex(i);

		m_moveProfModel.setData(m_moveProfModel.index(nr, 0), mtrProf._name);
		m_moveProfModel.setData(m_moveProfModel.index(nr, 1), mtrProf._ID);
		m_moveProfModel.setData(m_moveProfModel.index(nr, 2), mtrProf._velPf._vel);
		m_moveProfModel.setData(m_moveProfModel.index(nr, 3), mtrProf._velPf._acc);
		m_moveProfModel.setData(m_moveProfModel.index(nr, 4), mtrProf._velPf._dec);
		m_moveProfModel.setData(m_moveProfModel.index(nr, 5), mtrProf._smooth);
	}

	ui.comboBox_pt_prof->clear();
	for (int i = 0; i < m_pCtrl->getMotorProfilesNum(); i++)
	{
		ui.comboBox_pt_prof->addItem(QString("%1").arg(m_pCtrl->getMotorProfileByIndex(i)._ID));
	}
}

void MotionMotor::saveMtrProfConfig()
{
	for (int i = 0; i < m_pCtrl->getMotorProfilesNum(); i++)
	{
		QMtrMoveProfile mtrParam = m_pCtrl->getMotorProfileByIndex(i);

		mtrParam._name = m_moveProfModel.data(m_moveProfModel.index(i, 0)).toString();
		mtrParam._velPf._vel = m_moveProfModel.data(m_moveProfModel.index(i, 2)).toDouble();
		mtrParam._velPf._acc = m_moveProfModel.data(m_moveProfModel.index(i, 3)).toDouble();
		mtrParam._velPf._dec = m_moveProfModel.data(m_moveProfModel.index(i, 4)).toDouble();
		mtrParam._smooth = m_moveProfModel.data(m_moveProfModel.index(i, 5)).toInt();

		m_pCtrl->updateMotorProfile(mtrParam._ID, mtrParam);
	}

	QString path = QApplication::applicationDirPath();
	path += "/data/";

	std::string szMotorParamFile = QString(path + "motorProfileData.yml").toStdString();
	cv::FileStorage fs(szMotorParamFile, cv::FileStorage::WRITE);
	if (!fs.isOpened())
		return;

	write(fs, QString("inc_ID").toStdString(), m_pCtrl->incrementMotorProfileID());
	write(fs, QString("prof_num").toStdString(), m_pCtrl->getMotorProfilesNum());

	for (int i = 0; i < m_pCtrl->getMotorProfilesNum(); i++)
	{
		QMtrMoveProfile mtrParam = m_pCtrl->getMotorProfileByIndex(i);

		write(fs, QString("ID_%1").arg(i).toStdString(), mtrParam._ID);
		write(fs, QString("name_%1").arg(i).toStdString(), mtrParam._name.toStdString());
		write(fs, QString("vel_%1").arg(i).toStdString(), mtrParam._velPf._vel);
		write(fs, QString("acc_%1").arg(i).toStdString(), mtrParam._velPf._acc);
		write(fs, QString("dec_%1").arg(i).toStdString(), mtrParam._velPf._dec);
		write(fs, QString("smooth_%1").arg(i).toStdString(), mtrParam._smooth);
	}

	fs.release();
}

void MotionMotor::loadMtrPointConfig()
{
	m_pCtrl->clearMotorPoints();

	QString path = QApplication::applicationDirPath();
	path += "/data/";
	std::string  szMotorFile = QString(path + "motorPointData.yml").toStdString();

	cv::FileStorage fs(szMotorFile, cv::FileStorage::READ);

	if (!fs.isOpened())
	{
		return;
	}

	int dataNum = 0;
	cv::FileNode fileNode = fs[QString("point_num").toStdString()];
	cv::read(fileNode, dataNum, 0);

	int nIncID = 0;
	fileNode = fs[QString("inc_ID").toStdString()];
	cv::read(fileNode, nIncID, 0);
	m_pCtrl->setMotorPointID(nIncID);

	for (int i = 0; i < dataNum; i++)
	{
		QMtrMovePoint mtrParam;

		fileNode = fs[QString("ID_%1").arg(i).toStdString()];
		cv::read(fileNode, mtrParam._ID, 0);

		fileNode = fs[QString("name_%1").arg(i).toStdString()];
		cv::String name;
		cv::read(fileNode, name, "");
		mtrParam._name = name.c_str();

		fileNode = fs[QString("pos_%1").arg(i).toStdString()];
		cv::read(fileNode, mtrParam._posn, 0);
		fileNode = fs[QString("axis_%1").arg(i).toStdString()];
		cv::read(fileNode, mtrParam._AxisID, 0);
		fileNode = fs[QString("prof_%1").arg(i).toStdString()];
		cv::read(fileNode, mtrParam._ProfID, 0);
		m_pCtrl->addMotorPoint(mtrParam);
	}

	fs.release();
}

void MotionMotor::updateMtrPointData()
{
	m_movePointModel.clear();

	QStringList ls;
	ls << QStringLiteral("名称") << QStringLiteral("ID") << QStringLiteral("运动轴")
		<< QStringLiteral("Profile") << QStringLiteral("目标位置");
	m_movePointModel.setHorizontalHeaderLabels(ls);

	for (int i = 0; i < m_pCtrl->getMotorPointsNum(); i++)
	{
		int nr = m_movePointModel.rowCount();
		m_movePointModel.insertRow(nr);

		QMtrMovePoint mtrPoint = m_pCtrl->getMotorPointByIndex(i);

		m_movePointModel.setData(m_movePointModel.index(nr, 0), mtrPoint._name);
		m_movePointModel.setData(m_movePointModel.index(nr, 1), mtrPoint._ID);
		m_movePointModel.setData(m_movePointModel.index(nr, 2), mtrPoint._AxisID);
		m_movePointModel.setData(m_movePointModel.index(nr, 3), mtrPoint._ProfID);
		m_movePointModel.setData(m_movePointModel.index(nr, 4), mtrPoint._posn);
	}
}

void MotionMotor::saveMtrPointConfig()
{
	for (int i = 0; i < m_pCtrl->getMotorPointsNum(); i++)
	{
		QMtrMovePoint mtrPoint = m_pCtrl->getMotorPointByIndex(i);

		mtrPoint._name = m_movePointModel.data(m_movePointModel.index(i, 0)).toString();
		mtrPoint._AxisID = m_movePointModel.data(m_movePointModel.index(i, 2)).toInt();
		mtrPoint._ProfID = m_movePointModel.data(m_movePointModel.index(i, 3)).toInt();
		mtrPoint._posn = m_movePointModel.data(m_movePointModel.index(i, 4)).toDouble();

		m_pCtrl->updateMotorPoint(mtrPoint._ID, mtrPoint);
	}

	QString path = QApplication::applicationDirPath();
	path += "/data/";

	std::string szMotorParamFile = QString(path + "motorPointData.yml").toStdString();
	cv::FileStorage fs(szMotorParamFile, cv::FileStorage::WRITE);
	if (!fs.isOpened())
		return;

	write(fs, QString("inc_ID").toStdString(), m_pCtrl->incrementMotorPointID());
	write(fs, QString("point_num").toStdString(), m_pCtrl->getMotorPointsNum());

	for (int i = 0; i < m_pCtrl->getMotorPointsNum(); i++)
	{
		QMtrMovePoint mtrPoint = m_pCtrl->getMotorPointByIndex(i);

		write(fs, QString("ID_%1").arg(i).toStdString(), mtrPoint._ID);
		write(fs, QString("name_%1").arg(i).toStdString(), mtrPoint._name.toStdString());
		write(fs, QString("axis_%1").arg(i).toStdString(), mtrPoint._AxisID);
		write(fs, QString("prof_%1").arg(i).toStdString(), mtrPoint._ProfID);
		write(fs, QString("pos_%1").arg(i).toStdString(), mtrPoint._posn);	
	}

	fs.release();
}

void MotionMotor::updataStatus()
{
	for (int i = 0; i < AXIS_MOTOR_NUM; i++)
	{
		int AxisID = m_pCtrl->getMotorAxisID(i);
		if (AxisID > 0)
		{
			double dPos = 0;
			m_pCtrl->getCurrentPos(AxisID, &dPos);

			switch (i)
			{
			case 0:
				ui.lineEdit_mtr_posn0->setText(QString("%1").arg(dPos));
				ui.label_mtr_status0->setText(m_pCtrl->getCurrentStatus(AxisID));
				break;
			case 1:
				ui.lineEdit_mtr_posn1->setText(QString("%1").arg(dPos));
				ui.label_mtr_status1->setText(m_pCtrl->getCurrentStatus(AxisID));
				break;
			case 2:
				ui.lineEdit_mtr_posn2->setText(QString("%1").arg(dPos));
				ui.label_mtr_status2->setText(m_pCtrl->getCurrentStatus(AxisID));
				break;
			case 3:
				ui.lineEdit_mtr_posn3->setText(QString("%1").arg(dPos));
				ui.label_mtr_status3->setText(m_pCtrl->getCurrentStatus(AxisID));
				break;
			case 4:
				ui.lineEdit_mtr_posn4->setText(QString("%1").arg(dPos));
				ui.label_mtr_status4->setText(m_pCtrl->getCurrentStatus(AxisID));
				break;
			case 5:
				ui.lineEdit_mtr_posn5->setText(QString("%1").arg(dPos));
				ui.label_mtr_status5->setText(m_pCtrl->getCurrentStatus(AxisID));
				break;
			case 6:
				ui.lineEdit_mtr_posn6->setText(QString("%1").arg(dPos));
				ui.label_mtr_status6->setText(m_pCtrl->getCurrentStatus(AxisID));
				break;
			case 7:
				ui.lineEdit_mtr_posn7->setText(QString("%1").arg(dPos));
				ui.label_mtr_status7->setText(m_pCtrl->getCurrentStatus(AxisID));
				break;
			default:
				break;
			}
		}
	}
}

void MotionMotor::onEnable()
{
	bool axisSelected[AXIS_MOTOR_NUM];
	getAxisSelected(axisSelected, AXIS_MOTOR_NUM);

	for (int i = 0; i < m_pCtrl->getMotorAxisNum(); i++)
	{
		if (axisSelected[i] && !m_pCtrl->isEnabled(m_pCtrl->getMotorAxisID(i)))
		{
			m_pCtrl->enable(m_pCtrl->getMotorAxisID(i));
		}		
	}
}

void MotionMotor::onDisable()
{
	bool axisSelected[AXIS_MOTOR_NUM];
	getAxisSelected(axisSelected, AXIS_MOTOR_NUM);

	for (int i = 0; i < m_pCtrl->getMotorAxisNum(); i++)
	{
		if (axisSelected[i] && m_pCtrl->isEnabled(m_pCtrl->getMotorAxisID(i)))
		{
			m_pCtrl->disable(m_pCtrl->getMotorAxisID(i));
		}
	}
}

void MotionMotor::onHome()
{
	bool axisSelected[AXIS_MOTOR_NUM];
	getAxisSelected(axisSelected, AXIS_MOTOR_NUM);

	for (int i = 0; i < m_pCtrl->getMotorAxisNum(); i++)
	{
		if (axisSelected[i] && m_pCtrl->isEnabled(m_pCtrl->getMotorAxisID(i)))
		{
			m_pCtrl->home(m_pCtrl->getMotorAxisID(i), false);
		}
	}
}

void MotionMotor::onImStop()
{
	bool axisSelected[AXIS_MOTOR_NUM];
	getAxisSelected(axisSelected, AXIS_MOTOR_NUM);

	for (int i = 0; i < m_pCtrl->getMotorAxisNum(); i++)
	{
		if (axisSelected[i] /*&& m_pCtrl->isEnabled(m_pCtrl->getMotorAxisID(i))*/)
		{
			m_pCtrl->EmStop(m_pCtrl->getMotorAxisID(i));
		}
	}
}

void MotionMotor::onClearError()
{
	bool axisSelected[AXIS_MOTOR_NUM];
	getAxisSelected(axisSelected, AXIS_MOTOR_NUM);

	for (int i = 0; i < m_pCtrl->getMotorAxisNum(); i++)
	{
		if (axisSelected[i] /*&& m_pCtrl->isEnabled(m_pCtrl->getMotorAxisID(i))*/)
		{
			m_pCtrl->clearError(m_pCtrl->getMotorAxisID(i));
		}
	}
}

void MotionMotor::onMoveRel()
{
	bool axisSelected[AXIS_MOTOR_NUM];
	getAxisSelected(axisSelected, AXIS_MOTOR_NUM);

	double dVec = ui.lineEdit_mtr_vec->text().toDouble();
	double acc = ui.lineEdit_mtr_acc->text().toDouble();
	double dec = ui.lineEdit_mtr_dec->text().toDouble();
	int smooth = ui.lineEdit_mtr_smooth->text().toInt();
	double dDist = ui.lineEdit_mtr_dist->text().toDouble();

	for (int i = 0; i < m_pCtrl->getMotorAxisNum(); i++)
	{
		if (axisSelected[i] && m_pCtrl->isEnabled(m_pCtrl->getMotorAxisID(i)))
		{
			double dCurPos = 0;
			if (!m_pCtrl->getCurrentPos(m_pCtrl->getMotorAxisID(i), &dCurPos))
			{
				break;
			}

			if (!m_pCtrl->move(m_pCtrl->getMotorAxisID(i), dVec, acc, dec, smooth, dCurPos + dDist, false))
			{
				System->setTrackInfo(QStringLiteral("Move Motor Error!, AxisID=%1").arg(m_pCtrl->getMotorAxisID(i)));
				break;
			}
		}		
	}
}

void MotionMotor::onMoveAbs()
{
	bool axisSelected[AXIS_MOTOR_NUM];
	getAxisSelected(axisSelected, AXIS_MOTOR_NUM);

	double dVec = ui.lineEdit_mtr_vec->text().toDouble();
	double acc = ui.lineEdit_mtr_acc->text().toDouble();
	double dec = ui.lineEdit_mtr_dec->text().toDouble();
	int smooth = ui.lineEdit_mtr_smooth->text().toInt();
	double dPos = ui.lineEdit_mtr_posn->text().toDouble();

	for (int i = 0; i < m_pCtrl->getMotorAxisNum(); i++)
	{
		if (axisSelected[i] && m_pCtrl->isEnabled(m_pCtrl->getMotorAxisID(i)))
		{
			if (!m_pCtrl->move(m_pCtrl->getMotorAxisID(i), dVec, acc, dec, smooth, dPos, false))
			{
				System->setTrackInfo(QStringLiteral("Move Motor Error!, AxisID=%1").arg(m_pCtrl->getMotorAxisID(i)));
				break;
			}
		}
	}
}

void MotionMotor::onStop()
{
	bool axisSelected[AXIS_MOTOR_NUM];
	getAxisSelected(axisSelected, AXIS_MOTOR_NUM);

	for (int i = 0; i < m_pCtrl->getMotorAxisNum(); i++)
	{
		if (axisSelected[i] && m_pCtrl->isEnabled(m_pCtrl->getMotorAxisID(i)))
		{
			m_pCtrl->stopMove(m_pCtrl->getMotorAxisID(i));
		}
	}
}

void MotionMotor::onParamSave()
{
	saveMotorConfig();
}

void MotionMotor::onAutoLive()
{
	if (!m_pThreadOnLive)
	{
		ui.pushButton_onLive->setEnabled(false);
		ui.pushButton_onStop->setEnabled(true);

		m_pThreadOnLive = new MotionMotorOnLive(this);
		m_pThreadOnLive->start();
	}
}

void MotionMotor::onAutoStop()
{
	if (m_pThreadOnLive)
	{
		m_pThreadOnLive->setQuitFlag();
		while (m_pThreadOnLive->isRuning())
		{
			QThread::msleep(10);
			QApplication::processEvents();
		}
		QThread::msleep(200);
		delete m_pThreadOnLive;
		m_pThreadOnLive = NULL;

		ui.pushButton_onLive->setEnabled(true);
		ui.pushButton_onStop->setEnabled(false);
	}
}

void MotionMotor::getAxisSelected(bool* axisIDs, int axisNum)
{
	for (int i = 0; i < axisNum; i++)
	{
		switch (i)
		{
		case  0:
			axisIDs[i] = ui.checkBox_ch0->isChecked();
			break;
		case  1:
			axisIDs[i] = ui.checkBox_ch1->isChecked();
			break;
		case  2:
			axisIDs[i] = ui.checkBox_ch2->isChecked();
			break;
		case  3:
			axisIDs[i] = ui.checkBox_ch3->isChecked();
			break;
		case  4:
			axisIDs[i] = ui.checkBox_ch4->isChecked();
			break;
		case  5:
			axisIDs[i] = ui.checkBox_ch5->isChecked();
			break;
		case  6:
			axisIDs[i] = ui.checkBox_ch6->isChecked();
			break;
		case  7:
			axisIDs[i] = ui.checkBox_ch7->isChecked();
			break;
		default:
			break;
		}
	}
}

void MotionMotor::onAddProf()
{
	QMtrMoveProfile mtrParam;

	mtrParam._name = ui.lineEdit_move_name->text();
	mtrParam._ID = m_pCtrl->incrementMotorProfileID();
	mtrParam._velPf._vel = ui.lineEdit_move_vel->text().toDouble();
	mtrParam._velPf._acc = ui.lineEdit_move_acc->text().toDouble();
	mtrParam._velPf._dec = ui.lineEdit_move_dec->text().toDouble();
	mtrParam._smooth = 20;
	
	m_pCtrl->addMotorProfile(mtrParam);

	int nr = m_moveProfModel.rowCount();
	m_moveProfModel.insertRow(nr);

	m_moveProfModel.setData(m_moveProfModel.index(nr, 0), mtrParam._name);
	m_moveProfModel.setData(m_moveProfModel.index(nr, 1), mtrParam._ID);
	m_moveProfModel.setData(m_moveProfModel.index(nr, 2), mtrParam._velPf._vel);
	m_moveProfModel.setData(m_moveProfModel.index(nr, 3), mtrParam._velPf._acc);
	m_moveProfModel.setData(m_moveProfModel.index(nr, 4), mtrParam._velPf._dec);
	m_moveProfModel.setData(m_moveProfModel.index(nr, 5), mtrParam._smooth);

	updateMtrProfData();
}

void MotionMotor::onDelProf()
{
	QModelIndex index = ui.tableView_prof->currentIndex();
	int indexrow = index.row();

	int nProID = m_moveProfModel.data(m_moveProfModel.index(indexrow, 1)).toInt();
	m_pCtrl->removeMotorProfile(nProID);

	m_moveProfModel.removeRow(indexrow);

	updateMtrProfData();
}

void MotionMotor::onProfSave()
{
	saveMtrProfConfig();
}

void MotionMotor::onAddPoint()
{
	QMtrMovePoint mtrParam;

	mtrParam._name = ui.lineEdit_pt_name->text();
	mtrParam._ID = m_pCtrl->incrementMotorPointID();
	mtrParam._AxisID = ui.comboBox_pt_mtr->currentText().toInt();
	mtrParam._ProfID = ui.comboBox_pt_prof->currentText().toInt();
	mtrParam._posn = ui.lineEdit_pt_posn->text().toDouble();

	m_pCtrl->addMotorPoint(mtrParam);

	int nr = m_movePointModel.rowCount();
	m_movePointModel.insertRow(nr);

	m_movePointModel.setData(m_movePointModel.index(nr, 0), mtrParam._name);
	m_movePointModel.setData(m_movePointModel.index(nr, 1), mtrParam._ID);
	m_movePointModel.setData(m_movePointModel.index(nr, 2), mtrParam._AxisID);
	m_movePointModel.setData(m_movePointModel.index(nr, 3), mtrParam._ProfID);
	m_movePointModel.setData(m_movePointModel.index(nr, 4), mtrParam._posn);
}

void MotionMotor::onDelPoint()
{
	QModelIndex index = ui.tableView_pt->currentIndex();
	int indexrow = index.row();

	int nPointID = m_movePointModel.data(m_movePointModel.index(indexrow, 1)).toInt();
	m_pCtrl->removeMotorPoint(nPointID);

	m_movePointModel.removeRow(indexrow);
}

void MotionMotor::onPointMove()
{
	QModelIndex index = ui.tableView_pt->currentIndex();
	int indexrow = index.row();

	int nPointID = m_movePointModel.data(m_movePointModel.index(indexrow, 1)).toInt();

	if (!m_pCtrl->moveToPos(nPointID, false))
	{
		System->setTrackInfo("move to position error");
	}
}

void MotionMotor::onPointSave()
{
	saveMtrPointConfig();
}

