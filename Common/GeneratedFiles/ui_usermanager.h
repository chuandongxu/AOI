/********************************************************************************
** Form generated from reading UI file 'usermanager.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_USERMANAGER_H
#define UI_USERMANAGER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QUserManager
{
public:
    QTableView *tableView;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QPushButton *pushButton_3;

    void setupUi(QWidget *QUserManager)
    {
        if (QUserManager->objectName().isEmpty())
            QUserManager->setObjectName(QStringLiteral("QUserManager"));
        QUserManager->resize(574, 287);
        tableView = new QTableView(QUserManager);
        tableView->setObjectName(QStringLiteral("tableView"));
        tableView->setGeometry(QRect(20, 40, 541, 231));
        pushButton = new QPushButton(QUserManager);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(30, 10, 75, 23));
        pushButton_2 = new QPushButton(QUserManager);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setGeometry(QRect(120, 10, 75, 23));
        pushButton_3 = new QPushButton(QUserManager);
        pushButton_3->setObjectName(QStringLiteral("pushButton_3"));
        pushButton_3->setGeometry(QRect(210, 10, 75, 23));

        retranslateUi(QUserManager);

        QMetaObject::connectSlotsByName(QUserManager);
    } // setupUi

    void retranslateUi(QWidget *QUserManager)
    {
        QUserManager->setWindowTitle(QApplication::translate("QUserManager", "QUserManager", 0));
        pushButton->setText(QApplication::translate("QUserManager", "\345\242\236\345\212\240\347\224\250\346\210\267", 0));
        pushButton_2->setText(QApplication::translate("QUserManager", "\347\274\226\350\276\221\347\224\250\346\210\267", 0));
        pushButton_3->setText(QApplication::translate("QUserManager", "\345\210\240\351\231\244\347\224\250\346\210\267", 0));
    } // retranslateUi

};

namespace Ui {
    class QUserManager: public Ui_QUserManager {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_USERMANAGER_H
