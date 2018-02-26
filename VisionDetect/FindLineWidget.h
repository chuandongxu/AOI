#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <memory>
#include "ui_FindLineWidget.h"

class FindLineWidget : public QWidget
{
    Q_OBJECT

public:
    FindLineWidget(QWidget *parent = Q_NULLPTR);
    ~FindLineWidget();

private slots:
    void on_btnTryInsp_clicked();
    void on_btnConfirmWindow_clicked();

private:
    Ui::FindLineWidget ui;

    std::unique_ptr<QCheckBox>  m_pCheckBoxFindPair;
    std::unique_ptr<QComboBox>  m_pComboBoxFindLineDirection;
    std::unique_ptr<QLineEdit>  m_pEditCaliperCount;
    std::unique_ptr<QLineEdit>  m_pEditCaliperWidth;
    std::unique_ptr<QLineEdit>  m_pEditEdgeThreshold;
    std::unique_ptr<QComboBox>  m_pComboBoxEdgeSelectMethod;
    std::unique_ptr<QLineEdit>  m_pEditRmStrayPointRatio;
    std::unique_ptr<QLineEdit>  m_pEditDiffFilterHalfW;
    std::unique_ptr<QLineEdit>  m_pEditDiffFilterSigma;
    std::unique_ptr<QCheckBox>  m_pCheckLinerity;
    std::unique_ptr<QLineEdit>  m_pEditPointMaxOffset;
    std::unique_ptr<QLineEdit>  m_pEditMinLinearity;
    std::unique_ptr<QCheckBox>  m_pEditCheckAngle;
    std::unique_ptr<QLineEdit>  m_pEditExpectedAngle;
    std::unique_ptr<QLineEdit>  m_pEditAngleDiffTolerance;
};
