/********************************************************************************
** Form generated from reading UI file 'VisionTestSetting.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VISIONTESTSETTING_H
#define UI_VISIONTESTSETTING_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableView>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_VisionTestSetting
{
public:
    QWidget *centralWidget;
    QListView *listView_TestItem;
    QTreeView *treeView_Tools;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QTableView *tableView_msg;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *VisionTestSetting)
    {
        if (VisionTestSetting->objectName().isEmpty())
            VisionTestSetting->setObjectName(QStringLiteral("VisionTestSetting"));
        VisionTestSetting->resize(1200, 850);
        centralWidget = new QWidget(VisionTestSetting);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        listView_TestItem = new QListView(centralWidget);
        listView_TestItem->setObjectName(QStringLiteral("listView_TestItem"));
        listView_TestItem->setGeometry(QRect(890, 10, 260, 700));
        treeView_Tools = new QTreeView(centralWidget);
        treeView_Tools->setObjectName(QStringLiteral("treeView_Tools"));
        treeView_Tools->setGeometry(QRect(10, 10, 250, 600));
        verticalLayoutWidget = new QWidget(centralWidget);
        verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(270, 10, 611, 601));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        tableView_msg = new QTableView(centralWidget);
        tableView_msg->setObjectName(QStringLiteral("tableView_msg"));
        tableView_msg->setGeometry(QRect(10, 620, 871, 91));
        VisionTestSetting->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(VisionTestSetting);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1200, 26));
        VisionTestSetting->setMenuBar(menuBar);
        mainToolBar = new QToolBar(VisionTestSetting);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        VisionTestSetting->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(VisionTestSetting);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        VisionTestSetting->setStatusBar(statusBar);

        retranslateUi(VisionTestSetting);

        QMetaObject::connectSlotsByName(VisionTestSetting);
    } // setupUi

    void retranslateUi(QMainWindow *VisionTestSetting)
    {
        VisionTestSetting->setWindowTitle(QApplication::translate("VisionTestSetting", "VisionTestSetting", 0));
    } // retranslateUi

};

namespace Ui {
    class VisionTestSetting: public Ui_VisionTestSetting {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VISIONTESTSETTING_H
