#ifndef STATEWIDGET_H
#define STATEWIDGET_H

#include <QWidget>

namespace Ui {
class QStateWidget;
}

class QStateWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QStateWidget(QWidget *parent = 0);
    ~QStateWidget();

protected slots:
    void onClearProductCount();
    void onGoHomeState(const QVariantList &data);
    void onIsGoHomeIng(const QVariantList &data);
    void onRunState(const QVariantList &data);
    void onResoultEvent(const QVariantList &data);

protected:
    virtual void timerEvent(QTimerEvent * event);
    void updataStates();

    QString getWorkState();
    QString getRobtState();
    QString getLeftStationState();
    QString getRigthStationState();
    QString getWorkMod();
    int    calSum();
    double    calOkPrecent();

    void showErrorMessage();
    
private:
   Ui::QStateWidget *ui; 
    //QStandardItemModel m_homeState;
};

#endif // STATEWIDGET_H
