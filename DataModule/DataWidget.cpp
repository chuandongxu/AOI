#include "DataWidget.h"
#include "ImportCADWidget.h"
#include "EditCADWidget.h"
#include "BoardWidget.h"
#include "ScanImageWidget.h"
#include "FiducialMarkWidget.h"

DataWidget::DataWidget(DataCtrl *pDataCtrl, QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    ui.tabWidget->addTab(new ImportCADWidget(), QStringLiteral("导入CAD"));
    ui.tabWidget->addTab(new EditCADWidget(pDataCtrl, this), QStringLiteral("编辑CAD"));
    ui.tabWidget->addTab(new BoardWidget(), QStringLiteral("电路板"));
    ui.tabWidget->addTab(new ScanImageWidget(pDataCtrl), QStringLiteral("扫图"));
    ui.tabWidget->addTab(new FiducialMarkWidget(pDataCtrl, this), QStringLiteral("对位标记"));
}

DataWidget::~DataWidget()
{
}
