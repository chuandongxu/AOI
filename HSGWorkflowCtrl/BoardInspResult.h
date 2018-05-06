#pragma once
#include <QString>
#include <QDateTime>
#include <QMap>
#include <QMutex>
#include <memory>

class BoardInspResult
{
public:
    BoardInspResult() = default;
    BoardInspResult(const QString &boardName);
    ~BoardInspResult();
    void addWindowStatus(int winId, int status);
    inline QString getBoardName() const { return m_boardName; }

private:
    QString         m_boardName;
    QDateTime       m_startDateTime;
    QDateTime       m_endDateTime;
    QMutex          m_mutex;
    QMap<int, int>  m_mapWindowStatus;
};

using BoardInspResultPtr = std::shared_ptr<BoardInspResult>;
using MapBoardInspResult = QMap<QString, BoardInspResultPtr>;