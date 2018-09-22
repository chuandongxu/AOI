#ifndef WIDGET_H
#define WIDGET_H

#include <QFrame>
#include "TopWidget.h"
#include "CenterWidget.h"
#include "leftwidget.h"
#include "bottonwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

class QAppMainWidget : public QFrame
{
    Q_OBJECT

public:
    QAppMainWidget(QWidget *parent = 0);
    ~QAppMainWidget();

    void setStateWidget(QWidget * w);
    void setCenterWidget(QWidget * w);
    void setTitle(const QString &title,const QString &ver);

protected slots:
    void onCloseBtnclick();
    void onChangeModuleType(const QVariantList &vars);

private:
    QWidget * m_dispBkWidget;
    QTopWidget * m_topwidget;
    QLeftWidget * m_leftWidget;
    QCenterWidget * m_centerWidget;
    QBottonWidget * m_bottonWidget;
    QVBoxLayout * m_dispBkLayout;
    QHBoxLayout * m_leftLayout;
    QVBoxLayout * m_centerLayout;
    QVBoxLayout * m_mainLayout;
};

#endif // WIDGET_H
