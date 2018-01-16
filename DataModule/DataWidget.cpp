#include "DataWidget.h"
#include "ImportCADWidget.h"
#include "ScanImageWidget.h"
#include "FiducialMarkWidget.h"

DataWidget::DataWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    ui.tabWidget->addTab(new ImportCADWidget(), QStringLiteral("Import CAD"));
    ui.tabWidget->addTab(new ScanImageWidget(), QStringLiteral("Scan Image"));
    ui.tabWidget->addTab(new FiducialMarkWidget(), QStringLiteral("Fiducial Mark"));
}

DataWidget::~DataWidget()
{
}
