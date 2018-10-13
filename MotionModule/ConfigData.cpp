#include "ConfigData.h"
#include <qapplication.h>
#include <qdebug.h>
#include <qfile.h>
#include <QXmlStreamReader>

#include "../Common/ThreadPrioc.h"

//------------------------------------------------------------
ConfigData * ConfigData::_instance_ = NULL;
bool ConfigData::_destoryed_ = false;
//------------------------------------------------------------
ConfigData::ConfigData(QObject *parent)
    : QObject(parent)
{
    loadConfig();
}

ConfigData::~ConfigData()
{
}

ConfigData * ConfigData::instance()
{
    if (!_instance_)
    {
        if (!_destoryed_)
        {
            _instance_ = new ConfigData;
            atexit(&ConfigData::desstory);
        }
    }

    return _instance_;
}

void ConfigData::desstory()
{
    if (_instance_)
    {
        delete _instance_;
        _instance_ = NULL;
        _destoryed_ = true;
    }
}

void ConfigData::loadConfig()
{
    QString path = QApplication::applicationDirPath();
    path += "/config/ioConfig.xml";
    QFile file(path);
    if (file.open(QIODevice::ReadOnly))
    {
        QStr2IdMap* pDataMap = NULL;
        QId2NameMap* pNameMap = NULL;

        QXmlStreamReader xml(&file);
        while (!xml.atEnd())
        {
            if (xml.readNextStartElement())
            {
                QString name = xml.name().toString();
                if (name == "item")
                {
                    QXmlStreamAttributes attr = xml.attributes();
                    int id = attr.value("id").toInt();
                    QString var = attr.value("var").toString();
                    QString name = attr.value("name").toString(); 
                     
                    if (pDataMap) pDataMap->insert(var, id);
                    if (pNameMap) pNameMap->insert(id, name);
                }
                else if (name == "module")
                {
                    QXmlStreamAttributes attr = xml.attributes();
                    QString type = attr.value("type").toString();
                    QString outline = attr.value("outline").toString();
                    
                    if ("axis" == type)
                    {
                        pDataMap = &m_axisMap;
                        pNameMap = &m_axisNameMap;
                    }
                    else if ("input" == type)
                    {
                        pDataMap = &m_inputMap;
                        pNameMap = &m_inputNameMap;
                    }
                    else if ("output" == type)
                    {
                        pDataMap = &m_outputMap;
                        pNameMap = &m_outputNameMap;
                    }
                }
            }
        }
    }
}

int ConfigData::getAxisID(const QString& str)
{
    QAutoLocker loacker(&m_mutex);
    return m_axisMap.value(str, 0);
}

int ConfigData::getInpID(const QString& str)
{
    QAutoLocker loacker(&m_mutex);
    return m_inputMap.value(str, 0);
}

int ConfigData::getOupID(const QString& str)
{
    QAutoLocker loacker(&m_mutex);
    return m_outputMap.value(str, 0);
}

QString ConfigData::getAxisName(const QString& str)
{
    QAutoLocker loacker(&m_mutex);
    int nID = m_axisMap.value(str, 0);
    return m_axisNameMap.value(nID, "");
}

QString ConfigData::getInpName(const QString& str)
{
    QAutoLocker loacker(&m_mutex);
    int nID = m_inputMap.value(str, 0);
    return m_inputNameMap.value(nID, "");
}

QString ConfigData::getOupName(const QString& str)
{
    QAutoLocker loacker(&m_mutex);
    int nID = m_outputMap.value(str, 0);
    return m_outputNameMap.value(nID, "");
}

QString ConfigData::getAxisName(int nID)
{
    QAutoLocker loacker(&m_mutex);  
    return m_axisNameMap.value(nID, "");
}

QString ConfigData::getInpName(int nID)
{
    QAutoLocker loacker(&m_mutex);  
    return m_inputNameMap.value(nID, "");
}

QString ConfigData::getOupName(int nID)
{
    QAutoLocker loacker(&m_mutex);   
    return m_outputNameMap.value(nID, "");
}

int ConfigData::ID(const QString& str)
{
    int nID = getAxisID(str);
    if (nID > 0) return nID;

    nID = getInpID(str);
    if (nID > 0) return nID;

    nID = getOupID(str);
    if (nID > 0) return nID;
 
    return 0;
}

QString ConfigData::name(const QString& str)
{
    QString name = getAxisName(str);
    if (!name.isEmpty()) return name;

    name = getInpName(str);
    if (!name.isEmpty()) return name;

    name = getOupName(str);
    if (!name.isEmpty()) return name;

    return "";
}
