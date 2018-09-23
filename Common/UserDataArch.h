#ifndef USERDATAARCH_H
#define USERDATAARCH_H

#include <QObject>
#include <qstring.h>
#include <qmap.h>
#include <qvariant.h>

class QUserDataArch : public QObject
{
    Q_OBJECT

public:
    QUserDataArch();
    ~QUserDataArch();

    QVariant getParam(const QString &name);
    void setParam(const QString &name,const QVariant &data);
    void delParam(const QString &name);
    QStringList getParamKeys(const QString &condtion);

    void load(const QString & productName);
    
protected:
    void save();
    QVariant converStringToData(const QString &type,const QString &value);
    QString converDataToString(const QVariant &value);
    QString getValueType(const  QVariant &value);
    
    void initTimer();
    virtual void timerEvent(QTimerEvent * event);
    void unInitTimer();

private:
    bool m_bTimerInit;
    int m_timer;
    int m_newCount;
    QString m_file;
    QMap<QString,QVariant> m_param;
};

#endif // USERDATAARCH_H
