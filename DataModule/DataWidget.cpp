#include "DataWidget.h"
#include "ImportCADWidget.h"

DataWidget::DataWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    ui.tabWidget->addTab(new ImportCADWidget(), QStringLiteral("导入CAD"));
}

DataWidget::~DataWidget()
{
}
