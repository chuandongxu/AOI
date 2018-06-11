#include "BoardInspResult.h"

BoardInspResult::BoardInspResult(const QString &boardName) :
    m_boardName(boardName) {
    m_startDateTime = QDateTime::currentDateTime();
}

BoardInspResult::~BoardInspResult() {
}

void BoardInspResult::addWindowStatus(int winId, int status) {
    QMutexLocker ml(&m_mutex);
    m_mapWindowStatus.insert(winId, status);
    m_endDateTime = QDateTime::currentDateTime();
}

int BoardInspResult::getWindowStatus(int winId) const {
    if (m_mapWindowStatus.find(winId) != m_mapWindowStatus.end())
        return m_mapWindowStatus[winId];
    else
        return -1;
}

void BoardInspResult::addDeviceInspWindow(const DeviceInspWindow &deviceInspWindow) {
    auto pIter = std::find_if(m_vecDeviceInspWindow.begin(), m_vecDeviceInspWindow.end(), [deviceInspWindow](const DeviceInspWindow &iter) { return iter.device.Id == deviceInspWindow.device.Id;});
    if (pIter != m_vecDeviceInspWindow.end())
        *pIter = deviceInspWindow;
    else
        m_vecDeviceInspWindow.push_back(deviceInspWindow);
}

DeviceInspWindowVector BoardInspResult::getDeviceInspWindow() const {
    return m_vecDeviceInspWindow;
}