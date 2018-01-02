/********************************************************************************
** Form generated from reading UI file 'TimeWidget.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TIMEWIDGET_H
#define UI_TIMEWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QTimeWidget
{
public:
    QLabel *label;
    QLabel *label_2;

    void setupUi(QWidget *QTimeWidget)
    {
        if (QTimeWidget->objectName().isEmpty())
            QTimeWidget->setObjectName(QStringLiteral("QTimeWidget"));
        QTimeWidget->resize(201, 184);
        label = new QLabel(QTimeWidget);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(25, 40, 141, 26));
        label_2 = new QLabel(QTimeWidget);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(30, 90, 146, 16));

        retranslateUi(QTimeWidget);

        QMetaObject::connectSlotsByName(QTimeWidget);
    } // setupUi

    void retranslateUi(QWidget *QTimeWidget)
    {
        QTimeWidget->setWindowTitle(QApplication::translate("QTimeWidget", "QTimeWidget", 0));
        label->setText(QApplication::translate("QTimeWidget", "TextLabel", 0));
        label_2->setText(QApplication::translate("QTimeWidget", "TextLabel", 0));
    } // retranslateUi

};

namespace Ui {
    class QTimeWidget: public Ui_QTimeWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TIMEWIDGET_H
