#include <QMessageBox>

#include "InspWindowWidget.h"
#include "../include/IVisionUI.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"
#include "../Common/SystemData.h"
#include "DataStoreAPI.h"
#include "../DataModule/QDetectObj.h"

using namespace NFG::AOI;

InspWindowWidget::InspWindowWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    m_pInspWindowSelectWidget = new InspWindowSelectWidget(this);
    //m_pInspWindowSelectWidget->setParent(this);
    ui.stackedWidget->addWidget ( m_pInspWindowSelectWidget );

    m_pFindLineWidget = new FindLineWidget(this);
    ui.stackedWidget->addWidget ( m_pFindLineWidget );

    m_pInspVoidWidget = new InspVoidWidget(this);
    ui.stackedWidget->addWidget ( m_pInspVoidWidget );    
}

InspWindowWidget::~InspWindowWidget()
{
}

void InspWindowWidget::setCurrentIndex(int index)
{
    ui.stackedWidget->setCurrentIndex ( index );
}

void InspWindowWidget::showEvent(QShowEvent *event)
{
    IVisionUI* pUI = getModule<IVisionUI>(UI_MODEL);
    pUI->setViewState(VISION_VIEW_MODE::MODE_VIEW_SELECT_ROI);

    Engine::WindowVector vecWindow;
    auto result = Engine::GetAllWindows ( vecWindow );
    if (Engine::OK != result) {
        String errorType, errorMessage;
        Engine::GetErrorDetail(errorType, errorMessage);
        errorMessage = "Failed to get inspect windows from database, error message " + errorMessage;
        QMessageBox::critical(nullptr, QStringLiteral("Inspect Window"), errorMessage.c_str(), QStringLiteral("Quit"));
    }

    double dResolutionX, dResolutionY;
    bool bBoardRotated = false;
    dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();
    bBoardRotated = System->getSysParam("BOARD_ROTATED").toBool();    

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
}

void InspWindowWidget::on_btnReturn_clicked()
{
    ui.stackedWidget->setCurrentIndex ( INSP_WINDOW_SELECT );
}