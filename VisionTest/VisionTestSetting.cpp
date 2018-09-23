#include "VisionTestSetting.h"

#include <QMessageBox>
#include <QFileDialog>

#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../Common/SystemData.h"
#include "../include/ICamera.h"


VisionTestSetting::VisionTestSetting(VisionTestCtrl* pCtrl, QWidget *parent)
    : m_pCtrl(pCtrl), QMainWindow(parent)
{
    ui.setupUi(this);

    initUI();
}

VisionTestSetting::~VisionTestSetting()
{
}

void VisionTestSetting::initUI()
{
    setAttribute(Qt::WA_DeleteOnClose);

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();    

    setUnifiedTitleAndToolBarOnMac(true);
}

void VisionTestSetting::createActions()
{
    newAct = new QAction(QIcon("image/newFile.png"), QStringLiteral("新建"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    openAct = new QAction(QIcon("image/openFile.png"), QStringLiteral("打开..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(openFile()));

    saveAct = new QAction(QIcon("image/saveFile.png"), QStringLiteral("保存"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(saveFile()));

    saveAsAct = new QAction(QIcon("image/saveAsFile.png"), QStringLiteral("另存为..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAsFile()));

    runOnceAct = new QAction(QIcon("image/runOnce.png"), QStringLiteral("单步"), this);
    runOnceAct->setShortcuts(QKeySequence::ZoomIn);
    runOnceAct->setStatusTip(tr("Run Once"));
    connect(runOnceAct, SIGNAL(triggered()), this, SLOT(runOnce()));

    runOverAct = new QAction(QIcon("image/runOver.png"), QStringLiteral("运行"), this);
    runOverAct->setShortcuts(QKeySequence::ZoomOut);
    runOverAct->setStatusTip(tr("Continuous Run"));
    connect(runOverAct, SIGNAL(triggered()), this, SLOT(runOver()));

    stopAct = new QAction(QIcon("image/stop.png"), QStringLiteral("停止"), this);
    stopAct->setShortcuts(QKeySequence::FullScreen);
    stopAct->setStatusTip(tr("Stop"));
    connect(stopAct, SIGNAL(triggered()), this, SLOT(stop()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutSystemAct = new QAction(tr("About System"), this);
    aboutSystemAct->setStatusTip(tr("Show the System About box"));
    connect(aboutSystemAct, SIGNAL(triggered()), this, SLOT(aboutSystem()));
}

//! [implicit tr context]
void VisionTestSetting::createMenus()
{
    ui.mainToolBar->setVisible(false);

    fileMenu = menuBar()->addMenu(QStringLiteral("文件"));
    //! [implicit tr context]
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();

    editMenu = menuBar()->addMenu(QStringLiteral("操作"));
    editMenu->addAction(runOnceAct);
    editMenu->addAction(runOverAct);
    editMenu->addAction(stopAct);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutSystemAct);
}

void VisionTestSetting::createToolBars()
{
    //! [0]
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newAct);
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);
    fileToolBar->addAction(saveAsAct);

    editToolBar = addToolBar(tr("Operation"));
    editToolBar->addAction(runOnceAct);
    editToolBar->addAction(runOverAct);
    editToolBar->addAction(stopAct);
}

void VisionTestSetting::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void VisionTestSetting::newFile()
{
    //QString mapPath = QApplication::applicationDirPath();
    //mapPath += "/map/mapData" + QDateTime::currentDateTime().toString("MMddhhmmss") + ".xml";
}

void VisionTestSetting::openFile()
{
    /*QString fileName = QFileDialog::getOpenFileName(this, "Open Map File", pMap->getMapPath());
    if (!fileName.isEmpty())
    {
        pMap->clearMapInfo();
        pMap->loadMapInfo(fileName);
    }*/
}

void VisionTestSetting::saveFile()
{
    //if (!m_pCtrl->getMapPath().isEmpty())
    //{
    //    m_pCtrl->saveMapInfo(m_pCtrl->getMapPath(), false);
    //}
}

void VisionTestSetting::saveAsFile()
{
    //QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
    //    m_pCtrl->getMapPath());
    //if (fileName.isEmpty())
    //{
    //    return;
    //}

    //if (!fileName.isEmpty())
    //{
    //    m_pCtrl->saveMapInfo(fileName, true);
    //}
}

void VisionTestSetting::runOnce()
{    
}

void VisionTestSetting::runOver()
{    
}

void VisionTestSetting::stop()
{
}

void VisionTestSetting::about()
{
    QMessageBox::about(this, tr("About Vision Test"),
        tr("The <b>Tester</b> example demonstrates how to test vision "
        "detect procedure automatic using system."));
}

void VisionTestSetting::aboutSystem()
{
    QMessageBox::about(this, tr("About Vision Test"),
        tr("The <b>Tester</b> example demonstrates how to test vision "
        "detect procedure automatic using system."));
}
