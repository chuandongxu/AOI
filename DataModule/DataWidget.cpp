#include "DataWidget.h"
#include "ImportCADWidget.h"
#include "BoardWidget.h"
#include "ScanImageWidget.h"
#include "FiducialMarkWidget.h"

DataWidget::DataWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    ui.tabWidget->addTab(new ImportCADWidget(), QStringLiteral("导入CAD"));
    ui.tabWidget->addTab(new BoardWidget(), QStringLiteral("电路板"));
    ui.tabWidget->addTab(new ScanImageWidget(), QStringLiteral("扫图"));
    ui.tabWidget->addTab(new FiducialMarkWidget(), QStringLiteral("对位标记"));
}

DataWidget::~DataWidget()
{
}
