#include "SearchDeviceWidget.h"

#include "../common/SystemData.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../include/IVisionUI.h"
#include "../Common/eos.h"
#include "../Common/CommonFunc.h"

#include "opencv2/highgui.hpp"
#include "../DataModule/DataUtils.h"
#include "../include/constants.h"

using IconTypeMap = std::map < QString, QString >;
using IconTypeMap_value = std::map<QString, QString>::value_type;
const IconTypeMap_value _init_value[] = {
    IconTypeMap_value("MQFP44", "/image/dataimage/QFN.png")
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

static const char *WINDOW_USAGE_NAME[] {
    "Alignment",
    "Height Detect Base",
    "Height Detect",
    "Inspect Lead",
    "Inspect Chip",
    "Inspect Contour",
    "Inspect Hole",
    "Find Line",
    "Find Circle",
    "Inspect Polarity",
    "Inspect Polarity Ref",
    "Inspect Bridge",
    "Ocv",
    "Height Base Global",
    "Inspect 3D Solder"
};

static_assert(static_cast<int>(Engine::Window::Usage::COUNT) == sizeof(WINDOW_USAGE_NAME) / sizeof(WINDOW_USAGE_NAME[0]), "The window usage size is not correct");

SearchDeviceWidget::SearchDeviceWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    initUI();
}

SearchDeviceWidget::~SearchDeviceWidget()
{
}

QString SearchDeviceWidget::getDeviceType(long deviceID) const
{
    for (const auto &device : m_vecDevice) {
        if (device.Id == deviceID)
        {
            return QString::fromStdString(device.type);
        }
    }

    return "";
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

    //Device Type list
    ui.tableView_type->setModel(&m_modelType);
    ls.clear();
    ls << QStringLiteral("图标") << QStringLiteral("类型");
    m_modelType.setHorizontalHeaderLabels(ls);
    ui.tableView_type->setColumnWidth(0, 30);
    //ui.tableView_device->setColumnWidth(1, 170);
    ui.tableView_type->horizontalHeader()->setStretchLastSection(true);
    ui.tableView_type->horizontalHeader()->setVisible(false);
    ui.tableView_type->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.tableView_type->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.tableView_type->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.tableView_type->verticalHeader()->hide();
    connect(ui.tableView_type, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotTypeRowDoubleClicked(const QModelIndex &)));

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

