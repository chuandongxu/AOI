#include "SearchDeviceWidget.h"

#include "../common/SystemData.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../Common/eos.h"

#include "opencv2/highgui.hpp"
#include "../DataModule/DataUtils.h"
#include "../include/constants.h"

using IconTypeMap = std::map < QString, QString >;
using IconTypeMap_value = std::map<QString, QString>::value_type;
const IconTypeMap_value _init_value[] = {
    IconTypeMap_value("BGA", "/image/dataimage/BGA.png")
};
const static IconTypeMap _iconTypeMap(_init_value, _init_value + sizeof(_init_value) / sizeof(_init_value[0]));

QVariant DeviceItemModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{    
    if (!index.isValid())
        return QVariant();

    int col = index.column();
    if (col == 0 && role == Qt::DecorationRole)
    {
        QString filePath = QApplication::applicationDirPath();

        QString szIconType = this->itemData(index).find(0)->toString();       
       
        QString szFileName = "/image/dataimage/00.png";
        if (_iconTypeMap.find(szIconType) != _iconTypeMap.end())
           szFileName = _iconTypeMap.find(szIconType)->second;

        filePath += szFileName;

        return QIcon(filePath);
    }
    //else if (col == 0 && role == Qt::DisplayRole)
    //{
    //    return QVariant();
    //}
    else
    {
        return QStandardItemModel::data(index, role);
    }

    return QVariant();
}

SearchDeviceWidget::SearchDeviceWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    initUI();
}

SearchDeviceWidget::~SearchDeviceWidget()
{
}

void SearchDeviceWidget::initUI()
{
    connect(ui.comboBox_board, SIGNAL(currentIndexChanged(int)), SLOT(onComBoxBoard(int)));
    QStringList ls;
    ls << QStringLiteral("全部板子");
    ui.comboBox_board->addItems(ls);

    connect(ui.comboBox_type, SIGNAL(currentIndexChanged(int)), SLOT(onComBoxType(int)));
    ls.clear();
    ls << QStringLiteral("全部类型");
    ui.comboBox_type->addItems(ls);

    connect(ui.pushButton_search, SIGNAL(clicked()), SLOT(onSearch()));

    ui.tableView_device->setModel(&m_model);
    ls.clear();
    ls << QStringLiteral("图标") << QStringLiteral("名称");
    m_model.setHorizontalHeaderLabels(ls);
    ui.tableView_device->setColumnWidth(0, 30);
    //ui.tableView_device->setColumnWidth(1, 170);
    ui.tableView_device->horizontalHeader()->setStretchLastSection(true);
    ui.tableView_device->horizontalHeader()->setVisible(false);
    ui.tableView_device->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.tableView_device->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.tableView_device->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(ui.tableView_device, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotRowDoubleClicked(const QModelIndex &)));

    QEos::Attach(EVENT_SEARCH_DEVICE_STATE, this, SLOT(onSearchDeviceState(const QVariantList &)));
}

void SearchDeviceWidget::onSearchDeviceState(const QVariantList &data)
{
    int iBoard = data[0].toInt();
    int iEvent = data[1].toInt();

    switch (iEvent)
    {   
    case SEARCH_ALL_DEVICE:
    { 
        if (OK != _prepareRunData()) return;
        searchUpdateData(0, "", "");
    }
    break;
    default:      
        break;
    }
}

void SearchDeviceWidget::onComBoxBoard(int iIndex)
{
    int nIndex = ui.comboBox_board->currentIndex();

    ui.comboBox_type->setCurrentIndex(0);
    ui.lineEdit_search->setText("");

    searchUpdateData(0, "", "");
}

void SearchDeviceWidget::onComBoxType(int iIndex)
{
    int nIndex = ui.comboBox_type->currentIndex();

    QString type = ui.comboBox_type->currentText();
    if (0 == nIndex) type = "";
   
    ui.lineEdit_search->setText("");

    searchUpdateData(0, type, "");
}

void SearchDeviceWidget::onSearch()
{
    int nIndex = ui.comboBox_type->currentIndex();

    QString type = ui.comboBox_type->currentText();
    if (0 == nIndex) type = "";

    QString name = ui.lineEdit_search->text();
    searchUpdateData(0, type, name);
}

void SearchDeviceWidget::slotRowDoubleClicked(const QModelIndex& index)
{
    //QModelIndex index = ui.tableView_objList->currentIndex();
    if (index.isValid())
    {
        int row = index.row();

        QString str = m_model.data(m_model.index(row, 1)).toString();
        QEos::Notify(EVENT_SEARCH_DEVICE_STATE, 0, SEARCH_ONE_DEVICE, str);
    }
}

