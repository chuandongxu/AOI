#pragma once

#include <QDialog>
#include "ui_DialogCreateGroup.h"

class DialogCreateGroup : public QDialog
{
    Q_OBJECT

public:
    DialogCreateGroup(QWidget *parent = Q_NULLPTR);
    ~DialogCreateGroup();
    QString getGroupName() const;

private:
    Ui::DialogCreateGroup ui;
};
