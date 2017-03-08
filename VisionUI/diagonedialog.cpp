#include "diagonedialog.h"
#include "../common/ModuleMgr.h"

QDiagoneDialog::QDiagoneDialog(QWidget *parent) :
    QDialog(parent)
{
    this->setFixedSize(820,620);
    this->setWindowTitle(QStringLiteral("诊断"));

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
                p->addDiagonseWidget(m_tabWidget);
            }
        }
    }
}
