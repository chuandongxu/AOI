/********************************************************************************
** Form generated from reading UI file 'sysmessagebox.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SYSMESSAGEBOX_H
#define UI_SYSMESSAGEBOX_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_QSysMessageBox
{
public:
    QLabel *messageBox_icon;
    QPlainTextEdit *plainTextEdit;
    QPushButton *pushButton;
    QLabel *label;

    void setupUi(QDialog *QSysMessageBox)
    {
        if (QSysMessageBox->objectName().isEmpty())
            QSysMessageBox->setObjectName(QStringLiteral("QSysMessageBox"));
        QSysMessageBox->setWindowModality(Qt::ApplicationModal);
        QSysMessageBox->setEnabled(true);
        QSysMessageBox->resize(700, 200);
        messageBox_icon = new QLabel(QSysMessageBox);
        messageBox_icon->setObjectName(QStringLiteral("messageBox_icon"));
        messageBox_icon->setGeometry(QRect(20, 30, 120, 120));
        plainTextEdit = new QPlainTextEdit(QSysMessageBox);
        plainTextEdit->setObjectName(QStringLiteral("plainTextEdit"));
        plainTextEdit->setEnabled(false);
        plainTextEdit->setGeometry(QRect(180, 60, 500, 81));
        QFont font;
        font.setPointSize(16);
        plainTextEdit->setFont(font);
        plainTextEdit->setFrameShape(QFrame::NoFrame);
        pushButton = new QPushButton(QSysMessageBox);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(600, 160, 75, 23));
        label = new QLabel(QSysMessageBox);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(180, 30, 500, 31));
        QFont font1;
        font1.setPointSize(20);
        label->setFont(font1);

        retranslateUi(QSysMessageBox);

        QMetaObject::connectSlotsByName(QSysMessageBox);
    } // setupUi

    void retranslateUi(QDialog *QSysMessageBox)
    {
        QSysMessageBox->setWindowTitle(QApplication::translate("QSysMessageBox", "QSysMessageBox", 0));
        messageBox_icon->setText(QString());
        pushButton->setText(QApplication::translate("QSysMessageBox", "\345\205\263\351\227\255", 0));
        label->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class QSysMessageBox: public Ui_QSysMessageBox {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SYSMESSAGEBOX_H
