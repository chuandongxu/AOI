#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>

#include "Insp3DSolderWidget.h"
#include "InspWindowWidget.h"
#include "../Common/SystemData.h"
#include "DataStoreAPI.h"
#include "VisionAPI.h"
#include "../include/IVisionUI.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../DataModule/QDetectObj.h"
#include "../DataModule/CalcUtils.hpp"

using namespace NFG::AOI;
using namespace AOI;

enum BASIC_PARAM
{
    CONDUCTOR_ABS_HEIGHT,
    CONDUCTOR_ABS_HEIGHT_UP_LIMIT,
    CONDUCTOR_ABS_HEIGHT_LO_LIMIT,
    CONDUCTOR_REL_HEIGHT_UP_LIMIT,
    SOLDER_HEIGHT_LO_LIMIT,
    SOLDER_HEIGHT_RATIO_LO_LIMIT,
    SOLDER_COV_RATIO_LO_LIMIT,
    SOLDER_HEIGHT_UP_LIMIT,
    SOLDER_WETTING_WIDTH,
};

Insp3DSolderWidget::Insp3DSolderWidget(InspWindowWidget *parent)
    : EditInspWindowBaseWidget(parent)
{
    ui.setupUi(this);

    m_pConductorAbsHeight = std::make_unique<SpecAndResultWidget>(ui.tableWidget, 0, 5000);
    ui.tableWidget->setCellWidget(CONDUCTOR_ABS_HEIGHT, DATA_COLUMN, m_pConductorAbsHeight.get());

    m_pConductorAbsHeightUpLimit = std::make_unique<SpecAndResultWidget>(ui.tableWidget, 0, 5000);
    ui.tableWidget->setCellWidget(CONDUCTOR_ABS_HEIGHT_UP_LIMIT, DATA_COLUMN, m_pConductorAbsHeightUpLimit.get());

    m_pConductorAbsHeightLoLimit = std::make_unique<SpecAndResultWidget>(ui.tableWidget, 0, 5000);
    ui.tableWidget->setCellWidget(CONDUCTOR_ABS_HEIGHT_LO_LIMIT, DATA_COLUMN, m_pConductorAbsHeightLoLimit.get());

    m_pConductorRelHeightUpLimit = std::make_unique<SpecAndResultWidget>(ui.tableWidget, 0, 5000);
    ui.tableWidget->setCellWidget(CONDUCTOR_REL_HEIGHT_UP_LIMIT, DATA_COLUMN, m_pConductorRelHeightUpLimit.get());

    m_pSolderHeightLoLimit = std::make_unique<SpecAndResultWidget>(ui.tableWidget, 0, 5000);
    ui.tableWidget->setCellWidget(SOLDER_HEIGHT_LO_LIMIT, DATA_COLUMN, m_pSolderHeightLoLimit.get());

    m_pSolderHeightRatioLoLimit = std::make_unique<SpecAndResultWidget>(ui.tableWidget, 0, 100);
    ui.tableWidget->setCellWidget(SOLDER_HEIGHT_RATIO_LO_LIMIT, DATA_COLUMN, m_pSolderHeightRatioLoLimit.get());

    m_pSolderCovRatioLoLimit = std::make_unique<SpecAndResultWidget>(ui.tableWidget, 0, 100);
    ui.tableWidget->setCellWidget(SOLDER_COV_RATIO_LO_LIMIT, DATA_COLUMN, m_pSolderCovRatioLoLimit.get());

    m_pSolderHeightUpLimit = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pSolderHeightUpLimit->setValidator(new QDoubleValidator(300, 1000, 2, m_pSolderHeightUpLimit.get()));
    ui.tableWidget->setCellWidget(SOLDER_HEIGHT_UP_LIMIT, DATA_COLUMN, m_pSolderHeightUpLimit.get());

    m_pSolderWettingWidth = std::make_unique<QLineEdit>(ui.tableWidget);
    m_pSolderWettingWidth->setValidator(new QDoubleValidator(100, 500, 2, m_pSolderWettingWidth.get()));
    ui.tableWidget->setCellWidget(SOLDER_WETTING_WIDTH, DATA_COLUMN, m_pSolderWettingWidth.get());
}

Insp3DSolderWidget::~Insp3DSolderWidget() {
}

void Insp3DSolderWidget::setDefaultValue() {
    m_pConductorAbsHeight->setSpec(500.f);
    m_pConductorAbsHeightUpLimit->setSpec(100.f);
    m_pConductorAbsHeightLoLimit->setSpec(100.f);
    m_pConductorRelHeightUpLimit->setSpec(50.f);
    m_pSolderHeightLoLimit->setSpec(150.f);
    m_pSolderHeightRatioLoLimit->setSpec(25);
    m_pSolderCovRatioLoLimit->setSpec(20);
    m_pSolderHeightUpLimit->setText("500");
    m_pSolderWettingWidth->setText("180");

}

void Insp3DSolderWidget::setCurrentWindow(const Engine::Window &window) {
}

void Insp3DSolderWidget::tryInsp() {
    QString strTitle(QStringLiteral("3D Solder检测"));

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_SELECT_SUB_ROI);
    auto nReturn = System->showInteractMessage(strTitle, QStringLiteral("请拖动鼠标选择第一个小检测窗口"));
    if (nReturn != QDialog::Accepted)
        return;

    auto vecSubROIs = pUI->getSubROIs();
    auto lastSubROI = vecSubROIs.back();
    cv::Point2f ptWindowCtr(lastSubROI.x + lastSubROI.width / 2.f, lastSubROI.y + lastSubROI.height / 2.f);

    nReturn = System->showInteractMessage(strTitle, QStringLiteral("需要自动添加另一个小检测框吗?"));
    if (nReturn == QDialog::Accepted)
    {
        auto selectedDevice = pUI->getSelectedDevice();
        auto ptCenter = selectedDevice.getWindow().center;
        auto rectDevice = selectedDevice.getWindow().boundingRect();
        cv::Point ptNewWindowCtr;
        if (rectDevice.width > rectDevice.height) {
            ptNewWindowCtr.x = 2 * ptCenter.x - ptWindowCtr.x;
            ptNewWindowCtr.y = ptWindowCtr.y;
        }
        else {
            ptNewWindowCtr.x = ptWindowCtr.x;
            ptNewWindowCtr.y = 2 * ptCenter.y - ptWindowCtr.y;
        }
        cv::Rect rectNewWindow(ptNewWindowCtr.x - lastSubROI.width / 2, ptNewWindowCtr.y - lastSubROI.height / 2, lastSubROI.width, lastSubROI.height);
        vecSubROIs.push_back(rectNewWindow);
        pUI->setSubROIs(vecSubROIs);
    }
    else
    {
        pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_SELECT_SUB_ROI);
        auto nReturn = System->showInteractMessage(strTitle, QStringLiteral("请拖动鼠标选择另一个小检测窗口"));
        if (nReturn != QDialog::Accepted)
            return;
    }
}

void Insp3DSolderWidget::confirmWindow(OPERATION enOperation) {
}
