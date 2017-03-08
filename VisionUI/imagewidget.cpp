#include "imagewidget.h"
#include <QStylePainter>
#include <QBrush>
#include <QColor>
#include <QDebug>

QImageWidget::QImageWidget(QWidget *parent) :
    QLabel(parent)
{
    this->setFixedSize(500,400);
    //qDebug() << "crate.....";
}


void QImageWidget::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);

    painter.save();

    int n = (int)this;
    QString str = QString::number(n);

    QRect rect = this->geometry();
    painter.fillRect(QRect(QPoint(0,0),QSize(rect.width(),rect.height())),QBrush(QColor(0,0,0)));

    //qDebug() << rect;

    painter.setPen(Qt::blue);
    painter.setFont(QFont("Arial", 30));
    painter.drawText(0,20,str);

    painter.restore();
}

