#include <QMessageBox>

#include "InspWindowWidget.h"
#include "../include/IVisionUI.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../Common/SystemData.h"

#include "../DataModule/QDetectObj.h"
#include "../Common/eos.h"
#include "InspWindowSelectDialog.h"
#include "FindLineWidget.h"

InspWindowWidget::InspWindowWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    m_arrInspWindowWidget[static_cast<int>(INSP_WIDGET_INDEX::FIND_LINE)] = std::make_unique<FindLineWidget>(this);
    m_arrInspWindowWidget[static_cast<int>(INSP_WIDGET_INDEX::INSP_HOLE)] = std::make_unique<InspVoidWidget>(this);

    for ( const auto &ptrInspWindowWidget : m_arrInspWindowWidget )
        ui.stackedWidget->addWidget ( ptrInspWindowWidget.get() );

    QEos::Attach(EVENT_INSP_WINDOW_STATE, this, SLOT(onInspWindowState(const QVariantList &)));

    connect(ui.listWindowWidget, SIGNAL(currentRowChanged(int)), this, SLOT(onSelectedWindowChanged(int)));
}

InspWindowWidget::~InspWindowWidget()
{
}

void InspWindowWidget::setCurrentIndex(int index)
{
    ui.stackedWidget->setCurrentIndex ( index );
}

void InspWindowWidget::UpdateInspWindowList()
{
    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    
    auto result = Engine::GetDeviceWindows ( pUI->getSelectedDevice().getId(), m_vecCurrentDeviceWindows );
    if ( result != Engine::OK ) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        errorMessage = "Failed to get inspect windows from database, error message " + errorMessage;
        QMessageBox::critical(nullptr, QStringLiteral("Inspect Window"), errorMessage.c_str(), QStringLiteral("Quit"));
    }

    ui.listWindowWidget->clear();
    for ( const auto &window : m_vecCurrentDeviceWindows )
        ui.listWindowWidget->addItem(window.name.c_str());

    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_INSP_WINDOW);
}

void InspWindowWidget::showEvent(QShowEvent *event)
{
    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_EDIT_INSP_WINDOW);

    Engine::WindowVector vecWindow;
    auto result = Engine::GetAllWindows ( vecWindow );
    if (Engine::OK != result) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        errorMessage = "Failed to get inspect windows from database, error message " + errorMessage;
        QMessageBox::critical(nullptr, QStringLiteral("Inspect Window"), errorMessage.c_str(), QStringLiteral("Quit"));
    }

    double dResolutionX, dResolutionY;
    dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();   

    QVector<QDetectObj> vecDetectObjs;
    for ( const auto &window : vecWindow ) {
        auto x = window.x / dResolutionX;
        auto y = window.y / dResolutionY;
        auto width  = window.width  / dResolutionX;
        auto height = window.height / dResolutionY;
        cv::RotatedRect detectObjWin ( cv::Point2f(x, y), cv::Size2f(width, height), window.angle );
        QDetectObj detectObj ( window.Id, window.name.c_str() );
        detectObj.setFrame ( detectObjWin );
        vecDetectObjs.push_back ( detectObj );
    }
    
    pUI->setDetectObjs( vecDetectObjs );

    ui.stackedWidget->hide();
}

void InspWindowWidget::on_btnAddWindow_clicked()
{
    InspWindowSelectDialog dialog;
    int iReturn = dialog.exec();
    if ( iReturn != QDialog::Accepted )
        return;

    ui.stackedWidget->show();
    m_enCurrentInspWidget = dialog.getWindowIndex();
    ui.stackedWidget->setCurrentIndex ( static_cast<int> ( m_enCurrentInspWidget ) );

    m_enOperation = OPERATION::ADD;

    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_SELECT_ROI);
}

void InspWindowWidget::on_btnRemoveWindow_clicked()
{
    auto index = ui.listWindowWidget->currentIndex().row();
    if ( index >= 0 ) {
        auto windowId = m_vecCurrentDeviceWindows[index].Id;
        Engine::DeleteWindow ( windowId );
        ui.listWindowWidget->takeItem ( index );

        UpdateInspWindowList();

        IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
        auto vecDetectObjs = pUI->getDetectObjs();
        auto it = std::remove_if ( vecDetectObjs.begin(), vecDetectObjs.end(), [windowId](QDetectObj &detectObj) { return detectObj.getID() == windowId; } );
        vecDetectObjs.erase (it);
        pUI->setDetectObjs ( vecDetectObjs );
    }
}

void InspWindowWidget::on_btnTryInsp_clicked()
{
    m_arrInspWindowWidget[static_cast<int>(m_enCurrentInspWidget)]->tryInsp();
}

void InspWindowWidget::on_btnConfirmWindow_clicked()
{
    m_arrInspWindowWidget[static_cast<int>(m_enCurrentInspWidget)]->confirmWindow(m_enOperation);
}

void InspWindowWidget::onInspWindowState(const QVariantList &data)
{
    UpdateInspWindowList();
    if ( ui.listWindowWidget->count() > 0 ) {
        ui.listWindowWidget->item(0)->setSelected(true);
        onSelectedWindowChanged(0);
    }
}

void InspWindowWidget::onSelectedWindowChanged(int index)
{
    if ( index < 0 )
        return;
    ui.stackedWidget->show();
    if ( Engine::Window::Usage::FIND_LINE == m_vecCurrentDeviceWindows[index].usage )
        m_enCurrentInspWidget = INSP_WIDGET_INDEX::FIND_LINE;
    else if ( Engine::Window::Usage::INSP_HOLE == m_vecCurrentDeviceWindows[index].usage )
        m_enCurrentInspWidget = INSP_WIDGET_INDEX::INSP_HOLE;

    m_arrInspWindowWidget[static_cast<int>(m_enCurrentInspWidget)]->setCurrentWindow( m_vecCurrentDeviceWindows[index] );
    ui.stackedWidget->setCurrentIndex ( static_cast<int>(m_enCurrentInspWidget) );

    auto window = m_vecCurrentDeviceWindows[index];
    double dResolutionX, dResolutionY;
    dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    auto x = window.x / dResolutionX;
    auto y = window.y / dResolutionY;
    auto width = window.width / dResolutionX;
    auto height = window.height / dResolutionY;
    cv::RotatedRect detectObjWin(cv::Point2f(x, y), cv::Size2f(width, height), window.angle);
    QDetectObj detectObj(window.Id, window.name.c_str());
    detectObj.setFrame(detectObjWin);

    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setCurrentDetectObj ( detectObj );
    m_enOperation = OPERATION::EDIT;
}