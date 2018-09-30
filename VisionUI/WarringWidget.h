#ifndef WARRINGWIDGET_H
#define WARRINGWIDGET_H

#include <QDialog>
#include <QTableView>
#include <QHBoxLayout>
#include <QStandardItemModel>

class QWarringWidget : public QDialog
{
    Q_OBJECT

public:
    QWarringWidget(QWidget *parent = NULL);
    ~QWarringWidget();

protected:
    void loadData();

private:
    QTableView * m_view;
    QHBoxLayout * m_layout;
    QStandardItemModel m_model;
};

#endif // WARRINGWIDGET_H
