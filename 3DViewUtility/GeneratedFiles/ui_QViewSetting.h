/********************************************************************************
** Form generated from reading UI file 'QViewSetting.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QVIEWSETTING_H
#define UI_QVIEWSETTING_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QViewSetting
{
public:
    QGroupBox *groupBox;
    QComboBox *comboBoxCSG;
    QLabel *label;
    QComboBox *comboBoxSampFreq;
    QLabel *label_2;
    QGroupBox *groupBox_2;

    void setupUi(QWidget *QViewSetting)
    {
        if (QViewSetting->objectName().isEmpty())
            QViewSetting->setObjectName(QStringLiteral("QViewSetting"));
        QViewSetting->resize(600, 500);
        groupBox = new QGroupBox(QViewSetting);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(20, 20, 551, 161));
        comboBoxCSG = new QComboBox(groupBox);
        comboBoxCSG->setObjectName(QStringLiteral("comboBoxCSG"));
        comboBoxCSG->setGeometry(QRect(110, 36, 121, 24));
        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(20, 40, 81, 18));
        comboBoxSampFreq = new QComboBox(groupBox);
        comboBoxSampFreq->setObjectName(QStringLiteral("comboBoxSampFreq"));
        comboBoxSampFreq->setGeometry(QRect(110, 70, 121, 24));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(20, 74, 81, 18));
        groupBox_2 = new QGroupBox(QViewSetting);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        groupBox_2->setGeometry(QRect(20, 200, 551, 271));

        retranslateUi(QViewSetting);

        QMetaObject::connectSlotsByName(QViewSetting);
    } // setupUi

    void retranslateUi(QWidget *QViewSetting)
    {
        QViewSetting->setWindowTitle(QApplication::translate("QViewSetting", "\345\217\202\346\225\260\350\256\276\347\275\256", 0));
        groupBox->setTitle(QApplication::translate("QViewSetting", "\345\270\270\347\224\250\350\256\276\347\275\256", 0));
        label->setText(QApplication::translate("QViewSetting", "\345\210\207\345\211\262\347\256\227\346\263\225\357\274\232 ", 0));
        label_2->setText(QApplication::translate("QViewSetting", "\351\207\207\346\240\267\351\242\221\347\216\207\357\274\232 ", 0));
        groupBox_2->setTitle(QApplication::translate("QViewSetting", "3D\350\256\276\347\275\256", 0));
    } // retranslateUi

};

namespace Ui {
    class QViewSetting: public Ui_QViewSetting {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QVIEWSETTING_H
