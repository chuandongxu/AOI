#include "InspWindowSelectDialog.h"
#include <QMessageBox>

InspWindowSelectDialog::InspWindowSelectDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    // remove question mark from the title bar
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    m_enInspWidgetIndex = INSP_WIDGET_INDEX::UNDEFINED;
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(on_accept()));

    setFixedSize(size());
}

InspWindowSelectDialog::~InspWindowSelectDialog()
{
}

void InspWindowSelectDialog::on_btnFindLineWindow_clicked()
{
    m_enInspWidgetIndex = INSP_WIDGET_INDEX::FIND_LINE;
}

void InspWindowSelectDialog::on_btnCaliperCircleWindow_clicked()
{
	m_enInspWidgetIndex = INSP_WIDGET_INDEX::CALIPER_CIRCLE;
}

void InspWindowSelectDialog::on_btnVoidInspWindow_clicked()
{
    m_enInspWidgetIndex = INSP_WIDGET_INDEX::INSP_HOLE;
}

void InspWindowSelectDialog::on_btnAlignmentWindow_clicked()
{
	m_enInspWidgetIndex = INSP_WIDGET_INDEX::ALIGNMENT;
}

void InspWindowSelectDialog::on_btnHeightDetectWindow_clicked()
{
	m_enInspWidgetIndex = INSP_WIDGET_INDEX::HEIGHT_DETECT;
}

void InspWindowSelectDialog::on_btnInspPolarityWindow_clicked()
{
    m_enInspWidgetIndex = INSP_WIDGET_INDEX::INSP_POLARITY;
}

void InspWindowSelectDialog::on_btnInspContourWindow_clicked()
{
    m_enInspWidgetIndex = INSP_WIDGET_INDEX::INSP_CONTOUR;
}

void InspWindowSelectDialog::on_btnInspChipWindow_clicked()
{
    m_enInspWidgetIndex = INSP_WIDGET_INDEX::INSP_CHIP;
}

void InspWindowSelectDialog::on_btnInspLeadWindow_clicked()
{
    m_enInspWidgetIndex = INSP_WIDGET_INDEX::INSP_LEAD;
}

void InspWindowSelectDialog::on_btnInspBridgeWindow_clicked()
{
    m_enInspWidgetIndex = INSP_WIDGET_INDEX::INSP_BRIDGE;
}

void InspWindowSelectDialog::on_accept()
{
    if ( INSP_WIDGET_INDEX::UNDEFINED == m_enInspWidgetIndex ) {
        QMessageBox::critical ( this, QStringLiteral("Select Insp Window"), QStringLiteral("Please select one inspection window first."), QStringLiteral("Quit") );
        return;
    }
    accept();
}