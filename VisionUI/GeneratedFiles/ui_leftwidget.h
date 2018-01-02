/********************************************************************************
** Form generated from reading UI file 'leftwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LEFTWIDGET_H
#define UI_LEFTWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_leftWidget
{
public:
    QLabel *label_log;
    QWidget *widget_leftbk;
    QFrame *frame;
    QWidget *widget;
    QLabel *label_user;
    QPushButton *pushButton_4;
    QPushButton *pushButton;
    QLineEdit *lineEdit;
    QPushButton *pushButton_5;
    QPushButton *pushButton_3;
    QPushButton *pushButton_2;
    QLabel *label_spliter;
    QLabel *label_spliter2;
    QLabel *label_2;
    QPushButton *pushButton_6;
    QComboBox *comboBox;
    QLabel *label_4;
    QLabel *label;
    QLabel *label_3;

    void setupUi(QWidget *leftWidget)
    {
        if (leftWidget->objectName().isEmpty())
            leftWidget->setObjectName(QStringLiteral("leftWidget"));
        leftWidget->resize(280, 1102);
        label_log = new QLabel(leftWidget);
        label_log->setObjectName(QStringLiteral("label_log"));
        label_log->setGeometry(QRect(10, 970, 200, 100));
        widget_leftbk = new QWidget(leftWidget);
        widget_leftbk->setObjectName(QStringLiteral("widget_leftbk"));
        widget_leftbk->setGeometry(QRect(10, 130, 221, 821));
        frame = new QFrame(widget_leftbk);
        frame->setObjectName(QStringLiteral("frame"));
        frame->setGeometry(QRect(5, 400, 210, 415));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        widget = new QWidget(widget_leftbk);
        widget->setObjectName(QStringLiteral("widget"));
        widget->setGeometry(QRect(5, 5, 210, 391));
        label_user = new QLabel(widget);
        label_user->setObjectName(QStringLiteral("label_user"));
        label_user->setGeometry(QRect(10, 30, 70, 90));
        pushButton_4 = new QPushButton(widget);
        pushButton_4->setObjectName(QStringLiteral("pushButton_4"));
        pushButton_4->setGeometry(QRect(110, 255, 80, 110));
        pushButton = new QPushButton(widget);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(20, 135, 80, 110));
        lineEdit = new QLineEdit(widget);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));
        lineEdit->setGeometry(QRect(120, 25, 81, 21));
        QFont font;
        font.setPointSize(10);
        lineEdit->setFont(font);
        lineEdit->setFrame(false);
        lineEdit->setReadOnly(true);
        pushButton_5 = new QPushButton(widget);
        pushButton_5->setObjectName(QStringLiteral("pushButton_5"));
        pushButton_5->setGeometry(QRect(150, 55, 51, 23));
        pushButton_5->setDefault(false);
        pushButton_5->setFlat(false);
        pushButton_3 = new QPushButton(widget);
        pushButton_3->setObjectName(QStringLiteral("pushButton_3"));
        pushButton_3->setGeometry(QRect(110, 135, 81, 111));
        pushButton_2 = new QPushButton(widget);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setGeometry(QRect(20, 255, 81, 110));
        label_spliter = new QLabel(widget);
        label_spliter->setObjectName(QStringLiteral("label_spliter"));
        label_spliter->setGeometry(QRect(20, 120, 181, 3));
        label_spliter2 = new QLabel(widget);
        label_spliter2->setObjectName(QStringLiteral("label_spliter2"));
        label_spliter2->setGeometry(QRect(20, 380, 181, 3));
        label_2 = new QLabel(widget);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setEnabled(false);
        label_2->setGeometry(QRect(80, 28, 31, 16));
        QFont font1;
        font1.setFamily(QStringLiteral("Agency FB"));
        font1.setPointSize(10);
        font1.setBold(true);
        font1.setWeight(75);
        label_2->setFont(font1);
        pushButton_6 = new QPushButton(widget);
        pushButton_6->setObjectName(QStringLiteral("pushButton_6"));
        pushButton_6->setGeometry(QRect(80, 55, 61, 23));
        pushButton_6->setContextMenuPolicy(Qt::DefaultContextMenu);
        pushButton_6->setInputMethodHints(Qt::ImhNone);
        pushButton_6->setDefault(false);
        pushButton_6->setFlat(false);
        comboBox = new QComboBox(widget);
        comboBox->setObjectName(QStringLiteral("comboBox"));
        comboBox->setGeometry(QRect(120, 85, 81, 22));
        label_4 = new QLabel(widget);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setEnabled(false);
        label_4->setGeometry(QRect(80, 86, 31, 16));
        QFont font2;
        font2.setPointSize(10);
        font2.setBold(true);
        font2.setWeight(75);
        label_4->setFont(font2);
        label = new QLabel(leftWidget);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(40, 35, 176, 36));
        QPalette palette;
        QBrush brush(QColor(255, 255, 255, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
        QBrush brush1(QColor(120, 120, 120, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush1);
        label->setPalette(palette);
        QFont font3;
        font3.setFamily(QStringLiteral("Arial"));
        font3.setPointSize(16);
        font3.setBold(true);
        font3.setWeight(75);
        label->setFont(font3);
        label_3 = new QLabel(leftWidget);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(60, 70, 176, 36));
        QPalette palette1;
        palette1.setBrush(QPalette::Active, QPalette::WindowText, brush);
        palette1.setBrush(QPalette::Active, QPalette::Button, brush);
        palette1.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
        palette1.setBrush(QPalette::Inactive, QPalette::Button, brush);
        palette1.setBrush(QPalette::Disabled, QPalette::WindowText, brush1);
        palette1.setBrush(QPalette::Disabled, QPalette::Button, brush);
        label_3->setPalette(palette1);
        label_3->setFont(font3);

        retranslateUi(leftWidget);

        QMetaObject::connectSlotsByName(leftWidget);
    } // setupUi

    void retranslateUi(QWidget *leftWidget)
    {
        leftWidget->setWindowTitle(QApplication::translate("leftWidget", "Form", 0));
        label_log->setText(QString());
        label_user->setText(QString());
        pushButton_4->setText(QApplication::translate("leftWidget", "\345\233\236\351\233\266", 0));
        pushButton->setText(QApplication::translate("leftWidget", "\350\256\276\347\275\256/\350\260\203\350\257\225", 0));
        pushButton_5->setText(QApplication::translate("leftWidget", "\345\210\207\346\215\242", 0));
        pushButton_3->setText(QApplication::translate("leftWidget", "\345\221\212\350\255\246/\346\227\245\345\277\227", 0));
        pushButton_2->setText(QApplication::translate("leftWidget", "\350\257\212\346\226\255", 0));
        label_spliter->setText(QString());
        label_spliter2->setText(QString());
        label_2->setText(QApplication::translate("leftWidget", "USER\357\274\232", 0));
        pushButton_6->setText(QApplication::translate("leftWidget", "\347\256\241\347\220\206\347\224\250\346\210\267", 0));
        label_4->setText(QApplication::translate("leftWidget", "LAN\357\274\232", 0));
        label->setText(QString());
        label_3->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class leftWidget: public Ui_leftWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LEFTWIDGET_H
