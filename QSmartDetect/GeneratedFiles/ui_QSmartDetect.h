/********************************************************************************
** Form generated from reading UI file 'QSmartDetect.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QSMARTDETECT_H
#define UI_QSMARTDETECT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QSmartDetectClass
{
public:
    QWidget *centralWidget;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *QSmartDetectClass)
    {
        if (QSmartDetectClass->objectName().isEmpty())
            QSmartDetectClass->setObjectName(QStringLiteral("QSmartDetectClass"));
        QSmartDetectClass->resize(1200, 815);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(QSmartDetectClass->sizePolicy().hasHeightForWidth());
        QSmartDetectClass->setSizePolicy(sizePolicy);
        QSmartDetectClass->setMaximumSize(QSize(1200, 815));
        QSmartDetectClass->setMouseTracking(false);
        QSmartDetectClass->setLayoutDirection(Qt::LeftToRight);
        QSmartDetectClass->setDocumentMode(false);
        QSmartDetectClass->setUnifiedTitleAndToolBarOnMac(false);
        centralWidget = new QWidget(QSmartDetectClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        verticalLayoutWidget = new QWidget(centralWidget);
        verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(0, 0, 1201, 781));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        QSmartDetectClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(QSmartDetectClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1200, 26));
        QSmartDetectClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(QSmartDetectClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        QSmartDetectClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(QSmartDetectClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        QSmartDetectClass->setStatusBar(statusBar);

        retranslateUi(QSmartDetectClass);

        QMetaObject::connectSlotsByName(QSmartDetectClass);
    } // setupUi

    void retranslateUi(QMainWindow *QSmartDetectClass)
    {
        QSmartDetectClass->setWindowTitle(QApplication::translate("QSmartDetectClass", "Novo 3D Detect", 0));
    } // retranslateUi

};

namespace Ui {
    class QSmartDetectClass: public Ui_QSmartDetectClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QSMARTDETECT_H
