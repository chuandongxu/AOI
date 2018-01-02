/********************************************************************************
** Form generated from reading UI file 'userdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_USERDIALOG_H
#define UI_USERDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QUserDialog
{
public:
    QLabel *label;
    QLineEdit *lineEdit;
    QLineEdit *lineEdit_2;
    QLabel *label_2;
    QLabel *label_user;
    QPushButton *pushButton;
    QPushButton *pushButton_2;

    void setupUi(QWidget *QUserDialog)
    {
        if (QUserDialog->objectName().isEmpty())
            QUserDialog->setObjectName(QStringLiteral("QUserDialog"));
        QUserDialog->resize(371, 161);
        label = new QLabel(QUserDialog);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(120, 40, 51, 16));
        lineEdit = new QLineEdit(QUserDialog);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));
        lineEdit->setGeometry(QRect(172, 40, 161, 20));
        lineEdit_2 = new QLineEdit(QUserDialog);
        lineEdit_2->setObjectName(QStringLiteral("lineEdit_2"));
        lineEdit_2->setGeometry(QRect(172, 70, 161, 20));
        lineEdit_2->setEchoMode(QLineEdit::Password);
        label_2 = new QLabel(QUserDialog);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(120, 70, 54, 12));
        label_user = new QLabel(QUserDialog);
        label_user->setObjectName(QStringLiteral("label_user"));
        label_user->setGeometry(QRect(30, 41, 48, 48));
        pushButton = new QPushButton(QUserDialog);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(70, 120, 75, 23));
        pushButton_2 = new QPushButton(QUserDialog);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setGeometry(QRect(270, 120, 75, 23));

        retranslateUi(QUserDialog);

        QMetaObject::connectSlotsByName(QUserDialog);
    } // setupUi

    void retranslateUi(QWidget *QUserDialog)
    {
        QUserDialog->setWindowTitle(QApplication::translate("QUserDialog", "QUserDialog", 0));
        label->setText(QApplication::translate("QUserDialog", "\347\224\250\346\210\267\345\220\215", 0));
        lineEdit_2->setInputMask(QString());
        label_2->setText(QApplication::translate("QUserDialog", "\345\257\206\347\240\201", 0));
        label_user->setText(QString());
        pushButton->setText(QApplication::translate("QUserDialog", "\347\241\256\345\256\232", 0));
        pushButton_2->setText(QApplication::translate("QUserDialog", "\345\217\226\346\266\210", 0));
    } // retranslateUi

};

namespace Ui {
    class QUserDialog: public Ui_QUserDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_USERDIALOG_H
