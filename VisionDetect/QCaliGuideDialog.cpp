#include "QCaliGuideDialog.h"
#include <QApplication>

QCaliGuideDialog::QCaliGuideDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    QString path = QApplication::applicationDirPath();
    path += "/image/";

    QPixmap pixmap(path + "caliBg.png");

    ui.label->setPixmap(pixmap);

    connect(ui.pushButton_OK, SIGNAL(clicked()), SLOT(onOk()));
    connect(ui.pushButton_Cancel, SIGNAL(clicked()), SLOT(onExit()));
}

QCaliGuideDialog::~QCaliGuideDialog()
{
}

void QCaliGuideDialog::onOk()
{
    QDialog::accept();
}

void QCaliGuideDialog::onExit()
{
    QDialog::reject();
}
