#include "GraphicDisplay.h"
#include <qpainter.h>

static QColor g_color[14] = {Qt::red,Qt::darkRed,Qt::green,Qt::darkGreen,Qt::blue,Qt::darkBlue,Qt::cyan
                             ,Qt::darkCyan,Qt::magenta,Qt::darkMagenta,Qt::yellow,Qt::darkYellow,Qt::white,Qt::black};
#define DEFAULT_COLOR 14

#define X_COOR_AXIS_RANGE 1
#define Y_COOR_AXIS_RANGE 20

QGraphicDisplay::QGraphicDisplay(QWidget *parent)
	: QWidget(parent),m_xConf(1),m_baseX(0)
	, m_leftdin(40),m_rightdin(80),m_topdin(10),m_bottomdin(20)
	, m_decSize(10),m_xdec(10),m_ydec(2)
{
	m_bDrawHLine = true;
	m_bDrawVLine = true;

	m_xGradBtn = new QPushButton("Room +",this);
	m_yGradBtn = new QPushButton("Room -",this);
	//m_layout = new QVBoxLayout;

	m_xGradBtn->setFixedSize(60,20);
	m_yGradBtn->setFixedSize(60,20);

	m_dYAxisRange = Y_COOR_AXIS_RANGE;

	connect(m_xGradBtn,SIGNAL(clicked()),SLOT(onXGrad()));
	connect(m_yGradBtn,SIGNAL(clicked()),SLOT(onYGrad()));
}

QGraphicDisplay::~QGraphicDisplay()
{

}

void QGraphicDisplay::onXGrad()
{
	//m_bDrawVLine = !m_bDrawVLine;
	m_dYAxisRange*=2;
	this->update();
}

void QGraphicDisplay::onYGrad()
{
	//m_bDrawHLine = !m_bDrawHLine;
	m_dYAxisRange/=2;
	this->update();
}

void QGraphicDisplay::setBase(int ich,double base)
{
	m_baseY[ich] = base;
}

void QGraphicDisplay::setConf(int ich,double conf)
{
	m_yConf[ich] = conf;
}

void QGraphicDisplay::setColor(int ich,QColor color)
{
	m_color[ich] = color;
}

void QGraphicDisplay::setUnit(int ich,double unit,QString unitStr)
{
	m_unit[ich] = unit;
	m_unitStr[ich] = unitStr;
}

void QGraphicDisplay::setXUnit(double unit,QString unitStr)
{
	m_xUnit = unit;
	m_xUnitStr = unitStr;
}

void QGraphicDisplay::setXbase(double base)
{
	m_baseX = base;
}

void QGraphicDisplay::setXConf(double conf)
{
	m_xConf = conf;
}

void QGraphicDisplay::addData(int ich,unsigned int clock,double d,int nColor)
{
	if(!m_data.contains(ich))
	{
		m_data[ich] = QChangeData();
		m_yConf[ich] = 1;
		m_baseY[ich] = 0;
		m_unit[ich] = 1;
		m_unitStr[ich] = "";

		if((ich+nColor) < DEFAULT_COLOR)m_color[ich] = g_color[ich+nColor];
		else m_color[ich] = g_color[DEFAULT_COLOR-1];
	}

	m_data[ich][clock] = d;
}

void QGraphicDisplay::clearData(int ich)
{
	if(m_data.contains(ich))
	{
		m_data[ich].clear();		
	}
	m_data.remove(ich);
}

void QGraphicDisplay::setDefaultYRange(double dRange)
{
	m_dYAxisRange = dRange;
}

void QGraphicDisplay::resizeEvent(QResizeEvent * event)
{
	//QWidget::resizeEvent(event);

	QSize size = event->size();
	m_xGradBtn->move(size.width() -63, m_topdin+3);
	m_yGradBtn->move(size.width() -63, m_topdin+33);
	//m_layout->setGeometry()
}

void QGraphicDisplay::paintEvent(QPaintEvent * event)
{
	//QWidget::paintEvent(event);

	QPainter painter(this);

	drawBackground(painter);

	QGraphicData::iterator it = m_data.begin();
	for(int i=0; it != m_data.end(); ++it,i++)
	{
		QChangeData &chData = it.value();

		QVector<QPoint> lines;
		QChangeData::iterator ptr = chData.begin();
		for(; ptr != chData.end(); ++ptr)
		{
			bool s = false;
			QPoint pt = convertToPoint(i,ptr.key(),ptr.value(),s);
			if(s)lines.append(pt);
		}

		QPen old = painter.pen();
		painter.setPen(m_color[it.key()]);
		painter.drawPolyline(&lines[0],lines.size());
		painter.setPen(old);
	}

		
}

