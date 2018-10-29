#include <QMessageBox>

#include "EditCADWidget.h"
#include "DataStoreAPI.h"
#include "constants.h"
#include "SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/IVisionUI.h"
#include "../include/IVision.h"
#include "EditDeviceDialog.h"
#include "ScanImageWidget.h"

using namespace NFG::AOI;

EditCADWidget::EditCADWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

EditCADWidget::~EditCADWidget() {
}

void EditCADWidget::on_btnAddDevice_clicked() {
    EditDeviceDialog dialog(this);
    dialog.setModal(true);
    auto nReturn = dialog.exec();
    if (nReturn != QDialog::Accepted)
        return;

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_SELECT_ROI);

    QString strTitle(QStringLiteral("添加元件"));
    nReturn = System->showInteractMessage(strTitle, QStringLiteral("请选择添加元件位置"));
    if (nReturn != QDialog::Accepted)
        return;
    
    auto selectedROI = pUI->getSelectedROI();

    Int32 bBoardRotated = 0; Engine::GetParameter("BOARD_ROTATED", bBoardRotated, false);
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    float fCombinedImageScale = 1.f; Engine::GetParameter("ScanImageZoomFactor", fCombinedImageScale, 1.f);

    VisionViewDeviceVector vecVisionViewDevices;
    Engine::BoardVector vecBoard;
    nReturn = Engine::GetAllBoards(vecBoard);
    if (Engine::OK != nReturn) {
        std::string errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        errorMessage = "Failed to get board from project, error message " + errorMessage;
        QMessageBox::critical(nullptr, QStringLiteral("Scan Image"), errorMessage.c_str(), QStringLiteral("Quit"));
        return;
    }

    const auto& board = vecBoard[0];

    Engine::Device device;
    device.boardId = board.Id;
    dialog.getDeviceInfo(device.name, device.group, device.type);
    device.width  = selectedROI.width  * dResolutionX;
    device.height = selectedROI.height * dResolutionY;

    cv::Point2f ptCtr(selectedROI.x + selectedROI.width / 2, selectedROI.y + selectedROI.height / 2);
    auto ptCtrDisplay = ptCtr;
    auto matImage = pUI->getImage();

    if (bBoardRotated)
        ptCtr.x = matImage.cols - ptCtr.x;
    else
        ptCtr.y = matImage.rows - ptCtr.y; //In cad, up is positive, but in image, down is positive.

    device.x = ptCtr.x * dResolutionX - board.x;
    device.y = ptCtr.y * dResolutionY - board.y;

    nReturn = Engine::CreateDevice(board.Id, device);
    if (Engine::OK != nReturn) {
        std::string errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        errorMessage = "Failed to create device, error message " + errorMessage;
        QMessageBox::critical(nullptr, strTitle, errorMessage.c_str(), QStringLiteral("Quit"));
        return;
    }

    auto vecDeviceWindows = pUI->getDeviceWindows();
    cv::RotatedRect rectDevice(ptCtrDisplay, cv::Size2f(selectedROI.width, selectedROI.height), 0);
    vecDeviceWindows.emplace_back(device.Id, device.name, rectDevice);
    pUI->setDeviceWindows(vecDeviceWindows);
}

