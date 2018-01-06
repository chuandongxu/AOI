#include "ModuleMgr.h"
#include <qapplication.h>
#include <qdebug.h>
#include <qfile.h>
#include <QXmlStreamReader>
#include "Track.h"

typedef QModuleInterface * (*func_createModule)(int id,const QString &name);
typedef void (*func_destoryModule)(QModuleInterface * interface);

QMoudleMgr * QMoudleMgr::_instance_ = NULL;
bool QMoudleMgr::_bDestoryed_ = false;
int QMoudleMgr::_mainModuleId_ = -1;

QMoudleMgr::QMoudleMgr()
{

}

QMoudleMgr::~QMoudleMgr()
{

}


QMoudleMgr * QMoudleMgr::instance()
{
    if(!_instance_)
    {
        if(!_bDestoryed_)
        {
            _instance_ = new QMoudleMgr;
            atexit(&QMoudleMgr::destory);
        }
    }

    return _instance_;
}

void QMoudleMgr::destory()
{
    if(_instance_) delete _instance_;
    _instance_ = NULL;
    _bDestoryed_ = true;
}

QList<int> QMoudleMgr::getModelIdList()
{
    return m_moduleList.keys();
}

QModuleInterface * QMoudleMgr::getModule(int id)
{
    if(m_moduleList.contains(id))
    {
        return m_moduleList[id];
    }

    return NULL;
}

void QMoudleMgr::addModlue(QModuleInterface * pMoudle)
{
    if(pMoudle)
    {
        m_moduleList[pMoudle->getId()] = pMoudle;
    }
}

void QMoudleMgr::removeModlue(QModuleInterface * pMoudle)
{
    if(pMoudle)
    {
        m_moduleList.remove(pMoudle->getId());
    }
}

QModuleInterface * QMoudleMgr::loadModule(const QString &dllfile,int id,const QString &name)
{
	QString path = QApplication::applicationDirPath();
	path += "/" + dllfile;
	QLibrary * p = new QLibrary(path);
	if(p && p->load())
	{
		func_createModule func = (func_createModule)p->resolve("createModule");
		if(func)
		{
			QModuleInterface * pFace = (*func)(id,name);
			if(pFace)
			{
				m_moduleLib[id] = p;

				return pFace;
			}
		}
		else TR_WARRING("lib %s is not plugin.",path.toStdString().c_str());
	}
	else
	{
		if(p)
		{
			QString msg = p->errorString();
			TR_WARRING("****** load plugin : %s",msg.toStdString().c_str());
		}
	}

	return NULL;
}

void QMoudleMgr::freeModule(int id)
{
	if(m_moduleLib.contains(id))
	{
		QLibrary * p = m_moduleLib[id];
		func_destoryModule func = (func_destoryModule)p->resolve("destoryModule");
		if(func)
		{
			QModuleInterface * pFace = m_moduleList[id];
			if(pFace)
			{
				(*func)(pFace);
				m_moduleLib.remove(id);
			}
		}
	}
}
void QMoudleMgr::freeModules()
{
	QMap<int,QLibrary *>::iterator iter =m_moduleLib.begin();
	while(iter != m_moduleLib.end())
	{
		QLibrary * p = iter.value();
		int id = iter.key();
		func_destoryModule func = (func_destoryModule)p->resolve("destoryModule");
		if(func)
		{
			QModuleInterface * pFace = m_moduleList[id];
			if(pFace)
			{
				(*func)(pFace);
			}
		}
		++iter;
	}
}
QModuleInterface * QMoudleMgr::loadPlugin(const QString &dllfile,int id,const QString &name)
{
	QMoudleMgr * p = QMoudleMgr::instance();
	if(p)return p->loadModule(dllfile,id,name);

	return NULL;
}

void QMoudleMgr::removePlugin(int id)
{
	QMoudleMgr * p = QMoudleMgr::instance();
	if(p)p->freeModule(id);
}
void QMoudleMgr::removePlugins()
{
	QMoudleMgr * p = QMoudleMgr::instance();
	if(p)p->freeModules();
}
void QMoudleMgr::loadPlugins()
{
	QString path = QApplication::applicationDirPath();
	path += "/config/plugin.xml";
	QFile file(path);
	if(file.open(QIODevice::ReadOnly))
	{
		QXmlStreamReader xml(&file);
		while(!xml.atEnd()) 
		{
			if(xml.readNextStartElement())
			{
				QString name = xml.name().toString();
				if(name == "plugin")
				{
					QXmlStreamAttributes attr = xml.attributes();
					int id = attr.value("id").toInt();
					QString dllfile = attr.value("file").toString();
					QString name = attr.value("name").toString();

					if(!dllfile.isEmpty())
					{
						loadPlugin(dllfile,id,name);
					}
					else qDebug() << "load plugin error of id :" << id << "  name :" << name ;
				}
				else if(name == "mainModule")
				{
					QXmlStreamAttributes attr = xml.attributes();
					_mainModuleId_ = attr.value("id").toInt();
				}
			}
		}
	}
}

 QModuleInterface * QMoudleMgr::getMainModule()
 {
	 QMoudleMgr * p = instance();
	 if(p && (-1 != _mainModuleId_))
	 {
		 return p->getModule(_mainModuleId_);
	 }

	 return NULL;
 }