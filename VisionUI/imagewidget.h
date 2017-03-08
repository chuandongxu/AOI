#ifndef QIMAGEWIDGET_H
#define QIMAGEWIDGET_H

#include <QWidget>
#include <QLabel>

class QImageWidget : public QLabel
{
    Q_OBJECT
public:
    explicit QImageWidget(QWidget *parent = 0);

signals:

public slots:

protected:
    virtual void paintEvent(QPaintEvent * event);

};

#endif // QIMAGEWIDGET_H
