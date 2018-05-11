#include "EditInspWindowBaseWidget.h"
#include "InspWindowWidget.h"

EditInspWindowBaseWidget::EditInspWindowBaseWidget(InspWindowWidget *parent) : QWidget(parent), m_pParent(parent)
{
}

void EditInspWindowBaseWidget::setWindowGroup(const Engine::WindowGroup &windowGroup)
{
    m_windowGroup = windowGroup;
}