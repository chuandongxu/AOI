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
    void onDiagonseClick();
    void onSettingClick();
	void onShowWarring();
	void onGoHome();
protected:
	void paintEvent(QPaintEvent *event);
	virtual void timerEvent(QTimerEvent * event);
private:
    QHBoxLayout * m_mainLayout;
    QHBoxLayout * m_titleLayout;
	QVBoxLayout * m_verLayout;
    QLabel * m_titleLabel;
	QLabel * m_versionLabel;
    QPushButton * m_exitBtn;
	int m_nTimerId;
};

#endif // TOPWIDGET_H
