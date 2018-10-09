#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include "Inspect3DProfileWidget.h"
#include "qcustomplot.h"
#include "../Common/SystemData.h"
#include "TableCaliDataStorage.h"
#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"

#if defined(CreateWindow) // If Win32 defines "CreateWindow":
#undef CreateWindow       //   Undefine it to avoid conflict
#endif                    //   with the line display method.

#include "DataStoreAPI.h"
using namespace NFG::AOI;

#define ToInt(value)        (static_cast<int>(value))
#define ToFloat(param)      (static_cast<float>(param))

const int IMG_DISPLAY_WIDTH = 350;
const int IMG_DISPLAY_HEIGHT = 350;

class Inspect3DProfileWidget;
class DisplayScene : public QGraphicsScene
{
public:
    explicit DisplayScene(QObject *parent = 0);
    void setInspectWidget(Inspect3DProfileWidget* pInspectWidget) { m_pInspectWidget = pInspectWidget; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
private:
    Inspect3DProfileWidget* m_pInspectWidget;
};

DisplayScene::DisplayScene(QObject *parent)
    :QGraphicsScene(parent)
{
    clearFocus();
}

void DisplayScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    //qDebug() << event->scenePos().x() << " " << event->scenePos().y();
    if (m_pInspectWidget)
    {
        m_pInspectWidget->setInspectPos(event->scenePos().x(), event->scenePos().y());
    }
    QGraphicsScene::mousePressEvent(event);
}

Inspect3DProfileWidget::Inspect3DProfileWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    initUI();

    m_vecFrame3DHeights.resize(4);
    m_matHeights.resize(4);
    m_nTotalRow = 0;
    m_nTotalCol = 0;
}

Inspect3DProfileWidget::~Inspect3DProfileWidget()
{
}

void Inspect3DProfileWidget::initUI()
{
    // Image Display
    m_dlpImgScene1 = std::make_shared<DisplayScene>(this); 
    m_dlpImgScene1->setInspectWidget(this);
    ui.graphicsView_dlp1->setScene(m_dlpImgScene1.get());
    ui.graphicsView_dlp1->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_dlp1->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_dlp1->setFixedSize(IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT);
    //ui.graphicsView_dlp1->fitInView(QRectF(0, 0, IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT), Qt::KeepAspectRatio);    //这样就没法缩放了 
    //ui.graphicsView_dlp1->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    //ui.graphicsView_dlp1->setRenderHint(QPainter::Antialiasing);

    m_dlpImgScene2 = std::make_shared<DisplayScene>(this);
    m_dlpImgScene2->setInspectWidget(this);
    ui.graphicsView_dlp2->setScene(m_dlpImgScene2.get());
    ui.graphicsView_dlp2->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_dlp2->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_dlp2->setFixedSize(IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT);
    //ui.graphicsView_dlp2->fitInView(QRectF(0, 0, IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT), Qt::KeepAspectRatio);    //这样就没法缩放了 
    //ui.graphicsView_dlp2->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    //ui.graphicsView_dlp2->setRenderHint(QPainter::Antialiasing);

    m_dlpImgScene3 = std::make_shared<DisplayScene>(this);
    m_dlpImgScene3->setInspectWidget(this);
    ui.graphicsView_dlp3->setScene(m_dlpImgScene3.get());
    ui.graphicsView_dlp3->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_dlp3->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_dlp3->setFixedSize(IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT);
    //ui.graphicsView_dlp3->fitInView(QRectF(0, 0, IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT), Qt::KeepAspectRatio);    //这样就没法缩放了 
    //ui.graphicsView_dlp3->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    //ui.graphicsView_dlp3->setRenderHint(QPainter::Antialiasing);

    m_dlpImgScene4 = std::make_shared<DisplayScene>(this);
    m_dlpImgScene4->setInspectWidget(this);
    ui.graphicsView_dlp4->setScene(m_dlpImgScene4.get());
    ui.graphicsView_dlp4->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_dlp4->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_dlp4->setFixedSize(IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT);
    //ui.graphicsView_dlp4->fitInView(QRectF(0, 0, IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT), Qt::KeepAspectRatio);    //这样就没法缩放了 
    //ui.graphicsView_dlp4->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    //ui.graphicsView_dlp4->setRenderHint(QPainter::Antialiasing);

    ui.lineEdit_row->setText("0");
    ui.lineEdit_col->setText("0");
    ui.lineEdit_range->setText("1.0");
    ui.lineEdit_offset->setText("-1.0");
    connect(ui.pushButton_inspect, SIGNAL(clicked()), SLOT(onInspect()));

    m_pInspectRowPlot = std::make_shared<QCustomPlot>(this);
    setupPlotData(m_pInspectRowPlot, m_profRowDatas);
    ui.dockWidget_profChartRow->setWidget(m_pInspectRowPlot.get());

    m_pInspectColPlot = std::make_shared<QCustomPlot>(this);
    setupPlotData(m_pInspectColPlot, m_profColDatas);
    ui.dockWidget_profChartCol->setWidget(m_pInspectColPlot.get());
}

