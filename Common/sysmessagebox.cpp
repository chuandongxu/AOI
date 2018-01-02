#include "sysmessagebox.h"

QSysMessageBox::QSysMessageBox(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.pushButton,SIGNAL(clicked()),SLOT(onBtnClick()));

	QString path = QApplication::applicationDirPath();
	path += "/image/label_log3.png";

	QPixmap pixmap;
	pixmap.load(path);

	ui.messageBox_icon->setPixmap(pixmap);
}

QSysMessageBox::~QSysMessageBox()
{

}

void QSysMessageBox::setTitle(const QString &str)
{
	ui.label->setText(str);
}

void QSysMessageBox::setText(const QString &msg)
{
	//ui.label->setText(msg);
	ui.plainTextEdit->setPlainText(msg);
}

void QSysMessageBox::enableCloseBtn(bool flag)
{
	if(flag)ui.pushButton->show();
	else ui.pushButton->hide();
}

void QSysMessageBox::onBtnClick()
{
	this->hide();
}