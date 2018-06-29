#include "ImportCADWidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include <set>
#include "DataStoreAPI.h"
#include "constants.h"
#include "SystemData.h"
#include <map>
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/IVisionUI.h"

using namespace NFG::AOI;

ImportCADWidget::ImportCADWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    for (int i = CAD_DATA_COLUMNS::BEGIN; i < CAD_DATA_COLUMNS::END; ++ i) {
        ui.listWidgetOption->addItem(CAD_DATA_COLUMN_NAMES[i].c_str());
        ui.listWidgetSelected->addItem(CAD_DATA_COLUMN_NAMES[i].c_str());
        _vecCadDataColumns.push_back(static_cast<CAD_DATA_COLUMNS>(i));
    }
}

ImportCADWidget::~ImportCADWidget() {
}

void ImportCADWidget::on_btnSelectFile_clicked() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setNameFilter(tr("CAD Files (*.cad *.txt)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();
    }
    else
        return;

    _strCADFilePath = fileNames[0];
    ui.lineEditFilePath->setText(fileNames[0]);

    std::ifstream fs(_strCADFilePath.toStdString(), std::ifstream::in);
    if (! fs.is_open()) {
        QMessageBox::critical(nullptr, QStringLiteral("Import CAD"), QStringLiteral("Failed to open file."), QStringLiteral("Quit"));
        return;
    }
    std::string strLine;
    int nLineNumber = 1;
    std::getline(fs, strLine, '\r');
    ui.textEditCADPreview->setText(strLine.c_str());
}

void ImportCADWidget::on_btnSelectColumn_clicked() {
    auto currentSelect = ui.listWidgetOption->currentRow();
    bool bFound = false;
    for (const auto &column : _vecCadDataColumns) {
        if (column == currentSelect) {
            bFound = true;
            break;
        }
    }
    if (! bFound) {
        _vecCadDataColumns.push_back(static_cast<CAD_DATA_COLUMNS> (currentSelect));
        ui.listWidgetSelected->addItem(CAD_DATA_COLUMN_NAMES[currentSelect].c_str());
    }
    else {
        QMessageBox::warning(nullptr, QStringLiteral("Add Data Column"), "Column already added.", QStringLiteral("Quit"));
    }
}

void ImportCADWidget::on_btnMoveUpSelection_clicked() {
    auto currentSelect = ui.listWidgetSelected->currentRow();
    if (currentSelect > 0) {
        std::swap(_vecCadDataColumns[currentSelect], _vecCadDataColumns[currentSelect - 1]);
        ui.listWidgetSelected->clear();
        for (const auto &column : _vecCadDataColumns)
            ui.listWidgetSelected->addItem(CAD_DATA_COLUMN_NAMES[column].c_str());
    }
}

void ImportCADWidget::on_btnMoveDownSelection_clicked() {
    auto currentSelect = ui.listWidgetSelected->currentRow();
    if (0 <= currentSelect && currentSelect < _vecCadDataColumns.size() - 1) {
        std::swap(_vecCadDataColumns[currentSelect], _vecCadDataColumns[currentSelect + 1]);
        ui.listWidgetSelected->clear();
        for (const auto &column : _vecCadDataColumns)
            ui.listWidgetSelected->addItem(CAD_DATA_COLUMN_NAMES[column].c_str());
    }
}

void ImportCADWidget::on_btnDeleteSelection_clicked() {
    auto currentSelect = ui.listWidgetSelected->currentRow();
    if (currentSelect >= 0) {
        _vecCadDataColumns.erase(_vecCadDataColumns.begin() + currentSelect);
        ui.listWidgetSelected->takeItem(currentSelect);
    }
}

