#include "SpecAndResultWidget.h"

SpecAndResultWidget::SpecAndResultWidget(QWidget *parent, float fMin, float fMax, int nPrecesion)
    : QTableWidget(parent)
{
    ui.setupUi(this);

    m_pEditSpec = std::make_unique<QLineEdit>(this);
    m_pEditSpec->setValidator(new QDoubleValidator(fMin, fMax, nPrecesion, m_pEditSpec.get()));
    this->setCellWidget(0, 0, m_pEditSpec.get());

    m_pEditResult = std::make_unique<QLineEdit>(this);
    m_pEditResult->setDisabled(true);
    this->setCellWidget(0, 1, m_pEditResult.get());
}

SpecAndResultWidget::~SpecAndResultWidget() {
}

void SpecAndResultWidget::setSpec(float fSpec) {
    m_pEditSpec->setText(QString::number(fSpec));
}

float SpecAndResultWidget::getSpec() const {
    return m_pEditSpec->text().toFloat();
}

void SpecAndResultWidget::setResult(float result) {
    m_pEditResult->setText(QString::number(result));
}

void SpecAndResultWidget::clearResult() {
    m_pEditResult->setText("");
}