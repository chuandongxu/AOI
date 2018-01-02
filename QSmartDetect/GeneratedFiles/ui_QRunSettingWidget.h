/********************************************************************************
** Form generated from reading UI file 'QRunSettingWidget.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QRUNSETTINGWIDGET_H
#define UI_QRUNSETTINGWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableView>
#include <QtWidgets/QToolBox>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QRunSettingWidget
{
public:
    QGroupBox *groupBox_2;
    QPushButton *pushButton_3DDetectOpen;
    QLineEdit *lineEdit_3DDetectFile;
    QComboBox *comboBox_runType;
    QLabel *label_55;
    QComboBox *comboBox_runMode;
    QLabel *label_56;
    QToolBox *toolBox;
    QWidget *page;
    QWidget *page_1;
    QWidget *page_2;
    QWidget *page_3;
    QTableView *tableView_objList;
    QPushButton *pushButton_cellEditor;
    QPushButton *pushButton_profileEditor;

    void setupUi(QWidget *QRunSettingWidget)
    {
        if (QRunSettingWidget->objectName().isEmpty())
            QRunSettingWidget->setObjectName(QStringLiteral("QRunSettingWidget"));
        QRunSettingWidget->resize(390, 735);
        groupBox_2 = new QGroupBox(QRunSettingWidget);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        groupBox_2->setGeometry(QRect(5, 10, 361, 111));
        pushButton_3DDetectOpen = new QPushButton(groupBox_2);
        pushButton_3DDetectOpen->setObjectName(QStringLiteral("pushButton_3DDetectOpen"));
        pushButton_3DDetectOpen->setGeometry(QRect(299, 66, 51, 25));
        lineEdit_3DDetectFile = new QLineEdit(groupBox_2);
        lineEdit_3DDetectFile->setObjectName(QStringLiteral("lineEdit_3DDetectFile"));
        lineEdit_3DDetectFile->setGeometry(QRect(10, 66, 281, 25));
        comboBox_runType = new QComboBox(groupBox_2);
        comboBox_runType->setObjectName(QStringLiteral("comboBox_runType"));
        comboBox_runType->setGeometry(QRect(10, 33, 125, 22));
        label_55 = new QLabel(groupBox_2);
        label_55->setObjectName(QStringLiteral("label_55"));
        label_55->setGeometry(QRect(10, 10, 71, 18));
        comboBox_runMode = new QComboBox(groupBox_2);
        comboBox_runMode->setObjectName(QStringLiteral("comboBox_runMode"));
        comboBox_runMode->setGeometry(QRect(166, 33, 125, 22));
        label_56 = new QLabel(groupBox_2);
        label_56->setObjectName(QStringLiteral("label_56"));
        label_56->setGeometry(QRect(166, 10, 71, 18));
        toolBox = new QToolBox(QRunSettingWidget);
        toolBox->setObjectName(QStringLiteral("toolBox"));
        toolBox->setGeometry(QRect(5, 130, 361, 261));
        toolBox->setFrameShape(QFrame::Panel);
        toolBox->setFrameShadow(QFrame::Raised);
        page = new QWidget();
        page->setObjectName(QStringLiteral("page"));
        page->setGeometry(QRect(0, 0, 359, 139));
        toolBox->addItem(page, QString::fromUtf8("\346\240\207\345\207\206\347\273\204\344\273\266"));
        page_1 = new QWidget();
        page_1->setObjectName(QStringLiteral("page_1"));
        page_1->setGeometry(QRect(0, 0, 98, 28));
        toolBox->addItem(page_1, QString::fromUtf8("\347\256\241\350\204\232\345\231\250\344\273\266"));
        page_2 = new QWidget();
        page_2->setObjectName(QStringLiteral("page_2"));
        page_2->setGeometry(QRect(0, 0, 98, 28));
        toolBox->addItem(page_2, QString::fromUtf8("IC\350\212\257\347\211\207"));
        page_3 = new QWidget();
        page_3->setObjectName(QStringLiteral("page_3"));
        page_3->setGeometry(QRect(0, 0, 98, 28));
        toolBox->addItem(page_3, QString::fromUtf8("LED\345\205\203\344\273\266"));
        tableView_objList = new QTableView(QRunSettingWidget);
        tableView_objList->setObjectName(QStringLiteral("tableView_objList"));
        tableView_objList->setGeometry(QRect(5, 480, 361, 251));
        pushButton_cellEditor = new QPushButton(QRunSettingWidget);
        pushButton_cellEditor->setObjectName(QStringLiteral("pushButton_cellEditor"));
        pushButton_cellEditor->setGeometry(QRect(5, 400, 361, 35));
        pushButton_profileEditor = new QPushButton(QRunSettingWidget);
        pushButton_profileEditor->setObjectName(QStringLiteral("pushButton_profileEditor"));
        pushButton_profileEditor->setGeometry(QRect(5, 440, 361, 35));

        retranslateUi(QRunSettingWidget);

        toolBox->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(QRunSettingWidget);
    } // setupUi

    void retranslateUi(QWidget *QRunSettingWidget)
    {
        QRunSettingWidget->setWindowTitle(QApplication::translate("QRunSettingWidget", "QRunSettingWidget", 0));
        groupBox_2->setTitle(QString());
        pushButton_3DDetectOpen->setText(QApplication::translate("QRunSettingWidget", "\346\226\207\344\273\266\345\244\271", 0));
        label_55->setText(QApplication::translate("QRunSettingWidget", "\346\243\200\346\265\213\346\250\241\345\274\217:", 0));
        label_56->setText(QApplication::translate("QRunSettingWidget", "\346\265\213\350\257\225\346\254\241\346\225\260:", 0));
        toolBox->setItemText(toolBox->indexOf(page), QApplication::translate("QRunSettingWidget", "\346\240\207\345\207\206\347\273\204\344\273\266", 0));
        toolBox->setItemText(toolBox->indexOf(page_1), QApplication::translate("QRunSettingWidget", "\347\256\241\350\204\232\345\231\250\344\273\266", 0));
        toolBox->setItemText(toolBox->indexOf(page_2), QApplication::translate("QRunSettingWidget", "IC\350\212\257\347\211\207", 0));
        toolBox->setItemText(toolBox->indexOf(page_3), QApplication::translate("QRunSettingWidget", "LED\345\205\203\344\273\266", 0));
        pushButton_cellEditor->setText(QApplication::translate("QRunSettingWidget", "\347\274\226\350\276\221\345\205\203\345\231\250\344\273\266", 0));
        pushButton_profileEditor->setText(QApplication::translate("QRunSettingWidget", "\347\274\226\350\276\221\350\275\256\345\273\223\347\272\277", 0));
    } // retranslateUi

};

namespace Ui {
    class QRunSettingWidget: public Ui_QRunSettingWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QRUNSETTINGWIDGET_H
