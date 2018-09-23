#ifndef LEFTWIDGET_H
#define LEFTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>

#include "ui_leftwidget.h"
#include "statewidget.h"
#include "settingdialog.h"

class QLeftWidget : public QWidget
{
    Q_OBJECT

public:
    QLeftWidget(QWidget *parent = NULL);
    ~QLeftWidget();

    void setStateWidget(QWidget * w);

protected:
    void paintEvent(QPaintEvent *event);
    void timerEvent(QTimerEvent * event);
    void enableButton(int iLevel);

private slots:
    void onDiagonseClick();
    void onSettingClick();
    void onShowWarring();
    void onGoHome();
    void onChangeUser();
    void onManageUser();
    void onChangeLang(const QString &text);
    void onRunState(const QVariantList &data);
    void onUIState(const QVariantList &data);

private:
    Ui::leftWidget ui;
    QVBoxLayout   *m_subLayout;
    QWidget       *m_stateWidget;
    QWidget       *m_pDeviceListWidget;
    int            m_timerId;
    QSettingDialog m_dlgSetting;
};

#endif // LEFTWIDGET_H
