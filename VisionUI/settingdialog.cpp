#include "settingdialog.h"
#include "../common/ModuleMgr.h"
#include <QDesktopWidget>
#include <qapplication.h>

QSettingDialog::QSettingDialog(QWidget *parent) :
    QDialog(parent)
{
    this->setFixedSize(1200,850);
    this->setWindowTitle(QStringLiteral("设置"));

    QDesktopWidget *w = QApplication::desktop();
    QRect r = w->screenGeometry();
    
    this->move((r.width() - 1200) / 2, (r.height() - 850) / 4);

    m_tabWidget = new QTabWidget;
    m_layout = new QHBoxLayout;

    m_layout->addWidget(m_tabWidget);
    this->setLayout(m_layout);

    QMoudleMgr * mgr = QMoudleMgr::instance();
    if(mgr)
    {
        QList<int> ids = mgr->getModelIdList();
        for(int i=0; i<ids.size(); i++)
        {
            QModuleInterface * p = mgr->getModule(ids[i]);
            if(p)
            {
                p->addSettingWiddget(m_tabWidget);
            }
        }
    }
}


