/********************************************************************************
** Form generated from reading UI file 'QRightWidget.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QRIGHTWIDGET_H
#define UI_QRIGHTWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QRightWidget
{
public:
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout_tab;
    QWidget *verticalLayoutWidget_2;
    QVBoxLayout *verticalLayout_Content;

    void setupUi(QWidget *QRightWidget)
    {
        if (QRightWidget->objectName().isEmpty())
            QRightWidget->setObjectName(QStringLiteral("QRightWidget"));
        QRightWidget->resize(390, 800);
        verticalLayoutWidget = new QWidget(QRightWidget);
        verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(0, 740, 391, 51));
        verticalLayout_tab = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout_tab->setSpacing(6);
        verticalLayout_tab->setContentsMargins(11, 11, 11, 11);
        verticalLayout_tab->setObjectName(QStringLiteral("verticalLayout_tab"));
        verticalLayout_tab->setContentsMargins(0, 0, 0, 0);
        verticalLayoutWidget_2 = new QWidget(QRightWidget);
        verticalLayoutWidget_2->setObjectName(QStringLiteral("verticalLayoutWidget_2"));
        verticalLayoutWidget_2->setGeometry(QRect(0, 0, 391, 731));
        verticalLayout_Content = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_Content->setSpacing(6);
        verticalLayout_Content->setContentsMargins(11, 11, 11, 11);
        verticalLayout_Content->setObjectName(QStringLiteral("verticalLayout_Content"));
        verticalLayout_Content->setContentsMargins(0, 0, 0, 0);

        retranslateUi(QRightWidget);

        QMetaObject::connectSlotsByName(QRightWidget);
    } // setupUi

    void retranslateUi(QWidget *QRightWidget)
    {
        QRightWidget->setWindowTitle(QApplication::translate("QRightWidget", "QRightWidget", 0));
    } // retranslateUi

};

namespace Ui {
    class QRightWidget: public Ui_QRightWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QRIGHTWIDGET_H
