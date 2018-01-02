#include "QBottomWidget.h"
#include "../Common/SystemData.h"
#include "../include/IdDefine.h"
#include "../Common/eos.h"

QBottomWidget::QBottomWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	QErrorModel * model = System->getErrorModel();
	if (!model)return;

	ui.treeView->setModel(model);

	QHeaderView * header = ui.treeView->header();
	header->resizeSection(0, 50);
	header->resizeSection(1, 130);
	header->setStretchLastSection(true);

	connect(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)),
		this, SLOT(onDataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)));
}

QBottomWidget::~QBottomWidget()
{
}

void QBottomWidget::onDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> & roles)
{
	QAbstractItemModel * model = ui.treeView->model();
	if (!model)return;

	int row = model->rowCount();
	ui.treeView->scrollTo(model->index(row - 1, 1), QAbstractItemView::PositionAtBottom);

}
