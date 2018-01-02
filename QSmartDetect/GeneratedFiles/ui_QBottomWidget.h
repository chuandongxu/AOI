/********************************************************************************
** Form generated from reading UI file 'QBottomWidget.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QBOTTOMWIDGET_H
#define UI_QBOTTOMWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QBottomWidget
{
public:
    QTreeView *treeView;

    void setupUi(QWidget *QBottomWidget)
    {
        if (QBottomWidget->objectName().isEmpty())
            QBottomWidget->setObjectName(QStringLiteral("QBottomWidget"));
        QBottomWidget->resize(800, 100);
        treeView = new QTreeView(QBottomWidget);
        treeView->setObjectName(QStringLiteral("treeView"));
        treeView->setGeometry(QRect(0, 0, 800, 100));

        retranslateUi(QBottomWidget);

        QMetaObject::connectSlotsByName(QBottomWidget);
    } // setupUi

    void retranslateUi(QWidget *QBottomWidget)
    {
        QBottomWidget->setWindowTitle(QApplication::translate("QBottomWidget", "QBottomWidget", 0));
    } // retranslateUi

};

namespace Ui {
    class QBottomWidget: public Ui_QBottomWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QBOTTOMWIDGET_H
