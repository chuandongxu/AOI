/********************************************************************************
** Form generated from reading UI file 'LoginEdit.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGINEDIT_H
#define UI_LOGINEDIT_H

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

class Ui_QLoginEdit
{
public:
    QLineEdit *lineEdit;
    QLineEdit *lineEdit_2;
    QLabel *label;
    QLabel *label_2;
    QPushButton *pushButton;
    QPushButton *pushButton_2;

    void setupUi(QWidget *QLoginEdit)
    {
        if (QLoginEdit->objectName().isEmpty())
            QLoginEdit->setObjectName(QStringLiteral("QLoginEdit"));
        QLoginEdit->resize(219, 168);
        lineEdit = new QLineEdit(QLoginEdit);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));
        lineEdit->setGeometry(QRect(85, 40, 113, 20));
        lineEdit_2 = new QLineEdit(QLoginEdit);
        lineEdit_2->setObjectName(QStringLiteral("lineEdit_2"));
        lineEdit_2->setGeometry(QRect(85, 75, 113, 20));
        lineEdit_2->setEchoMode(QLineEdit::Password);
        label = new QLabel(QLoginEdit);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(20, 40, 54, 12));
        label_2 = new QLabel(QLoginEdit);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(20, 75, 54, 12));
        pushButton = new QPushButton(QLoginEdit);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(20, 130, 75, 23));
        pushButton_2 = new QPushButton(QLoginEdit);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setGeometry(QRect(130, 130, 75, 23));

        retranslateUi(QLoginEdit);

        QMetaObject::connectSlotsByName(QLoginEdit);
    } // setupUi

    void retranslateUi(QWidget *QLoginEdit)
    {
        QLoginEdit->setWindowTitle(QApplication::translate("QLoginEdit", "QLoginEdit", 0));
        label->setText(QApplication::translate("QLoginEdit", "\347\224\250\346\210\267\345\220\215", 0));
        label_2->setText(QApplication::translate("QLoginEdit", "\345\257\206\347\240\201", 0));
        pushButton->setText(QApplication::translate("QLoginEdit", "\347\231\273\351\231\206", 0));
        pushButton_2->setText(QApplication::translate("QLoginEdit", "\345\217\226\346\266\210", 0));
    } // retranslateUi

};

namespace Ui {
    class QLoginEdit: public Ui_QLoginEdit {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINEDIT_H
