#include <QDialog>

#include "BoardWidget.h"
#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/IMotion.h"
#include "DataStoreAPI.h"

using namespace NFG::AOI;

BoardWidget::BoardWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

BoardWidget::~BoardWidget()
{
}

void BoardWidget::showEvent(QShowEvent *event) {
    ui.btnBtmRight->setEnabled(false);
    _displayResult();
}

void BoardWidget::on_btnTopLeft_clicked() {
    auto nReturn = System->showInteractMessage(QStringLiteral("设置电路板"), QStringLiteral("请移动XY Table直到相机中心对准电路板左上角"));
    if (nReturn != QDialog::Accepted)
        return;

    ui.btnBtmRight->setEnabled(true);

    auto pMotion = getModule<IMotion>(MOTION_MODEL);
    pMotion->getCurrentPos(AXIS_MOTOR_X, &m_dLeftX);
    pMotion->getCurrentPos(AXIS_MOTOR_Y, &m_dTopY);
}

void BoardWidget::on_btnBtmRight_clicked() {
    auto nReturn = System->showInteractMessage(QStringLiteral("设置电路板"), QStringLiteral("请移动XY Table直到相机中心对准电路板右下角"));
    if (nReturn != QDialog::Accepted)
        return;

    auto pMotion = getModule<IMotion>(MOTION_MODEL);
    pMotion->getCurrentPos(AXIS_MOTOR_X, &m_dRightX);
    pMotion->getCurrentPos(AXIS_MOTOR_Y, &m_dBottomY);

    Engine::SetBigBoardCoords(m_dLeftX, m_dTopY, m_dRightX, m_dBottomY);
    _displayResult();
}

void BoardWidget::_displayResult() {
    float left = 0.f, top = 0.f, right = 0.f, bottom = 0.f;
    if (Engine::GetBigBoardCoords(left, top, right, bottom) != 0) {
        String errorType, errorMessage;
        Engine::GetErrorDetail ( errorType, errorMessage );
        System->showMessage(QStringLiteral("设置电路板"), QStringLiteral("Error at GetBigBoardCoords, type = %1, msg= %2").arg(errorType.c_str()).arg(errorMessage.c_str()));
        return;
    }

    QString strBoardLeftTop, strBoardRightBtm;
    strBoardLeftTop.sprintf("%.2f, %.2f", left, top);
    ui.lineEditBoardLeftTop->setText(strBoardLeftTop);
    strBoardRightBtm.sprintf("%.2f, %.2f", right, bottom);
    ui.lineEditBoardRightBtm->setText(strBoardRightBtm);
    ui.lineEditBoardWidth->setText(QString::number(right - left));
    ui.lineEditBoardHeight->setText(QString::number(top - bottom));
}