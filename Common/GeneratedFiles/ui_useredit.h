/********************************************************************************
** Form generated from reading UI file 'useredit.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_USEREDIT_H
#define UI_USEREDIT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>

QT_BEGIN_NAMESPACE

class Ui_QUserEdit
{
public:
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QLineEdit *lineEdit;
    QLineEdit *lineEdit_2;
    QLineEdit *lineEdit_3;
    QComboBox *comboBox;
    QTextEdit *textEdit;
    QPushButton *pushButton;
    QPushButton *pushButton_2;

    void setupUi(QDialog *QUserEdit)
    {
        if (QUserEdit->objectName().isEmpty())
            QUserEdit->setObjectName(QStringLiteral("QUserEdit"));
        QUserEdit->resize(454, 274);
        label = new QLabel(QUserEdit);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(70, 30, 54, 12));
        label_2 = new QLabel(QUserEdit);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(70, 60, 54, 12));
        label_3 = new QLabel(QUserEdit);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(70, 90, 54, 12));
        label_4 = new QLabel(QUserEdit);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(70, 120, 54, 12));
        label_5 = new QLabel(QUserEdit);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setGeometry(QRect(70, 150, 54, 12));
        lineEdit = new QLineEdit(QUserEdit);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));
        lineEdit->setGeometry(QRect(150, 30, 191, 20));
        lineEdit_2 = new QLineEdit(QUserEdit);
        lineEdit_2->setObjectName(QStringLiteral("lineEdit_2"));
        lineEdit_2->setGeometry(QRect(150, 60, 191, 20));
        lineEdit_2->setEchoMode(QLineEdit::Password);
        lineEdit_3 = new QLineEdit(QUserEdit);
        lineEdit_3->setObjectName(QStringLiteral("lineEdit_3"));
        lineEdit_3->setGeometry(QRect(150, 90, 191, 20));
        lineEdit_3->setEchoMode(QLineEdit::Password);
        comboBox = new QComboBox(QUserEdit);
        comboBox->setObjectName(QStringLiteral("comboBox"));
        comboBox->setGeometry(QRect(150, 120, 91, 22));
        textEdit = new QTextEdit(QUserEdit);
        textEdit->setObjectName(QStringLiteral("textEdit"));
        textEdit->setGeometry(QRect(150, 150, 251, 71));
        pushButton = new QPushButton(QUserEdit);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(300, 240, 75, 23));
        pushButton_2 = new QPushButton(QUserEdit);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setGeometry(QRect(130, 240, 75, 23));

        retranslateUi(QUserEdit);

        QMetaObject::connectSlotsByName(QUserEdit);
    } // setupUi

    void retranslateUi(QDialog *QUserEdit)
    {
        QUserEdit->setWindowTitle(QApplication::translate("QUserEdit", "QUserEdit", 0));
        label->setText(QApplication::translate("QUserEdit", "\347\224\250\346\210\267\345\220\215", 0));
        label_2->setText(QApplication::translate("QUserEdit", "\345\257\206\347\240\201", 0));
        label_3->setText(QApplication::translate("QUserEdit", "\345\257\206\347\240\201\347\241\256\350\256\244", 0));
        label_4->setText(QApplication::translate("QUserEdit", "\346\235\203\351\231\220", 0));
        label_5->setText(QApplication::translate("QUserEdit", "\345\244\207\346\263\250", 0));
        lineEdit_2->setInputMask(QString());
        lineEdit_3->setInputMask(QString());
        pushButton->setText(QApplication::translate("QUserEdit", "\345\217\226\346\266\210", 0));
        pushButton_2->setText(QApplication::translate("QUserEdit", "\347\241\256\345\256\232", 0));
    } // retranslateUi

};

namespace Ui {
    class QUserEdit: public Ui_QUserEdit {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_USEREDIT_H
