#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QHBoxLayout>

class QSettingDialog : public QDialog
{
    Q_OBJECT
public:
    explicit QSettingDialog(QWidget *parent = 0);

signals:

public slots:

private:
    QTabWidget * m_tabWidget;
    QHBoxLayout * m_layout;
};

#endif // SETTINGDIALOG_H
