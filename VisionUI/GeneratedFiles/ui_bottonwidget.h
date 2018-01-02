/********************************************************************************
** Form generated from reading UI file 'bottonwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BOTTONWIDGET_H
#define UI_BOTTONWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_bottonwidget
{
public:
    QTreeView *treeView;
    QTreeView *treeView_2;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;

    void setupUi(QWidget *bottonwidget)
    {
        if (bottonwidget->objectName().isEmpty())
            bottonwidget->setObjectName(QStringLiteral("bottonwidget"));
        bottonwidget->resize(1348, 172);
        treeView = new QTreeView(bottonwidget);
        treeView->setObjectName(QStringLiteral("treeView"));
        treeView->setGeometry(QRect(20, 10, 546, 140));
        treeView_2 = new QTreeView(bottonwidget);
        treeView_2->setObjectName(QStringLiteral("treeView_2"));
        treeView_2->setGeometry(QRect(590, 10, 706, 140));
        treeView_2->header()->setDefaultSectionSize(100);
        treeView_2->header()->setMinimumSectionSize(25);
        scrollArea = new QScrollArea(bottonwidget);
        scrollArea->setObjectName(QStringLiteral("scrollArea"));
        scrollArea->setGeometry(QRect(590, 10, 706, 140));
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 704, 138));
        scrollArea->setWidget(scrollAreaWidgetContents);

        retranslateUi(bottonwidget);

        QMetaObject::connectSlotsByName(bottonwidget);
    } // setupUi

    void retranslateUi(QWidget *bottonwidget)
    {
        bottonwidget->setWindowTitle(QApplication::translate("bottonwidget", "Form", 0));
    } // retranslateUi

};

namespace Ui {
    class bottonwidget: public Ui_bottonwidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BOTTONWIDGET_H
