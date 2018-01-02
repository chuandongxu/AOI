/********************************************************************************
** Form generated from reading UI file 'QRunTabWidget.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QRUNTABWIDGET_H
#define UI_QRUNTABWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QRunTabWidget
{
public:
    QPushButton *pushButton_setTab;
    QPushButton *pushButton_runTab;

    void setupUi(QWidget *QRunTabWidget)
    {
        if (QRunTabWidget->objectName().isEmpty())
            QRunTabWidget->setObjectName(QStringLiteral("QRunTabWidget"));
        QRunTabWidget->resize(360, 35);
        pushButton_setTab = new QPushButton(QRunTabWidget);
        pushButton_setTab->setObjectName(QStringLiteral("pushButton_setTab"));
        pushButton_setTab->setGeometry(QRect(5, 0, 170, 35));
        pushButton_runTab = new QPushButton(QRunTabWidget);
        pushButton_runTab->setObjectName(QStringLiteral("pushButton_runTab"));
        pushButton_runTab->setGeometry(QRect(185, 0, 170, 35));

        retranslateUi(QRunTabWidget);

        QMetaObject::connectSlotsByName(QRunTabWidget);
    } // setupUi

    void retranslateUi(QWidget *QRunTabWidget)
    {
        QRunTabWidget->setWindowTitle(QApplication::translate("QRunTabWidget", "QRunTabWidget", 0));
        pushButton_setTab->setText(QApplication::translate("QRunTabWidget", "\347\274\226\350\276\221\346\243\200\346\265\213", 0));
        pushButton_runTab->setText(QApplication::translate("QRunTabWidget", "\346\211\247\350\241\214\346\265\213\351\207\217", 0));
    } // retranslateUi

};

namespace Ui {
    class QRunTabWidget: public Ui_QRunTabWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QRUNTABWIDGET_H