void SearchDeviceWidget::searchUpdateData(int nBoardId, const QString& szType, const QString& name)
{
    DeviceInspWindowVector inspWinVec = _search(nBoardId, szType, name);
   
    m_model.clear();
    QStringList ls;
    ls << QStringLiteral("图标") << QStringLiteral("名称");
    m_model.setHorizontalHeaderLabels(ls);
    ui.tableView_device->setColumnWidth(0, 30);
    //ui.tableView_device->setColumnWidth(1, 170);
    ui.tableView_device->horizontalHeader()->setStretchLastSection(true);

    for (const auto &deviceInspWin : inspWinVec)
    {
        int nr = m_model.rowCount();
        m_model.insertRow(nr);

        m_model.setData(m_model.index(nr, 0), QStringLiteral("%1").arg(QString::fromStdString(deviceInspWin.device.type)));
        m_model.setData(m_model.index(nr, 1), QStringLiteral("%1").arg(QString::fromStdString(deviceInspWin.device.name)));
    } 
}

DeviceInspWindowVector SearchDeviceWidget::_search(int nBoardId, const QString& szType, const QString& name)
{
    if (name == "")
    {
        if (szType == "")
        {
            return m_vecDeviceInspWindow;
        }
        else
        {
            DeviceInspWindowVector inspWinVec;
            for (const auto &deviceInspWin : m_vecDeviceInspWindow) {
                if (QString::fromStdString(deviceInspWin.device.type) == szType)
                {
                    inspWinVec.push_back(deviceInspWin);
                }
            }
            return inspWinVec;
        }
    }
    else
    {
        DeviceInspWindowVector inspWinVecTmp = _search(nBoardId, szType, "");        

        DeviceInspWindowVector inspWinVec;
        for (const auto &deviceInspWin : inspWinVecTmp) {
            QString deviceName = QString::fromStdString(deviceInspWin.device.name);
            if (deviceName.contains(name))
            {
                inspWinVec.push_back(deviceInspWin);
            }
        }
        return inspWinVec;
    }
}

int SearchDeviceWidget::_prepareRunData()
{
    m_vecAlignments.clear();
    int nResult = Engine::GetAllAlignments(m_vecAlignments);
    if (Engine::OK != nResult) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        QString msg(QStringLiteral("读取定位标记信息失败, 错误消息: "));
        msg + errorMessage.c_str();
        System->showMessage(QStringLiteral("准备自动运行"), msg);
        return NOK;
    }

    Engine::DeviceVector vecDevice;
    nResult = Engine::GetAllDevices(vecDevice);
    if (Engine::OK != nResult) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        QString msg(QStringLiteral("读取元件信息失败, 错误消息: "));
        msg + errorMessage.c_str();
        System->showMessage(QStringLiteral("准备自动运行"), msg);
        return NOK;
    }

    m_vecDeviceType.clear();
    m_vecDeviceInspWindow.clear();
    for (const auto &device : vecDevice) {
        if(!device.type.empty()) m_vecDeviceType.push_back(device.type);

        Engine::WindowVector vecWindows;
        nResult = Engine::GetDeviceWindows(device.Id, vecWindows);
        if (Engine::OK != nResult) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            QString msg(QStringLiteral("读取元件检测框失败, 错误消息: "));
            msg + errorMessage.c_str();
            System->showMessage(QStringLiteral("准备自动运行"), msg);
            return NOK;
        }

        if (vecWindows.empty())
            continue;

        DeviceInspWindow deviceInpWindow;
        deviceInpWindow.bInspected = false;
        deviceInpWindow.device = device;
        nResult = Engine::GetDeviceUngroupedWindows(device.Id, deviceInpWindow.vecUngroupedWindows);
        if (Engine::OK != nResult) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            QString msg(QStringLiteral("读取元件未分组检测框失败, 错误消息: "));
            msg + errorMessage.c_str();
            System->showMessage(QStringLiteral("准备自动运行"), msg);
            return NOK;
        }       

        Int64Vector vecGroupId;
        nResult = Engine::GetDeviceWindowGroups(device.Id, vecGroupId);
        if (Engine::OK != nResult) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            QString msg(QStringLiteral("读取元件检测框组失败, 错误消息: "));
            msg + errorMessage.c_str();
            System->showMessage(QStringLiteral("准备自动运行"), msg);
            return NOK;
        }

        if (!vecGroupId.empty()) {
            for (const auto groupId : vecGroupId) {
                Engine::WindowGroup windowGroup;
                nResult = Engine::GetGroupWindows(groupId, windowGroup);
                if (Engine::OK != nResult) {
                    String errorType, errorMessage;
                    Engine::GetErrorDetail(errorType, errorMessage);
                    QString msg(QStringLiteral("读取元件检测框组失败, 错误消息: "));
                    msg + errorMessage.c_str();
                    System->showMessage(QStringLiteral("准备自动运行"), msg);
                    return NOK;
                }             

                deviceInpWindow.vecWindowGroup.push_back(windowGroup);
            }
        }

        m_vecDeviceInspWindow.push_back(deviceInpWindow);
    }

    std::sort(m_vecDeviceType.begin(), m_vecDeviceType.end());
    m_vecDeviceType.erase(std::unique(m_vecDeviceType.begin(), m_vecDeviceType.end()), m_vecDeviceType.end());

    ui.comboBox_type->clear();
    QStringList ls;
    ls << QStringLiteral("全部类型");  

    for (const auto &type : m_vecDeviceType) {
        ls << QString::fromStdString(type);
    }

    ui.comboBox_type->addItems(ls);

    return OK;
}