void ImportCADWidget::on_btnImportCAD_clicked() {
    VectorOfCadData vecCadData;
    std::string strErrorMsg;
    int nResult = DataUtils::parseCAD(_strCADFilePath.toStdString(), _vecCadDataColumns, vecCadData, strErrorMsg);
    if (nResult != 0) {
        QMessageBox::critical(nullptr, QStringLiteral("Import CAD"), strErrorMsg.c_str(), QStringLiteral("Quit"));
        return;
    }

    bool bCadWithWidthLength = false;
    for (const auto &column : _vecCadDataColumns) {
        if (column == CAD_DATA_COLUMNS::WIDTH) {
            bCadWithWidthLength = true;
            break;
        }
    }

    MapGroupPackageSize mapGroupPackageSize;
    if (! bCadWithWidthLength) {
        nResult = DataUtils::readPackageSize("./data/PackageSize.csv", mapGroupPackageSize, strErrorMsg);
        if (nResult != 0) {
            QMessageBox::critical(nullptr, QStringLiteral("Import CAD"), strErrorMsg.c_str(), QStringLiteral("Quit"));
            return;
        }
    }

    CAD_DATA_UNIT enDataUnit = static_cast<CAD_DATA_UNIT> (ui.cbSizeUnit->currentIndex());

    std::map<int, Engine::DeviceVector> mapVecDevice;
    for (auto &cadData : vecCadData) {
        if (! bCadWithWidthLength) {
            std::string strOutputPackage, strOutputType;
            PackageSize packageSize(5.f, 5.f);
            if (DataUtils::decodePackageAndType(cadData.type, cadData.group, strOutputPackage, strOutputType) == 0) {
                if (mapGroupPackageSize.find(strOutputPackage) != mapGroupPackageSize.end()) {
                    auto mapTypePackageSize = mapGroupPackageSize[strOutputPackage];
                    if (mapTypePackageSize.find(strOutputType) != mapTypePackageSize.end()) {
                        packageSize = mapTypePackageSize[strOutputType];
                    }
                }
            }
            cadData.width = packageSize.width  * MM_TO_UM;
            cadData.length = packageSize.length * MM_TO_UM;
        }

        Engine::Device device;

        device.name = cadData.name;
        device.boardId = cadData.boardNo;
        if (bCadWithWidthLength) {
            device.width = DataUtils::toUm(cadData.width, enDataUnit);
            device.height = DataUtils::toUm(cadData.length, enDataUnit);
        }
        else {
            device.width = cadData.width;
            device.height = cadData.length;
        }

        device.x = DataUtils::toUm(cadData.x, enDataUnit);
        device.y = DataUtils::toUm(cadData.y, enDataUnit);

        device.isBottom = cadData.isBottom;
        device.pinCount = cadData.pinCount;
        device.group = cadData.group;
        device.type = cadData.type;
        device.angle = cadData.angle;
        mapVecDevice[device.boardId].push_back(device);
    }

    String errorType, errorMessage;
    int boardIndex = 1;
    for (auto iter = mapVecDevice.begin(); iter != mapVecDevice.end(); ++iter) {
        Engine::Board board;
        board.name = std::to_string(boardIndex);
        board.abbr = std::to_string(boardIndex);
        auto nResult = Engine::CreateBoard(board);
        if (nResult != Engine::OK) {
            Engine::GetErrorDetail(errorType, errorMessage);
            errorMessage = "Failed to create board, error message " + errorMessage;
            QMessageBox::critical(nullptr, QStringLiteral("Import CAD"), errorMessage.c_str(), QStringLiteral("Quit"));
            return;
        }

        nResult = Engine::CreateDevice(board.Id, iter->second);
        if (nResult != Engine::OK) {
            Engine::GetErrorDetail(errorType, errorMessage);
            errorMessage = "Failed to create devices, error message " + errorMessage;
            QMessageBox::critical(nullptr, QStringLiteral("Import CAD"), errorMessage.c_str(), QStringLiteral("Quit"));
            return;
        }
        ++ boardIndex;
    }
    QMessageBox::information ( nullptr, QStringLiteral("Import CAD"), QStringLiteral("Import CAD Success"), QStringLiteral("Quit") );
}