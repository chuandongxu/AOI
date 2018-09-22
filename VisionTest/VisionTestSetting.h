#pragma once

#include <QMainWindow>
#include "ui_VisionTestSetting.h"
#include "VisionTestCtrl.h"

class VisionTestSetting : public QMainWindow
{
    Q_OBJECT

public:
    VisionTestSetting(VisionTestCtrl* pCtrl, QWidget *parent = Q_NULLPTR);
    ~VisionTestSetting();

private slots:
    void newFile();
    void openFile();
    void saveFile();
    void saveAsFile();

    void runOnce();
    void runOver();
    void stop();

    void about();
    void aboutSystem();

private:
    void initUI();
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *helpMenu;

    QToolBar *fileToolBar;
    QToolBar *editToolBar;

    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *runOnceAct;
    QAction *runOverAct;
    QAction *stopAct;
    QAction *aboutAct;
    QAction *aboutSystemAct;

private:

private:
    Ui::VisionTestSetting ui;
    VisionTestCtrl* m_pCtrl;
};
