#include "sysmessagebox.h"

QSysMessageBox::QSysMessageBox(QWidget *parent, bool bInteract)
    : QDialog(parent)
{
    ui.setupUi(this);
    connect(ui.pushButton,SIGNAL(clicked()),SLOT(onBtnClick()));

    QString path = QApplication::applicationDirPath();
    path += "/image/label_log3.png";

    QPixmap pixmap;
    pixmap.load(path);

    ui.messageBox_icon->setPixmap(pixmap);
    if (bInteract) {
        ui.pushButton->hide();
        ui.buttonBox->show();
    }else {
        ui.pushButton->show();
        ui.buttonBox->hide();
    }
}

QSysMessageBox::~QSysMessageBox()
{
}

void QSysMessageBox::setTitle(const QString &str)
{
    QDialog::setWindowTitle(str);
    ui.label->setText(str);
}

void QSysMessageBox::setText(const QString &msg)
{
    //ui.label->setText(msg);
    ui.plainTextEdit->setPlainText(msg);
}

void QSysMessageBox::enableCloseBtn(bool flag)
{
    if(flag) ui.pushButton->show();
    else ui.pushButton->hide();
}

void QSysMessageBox::onBtnClick()
{
    this->hide();
}