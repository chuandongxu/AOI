#include "UserDataArch.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <qfile.h>
#include <qapplication.h>

#define MAX_SAVE_TIME 100

QUserDataArch::QUserDataArch()
    :m_bTimerInit(false),m_timer(0),m_newCount(-1)
{
    
}

QUserDataArch::~QUserDataArch()
{
    if(-1 != m_newCount)this->save();
    this->unInitTimer();
}

void QUserDataArch::load(const QString & productName)
{
    m_param.clear();
    QString path = QApplication::applicationDirPath();
    path += "/data/";
    path += productName;
    path += ".xml";

    m_file = path;

    QFile file(m_file);
    if(file.open(QIODevice::ReadOnly))
    {
        QXmlStreamReader xmlReader(&file);

        while(!xmlReader.atEnd())
        {
            xmlReader.readNext();
            if(xmlReader.isStartElement())
            {
                QString name = xmlReader.name().toString();
                if("item" == name )
                {
                    QXmlStreamAttributes attrs = xmlReader.attributes();
                    QString key = attrs.value("key").toString();
                    QString type = attrs.value("type").toString();
                    QString value = attrs.value("value").toString();

                    m_param[key] = converStringToData(type,value);
                }
            }
        }
    }
}

void QUserDataArch::save()
{
    QFile file(m_file);
    if(file.open(QIODevice::WriteOnly))
    {
        QXmlStreamWriter xmlWriter(&file);

        xmlWriter.writeStartDocument("1.0","GBK");
        xmlWriter.writeStartElement("data");

        QMap<QString,QVariant>::iterator it = m_param.begin();
        for(; it != m_param.end(); ++it)
        {
            xmlWriter.writeStartElement("item");
            xmlWriter.writeAttribute("key", it.key());
            xmlWriter.writeAttribute("type",getValueType(it.value()));
            xmlWriter.writeAttribute("value",converDataToString(it.value()));
            xmlWriter.writeEndElement(); 
        }

        xmlWriter.writeEndElement(); 
        xmlWriter.writeEndDocument();
    }
}

QVariant QUserDataArch::converStringToData(const QString &type,const QString &value)
{
    if("int" == type)return value.toInt();
    else if("double" == type)return value.toDouble();
    else if("string" == type)return value;

    return QVariant();
}

QString QUserDataArch::converDataToString(const QVariant &value)
{
    if(value.type() == QVariant::Bool)return QString::number(value.toInt());

    return value.toString();
}

QString QUserDataArch::getValueType(const  QVariant &value)
{
    if(value.type() == QVariant::Int)return "int";
    else if(value.type() == QVariant::Bool)return "int";
    else if(value.type() == QVariant::Double)return "double";
    else if(value.type() == QVariant::String)return "string";

    return "unkown";
}

QVariant QUserDataArch::getParam(const QString &name)
{
    if(m_param.contains(name))return m_param[name];

    return QVariant();
}

void QUserDataArch::setParam(const QString &name,const QVariant &data)
{
    m_newCount = 0;
    if(m_param.contains(name))m_param[name] = data;
    else m_param.insert(name,data);

    this->initTimer();
}

void QUserDataArch::delParam(const QString &name)
{
    if(m_param.contains(name))
    {
        m_newCount = 0;
        m_param.remove(name);

        this->initTimer();
    }
}

QStringList QUserDataArch::getParamKeys(const QString &condtion)
{
    QStringList keys;
    QStringList ls = m_param.keys();
    for(int i=0; i< ls.size(); i++)
    {
        if(ls[i].contains(condtion))
        {
            keys.append(ls[i]);
        }
    }

    return keys;
}

void QUserDataArch::initTimer()
{
    if(!m_bTimerInit)
    {
        m_timer = this->startTimer(20);
        m_bTimerInit = true;
    }
}

void QUserDataArch::unInitTimer()
{
    this->killTimer(m_timer);
}

void QUserDataArch::timerEvent(QTimerEvent * event)
{
    if(m_newCount == -1)return;
    else if(m_newCount < MAX_SAVE_TIME)
    {
        ++m_newCount;
    }
    else if(m_newCount >= MAX_SAVE_TIME)
    {
        this->save();
        m_newCount = -1;
    }
}