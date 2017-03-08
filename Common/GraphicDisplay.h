#ifndef GRAPHICDISPLAY_H
#define GRAPHICDISPLAY_H

#include "Common_global.h"
#include <QLabel>
#include <QMap>
#include <QPushbutton>
#include <QVBoxLayout>
#include <QResizeEvent>

class COMMON_EXPORT QGraphicDisplay : public QWidget
{
	Q_OBJECT
	typedef QMap<unsigned int,double> QChangeData;
	typedef QMap<int,QMap<unsigned int,double>> QGraphicData;

public:
	QGraphicDisplay(QWidget *parent);
	~QGraphicDisplay();

	void addData(int ich,unsigned int clock,double d,int nColor=0);
	void setBase(int ich,double base);
	void setConf(int ich,double conf);
	void setColor(int ich,QColor color);
	void setUnit(int ich,double unit,QString unitStr);

	void setXUnit(double unit,QString unitStr);
	void setXbase(double base);
	void setXConf(double conf);

	void clearData(int ich);
	void setDefaultYRange(double dRange);
	
protected slots:
	void onXGrad();
	void onYGrad();

protected:
	virtual void paintEvent(QPaintEvent * event);
	virtual void resizeEvent(QResizeEvent * event);

	void drawBackground(QPainter &painter);

	QPoint convertToPoint(int ich,unsigned int clock,double val,bool &s);
	//QPoint descrConvertToPoint(int ich,unsigned int clock,double val);
private:
	QGraphicData m_data;
	QMap<int,double> m_unit;
	QMap<int,QString> m_unitStr;
	QMap<int,QColor> m_color;
	QMap<int,double> m_yConf;
	QMap<int,double> m_baseY;
	double m_xConf;
	unsigned int m_baseX;
	double m_xUnit;
	QString m_xUnitStr;

	int m_leftdin;   //左缩进
	int m_rightdin;  //右缩进
	int m_topdin;    //上缩进
	int m_bottomdin; //下缩进

	int m_decSize;   //刻线尺寸
	int m_xdec;      //X刻度间距
	int m_ydec;      //y刻度间距
	//int m_btnSize;   //按钮区宽度
	bool m_bDrawHLine;   //绘制水平背景线
	bool m_bDrawVLine;   //绘制垂直背景线

	QPushButton * m_xGradBtn;
	QPushButton * m_yGradBtn;

	double m_dYAxisRange;
};

/*
class QGraphicWidget : public QWidget
{
	Q_OBJECT
public:
	QGraphicWidget(QWidget *parent);
	~QGraphicWidget(){};

	void addData(int ich,unsigned int clock,double d);
protected:
	virtual void resizeEvent(QResizeEvent * event);

protected:
	QGraphicDisplay * m_display;
	QPushButton * m_xGradBtn;
	QPushButton * m_yGradBtn;
	QVBoxLayout * m_layout;
};
*/

#endif // GRAPHICDISPLAY_H
