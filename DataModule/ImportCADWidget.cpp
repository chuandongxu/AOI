#include "ImportCADWidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>

ImportCADWidget::ImportCADWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    for ( int i = CAD_DATA_COLUMNS::BEGIN; i < CAD_DATA_COLUMNS::END; ++ i ) {
        ui.listWidgetOption->addItem ( CAD_DATA_COLUMN_NAMES[i].c_str() );
        ui.listWidgetSelected->addItem ( CAD_DATA_COLUMN_NAMES[i].c_str() );
        _vecCadDataColumns.push_back ( static_cast<CAD_DATA_COLUMNS > ( i ) );
    }
}

ImportCADWidget::~ImportCADWidget()
{
}

void ImportCADWidget::on_btnSelectFile_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setNameFilter(tr("CAD Files (*.cad *.txt)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())  {
        fileNames = dialog.selectedFiles();
    }else
        return;

    _strCADFilePath = fileNames[0];
    ui.lineEditFilePath->setText(fileNames[0]);

    std::ifstream fs(_strCADFilePath.toStdString(), std::ifstream::in);
    if ( ! fs.is_open() ) {
        QMessageBox::critical(nullptr, QStringLiteral("Import CAD"), QStringLiteral("Failed to open file."), QStringLiteral("Quit"));
        return;
    }
    std::string strLine;
    int nLineNumber = 1;
    std::getline ( fs, strLine, '\r' );
    ui.textEditCADPreview->setText ( strLine.c_str() );
}

void ImportCADWidget::on_btnSelectColumn_clicked() {
    auto currentSelect = ui.listWidgetOption->currentRow();
    bool bFound = false;
    for ( const auto &column : _vecCadDataColumns ) {
        if ( column == currentSelect ) {
            bFound = true;
            break;
        }
    }
    if ( ! bFound ) {
        _vecCadDataColumns.push_back ( static_cast<CAD_DATA_COLUMNS > ( currentSelect ) );
        ui.listWidgetSelected->addItem ( CAD_DATA_COLUMN_NAMES[currentSelect].c_str() );
    }else {
        QMessageBox::warning(nullptr, QStringLiteral("Add Data Column"), "Column already added.", QStringLiteral("Quit"));
    }
}

void ImportCADWidget::on_btnMoveUpSelection_clicked() {
    auto currentSelect = ui.listWidgetSelected->currentRow();
    if ( currentSelect > 0 ) {
        std::swap ( _vecCadDataColumns[currentSelect], _vecCadDataColumns[currentSelect - 1] );
        ui.listWidgetSelected->clear();
        for ( const auto &column : _vecCadDataColumns )
            ui.listWidgetSelected->addItem ( CAD_DATA_COLUMN_NAMES[column].c_str() );
    }
}

void ImportCADWidget::on_btnMoveDownSelection_clicked() {
    auto currentSelect = ui.listWidgetSelected->currentRow();
    if ( 0 <= currentSelect && currentSelect < _vecCadDataColumns.size() - 1 ) {
        std::swap ( _vecCadDataColumns[currentSelect], _vecCadDataColumns[currentSelect + 1] );
        ui.listWidgetSelected->clear();
        for ( const auto &column : _vecCadDataColumns )
            ui.listWidgetSelected->addItem ( CAD_DATA_COLUMN_NAMES[column].c_str() );
    }
}

void ImportCADWidget::on_btnDeleteSelection_clicked() {
    auto currentSelect = ui.listWidgetSelected->currentRow();
    if ( currentSelect >= 0 ) {
        _vecCadDataColumns.erase ( _vecCadDataColumns.begin() + currentSelect );
        ui.listWidgetSelected->takeItem ( currentSelect );
    }
}

void ImportCADWidget::on_btnImportCAD_clicked() {
    VectorOfCadData vecCadData;
    std::string strErrorMsg;
    int nResult = DataUtils::parseCAD ( _strCADFilePath.toStdString(), _vecCadDataColumns, vecCadData, strErrorMsg );
    if ( nResult != 0 ) {
        QMessageBox::critical(nullptr, QStringLiteral("Import CAD"), strErrorMsg.c_str(), QStringLiteral("Quit"));
        return;
    }

    bool bCadWithWidthLength = false;
    for ( const auto &column : _vecCadDataColumns ) {
        if ( column == CAD_DATA_COLUMNS::WIDTH ) {
            bCadWithWidthLength = true;
            break;
        }
    }

    MapGroupPackageSize mapGroupPackageSize;
    if ( ! bCadWithWidthLength ) {
        nResult = DataUtils::readPackageSize ( "./data/PackageSize.csv", mapGroupPackageSize, strErrorMsg );
        if ( nResult != 0 ) {
            QMessageBox::critical(nullptr, QStringLiteral("Import CAD"), strErrorMsg.c_str(), QStringLiteral("Quit"));
            return;
        }
    }

    for ( auto &cadData : vecCadData ) {
        if ( ! bCadWithWidthLength ) {
            std::string strOutputPackage, strOutputType;
            PackageSize packageSize;
            if ( DataUtils::decodePackageAndType ( cadData.type, cadData.group, strOutputPackage, strOutputType ) == 0 ) {
                if ( mapGroupPackageSize.find(strOutputPackage) != mapGroupPackageSize.end() ) {
                    auto mapTypePackageSize = mapGroupPackageSize[strOutputPackage];
                    if ( mapTypePackageSize.find(strOutputType) != mapTypePackageSize.end() ) {
                        packageSize = mapTypePackageSize[strOutputType];
                    }
                }
            }
        }
    }
}