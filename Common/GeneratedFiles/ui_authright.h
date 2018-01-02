/********************************************************************************
** Form generated from reading UI file 'authright.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_AUTHRIGHT_H
#define UI_AUTHRIGHT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_QAuthRight
{
public:
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLineEdit *lineEdit;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QLabel *label_4;

    void setupUi(QDialog *QAuthRight)
    {
        if (QAuthRight->objectName().isEmpty())
            QAuthRight->setObjectName(QStringLiteral("QAuthRight"));
        QAuthRight->resize(388, 194);
        label = new QLabel(QAuthRight);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(30, 60, 71, 16));
        label_2 = new QLabel(QAuthRight);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(110, 60, 221, 16));
        label_3 = new QLabel(QAuthRight);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(30, 100, 61, 16));
        lineEdit = new QLineEdit(QAuthRight);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));
        lineEdit->setGeometry(QRect(110, 100, 221, 20));
        lineEdit->setEchoMode(QLineEdit::Password);
        pushButton = new QPushButton(QAuthRight);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(90, 140, 75, 23));
        pushButton_2 = new QPushButton(QAuthRight);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setGeometry(QRect(230, 140, 75, 23));
        label_4 = new QLabel(QAuthRight);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(30, 20, 321, 16));

        retranslateUi(QAuthRight);

        QMetaObject::connectSlotsByName(QAuthRight);
    } // setupUi

    void retranslateUi(QDialog *QAuthRight)
    {
        QAuthRight->setWindowTitle(QApplication::translate("QAuthRight", "QAuthRight", 0));
        label->setText(QApplication::translate("QAuthRight", "\347\211\210\346\234\254\344\277\241\346\201\257 \357\274\232 ", 0));
        label_2->setText(QApplication::translate("QAuthRight", "TPAssemble Vision 1.0", 0));
        label_3->setText(QApplication::translate("QAuthRight", "\346\216\210\346\235\203\347\240\201 \357\274\232", 0));
        pushButton->setText(QApplication::translate("QAuthRight", "\346\216\210\346\235\203", 0));
        pushButton_2->setText(QApplication::translate("QAuthRight", "\345\217\226\346\266\210", 0));
        label_4->setText(QApplication::translate("QAuthRight", "\346\202\250\347\232\204\350\275\257\344\273\266\350\277\230\346\262\241\346\234\211\346\216\210\346\235\203\357\274\214\350\257\267\350\276\223\345\205\245\346\216\210\346\235\203\344\277\241\346\201\257\346\277\200\346\264\273\344\272\247\345\223\201", 0));
    } // retranslateUi

};

namespace Ui {
    class QAuthRight: public Ui_QAuthRight {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AUTHRIGHT_H
