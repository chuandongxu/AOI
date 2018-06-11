#pragma once
#include <QString>
#include <QDateTime>
#include <QMap>
#include <QMutex>
#include <memory>

#include "AutoRunDataStructs.h"

class BoardInspResult
{
public:
    BoardInspResult() = default;
    BoardInspResult(const QString &boardName);
    ~BoardInspResult();
    void addWindowStatus(int winId, int status);
    int getWindowStatus(int winId) const;
    inline QString getBoardName() const { return m_boardName; }
    void addDeviceInspWindow(const DeviceInspWindow &deviceInspWindow);
    DeviceInspWindowVector getDeviceInspWindow() const;
    void setFatalError() { m_bWithFatalError = true; }
    bool isWithFatalError() const { return m_bWithFatalError; }
    void setErrorMsg(const QString &strErrMsg) { m_strErrorMsg = strErrMsg; }
    QString getErrorMsg() const { return m_strErrorMsg; }

private:
    QString                 m_boardName;
    QDateTime               m_startDateTime;
    QDateTime               m_endDateTime;
    QMutex                  m_mutex;
    QMap<int, int>          m_mapWindowStatus;
    DeviceInspWindowVector  m_vecDeviceInspWindow;

    bool                    m_bWithFatalError = false;
    QString                 m_strErrorMsg;
};

using BoardInspResultPtr = std::shared_ptr<BoardInspResult>;
using MapBoardInspResult = QMap<QString, BoardInspResultPtr>;