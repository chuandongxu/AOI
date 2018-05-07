#include "DialogCreateGroup.h"

DialogCreateGroup::DialogCreateGroup(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    setFixedSize(size());
}

DialogCreateGroup::~DialogCreateGroup()
{
}

QString DialogCreateGroup::getGroupName() const
{
    return ui.lineEditGroupName->text();
}