void Inspect3DProfileWidget::onInspect()
{
    doInspect();
}

bool Inspect3DProfileWidget::set3DHeight(QVector<cv::Mat>& matHeights, int nRow, int nCol, int nTotalRow, int nTotalCol)
{
    int ROWS = nTotalRow;
    int COLS = nTotalCol;
    int TOTAL = ROWS * COLS;

    if (m_nTotalRow < nTotalRow) m_nTotalRow = nTotalRow;
    if (m_nTotalCol < nTotalCol) m_nTotalCol = nTotalCol;

    int nImageNum = qMin(4, matHeights.size());
    for (int i = 0; i < nImageNum; i++)
    {
        if (m_vecFrame3DHeights[i].size() < TOTAL)
        {
            m_vecFrame3DHeights[i].resize(TOTAL, cv::Mat());
        }
    }
    m_matHeights.resize(matHeights.size());
  
    for (int i = 0; i < nImageNum; i++)
    {
        m_vecFrame3DHeights[i][nRow * COLS + nCol] = matHeights[i];
    } 
    //m_matHeights = matHeights;
    //System->setTrackInfo("Inspect 3D data loaded!");
    return true;
}

void Inspect3DProfileWidget::inspect(cv::Rect& rectROI)
{
    m_rectROI = rectROI;

    double dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    double dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();

    float fOverlapUmX = 0.f, fOverlapUmY = 0.f;
    Engine::GetParameter("ScanImageOverlapX", fOverlapUmX, 0.f);
    Engine::GetParameter("ScanImageOverlapY", fOverlapUmY, 0.f);

    Int32 nScanDirection = 0; Engine::GetParameter("ScanImageDirection", nScanDirection, 0);
    Vision::PR_SCAN_IMAGE_DIR enScanDir = static_cast<Vision::PR_SCAN_IMAGE_DIR>(nScanDirection);

    Vision::PR_COMBINE_IMG_CMD stCmd;
    Vision::PR_COMBINE_IMG_RPY stRpy;
    stCmd.nCountOfImgPerFrame = 1;
    stCmd.nCountOfFrameX = m_nTotalCol;
    stCmd.nCountOfFrameY = m_nTotalRow;
    stCmd.nOverlapX = fOverlapUmX / dResolutionX;
    stCmd.nOverlapY = fOverlapUmY / dResolutionY;
    stCmd.nCountOfImgPerRow = m_nTotalCol;
    stCmd.enScanDir = enScanDir;

    int nImageNum = qMin(4, m_matHeights.size());
    for (int i = 0; i < nImageNum; ++i) {
        stCmd.vecInputImages = m_vecFrame3DHeights[i];
        if (Vision::VisionStatus::OK == Vision::PR_CombineImg(&stCmd, &stRpy))
            m_matHeights[i] = stRpy.vecResultImages[0];
        else {
            System->setTrackInfo(QString(QStringLiteral("合并大图失败.")));
            return;
        }
    }  

    doInspect();
}

