/********************************************************************************
** Form generated from reading UI file 'lightsetting.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LIGHTSETTING_H
#define UI_LIGHTSETTING_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QLightSetting
{
public:
    QTabWidget *tabWidget;
    QPushButton *pushButton;
    QPushButton *pushButton_2;

    void setupUi(QWidget *QLightSetting)
    {
        if (QLightSetting->objectName().isEmpty())
            QLightSetting->setObjectName(QStringLiteral("QLightSetting"));
        QLightSetting->resize(901, 748);
        tabWidget = new QTabWidget(QLightSetting);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tabWidget->setGeometry(QRect(20, 20, 771, 721));
        pushButton = new QPushButton(QLightSetting);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(804, 40, 101, 23));
        pushButton_2 = new QPushButton(QLightSetting);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setGeometry(QRect(804, 90, 101, 23));

        retranslateUi(QLightSetting);

        QMetaObject::connectSlotsByName(QLightSetting);
    } // setupUi

    void retranslateUi(QWidget *QLightSetting)
    {
        QLightSetting->setWindowTitle(QApplication::translate("QLightSetting", "QLightSetting", 0));
        pushButton->setText(QApplication::translate("QLightSetting", "\345\242\236\345\212\240\345\205\211\346\272\220\346\216\247\345\210\266\345\231\250", 0));
        pushButton_2->setText(QApplication::translate("QLightSetting", "\345\210\240\351\231\244\345\205\211\346\272\220\346\216\247\345\210\266\345\231\250", 0));
    } // retranslateUi

};

namespace Ui {
    class QLightSetting: public Ui_QLightSetting {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LIGHTSETTING_H
