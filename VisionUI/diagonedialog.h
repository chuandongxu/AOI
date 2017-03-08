#ifndef DIAGONEDIALOG_H
#define DIAGONEDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QHBoxLayout>

class QDiagoneDialog : public QDialog
{
    Q_OBJECT
public:
    explicit QDiagoneDialog(QWidget *parent = 0);

signals:

public slots:

private:
    QTabWidget * m_tabWidget;
    QHBoxLayout * m_layout;
};

#endif // DIAGONEDIALOG_H