void SearchDeviceWidget::slotTypeRowDoubleClicked(const QModelIndex& index)
{
    //QModelIndex index = ui.tableView_objList->currentIndex();
    if (index.isValid())
    {
        int row = index.row();

        QString type = m_modelType.data(m_modelType.index(row, 1)).toString();  
        ui.comboBox_type->setCurrentText(type);
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

void SearchDeviceWidget::searchTypeUpdateData(int nBoardId)
{
    m_modelType.clear();
    QStringList ls;
    ls << QStringLiteral("图标") << QStringLiteral("类型");
    m_modelType.setHorizontalHeaderLabels(ls);
    ui.tableView_type->setColumnWidth(0, 30);
    //ui.tableView_device->setColumnWidth(1, 170);
    ui.tableView_type->horizontalHeader()->setStretchLastSection(true);

    for (const auto &type : m_vecDeviceType) {     
        int nr = m_modelType.rowCount();
        m_modelType.insertRow(nr);

        m_modelType.setData(m_modelType.index(nr, 0), QStringLiteral("%1").arg(QString::fromStdString(type)));
        m_modelType.setData(m_modelType.index(nr, 1), QStringLiteral("%1").arg(QString::fromStdString(type)));
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

    m_vecDevice.clear();
    nResult = Engine::GetAllDevices(m_vecDevice);
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
    for (const auto &device : m_vecDevice) {
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

        //if (vecWindows.empty())
        //    continue;

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

    searchTypeUpdateData(0);

    return OK;
}

bool SearchDeviceWidget::copyDeviceWindow(long srcID, long destID)
{
    if (srcID == destID) return false;

    auto deviceId = srcID;
    if (deviceId <= 0)
        return false;

    Engine::DeviceVector vecDevices;
    QString strTitle(QStringLiteral("复制检测框"));
    auto result = Engine::GetAllDevices(vecDevices);
    if (Engine::OK != result) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        QString strMsg(QStringLiteral("读取元件信息失败, 错误消息: "));
        strMsg += errorMessage.c_str();
        System->showMessage(strTitle, strMsg);
        return false;
    }

    auto iterDevice = std::find_if(vecDevices.begin(), vecDevices.end(), [deviceId](const Engine::Device &device) { return device.Id == deviceId; });
    if (vecDevices.end() == iterDevice) {
        QString strMsg(QStringLiteral("查询选择的元件失败."));
        System->showMessage(strTitle, strMsg);
        return false;
    }
    auto currentDevice = *iterDevice;

    Engine::WindowVector vecCurrentDeviceWindows;
    result = Engine::GetDeviceUngroupedWindows(deviceId, vecCurrentDeviceWindows);
    if (result != Engine::OK) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        QString msg(QStringLiteral("Failed to get inspect windows from database, error message "));
        msg += errorMessage.c_str();
        System->showMessage(QStringLiteral("检测框"), msg);
        return false;
    }

    Int64Vector vecGroupId;
    result = Engine::GetDeviceWindowGroups(deviceId, vecGroupId);
    if (result != Engine::OK) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        QString msg(QStringLiteral("Failed to get device groups from database, error message "));
        msg += errorMessage.c_str();
        System->showMessage(QStringLiteral("检测框"), msg);
        return false;
    }

    std::vector<Engine::WindowGroup> vecWindowGroup;
    for (const auto groupId : vecGroupId) {
        Engine::WindowGroup windowGroup;
        auto result = Engine::GetGroupWindows(groupId, windowGroup);
        if (result != Engine::OK) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            QString msg(QStringLiteral("Failed to get group windows from database, error message "));
            msg += errorMessage.c_str();
            System->showMessage(QStringLiteral("检测框"), msg);
            return false;
        }
        vecWindowGroup.push_back(windowGroup);
    }

    deviceId = destID;
    if (deviceId <= 0)
        return false;

    iterDevice = std::find_if(vecDevices.begin(), vecDevices.end(), [deviceId](const Engine::Device &device) { return device.Id == deviceId; });
    if (vecDevices.end() == iterDevice) {
        QString strMsg(QStringLiteral("查询选择的元件失败."));
        System->showMessage(strTitle, strMsg);
        return false;
    }
    auto destDevice = *iterDevice;

    Engine::WindowVector vecWindows;
    Engine::GetDeviceWindows(destDevice.Id, vecWindows);
    if (!vecWindows.empty())
    {
        QString strMsg(QStringLiteral("目标元件元件有检测框."));
        System->showMessage(strTitle, strMsg);
        return false;
    }

    auto offsetX = destDevice.x - currentDevice.x;
    auto offsetY = destDevice.y - currentDevice.y;
    for (const auto &win : vecCurrentDeviceWindows) {
        Engine::Window window = win;
        window.deviceId = destDevice.Id;
        window.x += offsetX;
        window.y += offsetY;
        char windowName[100];
        _snprintf(windowName, sizeof(windowName), "%s [%d, %d] @ %s", WINDOW_USAGE_NAME[Vision::ToInt32(window.usage)], Vision::ToInt32(window.x), Vision::ToInt32(window.y), destDevice.name.c_str());
        window.name = windowName;

        auto result = Engine::CreateWindow(window, true); // true means copy mode.
        if (result != Engine::OK) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            QString strMsg(QStringLiteral("创建检测框失败, 错误消息: "));
            strMsg += errorMessage.c_str();
            System->showMessage(strTitle, strMsg);
            return false;
        }
    }

    for (auto windowGroup : vecWindowGroup) {
        for (auto &window : windowGroup.vecWindows) {
            window.deviceId = destDevice.Id;
            window.x += offsetX;
            window.y += offsetY;
            char windowName[100];
            _snprintf(windowName, sizeof(windowName), "%s [%d, %d] @ %s", WINDOW_USAGE_NAME[Vision::ToInt32(window.usage)], Vision::ToInt32(window.x), Vision::ToInt32(window.y), destDevice.name.c_str());
            window.name = windowName;
            auto result = Engine::CreateWindow(window, true); // true means copy mode.
            if (result != Engine::OK) {
                String errorType, errorMessage;
                Engine::GetErrorDetail(errorType, errorMessage);
                QString strMsg(QStringLiteral("创建检测框失败, 错误消息: "));
                strMsg += errorMessage.c_str();
                System->showMessage(strTitle, strMsg);
                return false;
            }
        }
        windowGroup.deviceId = destDevice.Id;
        int nIndex = windowGroup.name.find_first_of('@');
        if (nIndex >= 0)
        {
            char groupName[100];
            _snprintf(groupName, sizeof(groupName), "%s @ %s", windowGroup.name.substr(0, nIndex - 1).c_str(), destDevice.name.c_str());
            windowGroup.name = groupName;
        }
        else
        {
            char groupName[100];
            _snprintf(groupName, sizeof(groupName), "%s @ %s", windowGroup.name.c_str(), destDevice.name.c_str());
            windowGroup.name = groupName;
        }
        auto result = Engine::CreateWindowGroup(windowGroup);
        if (Engine::OK != result) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            QString strMsg(QStringLiteral("创建检测框组失败, 错误消息: "));
            strMsg += errorMessage.c_str();
            System->showMessage(strTitle, strMsg);
            return false;
        }
    }

    return true;
}

