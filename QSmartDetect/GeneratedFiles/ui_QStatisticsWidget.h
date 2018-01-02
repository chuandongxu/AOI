/********************************************************************************
** Form generated from reading UI file 'QStatisticsWidget.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QSTATISTICSWIDGET_H
#define UI_QSTATISTICSWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QStatisticsWidget
{
public:
    QDockWidget *dockWidget_3;
    QWidget *dockWidgetContents_3;
    QGroupBox *groupBox;
    QPushButton *pushButton_runOnce;
    QGroupBox *groupBox_2;
    QLabel *label_3;
    QProgressBar *progressBar_resultPrg;
    QLabel *label_2;
    QLabel *label_resultOutput;
    QLabel *label_resultStatus;
    QTableView *tableView_resultList;
    QDockWidget *dockWidget_4;
    QWidget *dockWidgetContents_5;

    void setupUi(QWidget *QStatisticsWidget)
    {
        if (QStatisticsWidget->objectName().isEmpty())
            QStatisticsWidget->setObjectName(QStringLiteral("QStatisticsWidget"));
        QStatisticsWidget->resize(390, 735);
        dockWidget_3 = new QDockWidget(QStatisticsWidget);
        dockWidget_3->setObjectName(QStringLiteral("dockWidget_3"));
        dockWidget_3->setGeometry(QRect(5, 300, 361, 150));
        dockWidget_3->setFeatures(QDockWidget::NoDockWidgetFeatures);
        dockWidgetContents_3 = new QWidget();
        dockWidgetContents_3->setObjectName(QStringLiteral("dockWidgetContents_3"));
        dockWidget_3->setWidget(dockWidgetContents_3);
        groupBox = new QGroupBox(QStatisticsWidget);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(5, 660, 361, 71));
        pushButton_runOnce = new QPushButton(groupBox);
        pushButton_runOnce->setObjectName(QStringLiteral("pushButton_runOnce"));
        pushButton_runOnce->setGeometry(QRect(10, 20, 340, 45));
        groupBox_2 = new QGroupBox(QStatisticsWidget);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        groupBox_2->setGeometry(QRect(5, 10, 361, 131));
        label_3 = new QLabel(groupBox_2);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(20, 92, 51, 16));
        progressBar_resultPrg = new QProgressBar(groupBox_2);
        progressBar_resultPrg->setObjectName(QStringLiteral("progressBar_resultPrg"));
        progressBar_resultPrg->setGeometry(QRect(70, 90, 281, 23));
        progressBar_resultPrg->setValue(0);
        label_2 = new QLabel(groupBox_2);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(20, 55, 41, 16));
        label_resultOutput = new QLabel(groupBox_2);
        label_resultOutput->setObjectName(QStringLiteral("label_resultOutput"));
        label_resultOutput->setGeometry(QRect(180, 10, 171, 71));
        label_resultOutput->setStyleSheet(QString::fromUtf8("font: 75 48pt \"\345\256\213\344\275\223\";"));
        label_resultOutput->setAlignment(Qt::AlignCenter);
        label_resultStatus = new QLabel(groupBox_2);
        label_resultStatus->setObjectName(QStringLiteral("label_resultStatus"));
        label_resultStatus->setGeometry(QRect(70, 50, 101, 26));
        label_resultStatus->setStyleSheet(QStringLiteral("background-color: rgb(255, 170, 127);"));
        label_resultStatus->setAlignment(Qt::AlignCenter);
        tableView_resultList = new QTableView(QStatisticsWidget);
        tableView_resultList->setObjectName(QStringLiteral("tableView_resultList"));
        tableView_resultList->setGeometry(QRect(5, 450, 361, 200));
        dockWidget_4 = new QDockWidget(QStatisticsWidget);
        dockWidget_4->setObjectName(QStringLiteral("dockWidget_4"));
        dockWidget_4->setGeometry(QRect(5, 140, 361, 150));
        dockWidget_4->setFeatures(QDockWidget::NoDockWidgetFeatures);
        dockWidgetContents_5 = new QWidget();
        dockWidgetContents_5->setObjectName(QStringLiteral("dockWidgetContents_5"));
        dockWidget_4->setWidget(dockWidgetContents_5);

        retranslateUi(QStatisticsWidget);

        QMetaObject::connectSlotsByName(QStatisticsWidget);
    } // setupUi

    void retranslateUi(QWidget *QStatisticsWidget)
    {
        QStatisticsWidget->setWindowTitle(QApplication::translate("QStatisticsWidget", "QStatisticsWidget", 0));
        groupBox->setTitle(QApplication::translate("QStatisticsWidget", "\346\223\215\344\275\234\350\277\220\350\241\214", 0));
        pushButton_runOnce->setText(QApplication::translate("QStatisticsWidget", "\344\270\200\351\224\256\346\243\200\346\265\213", 0));
        groupBox_2->setTitle(QApplication::translate("QStatisticsWidget", "\350\277\220\350\241\214\347\212\266\346\200\201", 0));
        label_3->setText(QApplication::translate("QStatisticsWidget", "\350\277\233\345\272\246\357\274\232", 0));
        label_2->setText(QApplication::translate("QStatisticsWidget", "\347\212\266\346\200\201:", 0));
        label_resultOutput->setText(QApplication::translate("QStatisticsWidget", "NA", 0));
        label_resultStatus->setText(QApplication::translate("QStatisticsWidget", "\345\267\245\344\275\215\347\251\272\351\227\262\344\270\255", 0));
    } // retranslateUi

};

namespace Ui {
    class QStatisticsWidget: public Ui_QStatisticsWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QSTATISTICSWIDGET_H