void Inspect3DProfileWidget::doInspect()
{
    int nRow = ui.lineEdit_row->text().toInt();
    int nCol = ui.lineEdit_col->text().toInt();

    int nImageNum = qMin(4, m_matHeights.size());
    std::shared_ptr<DisplayScene> scene[4] = { m_dlpImgScene1, m_dlpImgScene2, m_dlpImgScene3, m_dlpImgScene4 };
    for (int i = 0; i < nImageNum; i++)
    {
        cv::Mat matSourceImg = drawHeightGray(m_matHeights[i](m_rectROI));
        cv::cvtColor(matSourceImg, matSourceImg, CV_BGR2RGB);

        cv::Point2f ptInt1, ptInt2;
        ptInt1.x = nCol; ptInt1.y = 0;
        ptInt2.x = nCol; ptInt2.y = matSourceImg.rows;
        cv::line(matSourceImg, ptInt1, ptInt2, cv::Scalar(255, 255, 0), 5);
        ptInt1.x = 0; ptInt1.y = nRow;
        ptInt2.x = matSourceImg.cols; ptInt2.y = nRow;
        cv::line(matSourceImg, ptInt1, ptInt2, cv::Scalar(255, 255, 0), 5);

        QImage img = QImage((uchar*)matSourceImg.data, matSourceImg.cols, matSourceImg.rows, ToInt(matSourceImg.step), QImage::Format_RGB888);
        scene[i]->clear();
        scene[i]->addPixmap(QPixmap::fromImage(img.scaled(QSize(IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT))));
    }

    generateProfData(true, nRow, m_matHeights, m_profRowDatas);
    generateProfData(false, nCol, m_matHeights, m_profColDatas);

    if (m_pInspectRowPlot)
    {
        m_pInspectRowPlot = std::make_shared<QCustomPlot>(this);
        setupPlotData(m_pInspectRowPlot, m_profRowDatas);
        ui.dockWidget_profChartRow->setWidget(m_pInspectRowPlot.get());
    }

    if (m_pInspectColPlot)
    {
        m_pInspectColPlot = std::make_shared<QCustomPlot>(this);
        setupPlotData(m_pInspectColPlot, m_profColDatas);
        ui.dockWidget_profChartCol->setWidget(m_pInspectColPlot.get());
    }
}

void Inspect3DProfileWidget::setInspectPos(double dX, double dY)
{
    if (m_matHeights.size() > 0)
    {
        cv::Mat matImg = m_matHeights[0](m_rectROI);
        int nMatRow = matImg.rows;
        int nMatCol = matImg.cols;

        int nRow = ToInt(dY * nMatRow / IMG_DISPLAY_HEIGHT);
        int nCol = ToInt(dX * nMatCol / IMG_DISPLAY_WIDTH);

        if (nRow < 0 || nRow >= nMatRow) nRow = 0;
        if (nCol < 0 || nCol >= nMatCol) nCol = 0;

        ui.lineEdit_row->setText(QString("%1").arg(nRow));
        ui.lineEdit_col->setText(QString("%1").arg(nCol));

        doInspect();
    }   
}
   

cv::Mat Inspect3DProfileWidget::drawHeightGray(const cv::Mat &matHeight)
{
    double dMinValue = 0, dMaxValue = 0;
    cv::Mat matMask = matHeight == matHeight;
    cv::minMaxIdx(matHeight, &dMinValue, &dMaxValue, 0, 0, matMask);

    cv::Mat matNewPhase = matHeight - dMinValue;

    float dRatio = 255.f / ToFloat(dMaxValue - dMinValue);
    matNewPhase = matNewPhase * dRatio;

    cv::Mat matResultImg;
    matNewPhase.convertTo(matResultImg, CV_8UC1);
    cv::cvtColor(matResultImg, matResultImg, CV_GRAY2BGR);

    return matResultImg;
}

