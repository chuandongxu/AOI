#pragma once

#include "EditInspWindowBaseWidget.h"
#include "ui_InspWindowWidget.h"
#include "DataStoreAPI.h"
#include "QColorWeight.h"
#include <map>

using namespace NFG::AOI;

enum class INSP_WIDGET_INDEX {
    UNDEFINED = -1,
    FIND_LINE,
    INSP_HOLE,
    CALIPER_CIRCLE,
    ALIGNMENT,
    HEIGHT_DETECT,
    HEIGHT_GLOBAL_BASE,
    INSP_POLARITY,
    INSP_CONTOUR,
    INSP_CHIP,
    INSP_BRIDGE,
    INSP_LEAD,
    OCV,
    INSP_3D_SOLDER,
    SIZE,
};

static const int DATA_COLUMN = 1;

const int INSP_WIDGET_INDEX_SIZE = static_cast<int>(INSP_WIDGET_INDEX::SIZE);
using MapIdWindow = std::map<Int64, Engine::Window>;

class InspMaskEditorWidget;
class InspWindowWidget : public QWidget
{
    Q_OBJECT

public:
    InspWindowWidget(QWidget *parent, QColorWeight *pColorWidget);
    ~InspWindowWidget();

    void setCurrentIndex(int index);
    void updateInspWindowList();
    void refreshAllDeviceWindows();
    void showInspDetectObjs();
    int getSelectedLighting() const;
    QColorWeight *getColorWidget() const;

protected:
    virtual void showEvent(QShowEvent *event) override;
    void _showWidgets();
    void _hideWidgets();
    void _tryInspHeight();
    void _tryInspectPolarity();

private slots:
    void on_btnAddWindow_clicked();
    void on_btnRemoveWindow_clicked();
    void on_btnCreateGroup_clicked();
    void on_btnCopyToAll_clicked();
    void on_btnCopyAsMirror_clicked();
    void on_btnEditMask_clicked();
    void on_btnTryInsp_clicked();
    void on_btnConfirmWindow_clicked();
    void on_regrouped();

    void onInspWindowState(const QVariantList &data);
    void onSelectedWindowChanged();
    void on_comboBoxLighting_indexChanged(int index);

private:
    Ui::InspWindowWidget        ui;
    QColorWeight               *m_pColorWidget;
    std::unique_ptr<QComboBox>  m_pComboBoxLighting;
    InspWindowBaseWidgetPtr     m_arrInspWindowWidget[INSP_WIDGET_INDEX_SIZE];
    std::vector<Engine::WindowGroup> m_vecWindowGroup;
    MapIdWindow                 m_mapIdWindow;
    OPERATION                   m_enOperation;
    INSP_WIDGET_INDEX           m_enCurrentInspWidget = INSP_WIDGET_INDEX::UNDEFINED;
    int                         m_nBigImgWidth;
    int                         m_nBigImgHeight;
    std::unique_ptr<InspMaskEditorWidget> m_pMaskEditorWidget;
};
