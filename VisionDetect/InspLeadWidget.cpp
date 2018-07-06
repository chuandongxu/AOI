#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>

#include "InspLeadWidget.h"
#include "../Common/SystemData.h"
#include "DataStoreAPI.h"
#include "VisionAPI.h"
#include "../include/IVisionUI.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../Common/CommonFunc.h"
#include "../DataModule/QDetectObj.h"
#include "InspWindowWidget.h"
#include "../DataModule/CalcUtils.hpp"

using namespace NFG::AOI;
using namespace AOI;

InspLeadWidget::InspLeadWidget(InspWindowWidget *parent)
    : EditInspWindowBaseWidget(parent)
{
    ui.setupUi(this);
}

InspLeadWidget::~InspLeadWidget()
{
}

void InspLeadWidget::setDefaultValue() {
}

void InspLeadWidget::setCurrentWindow(const Engine::Window &window) {
}

void InspLeadWidget::tryInsp() {
}

void InspLeadWidget::confirmWindow(OPERATION enOperation) {
}

void InspLeadWidget::on_btnAutoLocateLead_clicked() {
    QString strTitle(QStringLiteral("自动生成检测框"));
    auto pUI = getModule<IVisionUI>(UI_MODEL);
    auto vvDevice = pUI->getSelectedDevice();
    if (vvDevice.getId() <= 0) {
        System->showMessage(strTitle, QStringLiteral("请先选择一个元件!"));
        return;
    }

    auto rectDevice = vvDevice.getWindow().boundingRect();

     cv::Rect rectDefaultSrchWindow = CalcUtils::resizeRect(rectDevice, cv::Size(rectDevice.width + 50, rectDevice.height + 50));
     pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_SRCH_WINDOW);
     pUI->setSrchWindow(rectDefaultSrchWindow);
     auto nReturn = System->showInteractMessage(strTitle, QStringLiteral("请拖动鼠标选择搜寻窗口"));
     if (nReturn != QDialog::Accepted)
         return;

    auto rectSrchWindow = pUI->getSrchWindow();

    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_INSP_WINDOW);
    nReturn = System->showInteractMessage(strTitle, QStringLiteral("请拖动鼠标选择Pad"));
    if (nReturn != QDialog::Accepted)
        return;

    auto rectPad = pUI->getSelectedROI();

    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_INSP_WINDOW);
    nReturn = System->showInteractMessage(strTitle, QStringLiteral("请拖动鼠标选择Lead"));
    if (nReturn != QDialog::Accepted)
        return;

    auto rectLead = pUI->getSelectedROI();

    Vision::PR_AUTO_LOCATE_LEAD_CMD stCmd;
    Vision::PR_AUTO_LOCATE_LEAD_RPY stRpy;
    stCmd.rectChipBody = rectDevice;
    stCmd.rectSrchWindow = rectSrchWindow;
    stCmd.rectLeadWindow = rectLead;
    stCmd.rectPadWindow = rectPad;
    stCmd.enMethod = Vision::PR_AUTO_LOCATE_LEAD_METHOD::TEMPLATE_MATCH;

    Vision::PR_AutoLocateLead(&stCmd, &stRpy);
}