bool SearchDeviceWidget::copyDeviceWindowAsMirror(long srcID, bool bHorizontal, QVector<int>& winIDs, QVector<int>& groupIDs)
{
    auto deviceId = srcID;
    if (deviceId <= 0)
        return false;

    Engine::DeviceVector vecDevices;
    QString strTitle(QStringLiteral("复制镜像检测框"));
    auto result = Engine::GetAllDevices(vecDevices);
    if (Engine::OK != result) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        QString strMsg(QStringLiteral("读取元件信息失败, 错误消息: "));
        strMsg += errorMessage.c_str();
        System->showMessage(strTitle, strMsg);
        return false;
    }

    auto iterDevice = std::find_if(vecDevices.begin(), vecDevices.end(), [deviceId](const Engine::Device &device) { return device.Id == deviceId; });
    if (vecDevices.end() == iterDevice) {
        QString strMsg(QStringLiteral("查询选择的元件失败."));
        System->showMessage(strTitle, strMsg);
        return false;
    }
    auto currentDevice = *iterDevice;

    Engine::WindowVector vecCurrentDeviceWindows;
    result = Engine::GetDeviceUngroupedWindows(deviceId, vecCurrentDeviceWindows);
    if (result != Engine::OK) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        QString msg(QStringLiteral("Failed to get inspect windows from database, error message "));
        msg += errorMessage.c_str();
        System->showMessage(QStringLiteral("检测框"), msg);
        return false;
    }

    Int64Vector vecGroupId;
    result = Engine::GetDeviceWindowGroups(deviceId, vecGroupId);
    if (result != Engine::OK) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        QString msg(QStringLiteral("Failed to get device groups from database, error message "));
        msg += errorMessage.c_str();
        System->showMessage(QStringLiteral("检测框"), msg);
        return false;
    }

    std::vector<Engine::WindowGroup> vecWindowGroup;
    for (const auto groupId : vecGroupId) {
        if (groupIDs.indexOf(groupId) < 0) continue;

        Engine::WindowGroup windowGroup;
        auto result = Engine::GetGroupWindows(groupId, windowGroup);
        if (result != Engine::OK) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            QString msg(QStringLiteral("Failed to get group windows from database, error message "));
            msg += errorMessage.c_str();
            System->showMessage(QStringLiteral("检测框"), msg);
            return false;
        }
        vecWindowGroup.push_back(windowGroup);
    }

    auto destDevice = currentDevice;

    auto offsetX = bHorizontal ? 2 : 0;
    auto offsetY = bHorizontal ? 0 : 2;

    Engine::BoardVector vecBoard;
    result = Engine::GetAllBoards(vecBoard);
    if (Engine::OK != result) {
        std::string errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        errorMessage = "Failed to get board from project, error message " + errorMessage;
        System->showMessage(strTitle, errorMessage.c_str());
        return false;
    }

    auto boardId = currentDevice.boardId;
    auto iterBoard = std::find_if(vecBoard.begin(), vecBoard.end(), [boardId](const Engine::Board &board) { return board.Id == boardId; });
    if (vecBoard.end() == iterBoard) {
        QString strMsg(QStringLiteral("查询选择的电路板失败."));
        System->showMessage(strTitle, strMsg);
        return false;
    }
    auto currentBoard = *iterBoard;

    for (const auto &win : vecCurrentDeviceWindows) {
        if (winIDs.indexOf(win.Id) < 0) continue;

        Engine::Window window = win;
        window.deviceId = destDevice.Id;

        window.x += ((currentBoard.x + currentDevice.x ) - window.x) * offsetX;
        window.y += ((currentBoard.y + currentDevice.y ) - window.y) * offsetY;

        char windowName[100];
        _snprintf(windowName, sizeof(windowName), "%s [%d, %d] @ %s", WINDOW_USAGE_NAME[Vision::ToInt32(window.usage)], Vision::ToInt32(window.x), Vision::ToInt32(window.y), destDevice.name.c_str());
        window.name = windowName;

        auto result = Engine::CreateWindow(window, true); // true means copy mode.
        if (result != Engine::OK) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            QString strMsg(QStringLiteral("创建检测框失败, 错误消息: "));
            strMsg += errorMessage.c_str();
            System->showMessage(strTitle, strMsg);
            return false;
        }
    }

    for (auto windowGroup : vecWindowGroup) {
        for (auto &window : windowGroup.vecWindows) {
            window.deviceId = destDevice.Id;

            window.x += ((currentBoard.x + currentDevice.x) - window.x) * offsetX;
            window.y += ((currentBoard.y + currentDevice.y) - window.y) * offsetY;

            char windowName[100];
            _snprintf(windowName, sizeof(windowName), "%s [%d, %d] @ %s", WINDOW_USAGE_NAME[Vision::ToInt32(window.usage)], Vision::ToInt32(window.x), Vision::ToInt32(window.y), destDevice.name.c_str());
            window.name = windowName;

            auto result = Engine::CreateWindow(window, true); // true means copy mode.
            if (result != Engine::OK) {
                String errorType, errorMessage;
                Engine::GetErrorDetail(errorType, errorMessage);
                QString strMsg(QStringLiteral("创建检测框失败, 错误消息: "));
                strMsg += errorMessage.c_str();
                System->showMessage(strTitle, strMsg);
                return false;
            }
        }
        windowGroup.deviceId = destDevice.Id;
        int nIndex = windowGroup.name.find_first_of('@');
        if (nIndex >= 0)
        {
            char groupName[100];
            _snprintf(groupName, sizeof(groupName), "%s @ %s", windowGroup.name.substr(0, nIndex - 1).c_str(), destDevice.name.c_str());
            windowGroup.name = groupName;
        }
        else
        {
            char groupName[100];
            _snprintf(groupName, sizeof(groupName), "%s @ %s", windowGroup.name.c_str(), destDevice.name.c_str());
            windowGroup.name = groupName;
        }

        auto result = Engine::CreateWindowGroup(windowGroup);
        if (Engine::OK != result) {
            String errorType, errorMessage;
            Engine::GetErrorDetail(errorType, errorMessage);
            QString strMsg(QStringLiteral("创建检测框组失败, 错误消息: "));
            strMsg += errorMessage.c_str();
            System->showMessage(strTitle, strMsg);
            return false;
        }
    }

    return true;
}

