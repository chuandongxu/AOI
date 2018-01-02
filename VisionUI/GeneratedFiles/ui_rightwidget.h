/********************************************************************************
** Form generated from reading UI file 'rightwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RIGHTWIDGET_H
#define UI_RIGHTWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_rightwidget
{
public:

    void setupUi(QWidget *rightwidget)
    {
        if (rightwidget->objectName().isEmpty())
            rightwidget->setObjectName(QStringLiteral("rightwidget"));
        rightwidget->resize(410, 900);

        retranslateUi(rightwidget);

        QMetaObject::connectSlotsByName(rightwidget);
    } // setupUi

    void retranslateUi(QWidget *rightwidget)
    {
        rightwidget->setWindowTitle(QApplication::translate("rightwidget", "rightwidget", 0));
    } // retranslateUi

};

namespace Ui {
    class rightwidget: public Ui_rightwidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RIGHTWIDGET_H
