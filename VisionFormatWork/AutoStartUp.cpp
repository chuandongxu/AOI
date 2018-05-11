#include "AutoStartUp.h"

#include <QDir>
#include <QApplication>
#include <qthread.h>

#include "../Common/ModuleMgr.h"
#include "../Common/SystemData.h"
#include "../include/IMotion.h"
#include "../include/IData.h"
#include "../include/IDlp.h"
#include "../include/ICamera.h"
#include "../include/IVision.h"
#include "../include/IVisionUI.h"
#include "../include/ILight.h"
#include "../include/IdDefine.h"

AutoStartUp::AutoStartUp(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    ui.tableView_Loading->setModel(&m_dataModel);

    ui.tableView_Loading->setColumnWidth(0, 220);
    ui.tableView_Loading->setColumnWidth(1, 80);
    ui.tableView_Loading->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);   
    ui.tableView_Loading->horizontalHeader()->setStretchLastSection(false);
    ui.tableView_Loading->setStyleSheet("background-color:rgba(244,244,244,200)");

    connect(&m_dataModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)),
        this, SLOT(onDataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)));

    QStringList ls;
    ls << QStringLiteral("名称") << QStringLiteral("状态");
    m_dataModel.setHorizontalHeaderLabels(ls);

    setWindowFlags(windowFlags()&~(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint));
    setFixedSize(800, 800);

    setAutoFillBackground(true);

    QString path = QApplication::applicationDirPath();
    path += "/image/";

    QPalette pal;
    QString filename = path + "loading.jpg";
    QPixmap pixmap(filename);
    pal.setBrush(QPalette::Window, QBrush(pixmap));
    setPalette(pal);
}

AutoStartUp::~AutoStartUp()
{
}

void AutoStartUp::onDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> & roles)
{
    QAbstractItemModel * model = ui.tableView_Loading->model();
    if (!model)return;

    int row = model->rowCount();
    ui.tableView_Loading->scrollTo(model->index(row - 1, 1), QAbstractItemView::PositionAtBottom);

}

void AutoStartUp::initStartUp(int nStepCount)
{
    m_nStepCount = nStepCount;
    ui.progressBar_loading->setValue(0);
}

void AutoStartUp::insertMsg(const QString& msg, bool bOK)
{
    int nr = m_dataModel.rowCount();
    m_dataModel.insertRow(nr);

    m_dataModel.setData(m_dataModel.index(nr, 0), msg);
    m_dataModel.setData(m_dataModel.index(nr, 1), bOK ? QStringLiteral("OK") : QStringLiteral("FAIL"));
    m_dataModel.item(nr, 1)->setForeground(QBrush(bOK ? QColor(0, 128, 0) : QColor(255, 0, 0)));

    int nProgressValue = 100.0 * (nr + 1) / m_nStepCount;
    ui.progressBar_loading->setValue(nProgressValue);
}

void AutoStartUp::showProgress()
{
    if (System->isRunOffline()) return;

    initStartUp(5);

    this->show();

    IMotion * pMotion = getModule<IMotion>(MOTION_MODEL);
    if (pMotion)
    {
        bool bStartUpEnable = System->getParam("auto_startup_home_enable").toBool();
        if (bStartUpEnable)
        {            
            System->userGoHome();
            insertMsg(QStringLiteral("系统回零"), true);
        }

        bStartUpEnable = System->getParam("auto_startup_zready_enable").toBool();
        if (bStartUpEnable)
        {
            int nZReadyID = System->getParam("auto_startup_zready_id").toInt();
            bool bRet = pMotion->moveToPos(nZReadyID, true);
            if (!bRet)
            {
                System->setTrackInfo("move to Z Ready position error");               
            }
            insertMsg(QStringLiteral("运行到检查高度"), bRet);
        }
    }

    IData* pData = getModule<IData>(DATA_MODEL);
    if (pData)
    {
        bool bStartUpEnable = System->getParam("auto_startup_loaddb_enable").toBool();
        if (bStartUpEnable)
        {
            QString szDBPath = System->getParam("auto_startup_db_path").toString();
            pData->openProject(szDBPath);

            insertMsg(QStringLiteral("成功导入运行数据"), true);
        }
    }


    IDlp* pDlp = getModule<IDlp>(DLP_MODEL);
    if (pDlp)
    {
        bool bStartUpEnable = System->getParam("auto_startup_dlp_enable").toBool();
        if (bStartUpEnable)
        {
            bool bDLPInitOK = true;
            int nStationNum = System->getParam("motion_trigger_dlp_num_index").toInt() == 0 ? 2 : 4;
            for (int i = 0; i < nStationNum; ++i) {
                if (pDlp->isConnected(i)) {
                    if (!pDlp->startUpCapture(i))
                    {
                        bDLPInitOK = false;
                        continue;
                    }                       
                }
                else
                    System->setTrackInfo(QString(QStringLiteral("工位%0启动失败, 请检查DLP硬件！")).arg(i + 1));
            }

            insertMsg(QStringLiteral("DLP模块初始化完成"), bDLPInitOK);
        }
    }

    ILight* pLight = getModule<ILight>(LIGHT_MODEL);
    if (pLight)
    {
        pLight->init();
        insertMsg(QStringLiteral("成功启动光源控制器"), true);
    }
    
    QThread::msleep(1000);
    this->hide();
}
