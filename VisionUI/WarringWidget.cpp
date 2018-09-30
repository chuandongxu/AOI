#include "WarringWidget.h"
#include "../Common/SystemData.h"
#include <qheaderview>
#include <QByteArray>

QWarringWidget::QWarringWidget(QWidget *parent)
    : QDialog(parent)
{
    m_view = new QTableView;
    m_layout = new QHBoxLayout;
    m_layout->addWidget(m_view);

    this->setLayout(m_layout);
    this->setFixedSize(800,600);
    
    this->loadData();
    m_view->setModel(&m_model);
    QStringList ls;
    ls << QStringLiteral("时间") <<  QStringLiteral("错误代码") << QStringLiteral("描述");
    m_model.setHorizontalHeaderLabels(ls);

    m_view->setColumnWidth(0,150);
    m_view->setColumnWidth(1,80);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QHeaderView * header = m_view->horizontalHeader();
    header->setStretchLastSection(true);
}

QWarringWidget::~QWarringWidget()
{
}

void QWarringWidget::loadData()
{
    m_model.clear();
    QSystem::queryWarring(m_model);    
}