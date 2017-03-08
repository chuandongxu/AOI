#ifndef TRANSLATORMGR_H
#define TRANSLATORMGR_H

#include <QObject>
#include <QTranslator>
#include <qlist.h>
#include <qmap.h>
#include "Common_global.h"

class COMMON_EXPORT QTranslatorMgr : public QObject
{
    Q_OBJECT
public:
    explicit QTranslatorMgr(QObject *parent = 0);

	static QTranslatorMgr * instance();
	static void initRetranslangSlot(QObject * obj, const char *slot);
    static void changeLange(const QString & name);
	static QStringList getLangeKeys();
    
	QStringList getLangConfig();
	QString getLangPostfix(const QString &langName);
	QString getLangName(const QString &postfix);

signals:
    void onChangeLang();

protected:
    void loadLang(const QString &name);
	void clearLang();
	void loadLangFiles();
	
	static void destory();

protected:
	QMap<QString,QString> m_langConfig;
	QList<QString> m_langs; 
    QList<QTranslator*> m_trans;
    QString m_name;

private:
	static QTranslatorMgr * _instance_;
	static bool _bDestoryed_;
};

#endif // TRANSLATORMGR_H
