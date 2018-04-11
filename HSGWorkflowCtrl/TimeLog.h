#ifndef _TIME_LOG_H_
#define _TIME_LOG_H_

#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include "StopWatch.h"

class TimeLog
{
protected:
    TimeLog(): _anIndex ( 0 ), _vecStringTimeLog ( _SIZE, "" ) {};
public:
    static TimeLog *GetInstance();
    void addTimeLog(const std::string &strMsg);
    void addTimeLog(const std::string &strMsg, __int64 nTimeSpan);
    void dumpTimeLog(const std::string &strFilePath);
private:
    static std::atomic<TimeLog*>    _pInstance;
    static std::mutex               _mutex;
    static const size_t             _SIZE   = 5000;
    static const size_t             _MSG_WIDTH = 100;

    std::atomic<size_t>             _anIndex;
    std::vector<std::string>        _vecStringTimeLog;
    CStopWatch                      _stopWatch;
};

#define TimeLogInstance TimeLog::GetInstance()

#endif