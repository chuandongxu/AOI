#pragma once

#include <QObject>

class QBaseTestCase : public QObject
{
    Q_OBJECT

public:
    QBaseTestCase(QObject *parent=NULL);
    ~QBaseTestCase();

    virtual void setup() = 0;
    virtual bool run() = 0;
    virtual bool result() = 0;
};


/***************
* Test Caliper *
***************/
class QCaliper : public QBaseTestCase
{
    Q_OBJECT

public:
    QCaliper(QObject *parent = NULL);
    ~QCaliper();

    virtual void setup();
    virtual bool run();
    virtual bool result();
};
