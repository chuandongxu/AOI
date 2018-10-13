#pragma once

#include <QObject>
#include <QMap>
#include <QMutex>
#include "motionmodule_global.h"

typedef QMap<QString, unsigned int> QStr2IdMap;
typedef QMap<unsigned int, QString> QId2NameMap;

class MOTIONMODULE_EXPORT ConfigData : public QObject
{
    Q_OBJECT

public:
    ConfigData(QObject *parent = NULL);
    ~ConfigData();

    static ConfigData * instance();

private:
    static void desstory();

public:
    int getAxisID(const QString& str);
    int getInpID(const QString& str);
    int getOupID(const QString& str);

    QString getAxisName(const QString& str);
    QString getInpName(const QString& str);
    QString getOupName(const QString& str);

    QString getAxisName(int nID);
    QString getInpName(int nID);
    QString getOupName(int nID);

    int ID(const QString& str);
    QString name(const QString& str);  

private:
    void loadConfig();

private:
    QStr2IdMap m_axisMap;
    QStr2IdMap m_inputMap;
    QStr2IdMap m_outputMap;

    QId2NameMap m_axisNameMap;
    QId2NameMap m_inputNameMap;
    QId2NameMap m_outputNameMap;

private:
    static ConfigData * _instance_;
    static bool _destoryed_;
    QMutex m_mutex;
};

#define Config ConfigData::instance()
