/********************************************************************************
** Form generated from reading UI file 'WorkSetting.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WORKSETTING_H
#define UI_WORKSETTING_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_WorkSetting
{
public:
    QTabWidget *tabWidget;
    QWidget *tab;
    QGroupBox *groupBox_11;
    QPushButton *pushButton_imStop;
    QPushButton *pushButton_clearError;
    QPushButton *pushButton_home;
    QPushButton *pushButton_enable;
    QPushButton *pushButton_disable;
    QGroupBox *groupBox_5;
    QGroupBox *groupBox_8;
    QLabel *label_13;
    QLineEdit *lineEdit_2;
    QRadioButton *radioButton_stopSensor;
    QRadioButton *radioButton_inplaceSensor;
    QRadioButton *radioButton_outputSensor;
    QRadioButton *radioButton_inputSensor;
    QPushButton *pushButton_onLive;
    QPushButton *pushButton_onStop;
    QGroupBox *groupBox_6;
    QGroupBox *groupBox_9;
    QLabel *label_14;
    QLineEdit *lineEdit_3;
    QPushButton *pushButton_move;
    QPushButton *pushButton_stop;
    QCheckBox *checkBox_cylinder;
    QPushButton *pushButton_moveInput;
    QPushButton *pushButton_moveOutput;
    QWidget *tab_2;
    QGroupBox *groupBox_12;
    QLabel *label;
    QLineEdit *lineEdit_width;
    QPushButton *pushButton_joystickWidth;
    QGroupBox *groupBox_13;
    QPushButton *pushButton_imStopWidth;
    QPushButton *pushButton_clearErrorWidth;
    QPushButton *pushButton_homeWidth;
    QPushButton *pushButton_enableWidth;
    QPushButton *pushButton_disableWidth;
    QPushButton *pushButton_moveToReady;
    QPushButton *pushButton_moveToWidth;

    void setupUi(QWidget *WorkSetting)
    {
        if (WorkSetting->objectName().isEmpty())
            WorkSetting->setObjectName(QStringLiteral("WorkSetting"));
        WorkSetting->resize(1200, 850);
        tabWidget = new QTabWidget(WorkSetting);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tabWidget->setGeometry(QRect(10, 10, 900, 800));
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        groupBox_11 = new QGroupBox(tab);
        groupBox_11->setObjectName(QStringLiteral("groupBox_11"));
        groupBox_11->setGeometry(QRect(20, 20, 810, 80));
        pushButton_imStop = new QPushButton(groupBox_11);
        pushButton_imStop->setObjectName(QStringLiteral("pushButton_imStop"));
        pushButton_imStop->setGeometry(QRect(575, 30, 100, 23));
        pushButton_clearError = new QPushButton(groupBox_11);
        pushButton_clearError->setObjectName(QStringLiteral("pushButton_clearError"));
        pushButton_clearError->setGeometry(QRect(690, 30, 100, 23));
        pushButton_home = new QPushButton(groupBox_11);
        pushButton_home->setObjectName(QStringLiteral("pushButton_home"));
        pushButton_home->setEnabled(false);
        pushButton_home->setGeometry(QRect(240, 30, 100, 23));
        pushButton_enable = new QPushButton(groupBox_11);
        pushButton_enable->setObjectName(QStringLiteral("pushButton_enable"));
        pushButton_enable->setGeometry(QRect(15, 30, 100, 23));
        pushButton_disable = new QPushButton(groupBox_11);
        pushButton_disable->setObjectName(QStringLiteral("pushButton_disable"));
        pushButton_disable->setGeometry(QRect(130, 30, 100, 23));
        groupBox_5 = new QGroupBox(tab);
        groupBox_5->setObjectName(QStringLiteral("groupBox_5"));
        groupBox_5->setGeometry(QRect(20, 120, 810, 80));
        groupBox_8 = new QGroupBox(groupBox_5);
        groupBox_8->setObjectName(QStringLiteral("groupBox_8"));
        groupBox_8->setGeometry(QRect(150, 240, 120, 191));
        label_13 = new QLabel(groupBox_8);
        label_13->setObjectName(QStringLiteral("label_13"));
        label_13->setGeometry(QRect(10, 30, 31, 16));
        lineEdit_2 = new QLineEdit(groupBox_8);
        lineEdit_2->setObjectName(QStringLiteral("lineEdit_2"));
        lineEdit_2->setGeometry(QRect(50, 30, 61, 20));
        radioButton_stopSensor = new QRadioButton(groupBox_5);
        radioButton_stopSensor->setObjectName(QStringLiteral("radioButton_stopSensor"));
        radioButton_stopSensor->setEnabled(false);
        radioButton_stopSensor->setGeometry(QRect(150, 30, 100, 19));
        radioButton_inplaceSensor = new QRadioButton(groupBox_5);
        radioButton_inplaceSensor->setObjectName(QStringLiteral("radioButton_inplaceSensor"));
        radioButton_inplaceSensor->setEnabled(false);
        radioButton_inplaceSensor->setGeometry(QRect(280, 30, 100, 19));
        radioButton_outputSensor = new QRadioButton(groupBox_5);
        radioButton_outputSensor->setObjectName(QStringLiteral("radioButton_outputSensor"));
        radioButton_outputSensor->setEnabled(false);
        radioButton_outputSensor->setGeometry(QRect(410, 30, 100, 19));
        radioButton_inputSensor = new QRadioButton(groupBox_5);
        radioButton_inputSensor->setObjectName(QStringLiteral("radioButton_inputSensor"));
        radioButton_inputSensor->setEnabled(false);
        radioButton_inputSensor->setGeometry(QRect(20, 30, 100, 19));
        pushButton_onLive = new QPushButton(groupBox_5);
        pushButton_onLive->setObjectName(QStringLiteral("pushButton_onLive"));
        pushButton_onLive->setGeometry(QRect(580, 30, 100, 23));
        pushButton_onStop = new QPushButton(groupBox_5);
        pushButton_onStop->setObjectName(QStringLiteral("pushButton_onStop"));
        pushButton_onStop->setGeometry(QRect(690, 30, 100, 23));
        groupBox_6 = new QGroupBox(tab);
        groupBox_6->setObjectName(QStringLiteral("groupBox_6"));
        groupBox_6->setGeometry(QRect(20, 220, 810, 80));
        groupBox_9 = new QGroupBox(groupBox_6);
        groupBox_9->setObjectName(QStringLiteral("groupBox_9"));
        groupBox_9->setGeometry(QRect(150, 240, 120, 191));
        label_14 = new QLabel(groupBox_9);
        label_14->setObjectName(QStringLiteral("label_14"));
        label_14->setGeometry(QRect(10, 30, 31, 16));
        lineEdit_3 = new QLineEdit(groupBox_9);
        lineEdit_3->setObjectName(QStringLiteral("lineEdit_3"));
        lineEdit_3->setGeometry(QRect(50, 30, 61, 20));
        pushButton_move = new QPushButton(groupBox_6);
        pushButton_move->setObjectName(QStringLiteral("pushButton_move"));
        pushButton_move->setGeometry(QRect(10, 30, 221, 35));
        pushButton_stop = new QPushButton(groupBox_6);
        pushButton_stop->setObjectName(QStringLiteral("pushButton_stop"));
        pushButton_stop->setGeometry(QRect(250, 30, 151, 35));
        checkBox_cylinder = new QCheckBox(groupBox_6);
        checkBox_cylinder->setObjectName(QStringLiteral("checkBox_cylinder"));
        checkBox_cylinder->setGeometry(QRect(420, 40, 181, 19));
        pushButton_moveInput = new QPushButton(tab);
        pushButton_moveInput->setObjectName(QStringLiteral("pushButton_moveInput"));
        pushButton_moveInput->setGeometry(QRect(20, 330, 400, 35));
        pushButton_moveOutput = new QPushButton(tab);
        pushButton_moveOutput->setObjectName(QStringLiteral("pushButton_moveOutput"));
        pushButton_moveOutput->setGeometry(QRect(430, 330, 400, 35));
        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        groupBox_12 = new QGroupBox(tab_2);
        groupBox_12->setObjectName(QStringLiteral("groupBox_12"));
        groupBox_12->setGeometry(QRect(20, 120, 810, 81));
        label = new QLabel(groupBox_12);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(20, 30, 72, 15));
        lineEdit_width = new QLineEdit(groupBox_12);
        lineEdit_width->setObjectName(QStringLiteral("lineEdit_width"));
        lineEdit_width->setGeometry(QRect(70, 30, 113, 21));
        pushButton_joystickWidth = new QPushButton(groupBox_12);
        pushButton_joystickWidth->setObjectName(QStringLiteral("pushButton_joystickWidth"));
        pushButton_joystickWidth->setGeometry(QRect(210, 20, 60, 40));
        groupBox_13 = new QGroupBox(tab_2);
        groupBox_13->setObjectName(QStringLiteral("groupBox_13"));
        groupBox_13->setGeometry(QRect(20, 20, 810, 80));
        pushButton_imStopWidth = new QPushButton(groupBox_13);
        pushButton_imStopWidth->setObjectName(QStringLiteral("pushButton_imStopWidth"));
        pushButton_imStopWidth->setGeometry(QRect(575, 30, 100, 23));
        pushButton_clearErrorWidth = new QPushButton(groupBox_13);
        pushButton_clearErrorWidth->setObjectName(QStringLiteral("pushButton_clearErrorWidth"));
        pushButton_clearErrorWidth->setGeometry(QRect(690, 30, 100, 23));
        pushButton_homeWidth = new QPushButton(groupBox_13);
        pushButton_homeWidth->setObjectName(QStringLiteral("pushButton_homeWidth"));
        pushButton_homeWidth->setEnabled(true);
        pushButton_homeWidth->setGeometry(QRect(240, 30, 100, 23));
        pushButton_enableWidth = new QPushButton(groupBox_13);
        pushButton_enableWidth->setObjectName(QStringLiteral("pushButton_enableWidth"));
        pushButton_enableWidth->setGeometry(QRect(15, 30, 100, 23));
        pushButton_disableWidth = new QPushButton(groupBox_13);
        pushButton_disableWidth->setObjectName(QStringLiteral("pushButton_disableWidth"));
        pushButton_disableWidth->setGeometry(QRect(130, 30, 100, 23));
        pushButton_moveToReady = new QPushButton(tab_2);
        pushButton_moveToReady->setObjectName(QStringLiteral("pushButton_moveToReady"));
        pushButton_moveToReady->setGeometry(QRect(20, 230, 400, 35));
        pushButton_moveToWidth = new QPushButton(tab_2);
        pushButton_moveToWidth->setObjectName(QStringLiteral("pushButton_moveToWidth"));
        pushButton_moveToWidth->setGeometry(QRect(430, 230, 400, 35));
        tabWidget->addTab(tab_2, QString());

        retranslateUi(WorkSetting);

        tabWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(WorkSetting);
    } // setupUi

    void retranslateUi(QWidget *WorkSetting)
    {
        WorkSetting->setWindowTitle(QApplication::translate("WorkSetting", "WorkSetting", 0));
        groupBox_11->setTitle(QApplication::translate("WorkSetting", "\345\271\263\345\217\260\346\223\215\344\275\234", 0));
        pushButton_imStop->setText(QApplication::translate("WorkSetting", "\347\264\247\346\200\245\345\201\234\346\255\242", 0));
        pushButton_clearError->setText(QApplication::translate("WorkSetting", "\346\270\205\351\231\244\346\212\245\350\255\246", 0));
        pushButton_home->setText(QApplication::translate("WorkSetting", "\345\233\236\351\233\266", 0));
        pushButton_enable->setText(QApplication::translate("WorkSetting", "\344\275\277\350\203\275", 0));
        pushButton_disable->setText(QApplication::translate("WorkSetting", "\345\216\273\344\275\277\350\203\275", 0));
        groupBox_5->setTitle(QApplication::translate("WorkSetting", "\346\235\245\346\226\231\347\212\266\346\200\201", 0));
        groupBox_8->setTitle(QApplication::translate("WorkSetting", "\347\233\270\345\257\271\350\277\220\345\212\250", 0));
        label_13->setText(QApplication::translate("WorkSetting", "\350\267\235\347\246\273\357\274\232", 0));
        radioButton_stopSensor->setText(QApplication::translate("WorkSetting", "\345\201\234\346\235\277Sensor", 0));
        radioButton_inplaceSensor->setText(QApplication::translate("WorkSetting", "\345\210\260\344\275\215Sensor", 0));
        radioButton_outputSensor->setText(QApplication::translate("WorkSetting", "\345\207\272\346\226\231Sensor", 0));
        radioButton_inputSensor->setText(QApplication::translate("WorkSetting", "\350\277\233\346\226\231Sensor", 0));
        pushButton_onLive->setText(QApplication::translate("WorkSetting", "Live", 0));
        pushButton_onStop->setText(QApplication::translate("WorkSetting", "Stop", 0));
        groupBox_6->setTitle(QApplication::translate("WorkSetting", "\344\270\212\344\270\213\346\226\231\346\223\215\344\275\234", 0));
        groupBox_9->setTitle(QApplication::translate("WorkSetting", "\347\233\270\345\257\271\350\277\220\345\212\250", 0));
        label_14->setText(QApplication::translate("WorkSetting", "\350\267\235\347\246\273\357\274\232", 0));
        pushButton_move->setText(QApplication::translate("WorkSetting", "\350\277\233\346\226\231\350\277\220\345\212\250", 0));
        pushButton_stop->setText(QApplication::translate("WorkSetting", "\345\201\234\346\255\242", 0));
        checkBox_cylinder->setText(QApplication::translate("WorkSetting", "\351\241\266\345\215\207\346\260\224\347\274\270\344\270\212\345\215\207/\344\270\213\351\231\215", 0));
        pushButton_moveInput->setText(QApplication::translate("WorkSetting", "\344\270\212\346\226\231\350\277\220\345\212\250", 0));
        pushButton_moveOutput->setText(QApplication::translate("WorkSetting", "\344\270\213\346\226\231\350\277\220\345\212\250", 0));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("WorkSetting", "\350\275\250\351\201\223\350\256\276\347\275\256", 0));
        groupBox_12->setTitle(QApplication::translate("WorkSetting", "\350\275\250\351\201\223\350\256\276\347\275\256", 0));
        label->setText(QApplication::translate("WorkSetting", "\345\256\275\345\272\246\357\274\232", 0));
        pushButton_joystickWidth->setText(QString());
        groupBox_13->setTitle(QApplication::translate("WorkSetting", "\345\271\263\345\217\260\346\223\215\344\275\234", 0));
        pushButton_imStopWidth->setText(QApplication::translate("WorkSetting", "\347\264\247\346\200\245\345\201\234\346\255\242", 0));
        pushButton_clearErrorWidth->setText(QApplication::translate("WorkSetting", "\346\270\205\351\231\244\346\212\245\350\255\246", 0));
        pushButton_homeWidth->setText(QApplication::translate("WorkSetting", "\345\233\236\351\233\266", 0));
        pushButton_enableWidth->setText(QApplication::translate("WorkSetting", "\344\275\277\350\203\275", 0));
        pushButton_disableWidth->setText(QApplication::translate("WorkSetting", "\345\216\273\344\275\277\350\203\275", 0));
        pushButton_moveToReady->setText(QApplication::translate("WorkSetting", "\347\247\273\345\212\250\345\210\260\345\210\235\345\247\213\344\275\215\347\275\256", 0));
        pushButton_moveToWidth->setText(QApplication::translate("WorkSetting", "\347\247\273\345\212\250\345\210\260\350\275\250\351\201\223\345\256\275\345\272\246", 0));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("WorkSetting", "\350\275\250\351\201\223\345\256\275\345\272\246", 0));
    } // retranslateUi

};

namespace Ui {
    class WorkSetting: public Ui_WorkSetting {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WORKSETTING_H