void QGraphicDisplay::drawBackground(QPainter &painter)
{
	QRect rect = this->geometry();
	int baseY = rect.height()/2;

	int offset = 2;//绘图区域，刻线偏移
	QRect inRect = rect;
	inRect.adjust(m_leftdin+offset,m_topdin+offset,-m_rightdin-offset,-m_bottomdin-offset);
	painter.fillRect(inRect,QColor(100,100,100));

	int xdec = m_xdec;  //刻度间距
	int ydec = m_ydec;

	//int ydm = m_din;
	//if(ydm > 2 * ydec)ydm = ydm/2;

	//绘制垂直刻度线
	int x0 = rect.left() + m_leftdin;
	int y0 = rect.top();
	int x1 = rect.left()+m_leftdin;
	int y1 = rect.bottom() - m_bottomdin;
	painter.drawLine(x0,y0,x1,y1);
	
	int yBaseValue = 0;
	for(int pos = rect.bottom()- m_bottomdin,i = 0; pos > rect.top() + m_topdin/2 ; pos -= ydec,i++)
	{
		yBaseValue = i;
	}

	for(int pos = rect.bottom()- m_bottomdin,i = 0; pos > rect.top() + m_topdin/2 ; pos -= ydec,i++)
	{
		if(0 == (i%2))x0 =  m_leftdin - m_decSize;
		else x0 = m_leftdin - m_decSize/2;

		y0 = pos;
		x1 = m_leftdin;
		y1 = pos;
		painter.drawLine(x0,y0,x1,y1);  //绘制刻线

		if(m_bDrawHLine && (0 == i%2) && y0 > inRect.top())
		{
			x0 = m_leftdin;
			x1 = rect.right() - m_rightdin -2;
			if(0 == i%5)
			{
				painter.drawLine(x0,y0,x1,y1);  //绘制绘制水平横线
			}
			else
			{
				QPen old = painter.pen();
				painter.setPen(QColor(120,120,120));
				painter.drawLine(x0,y0,x1,y1);  //绘制绘制水平横线
				painter.setPen(old);
			}
		}

		//绘制刻度读数
		if(0 == i%10)
		{
			QRect rect(0,pos-6,m_leftdin/2+10,15);
			painter.drawText(rect,Qt::AlignRight|Qt::AlignHCenter,QString("%0").arg((double)(i-yBaseValue/2)/m_dYAxisRange,0,'g',3,'0'));
		}
	}

	//绘制水平刻度线
	int xdm = m_rightdin - xdec;
	painter.drawLine(rect.left() + m_leftdin,rect.bottom()-m_bottomdin,rect.right(),rect.bottom()-m_bottomdin);

	for(int pos = m_leftdin,i=0;pos < rect.right() - xdm; pos += xdec,i++)
	{
		x0 = rect.left() + pos;
		y0 = rect.bottom() - m_bottomdin;
		x1 =  rect.left() + pos;
		
		if(0 == (i%2))y1 = (rect.bottom() - m_bottomdin) + m_decSize;
		else y1 = (rect.bottom()- m_bottomdin) + m_decSize/2;

		painter.drawLine(x0,y0,x1,y1);  //绘制垂直刻线
		
		if(m_bDrawVLine && (0 == i%2) && x0 < inRect.right())
		{
			y0 = rect.top() + m_topdin + 2;
			y1 = rect.bottom() - m_bottomdin;

			if(0 == i%5)
			{
				painter.drawLine(x0,y0,x1,y1);  //绘制垂直竖线
			}
			else
			{
				QPen old = painter.pen();
				painter.setPen(QColor(120,120,120));
				painter.drawLine(x0,y0,x1,y1);  //绘制垂直竖线
				painter.setPen(old);
			}
		}

		//绘制刻度读数
		if(0 == i%10)
		{
			QRect rect(pos-10,rect.bottom()-m_bottomdin/2 - 2,20,15);
			painter.drawText(rect,Qt::AlignVCenter|Qt::AlignHCenter,QString("%0").arg(i/X_COOR_AXIS_RANGE));
		}
	}

	//显示y刻度单位
	QPen old = painter.pen();
	QGraphicData::iterator it = m_data.begin();
	for(int i=0; it != m_data.end(); ++it,i++)
	{
		
		painter.setPen(m_color[it.key()]);
		QRect textRect(rect.right()-m_rightdin, rect.top()+m_topdin + i*20 + 80,m_rightdin,20);
		double yGrad = m_unit[it.key()] * m_yConf[it.key()];
		painter.drawText(textRect,Qt::AlignRight|Qt::AlignHCenter,QString("%0%1/dec").arg(yGrad).arg(m_unitStr[it.key()]));
	}
	painter.setPen(old);

	//显示X刻度单位
	double xGrad = (m_xUnit * m_xConf);
	painter.drawText(rect.right()-m_rightdin + 10, rect.bottom(),QString("%0%1/dec").arg(xGrad).arg(m_xUnitStr));
}


QPoint QGraphicDisplay::convertToPoint(int ich,unsigned int clock,double val,bool &s)
{
	QRect rect = this->geometry();
	QSize size = rect.size();

	int x = (clock + m_baseX) *  (m_xdec *X_COOR_AXIS_RANGE)/m_xConf + m_leftdin;  //变化到坐标点

	val += m_baseY[ich];  //加入偏移量
	int y = (size.height() - m_bottomdin)/2 + m_topdin/2 - (val * m_ydec * m_dYAxisRange) / m_yConf[ich];
	
	int offset = 2;//绘图区域，刻线偏移
	QRect inRect = rect;
	inRect.adjust(m_leftdin-1,m_topdin+offset,-m_rightdin-offset,-m_bottomdin+1);
	
	//s = inRect.contains(x,y);
	s = true;

	return QPoint(x,y);
}


/*

QGraphicWidget::QGraphicWidget(QWidget * parent)
	:QWidget(parent)
{
	m_display = new QGraphicDisplay(this);
	m_xGradBtn = new QPushButton("xGrad",this);
	m_yGradBtn = new QPushButton("yGrad",this);
	m_layout = new QVBoxLayout;

	m_xGradBtn->setFixedSize(18,20);
	m_yGradBtn->setFixedSize(18,20);

	m_layout->addWidget(m_xGradBtn);
	m_layout->addWidget(m_yGradBtn);

	this->setLayout(m_layout);
}

void QGraphicWidget::resizeEvent(QResizeEvent * event)
{
	//QWidget::resizeEvent(event);

	QSize size = event->size();
	m_display->setGeometry(0,0,size.width()-40,size.height());
	m_xGradBtn->move(size.width() -20 ,10);
	m_yGradBtn->move(size.width() -20,40);
	//m_layout->setGeometry()
}

void QGraphicWidget::addData(int ich,unsigned int clock,double d)
{
	m_display->addData(ich,clock,d);
}
*/