cv::Rect SearchDeviceWidget::_calcRectROI(Engine::Window& window)
{
    auto dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    auto dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();
    auto dCombinedImageScale = System->getParam("scan_image_ZoomFactor").toDouble();

    cv::Rect rectROI;
    rectROI.width = window.width / dResolutionX;
    rectROI.height = window.height / dResolutionY;

    cv::Point2f ptWindowCtr;
    auto pUI = getModule<IVisionUI>(UI_MODEL);
    auto matBigImage = pUI->getImage();
    int nBigImgWidth = matBigImage.cols / dCombinedImageScale;
    int nBigImgHeight = matBigImage.rows / dCombinedImageScale;
    if (bBoardRotated) {
        ptWindowCtr.x = nBigImgWidth - window.x / dResolutionX;
        ptWindowCtr.y = window.y / dResolutionY;
    }
    else {
        ptWindowCtr.x = window.x / dResolutionX;
        ptWindowCtr.y = nBigImgHeight - window.y / dResolutionY;
    }

    rectROI.x = ptWindowCtr.x - rectROI.width / 2.f;
    rectROI.y = ptWindowCtr.y - rectROI.height / 2.f;   

    return rectROI;
}

bool SearchDeviceWidget::_learnTemplate(cv::Rect& rectROI, int &recordId) {
    Vision::PR_LRN_TEMPLATE_CMD stCmd;
    Vision::PR_LRN_TEMPLATE_RPY stRpy;

    stCmd.enAlgorithm = Vision::PR_MATCH_TMPL_ALGORITHM::SQUARE_DIFF;

    auto pUI = getModule<IVisionUI>(UI_MODEL);
    stCmd.matInputImg = pUI->getImage();
 
    stCmd.rectROI = rectROI;

    Vision::PR_LrnTmpl(&stCmd, &stRpy);
    if (Vision::VisionStatus::OK != stRpy.enStatus) {
        System->showMessage(QStringLiteral("定位框"), QStringLiteral("学习模板失败"));
        recordId = 0;
        return false;
    }

    recordId = stRpy.nRecordId;
    return true;
}
