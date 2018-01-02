/********************************************************************************
** Form generated from reading UI file 'graphiceditor.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GRAPHICEDITOR_H
#define UI_GRAPHICEDITOR_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_graphiceditorClass
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QComboBox *comboBox_item;
    QComboBox *comboBox_type;
    QSpacerItem *horizontalSpacer_2;
    QLabel *label;
    QLineEdit *lineEdit_input;
    QLabel *label_2;
    QLineEdit *lineEdit_output;
    QPushButton *pushButton_edit;
    QSpacerItem *horizontalSpacer;
    QFrame *frame_spline;

    void setupUi(QDialog *graphiceditorClass)
    {
        if (graphiceditorClass->objectName().isEmpty())
            graphiceditorClass->setObjectName(QStringLiteral("graphiceditorClass"));
        graphiceditorClass->resize(575, 375);
        verticalLayout = new QVBoxLayout(graphiceditorClass);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
        comboBox_item = new QComboBox(graphiceditorClass);
        comboBox_item->setObjectName(QStringLiteral("comboBox_item"));

        horizontalLayout->addWidget(comboBox_item);

        comboBox_type = new QComboBox(graphiceditorClass);
        comboBox_type->setObjectName(QStringLiteral("comboBox_type"));

        horizontalLayout->addWidget(comboBox_type);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        label = new QLabel(graphiceditorClass);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label);

        lineEdit_input = new QLineEdit(graphiceditorClass);
        lineEdit_input->setObjectName(QStringLiteral("lineEdit_input"));
        lineEdit_input->setFrame(false);
        lineEdit_input->setReadOnly(true);

        horizontalLayout->addWidget(lineEdit_input);

        label_2 = new QLabel(graphiceditorClass);
        label_2->setObjectName(QStringLiteral("label_2"));

        horizontalLayout->addWidget(label_2);

        lineEdit_output = new QLineEdit(graphiceditorClass);
        lineEdit_output->setObjectName(QStringLiteral("lineEdit_output"));
        lineEdit_output->setFrame(false);
        lineEdit_output->setReadOnly(true);

        horizontalLayout->addWidget(lineEdit_output);

        pushButton_edit = new QPushButton(graphiceditorClass);
        pushButton_edit->setObjectName(QStringLiteral("pushButton_edit"));

        horizontalLayout->addWidget(pushButton_edit);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout);

        frame_spline = new QFrame(graphiceditorClass);
        frame_spline->setObjectName(QStringLiteral("frame_spline"));
        frame_spline->setMinimumSize(QSize(400, 300));
        frame_spline->setFrameShape(QFrame::StyledPanel);
        frame_spline->setFrameShadow(QFrame::Raised);

        verticalLayout->addWidget(frame_spline);


        retranslateUi(graphiceditorClass);

        QMetaObject::connectSlotsByName(graphiceditorClass);
    } // setupUi

    void retranslateUi(QDialog *graphiceditorClass)
    {
        graphiceditorClass->setWindowTitle(QApplication::translate("graphiceditorClass", "GraphicEditor", 0));
        label->setText(QApplication::translate("graphiceditorClass", "input(X)", 0));
        lineEdit_input->setText(QApplication::translate("graphiceditorClass", "0", 0));
        label_2->setText(QApplication::translate("graphiceditorClass", "output(Y)", 0));
        lineEdit_output->setText(QApplication::translate("graphiceditorClass", "0", 0));
        pushButton_edit->setText(QApplication::translate("graphiceditorClass", "\347\274\226\350\276\221\350\275\256\345\273\223", 0));
    } // retranslateUi

};

namespace Ui {
    class graphiceditorClass: public Ui_graphiceditorClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GRAPHICEDITOR_H
