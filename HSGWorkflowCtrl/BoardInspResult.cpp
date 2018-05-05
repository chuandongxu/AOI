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