void Inspect3DProfileWidget::generateProfData(bool bRow, int nIndex, QVector<cv::Mat>& matHeights, InspectProfDataVector& profData)
{
    profData.clear();

    if (bRow && (nIndex >= m_rectROI.height)) return;
    if (!bRow && (nIndex >= m_rectROI.width)) return;

    int nSlopNum = matHeights.size();

    bool bAutoCompOffset = ui.checkBoxAutoCompOffset->isChecked();

    for (int i = 0; i < nSlopNum; i++)
    {
        QVector<cv::Point2d> points;

        cv::Mat matImg = matHeights[i](m_rectROI);
        int nRow = matImg.rows;
        int nCol = matImg.cols;       

        int nPtNum = bRow ? nCol : nRow;
        for (int j = 0; j < nPtNum; j++)
        {
            double dHeight = 0.0;
           
            if (bRow)
            {                
                dHeight = matImg.at<float>(nIndex, j);
            }
            else
            {
               dHeight = matImg.at<float>(j, nIndex);
            } 

            if (bAutoCompOffset)
            {
                cv::Point2f ptPos;
                if (bRow)
                { 
                    ptPos.x = m_rectROI.x + nIndex;
                    ptPos.y = m_rectROI.y + j;
                }
                else 
                {
                    ptPos.x = m_rectROI.x + j;
                    ptPos.y = m_rectROI.y + nIndex;
                }

                float offsets[4];
                TableCalData->getFrameOffsetByPixel(ptPos, offsets);

                dHeight += offsets[i];
            }

            cv::Point2d pt;
            pt.x = j;
            pt.y = dHeight;
            points.push_back(pt);
        }

        profData.push_back(points);
    }
}

void Inspect3DProfileWidget::setupPlotData(std::shared_ptr<QCustomPlot> customPlot, InspectProfDataVector& profData)
{
    float fRange = ui.lineEdit_range->text().toFloat();
    float fOffset = ui.lineEdit_offset->text().toFloat();

    customPlot->legend->setVisible(true);
    customPlot->legend->setFont(QFont("Helvetica", 9));

    QPen pen;
    int nSlopNum = profData.size();
    // add graphs with different line styles:
    for (int i = 0; i < nSlopNum; i++)
    {
        customPlot->addGraph();
        pen.setColor(QColor(qSin(i * 3 + 1.2) * 80 + 80, qSin(i * 1 + 0) * 80 + 80, qSin(i*0.3 + 1.5) * 80 + 80));
        customPlot->graph()->setPen(pen);
        customPlot->graph()->setName(QString("DLP%1").arg(i+1));
        customPlot->graph()->setLineStyle(QCPGraph::lsLine);
        customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 1));
        // generate data:
        int nDataNum = profData[i].size();
        QVector<double> x(nDataNum), y(nDataNum);
        for (int j = 0; j < nDataNum; ++j)
        {
            x[j] = profData[i][j].x;
            y[j] = profData[i][j].y;
        }
        customPlot->graph()->setData(x, y);
        customPlot->graph()->rescaleAxes(true);
    }

    // zoom out a bit:
    customPlot->yAxis->scaleRange(fRange / 2.5, customPlot->yAxis->range().center()*(fOffset/2.5));
    customPlot->xAxis->scaleRange(1.1, customPlot->xAxis->range().center());
    // set blank axis lines:
    customPlot->xAxis->setTicks(true);
    customPlot->yAxis->setTicks(true);
    customPlot->xAxis->setTickLabels(true);
    customPlot->yAxis->setTickLabels(true);
    // make top right axes clones of bottom left axes:
    customPlot->axisRect()->setupFullAxesBox();
}
