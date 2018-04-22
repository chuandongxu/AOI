#ifndef TOPWIDGET_H
#define TOPWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

class QTopWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QTopWidget(QWidget *parent = 0);
	void setTitle(const QString &title,const QString &ver);
	QString getTitle();
	void setTitle(const QString &title);

signals:
    void closeBtnclick();

public slots:
    void onAbout();
    void onNewProject();
    void onOpenProject();
	void onAutoRun();
	void onSystem();
	void onHardware();
	void onTools();
	void onSetting();
	void onData();

protected:
	void paintEvent(QPaintEvent *event);
	virtual void timerEvent(QTimerEvent * event);
private:
    QHBoxLayout * m_mainLayout;
    QHBoxLayout * m_titleLayout;
	QHBoxLayout * m_toolLayout;
	QVBoxLayout * m_verLayout;
    QLabel * m_titleLabel;
	QLabel * m_versionLabel;
    QPushButton * m_exitBtn;
	int m_nTimerId;

    QPushButton *m_toolBtnAbout;
    QPushButton *m_toolBtnNewProject;
    QPushButton *m_toolBtnOpenProject;
	QPushButton *m_toolBtnAutoRun;
	QPushButton *m_toolBtnSys;
	QPushButton *m_toolBtnHw;
	QPushButton *m_toolBtnTools;
	QPushButton *m_toolBtnSetting;
	QPushButton *m_toolBtnData;
};

#endif // TOPWIDGET_H
