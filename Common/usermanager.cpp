#include "usermanager.h"
#include <qheaderview.h>
#include "useredit.h"
#include "SystemData.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QMessagebox>

QUserManager::QUserManager(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	ui.tableView->setModel(&m_model);
	this->queryUser();

	connect(ui.pushButton,SIGNAL(clicked()),this,SLOT(onAddUser()));
	connect(ui.pushButton_2,SIGNAL(clicked()),this,SLOT(onEditUser()));
	connect(ui.pushButton_3,SIGNAL(clicked()),this,SLOT(onDelUser()));
}

QUserManager::~QUserManager()
{

}

void QUserManager::onAddUser()
{
	QUserEdit dlg;
	dlg.initUIData(false);
	if(dlg.exec() == QDialog::Accepted)
	{
		int row = m_model.rowCount();
		m_model.insertRow(row);
		
		m_model.setData(m_model.index(row,1),dlg.m_userName);
		if(dlg.m_level == USER_LEVEL_MANAGER)m_model.setData(m_model.index(row,2),USER_MANAGER);
		else if(dlg.m_level == USER_LEVEL_TECH)m_model.setData(m_model.index(row,2),USER_TECH);
		else if(dlg.m_level == USER_LEVEL_DEBUG)m_model.setData(m_model.index(row,2),USER_DEBUGER);
		else m_model.setData(m_model.index(row,2),USER_OPT);

		m_model.setData(m_model.index(row,3),dlg.m_password);
		m_model.setData(m_model.index(row,4),dlg.m_remark);

		QSqlQuery query;
		QString strQuery = QString("insert into T_userManager(userName,Level,password,remark) values('%0',%1,'%2','%3')")
			.arg(dlg.m_userName).arg(dlg.m_level).arg(dlg.m_password).arg(dlg.m_remark);
		query.exec(strQuery);
	}
}

void QUserManager::onDelUser()
{
	QModelIndex idx = ui.tableView->currentIndex();
	if(!idx.isValid())return;

	if(QMessageBox::Ok == QMessageBox::warning(NULL,QStringLiteral("确认"),
		QStringLiteral("请确认是当否要删除当前用户。"),QMessageBox::Ok,QMessageBox::Cancel))
	{
		int row = idx.row();
		QString userName = m_model.data(m_model.index(row,1)).toString();

		QSqlQuery query;
		QString strQuery = QString("delete from T_userManager where userName = '%0'").arg(userName);
		query.exec(strQuery);

		m_model.removeRow(row);
	}
}

void QUserManager::onEditUser()
{
	QUserEdit dlg;
	QModelIndex idx = ui.tableView->currentIndex();
	if(!idx.isValid())return;

	int row = idx.row();
	dlg.m_userName = m_model.data(m_model.index(row,1)).toString();
	dlg.m_password = m_model.data(m_model.index(row,3)).toString();
	QString szLevel = m_model.data(m_model.index(row, 2)).toString();
	if (szLevel == USER_MANAGER) dlg.m_level = USER_LEVEL_MANAGER;
	else if (szLevel == USER_TECH) dlg.m_level = USER_LEVEL_TECH;
	else if (szLevel == USER_DEBUGER) dlg.m_level = USER_LEVEL_DEBUG;
	else dlg.m_level = USER_LEVEL_OPT;
	dlg.m_remark = m_model.data(m_model.index(row,4)).toString();

	dlg.initUIData(true);
	if(dlg.exec() == QDialog::Accepted)
	{
		m_model.setData(m_model.index(row,1),dlg.m_userName);
		int level = dlg.m_level;
		if (level == USER_LEVEL_MANAGER)m_model.setData(m_model.index(row, 2), USER_MANAGER);
		else if (level == USER_LEVEL_TECH)m_model.setData(m_model.index(row, 2), USER_TECH);
		else if (level == USER_LEVEL_DEBUG)m_model.setData(m_model.index(row, 2), USER_DEBUGER);
		else m_model.setData(m_model.index(row, 2), USER_OPT);

		m_model.setData(m_model.index(row,3),dlg.m_password);
		m_model.setData(m_model.index(row,4),dlg.m_remark);

		//int id = m_model.data(m_model.index(row,0)).toInt();
		QSqlQuery query;
		QString strQuery = QString("update T_UserManager set Level=%0,password='%1',remark='%2' where userName = '%3'")
			.arg(dlg.m_level).arg(dlg.m_password).arg(dlg.m_remark).arg(dlg.m_userName);
		query.exec(strQuery);
	}
}

void QUserManager::setHeader()
{
	QStringList ls;
	ls << QStringLiteral("序号") << QStringLiteral("用户名") 
		<< QStringLiteral("用户权限") << QStringLiteral("密码") << QStringLiteral("备注");
	m_model.setHorizontalHeaderLabels(ls);

	ui.tableView->setColumnWidth(0,60);
	ui.tableView->setColumnWidth(1,120);
	ui.tableView->setColumnWidth(2,80);
	ui.tableView->setColumnHidden(3,true);
	ui.tableView->horizontalHeader()->setStretchLastSection(true);
}

void QUserManager::queryUser()
{
	m_model.clear();
	this->setHeader();

	QString str = QString("select id,userName,Level,password,remark from T_userManager");
	QSqlQuery query(str);
	QSqlRecord rec = query.record();

	int n = rec.count();
	//m_model.insertColumns(0,n);
	int row = 0;
	while (query.next())
	{
		m_model.insertRow(row);
		for(int i = 0; i< n; i++)
		{
			if(i == 0){}
			else if(i == 2)
			{
				int level = query.value(i).toInt();
				//if(level == 0)m_model.setData(m_model.index(row,i),USER_OPT);
				//else m_model.setData(m_model.index(row,i),USER_ADMIN);
				if(level == USER_LEVEL_MANAGER)m_model.setData(m_model.index(row,2),USER_MANAGER);
				else if(level == USER_LEVEL_TECH)m_model.setData(m_model.index(row,2),USER_TECH);
				else if(level == USER_LEVEL_DEBUG)m_model.setData(m_model.index(row,2),USER_DEBUGER);
				else m_model.setData(m_model.index(row,2),USER_OPT);
			}
			else
			{
				m_model.setData(m_model.index(row,i),query.value(i));
			}
		}
		row++;
	}
}