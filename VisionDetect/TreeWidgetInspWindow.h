#pragma once

#include <QTreeWidget>
#include <QDropEvent>
#include <QDrag>

const int TREE_ITEM_GROUP = 2000;
const int TREE_ITEM_WINDOW = 2001;

class TreeWidgetInspWindow : public QTreeWidget
{
    Q_OBJECT

public:
    TreeWidgetInspWindow(QWidget *parent);
    ~TreeWidgetInspWindow();

signals:
    void regrouped();

protected:
    void dropEvent(QDropEvent * event ) override;
};
