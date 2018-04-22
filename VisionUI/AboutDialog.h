#pragma once

#include <QDialog>
#include "ui_AboutDialog.h"

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    AboutDialog(QWidget *parent = Q_NULLPTR);
    ~AboutDialog();

    void setVisionLibraryVersion(const QString &strVersion);
    void setDataStoreApiVersion(const QString &strVersion);

private:
    Ui::AboutDialog ui;
};
