/********************************************************************************
** Form generated from reading UI file 'TowerSetting.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TOWERSETTING_H
#define UI_TOWERSETTING_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TowerSetting
{
public:
    QTabWidget *tabWidget;
    QWidget *tab;
    QGroupBox *groupBox;
    QGroupBox *groupBox_1;
    QRadioButton *radioButton;
    QRadioButton *radioButton_2;
    QRadioButton *radioButton_3;
    QGroupBox *groupBox_2;
    QRadioButton *radioButton_4;
    QRadioButton *radioButton_5;
    QRadioButton *radioButton_6;
    QGroupBox *groupBox_3;
    QRadioButton *radioButton_7;
    QRadioButton *radioButton_8;
    QRadioButton *radioButton_9;
    QGroupBox *groupBox_4;
    QRadioButton *radioButton_45;
    QRadioButton *radioButton_46;
    QGroupBox *groupBox_5;
    QGroupBox *groupBox_6;
    QRadioButton *radioButton_10;
    QRadioButton *radioButton_11;
    QRadioButton *radioButton_12;
    QGroupBox *groupBox_7;
    QRadioButton *radioButton_13;
    QRadioButton *radioButton_14;
    QRadioButton *radioButton_15;
    QGroupBox *groupBox_8;
    QRadioButton *radioButton_16;
    QRadioButton *radioButton_17;
    QRadioButton *radioButton_18;
    QGroupBox *groupBox_9;
    QRadioButton *radioButton_47;
    QRadioButton *radioButton_48;
    QGroupBox *groupBox_10;
    QGroupBox *groupBox_11;
    QRadioButton *radioButton_19;
    QRadioButton *radioButton_20;
    QRadioButton *radioButton_21;
    QGroupBox *groupBox_12;
    QRadioButton *radioButton_22;
    QRadioButton *radioButton_23;
    QRadioButton *radioButton_24;
    QGroupBox *groupBox_13;
    QRadioButton *radioButton_25;
    QRadioButton *radioButton_26;
    QRadioButton *radioButton_27;
    QGroupBox *groupBox_14;
    QRadioButton *radioButton_49;
    QRadioButton *radioButton_50;
    QGroupBox *groupBox_15;
    QGroupBox *groupBox_16;
    QRadioButton *radioButton_28;
    QRadioButton *radioButton_29;
    QRadioButton *radioButton_30;
    QGroupBox *groupBox_17;
    QRadioButton *radioButton_31;
    QRadioButton *radioButton_32;
    QRadioButton *radioButton_33;
    QGroupBox *groupBox_18;
    QRadioButton *radioButton_34;
    QRadioButton *radioButton_35;
    QRadioButton *radioButton_36;
    QGroupBox *groupBox_19;
    QRadioButton *radioButton_51;
    QRadioButton *radioButton_52;
    QPushButton *pushButton_LoadAll;
    QPushButton *pushButton_SaveAll;
    QPushButton *pushButton_Trigger;
    QComboBox *comboBox_mode;
    QPushButton *pushButton_Stop;
    QLabel *label;

    void setupUi(QWidget *TowerSetting)
    {
        if (TowerSetting->objectName().isEmpty())
            TowerSetting->setObjectName(QStringLiteral("TowerSetting"));
        TowerSetting->resize(1200, 850);
        tabWidget = new QTabWidget(TowerSetting);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tabWidget->setGeometry(QRect(10, 10, 900, 821));
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        groupBox = new QGroupBox(tab);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(30, 30, 591, 171));
        groupBox_1 = new QGroupBox(groupBox);
        groupBox_1->setObjectName(QStringLiteral("groupBox_1"));
        groupBox_1->setGeometry(QRect(30, 30, 111, 120));
        radioButton = new QRadioButton(groupBox_1);
        radioButton->setObjectName(QStringLiteral("radioButton"));
        radioButton->setGeometry(QRect(20, 20, 80, 19));
        radioButton->setChecked(false);
        radioButton_2 = new QRadioButton(groupBox_1);
        radioButton_2->setObjectName(QStringLiteral("radioButton_2"));
        radioButton_2->setGeometry(QRect(20, 50, 80, 19));
        radioButton_2->setChecked(true);
        radioButton_3 = new QRadioButton(groupBox_1);
        radioButton_3->setObjectName(QStringLiteral("radioButton_3"));
        radioButton_3->setEnabled(false);
        radioButton_3->setGeometry(QRect(20, 80, 80, 19));
        radioButton_3->setChecked(false);
        groupBox_2 = new QGroupBox(groupBox);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        groupBox_2->setGeometry(QRect(170, 30, 111, 120));
        radioButton_4 = new QRadioButton(groupBox_2);
        radioButton_4->setObjectName(QStringLiteral("radioButton_4"));
        radioButton_4->setGeometry(QRect(20, 20, 80, 19));
        radioButton_5 = new QRadioButton(groupBox_2);
        radioButton_5->setObjectName(QStringLiteral("radioButton_5"));
        radioButton_5->setGeometry(QRect(20, 50, 80, 19));
        radioButton_6 = new QRadioButton(groupBox_2);
        radioButton_6->setObjectName(QStringLiteral("radioButton_6"));
        radioButton_6->setEnabled(false);
        radioButton_6->setGeometry(QRect(20, 80, 80, 19));
        groupBox_3 = new QGroupBox(groupBox);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        groupBox_3->setGeometry(QRect(310, 30, 111, 120));
        radioButton_7 = new QRadioButton(groupBox_3);
        radioButton_7->setObjectName(QStringLiteral("radioButton_7"));
        radioButton_7->setGeometry(QRect(20, 20, 80, 19));
        radioButton_8 = new QRadioButton(groupBox_3);
        radioButton_8->setObjectName(QStringLiteral("radioButton_8"));
        radioButton_8->setGeometry(QRect(20, 50, 80, 19));
        radioButton_9 = new QRadioButton(groupBox_3);
        radioButton_9->setObjectName(QStringLiteral("radioButton_9"));
        radioButton_9->setEnabled(false);
        radioButton_9->setGeometry(QRect(20, 80, 80, 19));
        groupBox_4 = new QGroupBox(groupBox);
        groupBox_4->setObjectName(QStringLiteral("groupBox_4"));
        groupBox_4->setGeometry(QRect(450, 30, 111, 120));
        radioButton_45 = new QRadioButton(groupBox_4);
        radioButton_45->setObjectName(QStringLiteral("radioButton_45"));
        radioButton_45->setGeometry(QRect(20, 20, 80, 19));
        radioButton_46 = new QRadioButton(groupBox_4);
        radioButton_46->setObjectName(QStringLiteral("radioButton_46"));
        radioButton_46->setGeometry(QRect(20, 50, 80, 19));
        groupBox_5 = new QGroupBox(tab);
        groupBox_5->setObjectName(QStringLiteral("groupBox_5"));
        groupBox_5->setGeometry(QRect(30, 220, 591, 171));
        groupBox_6 = new QGroupBox(groupBox_5);
        groupBox_6->setObjectName(QStringLiteral("groupBox_6"));
        groupBox_6->setGeometry(QRect(30, 30, 111, 120));
        radioButton_10 = new QRadioButton(groupBox_6);
        radioButton_10->setObjectName(QStringLiteral("radioButton_10"));
        radioButton_10->setGeometry(QRect(20, 20, 80, 19));
        radioButton_11 = new QRadioButton(groupBox_6);
        radioButton_11->setObjectName(QStringLiteral("radioButton_11"));
        radioButton_11->setGeometry(QRect(20, 50, 80, 19));
        radioButton_12 = new QRadioButton(groupBox_6);
        radioButton_12->setObjectName(QStringLiteral("radioButton_12"));
        radioButton_12->setEnabled(false);
        radioButton_12->setGeometry(QRect(20, 80, 80, 19));
        groupBox_7 = new QGroupBox(groupBox_5);
        groupBox_7->setObjectName(QStringLiteral("groupBox_7"));
        groupBox_7->setGeometry(QRect(170, 30, 111, 120));
        radioButton_13 = new QRadioButton(groupBox_7);
        radioButton_13->setObjectName(QStringLiteral("radioButton_13"));
        radioButton_13->setGeometry(QRect(20, 20, 80, 19));
        radioButton_14 = new QRadioButton(groupBox_7);
        radioButton_14->setObjectName(QStringLiteral("radioButton_14"));
        radioButton_14->setGeometry(QRect(20, 50, 80, 19));
        radioButton_15 = new QRadioButton(groupBox_7);
        radioButton_15->setObjectName(QStringLiteral("radioButton_15"));
        radioButton_15->setEnabled(false);
        radioButton_15->setGeometry(QRect(20, 80, 80, 19));
        groupBox_8 = new QGroupBox(groupBox_5);
        groupBox_8->setObjectName(QStringLiteral("groupBox_8"));
        groupBox_8->setGeometry(QRect(310, 30, 111, 120));
        radioButton_16 = new QRadioButton(groupBox_8);
        radioButton_16->setObjectName(QStringLiteral("radioButton_16"));
        radioButton_16->setGeometry(QRect(20, 20, 80, 19));
        radioButton_17 = new QRadioButton(groupBox_8);
        radioButton_17->setObjectName(QStringLiteral("radioButton_17"));
        radioButton_17->setGeometry(QRect(20, 50, 80, 19));
        radioButton_18 = new QRadioButton(groupBox_8);
        radioButton_18->setObjectName(QStringLiteral("radioButton_18"));
        radioButton_18->setEnabled(false);
        radioButton_18->setGeometry(QRect(20, 80, 80, 19));
        groupBox_9 = new QGroupBox(groupBox_5);
        groupBox_9->setObjectName(QStringLiteral("groupBox_9"));
        groupBox_9->setGeometry(QRect(450, 30, 111, 120));
        radioButton_47 = new QRadioButton(groupBox_9);
        radioButton_47->setObjectName(QStringLiteral("radioButton_47"));
        radioButton_47->setGeometry(QRect(20, 20, 80, 19));
        radioButton_48 = new QRadioButton(groupBox_9);
        radioButton_48->setObjectName(QStringLiteral("radioButton_48"));
        radioButton_48->setGeometry(QRect(20, 50, 80, 19));
        groupBox_10 = new QGroupBox(tab);
        groupBox_10->setObjectName(QStringLiteral("groupBox_10"));
        groupBox_10->setGeometry(QRect(30, 410, 591, 171));
        groupBox_11 = new QGroupBox(groupBox_10);
        groupBox_11->setObjectName(QStringLiteral("groupBox_11"));
        groupBox_11->setGeometry(QRect(30, 30, 111, 120));
        radioButton_19 = new QRadioButton(groupBox_11);
        radioButton_19->setObjectName(QStringLiteral("radioButton_19"));
        radioButton_19->setGeometry(QRect(20, 20, 80, 19));
        radioButton_20 = new QRadioButton(groupBox_11);
        radioButton_20->setObjectName(QStringLiteral("radioButton_20"));
        radioButton_20->setGeometry(QRect(20, 50, 80, 19));
        radioButton_21 = new QRadioButton(groupBox_11);
        radioButton_21->setObjectName(QStringLiteral("radioButton_21"));
        radioButton_21->setEnabled(false);
        radioButton_21->setGeometry(QRect(20, 80, 80, 19));
        groupBox_12 = new QGroupBox(groupBox_10);
        groupBox_12->setObjectName(QStringLiteral("groupBox_12"));
        groupBox_12->setGeometry(QRect(170, 30, 111, 120));
        radioButton_22 = new QRadioButton(groupBox_12);
        radioButton_22->setObjectName(QStringLiteral("radioButton_22"));
        radioButton_22->setGeometry(QRect(20, 20, 80, 19));
        radioButton_23 = new QRadioButton(groupBox_12);
        radioButton_23->setObjectName(QStringLiteral("radioButton_23"));
        radioButton_23->setGeometry(QRect(20, 50, 80, 19));
        radioButton_24 = new QRadioButton(groupBox_12);
        radioButton_24->setObjectName(QStringLiteral("radioButton_24"));
        radioButton_24->setEnabled(false);
        radioButton_24->setGeometry(QRect(20, 80, 80, 19));
        groupBox_13 = new QGroupBox(groupBox_10);
        groupBox_13->setObjectName(QStringLiteral("groupBox_13"));
        groupBox_13->setGeometry(QRect(310, 30, 111, 120));
        radioButton_25 = new QRadioButton(groupBox_13);
        radioButton_25->setObjectName(QStringLiteral("radioButton_25"));
        radioButton_25->setGeometry(QRect(20, 20, 80, 19));
        radioButton_26 = new QRadioButton(groupBox_13);
        radioButton_26->setObjectName(QStringLiteral("radioButton_26"));
        radioButton_26->setGeometry(QRect(20, 50, 80, 19));
        radioButton_27 = new QRadioButton(groupBox_13);
        radioButton_27->setObjectName(QStringLiteral("radioButton_27"));
        radioButton_27->setEnabled(false);
        radioButton_27->setGeometry(QRect(20, 80, 80, 19));
        groupBox_14 = new QGroupBox(groupBox_10);
        groupBox_14->setObjectName(QStringLiteral("groupBox_14"));
        groupBox_14->setGeometry(QRect(450, 30, 111, 120));
        radioButton_49 = new QRadioButton(groupBox_14);
        radioButton_49->setObjectName(QStringLiteral("radioButton_49"));
        radioButton_49->setGeometry(QRect(20, 20, 80, 19));
        radioButton_50 = new QRadioButton(groupBox_14);
        radioButton_50->setObjectName(QStringLiteral("radioButton_50"));
        radioButton_50->setGeometry(QRect(20, 50, 80, 19));
        groupBox_15 = new QGroupBox(tab);
        groupBox_15->setObjectName(QStringLiteral("groupBox_15"));
        groupBox_15->setGeometry(QRect(30, 600, 591, 171));
        groupBox_16 = new QGroupBox(groupBox_15);
        groupBox_16->setObjectName(QStringLiteral("groupBox_16"));
        groupBox_16->setGeometry(QRect(30, 30, 111, 120));
        radioButton_28 = new QRadioButton(groupBox_16);
        radioButton_28->setObjectName(QStringLiteral("radioButton_28"));
        radioButton_28->setGeometry(QRect(20, 20, 80, 19));
        radioButton_29 = new QRadioButton(groupBox_16);
        radioButton_29->setObjectName(QStringLiteral("radioButton_29"));
        radioButton_29->setGeometry(QRect(20, 50, 80, 19));
        radioButton_30 = new QRadioButton(groupBox_16);
        radioButton_30->setObjectName(QStringLiteral("radioButton_30"));
        radioButton_30->setEnabled(false);
        radioButton_30->setGeometry(QRect(20, 80, 80, 19));
        groupBox_17 = new QGroupBox(groupBox_15);
        groupBox_17->setObjectName(QStringLiteral("groupBox_17"));
        groupBox_17->setGeometry(QRect(170, 30, 111, 120));
        radioButton_31 = new QRadioButton(groupBox_17);
        radioButton_31->setObjectName(QStringLiteral("radioButton_31"));
        radioButton_31->setGeometry(QRect(20, 20, 80, 19));
        radioButton_32 = new QRadioButton(groupBox_17);
        radioButton_32->setObjectName(QStringLiteral("radioButton_32"));
        radioButton_32->setGeometry(QRect(20, 50, 80, 19));
        radioButton_33 = new QRadioButton(groupBox_17);
        radioButton_33->setObjectName(QStringLiteral("radioButton_33"));
        radioButton_33->setEnabled(false);
        radioButton_33->setGeometry(QRect(20, 80, 80, 19));
        groupBox_18 = new QGroupBox(groupBox_15);
        groupBox_18->setObjectName(QStringLiteral("groupBox_18"));
        groupBox_18->setGeometry(QRect(310, 30, 111, 120));
        radioButton_34 = new QRadioButton(groupBox_18);
        radioButton_34->setObjectName(QStringLiteral("radioButton_34"));
        radioButton_34->setGeometry(QRect(20, 20, 80, 19));
        radioButton_35 = new QRadioButton(groupBox_18);
        radioButton_35->setObjectName(QStringLiteral("radioButton_35"));
        radioButton_35->setGeometry(QRect(20, 50, 80, 19));
        radioButton_36 = new QRadioButton(groupBox_18);
        radioButton_36->setObjectName(QStringLiteral("radioButton_36"));
        radioButton_36->setEnabled(false);
        radioButton_36->setGeometry(QRect(20, 80, 80, 19));
        groupBox_19 = new QGroupBox(groupBox_15);
        groupBox_19->setObjectName(QStringLiteral("groupBox_19"));
        groupBox_19->setGeometry(QRect(450, 30, 111, 120));
        radioButton_51 = new QRadioButton(groupBox_19);
        radioButton_51->setObjectName(QStringLiteral("radioButton_51"));
        radioButton_51->setGeometry(QRect(20, 20, 80, 19));
        radioButton_52 = new QRadioButton(groupBox_19);
        radioButton_52->setObjectName(QStringLiteral("radioButton_52"));
        radioButton_52->setGeometry(QRect(20, 50, 80, 19));
        pushButton_LoadAll = new QPushButton(tab);
        pushButton_LoadAll->setObjectName(QStringLiteral("pushButton_LoadAll"));
        pushButton_LoadAll->setGeometry(QRect(650, 40, 150, 23));
        pushButton_SaveAll = new QPushButton(tab);
        pushButton_SaveAll->setObjectName(QStringLiteral("pushButton_SaveAll"));
        pushButton_SaveAll->setGeometry(QRect(650, 70, 150, 23));
        pushButton_Trigger = new QPushButton(tab);
        pushButton_Trigger->setObjectName(QStringLiteral("pushButton_Trigger"));
        pushButton_Trigger->setGeometry(QRect(650, 160, 65, 28));
        comboBox_mode = new QComboBox(tab);
        comboBox_mode->setObjectName(QStringLiteral("comboBox_mode"));
        comboBox_mode->setGeometry(QRect(700, 120, 100, 22));
        pushButton_Stop = new QPushButton(tab);
        pushButton_Stop->setObjectName(QStringLiteral("pushButton_Stop"));
        pushButton_Stop->setGeometry(QRect(735, 160, 65, 28));
        label = new QLabel(tab);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(650, 120, 51, 21));
        tabWidget->addTab(tab, QString());

        retranslateUi(TowerSetting);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(TowerSetting);
    } // setupUi

    void retranslateUi(QWidget *TowerSetting)
    {
        TowerSetting->setWindowTitle(QApplication::translate("TowerSetting", "TowerSetting", 0));
        groupBox->setTitle(QApplication::translate("TowerSetting", "\347\251\272\351\227\262\347\212\266\346\200\201", 0));
        groupBox_1->setTitle(QApplication::translate("TowerSetting", "\347\272\242", 0));
        radioButton->setText(QApplication::translate("TowerSetting", "\345\205\263", 0));
        radioButton_2->setText(QApplication::translate("TowerSetting", "\345\274\200", 0));
        radioButton_3->setText(QApplication::translate("TowerSetting", "\351\227\252\347\203\201", 0));
        groupBox_2->setTitle(QApplication::translate("TowerSetting", "\351\273\204", 0));
        radioButton_4->setText(QApplication::translate("TowerSetting", "\345\205\263", 0));
        radioButton_5->setText(QApplication::translate("TowerSetting", "\345\274\200", 0));
        radioButton_6->setText(QApplication::translate("TowerSetting", "\351\227\252\347\203\201", 0));
        groupBox_3->setTitle(QApplication::translate("TowerSetting", "\347\273\277", 0));
        radioButton_7->setText(QApplication::translate("TowerSetting", "\345\205\263", 0));
        radioButton_8->setText(QApplication::translate("TowerSetting", "\345\274\200", 0));
        radioButton_9->setText(QApplication::translate("TowerSetting", "\351\227\252\347\203\201", 0));
        groupBox_4->setTitle(QApplication::translate("TowerSetting", "\350\234\202\351\270\243", 0));
        radioButton_45->setText(QApplication::translate("TowerSetting", "\345\205\263", 0));
        radioButton_46->setText(QApplication::translate("TowerSetting", "\345\274\200", 0));
        groupBox_5->setTitle(QApplication::translate("TowerSetting", "\351\207\217\344\272\247\347\212\266\346\200\201", 0));
        groupBox_6->setTitle(QApplication::translate("TowerSetting", "\347\272\242", 0));
        radioButton_10->setText(QApplication::translate("TowerSetting", "\345\205\263", 0));
        radioButton_11->setText(QApplication::translate("TowerSetting", "\345\274\200", 0));
        radioButton_12->setText(QApplication::translate("TowerSetting", "\351\227\252\347\203\201", 0));
        groupBox_7->setTitle(QApplication::translate("TowerSetting", "\351\273\204", 0));
        radioButton_13->setText(QApplication::translate("TowerSetting", "\345\205\263", 0));
        radioButton_14->setText(QApplication::translate("TowerSetting", "\345\274\200", 0));
        radioButton_15->setText(QApplication::translate("TowerSetting", "\351\227\252\347\203\201", 0));
        groupBox_8->setTitle(QApplication::translate("TowerSetting", "\347\273\277", 0));
        radioButton_16->setText(QApplication::translate("TowerSetting", "\345\205\263", 0));
        radioButton_17->setText(QApplication::translate("TowerSetting", "\345\274\200", 0));
        radioButton_18->setText(QApplication::translate("TowerSetting", "\351\227\252\347\203\201", 0));
        groupBox_9->setTitle(QApplication::translate("TowerSetting", "\350\234\202\351\270\243", 0));
        radioButton_47->setText(QApplication::translate("TowerSetting", "\345\205\263", 0));
        radioButton_48->setText(QApplication::translate("TowerSetting", "\345\274\200", 0));
        groupBox_10->setTitle(QApplication::translate("TowerSetting", "\345\221\212\350\255\246\347\212\266\346\200\201", 0));
        groupBox_11->setTitle(QApplication::translate("TowerSetting", "\347\272\242", 0));
        radioButton_19->setText(QApplication::translate("TowerSetting", "\345\205\263", 0));
        radioButton_20->setText(QApplication::translate("TowerSetting", "\345\274\200", 0));
        radioButton_21->setText(QApplication::translate("TowerSetting", "\351\227\252\347\203\201", 0));
        groupBox_12->setTitle(QApplication::translate("TowerSetting", "\351\273\204", 0));
        radioButton_22->setText(QApplication::translate("TowerSetting", "\345\205\263", 0));
        radioButton_23->setText(QApplication::translate("TowerSetting", "\345\274\200", 0));
        radioButton_24->setText(QApplication::translate("TowerSetting", "\351\227\252\347\203\201", 0));
        groupBox_13->setTitle(QApplication::translate("TowerSetting", "\347\273\277", 0));
        radioButton_25->setText(QApplication::translate("TowerSetting", "\345\205\263", 0));
        radioButton_26->setText(QApplication::translate("TowerSetting", "\345\274\200", 0));
        radioButton_27->setText(QApplication::translate("TowerSetting", "\351\227\252\347\203\201", 0));
        groupBox_14->setTitle(QApplication::translate("TowerSetting", "\350\234\202\351\270\243", 0));
        radioButton_49->setText(QApplication::translate("TowerSetting", "\345\205\263", 0));
        radioButton_50->setText(QApplication::translate("TowerSetting", "\345\274\200", 0));
        groupBox_15->setTitle(QApplication::translate("TowerSetting", "\351\224\231\350\257\257\347\212\266\346\200\201", 0));
        groupBox_16->setTitle(QApplication::translate("TowerSetting", "\347\272\242", 0));
        radioButton_28->setText(QApplication::translate("TowerSetting", "\345\205\263", 0));
        radioButton_29->setText(QApplication::translate("TowerSetting", "\345\274\200", 0));
        radioButton_30->setText(QApplication::translate("TowerSetting", "\351\227\252\347\203\201", 0));
        groupBox_17->setTitle(QApplication::translate("TowerSetting", "\351\273\204", 0));
        radioButton_31->setText(QApplication::translate("TowerSetting", "\345\205\263", 0));
        radioButton_32->setText(QApplication::translate("TowerSetting", "\345\274\200", 0));
        radioButton_33->setText(QApplication::translate("TowerSetting", "\351\227\252\347\203\201", 0));
        groupBox_18->setTitle(QApplication::translate("TowerSetting", "\347\273\277", 0));
        radioButton_34->setText(QApplication::translate("TowerSetting", "\345\205\263", 0));
        radioButton_35->setText(QApplication::translate("TowerSetting", "\345\274\200", 0));
        radioButton_36->setText(QApplication::translate("TowerSetting", "\351\227\252\347\203\201", 0));
        groupBox_19->setTitle(QApplication::translate("TowerSetting", "\350\234\202\351\270\243", 0));
        radioButton_51->setText(QApplication::translate("TowerSetting", "\345\205\263", 0));
        radioButton_52->setText(QApplication::translate("TowerSetting", "\345\274\200", 0));
        pushButton_LoadAll->setText(QApplication::translate("TowerSetting", "\345\257\274\345\205\245\351\205\215\347\275\256", 0));
        pushButton_SaveAll->setText(QApplication::translate("TowerSetting", "\344\277\235\345\255\230\351\205\215\347\275\256", 0));
        pushButton_Trigger->setText(QApplication::translate("TowerSetting", "\350\247\246\345\217\221", 0));
        pushButton_Stop->setText(QApplication::translate("TowerSetting", "\345\201\234\346\255\242", 0));
        label->setText(QApplication::translate("TowerSetting", "\346\250\241\345\274\217\357\274\232", 0));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("TowerSetting", "\344\270\211\350\211\262\347\201\257\345\205\211\350\256\276\347\275\256", 0));
    } // retranslateUi

};

namespace Ui {
    class TowerSetting: public Ui_TowerSetting {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TOWERSETTING_H
