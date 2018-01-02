/********************************************************************************
** Form generated from reading UI file 'ViewEditor.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VIEWEDITOR_H
#define UI_VIEWEDITOR_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ViewEditor
{
public:
    QWidget *centralWidget;
    QFrame *frame;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *ViewEditor)
    {
        if (ViewEditor->objectName().isEmpty())
            ViewEditor->setObjectName(QStringLiteral("ViewEditor"));
        ViewEditor->resize(820, 688);
        centralWidget = new QWidget(ViewEditor);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        frame = new QFrame(centralWidget);
        frame->setObjectName(QStringLiteral("frame"));
        frame->setGeometry(QRect(10, 10, 800, 600));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        ViewEditor->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(ViewEditor);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 820, 26));
        ViewEditor->setMenuBar(menuBar);
        mainToolBar = new QToolBar(ViewEditor);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        ViewEditor->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(ViewEditor);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        ViewEditor->setStatusBar(statusBar);

        retranslateUi(ViewEditor);

        QMetaObject::connectSlotsByName(ViewEditor);
    } // setupUi

    void retranslateUi(QMainWindow *ViewEditor)
    {
        ViewEditor->setWindowTitle(QApplication::translate("ViewEditor", "ViewEditor", 0));
    } // retranslateUi

};

namespace Ui {
    class ViewEditor: public Ui_ViewEditor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VIEWEDITOR_H
