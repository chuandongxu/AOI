#ifndef EOS_H
#define EOS_H

#include <QObject>
#include <QVariantList>
#include "Common_global.h"

class COMMON_EXPORT QDispth : public QObject
{
    Q_OBJECT
public:
    QDispth(){};
signals:
    void onNotify(const QVariantList &data);
};

class COMMON_EXPORT QEos : public QObject
{
    Q_OBJECT

protected:
    QEos(QObject *parent);
    ~QEos();

public:
    static void Attach(const QString &notifyKey,const QObject * receiver, const char * method);
    static void Detach(const QString &notifyKey,const QObject * receiver, const char * method);

    static void Notify(const QString notifyKey,const QVariantList &data);
    static void Notify(const QString notifyKey,const QVariant &d1);
    static void Notify(const QString notifyKey,const QVariant &d1,const QVariant &d2);
    static void Notify(const QString notifyKey,const QVariant &d1,const QVariant &d2,const QVariant &d3);
    static void Notify(const QString notifyKey,const QVariant &d1,const QVariant &d2,const QVariant &d3,const QVariant &d4);
    static void Notify(const QString notifyKey,const QVariant &d1,const QVariant &d2,const QVariant &d3,const QVariant &d4,const QVariant &d5);
    static void Notify(const QString notifyKey,const QVariant &d1,const QVariant &d2,const QVariant &d3,const QVariant &d4,const QVariant &d5,
                       const QVariant &d6);
    static void Notify(const QString notifyKey,const QVariant &d1,const QVariant &d2,const QVariant &d3,const QVariant &d4,const QVariant &d5,
                       const QVariant &d6,const QVariant &d7);
    static void Notify(const QString notifyKey,const QVariant &d1,const QVariant &d2,const QVariant &d3,const QVariant &d4,const QVariant &d5,
                       const QVariant &d6,const QVariant &d7,const QVariant &d8);
    static void Notify(const QString notifyKey,const QVariant &d1,const QVariant &d2,const QVariant &d3,const QVariant &d4,const QVariant &d5,
        const QVariant &d6,const QVariant &d7,const QVariant &d8,const QVariant &d9);
private:
    static QMap<QString,QDispth*> m_dispthes;
    static QMutex m_mutex;
};

#endif // EOS_H