void EditCADWidget::on_btnEditDevice_clicked() {
    auto pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_DEVICE);

    QString strTitle(QStringLiteral("编辑元件"));
    auto nReturn = System->showInteractMessage(strTitle, QStringLiteral("请用鼠标左键选择编辑元件"));
    if (nReturn != QDialog::Accepted)
        return;

    auto selectedDevice = pUI->getSelectedDevice();
    Engine::Device device;
    nReturn = Engine::GetDevice(selectedDevice.getId(), device);
    if (Engine::OK != nReturn) {
        std::string errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        errorMessage = "Failed to get device, error message " + errorMessage;
        QMessageBox::critical(nullptr, strTitle, errorMessage.c_str(), QStringLiteral("Quit"));
        return;
    }

    EditDeviceDialog dialog(this);
    dialog.setDeviceInfo(device.name, device.group, device.type);
    dialog.setModal(true);
    nReturn = dialog.exec();
    if (nReturn != QDialog::Accepted)
        return;

    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_SELECT_ROI);
    nReturn = System->showInteractMessage(strTitle, QStringLiteral("请选择新的元件位置"));
    if (nReturn != QDialog::Accepted)
        return;
    
    auto selectedROI = pUI->getSelectedROI();

    Int32 bBoardRotated = 0; Engine::GetParameter("BOARD_ROTATED", bBoardRotated, false);
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    float fCombinedImageScale = 1.f; Engine::GetParameter("ScanImageZoomFactor", fCombinedImageScale, 1.f);

    VisionViewDeviceVector vecVisionViewDevices;
    Engine::BoardVector vecBoard;
    nReturn = Engine::GetAllBoards(vecBoard);
    if (Engine::OK != nReturn) {
        std::string errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        errorMessage = "Failed to get board from project, error message " + errorMessage;
        QMessageBox::critical(nullptr, QStringLiteral("Scan Image"), errorMessage.c_str(), QStringLiteral("Quit"));
        return;
    }
    auto iterBoard = std::find_if(vecBoard.cbegin(), vecBoard.cend(), [device](const Engine::Board &board) { return device.boardId == board.Id; });
    if (iterBoard == vecBoard.cend()) {
        return;
    }
    auto board = *iterBoard;

    dialog.getDeviceInfo(device.name, device.group, device.type);
    device.width  = selectedROI.width  * dResolutionX;
    device.height = selectedROI.height * dResolutionY;
    if (Vision::ToInt32(device.angle) % 90 == 0)
        std::swap(device.width, device.height);
    cv::Point2f ptCtr(selectedROI.x + selectedROI.width / 2, selectedROI.y + selectedROI.height / 2);
    auto ptCtrDisplay = ptCtr;
    auto matImage = pUI->getImage();

    if (bBoardRotated)
        ptCtr.x = matImage.cols - ptCtr.x;
    else
        ptCtr.y = matImage.rows - ptCtr.y; //In cad, up is positive, but in image, down is positive.

    device.x = ptCtr.x * dResolutionX - board.x;
    device.y = ptCtr.y * dResolutionY - board.y;

    nReturn = Engine::UpdateDevice(device);
    if (Engine::OK != nReturn) {
        std::string errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        errorMessage = "Failed to update device, error message " + errorMessage;
        QMessageBox::critical(nullptr, strTitle, errorMessage.c_str(), QStringLiteral("Quit"));
        return;
    }

    auto vecDeviceWindows = pUI->getDeviceWindows();
    auto iterWindow = std::find_if(vecDeviceWindows.begin(), vecDeviceWindows.end(), [device] (const VisionViewDevice& vvDevice){ return device.Id == vvDevice.getId(); });
    if (iterWindow != vecDeviceWindows.end()) {
        cv::RotatedRect rectDevice(ptCtrDisplay, cv::Size2f(selectedROI.width, selectedROI.height), 0);
        *iterWindow = VisionViewDevice(iterWindow->getId(), iterWindow->getName(), rectDevice);
        pUI->setDeviceWindows(vecDeviceWindows);
    }
}

void EditCADWidget::on_btnDeleteDevice_clicked() {
    auto pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_DEVICE);

    QString strTitle(QStringLiteral("删除元件"));
    auto nReturn = System->showInteractMessage(strTitle, QStringLiteral("请用鼠标左键选择要删除元件"));
    if (nReturn != QDialog::Accepted)
        return;

    auto selectedDevice = pUI->getSelectedDevice();
    Engine::Device device;
    nReturn = Engine::GetDevice(selectedDevice.getId(), device);
    if (Engine::OK != nReturn) {
        std::string errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        errorMessage = "Failed to get device, error message " + errorMessage;
        QMessageBox::critical(nullptr, strTitle, errorMessage.c_str(), QStringLiteral("Quit"));
        return;
    }

    nReturn = Engine::DeleteDevice(device.boardId, device.name);
    if (Engine::OK != nReturn) {
        std::string errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        errorMessage = "Failed to delete device, error message " + errorMessage;
        QMessageBox::critical(nullptr, strTitle, errorMessage.c_str(), QStringLiteral("Quit"));
        return;
    }

    auto vecDeviceWindows = pUI->getDeviceWindows();
    auto iterWindow = std::find_if(vecDeviceWindows.begin(), vecDeviceWindows.end(), [device] (const VisionViewDevice& vvDevice){ return device.Id == vvDevice.getId(); });
    if (iterWindow != vecDeviceWindows.end()) {
        vecDeviceWindows.erase(iterWindow);
        pUI->setDeviceWindows(vecDeviceWindows);
    }
}
