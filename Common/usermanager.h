#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QDialog>
#include "ui_usermanager.h"
#include <QStandardItemModel>

class QUserManager : public QDialog
{
    Q_OBJECT

public:
    QUserManager(QWidget *parent = 0);
    ~QUserManager();

protected slots:
    void onAddUser();
    void onDelUser();
    void onEditUser();

protected:
    void queryUser();
    void setHeader();

private:
    Ui::QUserManager ui;
    QStandardItemModel m_model;
};

#endif // USERMANAGER_H
