#include "TreeWidgetInspWindow.h"
#include "DataStoreAPI.h"
#include "../Common/SystemData.h"

using namespace NFG::AOI;

TreeWidgetInspWindow::TreeWidgetInspWindow(QWidget *parent)
    : QTreeWidget(parent)
{
}

TreeWidgetInspWindow::~TreeWidgetInspWindow()
{
}

void TreeWidgetInspWindow::dropEvent(QDropEvent * event) {
    if (this->selectedItems().size() <= 0)
        return;

    auto pDraggingItem = this->selectedItems()[0];
    if (NULL != pDraggingItem->parent())
        return;

    if (pDraggingItem->type() == TREE_ITEM_GROUP)
        return;

    auto pDropingOnItem = this->itemAt(event->pos());

    if (pDropingOnItem) {
        auto strMyName = pDropingOnItem->text(0);
        if (pDropingOnItem->type() == TREE_ITEM_WINDOW) {
            pDropingOnItem = pDropingOnItem->parent();
            if (NULL == pDropingOnItem)
                return;
        }else if (pDropingOnItem->indexOfChild(pDraggingItem) >= 0)
            return;

        int groupId = pDropingOnItem->data(0, Qt::UserRole).toInt();
        int windowId = pDraggingItem->data(0, Qt::UserRole).toInt();

        auto result = Engine::AddWindowToGroup(windowId, groupId);
        if (result != Engine::OK) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            QString msg(QStringLiteral("添加检测框到组失败, 错误消息: "));
            msg += errorMessage.c_str();
            System->showMessage(QStringLiteral("检测框"), msg);
            return;
        }

        //auto pNewItem = new QTreeWidgetItem(*pDraggingItem);
        //pDropingOnItem->addChild(pNewItem);
        //
        //auto ptrParent = pDraggingItem->parent();
        //if (ptrParent == NULL) {
        //    this->takeTopLevelItem(this->indexOfTopLevelItem(pDraggingItem));
        //}else {            
        //    auto index = ptrParent->indexOfChild(pDraggingItem);
        //    ptrParent->takeChild(index);
        //}

        emit regrouped();
    }
}