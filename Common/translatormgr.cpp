#include "translatormgr.h"
#include <QApplication>
#include "ModuleMgr.h"
#include <QXmlStreamReader>
#include <qfile.h>

#define CHINA_LANG    "ch"

QTranslatorMgr * QTranslatorMgr::_instance_ = NULL;
bool QTranslatorMgr::_bDestoryed_ = false;

QTranslatorMgr::QTranslatorMgr(QObject *parent) :
    QObject(parent)
{
    
}

QTranslatorMgr * QTranslatorMgr::instance()
{
	if(!_instance_)
	{
		if(!_bDestoryed_)
		{
			_instance_ = new QTranslatorMgr(NULL);
			atexit(QTranslatorMgr::destory);
		}
	}

	return _instance_;
}

void QTranslatorMgr::destory()
{
	if(_instance_)
	{
		delete _instance_;
		_instance_ = NULL;
		_bDestoryed_ = true;
	}
}

void QTranslatorMgr::changeLange(const QString & postfix)
{
	QTranslatorMgr *p = QTranslatorMgr::instance();
    if(p)p->loadLang(postfix);
}

QStringList QTranslatorMgr::getLangeKeys()
{
	QTranslatorMgr *p = QTranslatorMgr::instance();
	if(p)return p->getLangConfig();

	return QStringList();
}

void QTranslatorMgr::loadLang(const QString &postfix)
{
	loadLangFiles();
    QCoreApplication * p = QApplication::instance();
    if(p)
    {
        clearLang();
       
        QString path = QApplication::applicationDirPath();

		QMoudleMgr *pMgr = QMoudleMgr::instance();
		if(pMgr)
		{
			for(int i=0; i<m_langs.size(); i++)
			{
				QString file = m_langs[i];
				if(!file.isEmpty())
				{
					QTranslator * pTrans = new QTranslator;
					file = path + "/lang/" + file + "_"+ postfix + ".qm";
					if(pTrans->load(file))
					{
						p->installTranslator(pTrans);
						m_trans.append(pTrans);
					}
					else 
					{
						delete pTrans;
					}
				}
			}
		}

        emit onChangeLang();
    }
}

void QTranslatorMgr::clearLang()
{
	QCoreApplication * p = QApplication::instance();
    if(p)
    {
		QList<QTranslator*>::iterator it = m_trans.begin();
		for(; it != m_trans.end(); ++it)
		{
			p->removeTranslator(*it);
			(*it)->deleteLater();
		}
		m_trans.clear();
	}
}

void QTranslatorMgr::loadLangFiles()
{
	if(!m_langs.isEmpty())return;

	QString path = QApplication::applicationDirPath();
	path += "/config/lang.xml";
	QFile file(path);
	if(file.open(QIODevice::ReadOnly))
	{
		QXmlStreamReader xml(&file);
		while(!xml.atEnd()) 
		{
			if(xml.readNextStartElement())
			{
				QString name = xml.name().toString();
				if("lang" == name)
				{
					QXmlStreamAttributes attr = xml.attributes();
					QString langname = attr.value("file").toString();
					m_langs.append(langname);
				}
				else if("item" == name)
				{
					QXmlStreamAttributes attr = xml.attributes();
					QString langname = attr.value("name").toString();
					QString postfix = attr.value("postfix").toString();

					m_langConfig[langname] = postfix;
				}
			}
		}
	}
}

QStringList QTranslatorMgr::getLangConfig()
{
	return m_langConfig.keys();
}

QString QTranslatorMgr::getLangPostfix(const QString &langName)
{
	if(m_langConfig.contains(langName))
	{
		return m_langConfig[langName];
	}

	return CHINA_LANG;
}

QString QTranslatorMgr::getLangName(const QString &postfix)
{
	QMap<QString,QString>::iterator it = m_langConfig.begin();
	for(; it != m_langConfig.end(); ++it)
	{
		if(it.value() == postfix)return it.key();
	}

	return "";
}

void QTranslatorMgr::initRetranslangSlot(QObject * obj,const char * slot)
{
	QTranslatorMgr *p = QTranslatorMgr::instance();
    if(p) connect(p,SIGNAL(onChangeLang()),obj,slot);
}
