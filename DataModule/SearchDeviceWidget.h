#pragma once

#include <QWidget>
#include "ui_SearchDeviceWidget.h"

#include <qstandarditemmodel>
#include <qmap.h>

#include "DataStoreAPI.h"
using namespace NFG::AOI;

using WindowGroupVector = std::vector<Engine::WindowGroup>;

struct DeviceInspWindow
{
    Engine::Device          device;
    Engine::WindowVector    vecUngroupedWindows;
    WindowGroupVector       vecWindowGroup;
    bool                    bInspected = false;
    bool                    bAlignmentPassed = true;
    bool                    bGood = true;
};

using DeviceInspWindowVector = std::vector<DeviceInspWindow>;

class DeviceItemModel : public QStandardItemModel
{
public:
   virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

class SearchDeviceWidget : public QWidget
{
    Q_OBJECT

public:
    SearchDeviceWidget(QWidget *parent = Q_NULLPTR);
    ~SearchDeviceWidget();

private:
    void initUI();

protected slots:
    void onSearchDeviceState(const QVariantList &data);

    void onComBoxBoard(int iState);
    void onComBoxType(int iState);

    void onSearch();

    void slotRowDoubleClicked(const QModelIndex &);

private:
    int _prepareRunData();

    void searchUpdateData(int nBoardId, const QString& szType, const QString& name);    
    DeviceInspWindowVector _search(int nBoardId, const QString& szType, const QString& name);

private:
    Ui::SearchDeviceWidget ui;
    DeviceItemModel m_model;

    Engine::AlignmentVector         m_vecAlignments;
    DeviceInspWindowVector          m_vecDeviceInspWindow;
    std::vector<String>             m_vecDeviceType;
};
