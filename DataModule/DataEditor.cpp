#include "DataEditor.h"

#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"

#include "../Common/SystemData.h"
#include "DataStoreAPI.h"
#include "VisionAPI.h"
#include "../include/IVisionUI.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#define ToInt(value)        (static_cast<int>(value))
#define ToFloat(param)      (static_cast<float>(param))

using namespace NFG::AOI;
using namespace AOI;

const int IMG_DISPLAY_WIDTH = 150;
const int IMG_DISPLAY_HEIGHT = 100;

DataEditor::DataEditor(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    initUI();
}

DataEditor::~DataEditor()
{
}

void DataEditor::initUI()
{
    // Image Display
    m_recordImgScene = new QGraphicsScene();
    ui.graphicsView_recordImg->setScene(m_recordImgScene);
    ui.graphicsView_recordImg->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_recordImg->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_recordImg->fitInView(QRectF(0, 0, IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT), Qt::KeepAspectRatio);    //这样就没法缩放了 
    ui.graphicsView_recordImg->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    ui.graphicsView_recordImg->setRenderHint(QPainter::Antialiasing);
}

bool DataEditor::displayRecord(int recordID)
{
    cv::Mat matSourceImg;
    if (recordID <= 0)
    {
        cv::Mat matImg = cv::Mat::ones(cv::Size(IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT), CV_8UC3);
        matSourceImg = matImg;
    }
    else
    {
        Vision::PR_GET_RECORD_INFO_RPY stRpy;
        Vision::PR_GetRecordInfo(recordID, &stRpy);
        if (Vision::VisionStatus::OK != stRpy.enStatus)
        {
            return false;
        }
       
        if (stRpy.matImage.type() == CV_8UC3)
        {
            cv::cvtColor(stRpy.matImage, matSourceImg, CV_BGR2RGB);
        }
        else if (stRpy.matImage.type() == CV_8UC1)
        {
            cv::cvtColor(stRpy.matImage, matSourceImg, CV_GRAY2RGB);
        }
    }   
   
    QImage img = QImage((uchar*)matSourceImg.data, matSourceImg.cols, matSourceImg.rows, ToInt(matSourceImg.step), QImage::Format_RGB888);
    m_recordImgScene->clear();
    m_recordImgScene->addPixmap(QPixmap::fromImage(img.scaled(QSize(IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT))));

    return true;
}
