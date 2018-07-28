#include "InspMaskEditorWidget.h"
#include "../Common/SystemData.h"
#include "../include/IdDefine.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QMath.h>
#include  <QMouseEvent>
#include  <QWheelEvent>

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "../lib/VisionLibrary/include/VisionAPI.h"

#define ToInt(value)                (static_cast<int>(value))

const int LABEL_IMAGE_WIDTH = 800;
const int LABEL_IMAGE_HEIGHT = 800;

const int MODE_VIEW_NONE = 1;
const int MODE_VIEW_SELECT = 2;
const int MODE_VIEW_ADD_SELECT = 3;
const int MODE_VIEW_ADD_MASK = 4;

using namespace AOI;

bool RectMask::IsPointIn(const cv::Point2f& pt)
{
    cv::Point2f vertices[4];
    _rect.points(vertices);

    std::vector<cv::Point2f> vecTmp;
    for (int j = 0; j < 4; j++)
    {
        vecTmp.push_back(vertices[j]);
    }

    cv::Mat matContour = cv::Mat(vecTmp, true);
    double dDist = pointPolygonTest(matContour, pt, 0);

    if (dDist > 0)
    {
        return true;
    }

    return false;
}

bool CircleMask::IsPointIn(const cv::Point2f& pt)
{
    double fDistance = qSqrt(qPow(pt.x - _center.x, 2) + qPow(pt.y - _center.y, 2));
    return fDistance <= _radius;
}

bool PolyLineMask::IsPointIn(const cv::Point2f& pt)
{
    cv::Mat matContour = cv::Mat(_polyPts, true);
    double dDist = pointPolygonTest(matContour, pt, 0);

    if (dDist > 0)
    {
        return true;
    }

    return false;
}

void RectMask::Move(const cv::Point2f& dist, double imgWidth, double imgHeight)
{
    cv::Point2f vertices[4];
    _rect.points(vertices);
    for each (auto pt in vertices)
    {
        pt += dist;
        if (pt.x < 0 || pt.x >= imgWidth || pt.y < 0 || pt.y >= imgHeight)
        {
            return;
        }
    }

    _rect.center += dist;
}

void CircleMask::Move(const cv::Point2f& dist, double imgWidth, double imgHeight)
{
    auto centerTmp = _center + dist;
    if ((centerTmp.x - _radius) < 0 || (centerTmp.x + _radius) >= imgWidth || (centerTmp.y - _radius) < 0 || (centerTmp.y + _radius) >= imgHeight)
    {
        return;
    }
    _center += dist;
}

void PolyLineMask::Move(const cv::Point2f& dist, double imgWidth, double imgHeight)
{
    for each (auto pt in _polyPts)
    {
        pt += dist;
        if (pt.x < 0 || pt.x >= imgWidth || pt.y < 0 || pt.y >= imgHeight)
        {
            return;
        }
    }

    for(auto& pt : _polyPts)
    {
        pt += dist;
    }
}

InspMaskEditorWidget::InspMaskEditorWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    //setWindowFlags(Qt::Dialog);
    //setWindowModality(Qt::ApplicationModal);

    m_stateView = MODE_VIEW_SELECT;
    m_windowWidth = LABEL_IMAGE_WIDTH;
    m_windowHeight = LABEL_IMAGE_HEIGHT;
    m_startX = 0;
    m_startY = 0;
    m_preMoveX = 0;
    m_preMoveY = 0;
    m_imageHeight = 1;
    m_imageWidth = 1;
    m_mouseLeftPressed = false;
    m_mouseRightPressed = false;

    m_nSelectIndex = -1;

    initUI();
}

InspMaskEditorWidget::~InspMaskEditorWidget()
{ 
    for (int i = 0; i < m_maskObjs.size(); i++)
    {
        delete m_maskObjs[i];
    }
    m_maskObjs.clear();
}

void InspMaskEditorWidget::closeEvent(QCloseEvent *e){
    //qDebug() << "关闭事件";
    //e->ignore();

    this->hide();
}

void InspMaskEditorWidget::initUI()
{
    connect(ui.comboBox_maskIndex, SIGNAL(currentIndexChanged(int)), SLOT(onMaskIndexChanged(int)));
    connect(ui.pushButton_addMask, SIGNAL(clicked()), SLOT(onAddMask()));
    connect(ui.pushButton_deleteMask, SIGNAL(clicked()), SLOT(onDeleteMask()));

    connect(ui.pushButton_editDone, SIGNAL(clicked()), SLOT(onEditDone()));

    ui.comboBox_maskIndex->addItem(QString("%1").arg(QStringLiteral("请选择区域")));

    ui.radioButton_rect->setChecked(true);
    ui.radioButton_circle->setChecked(false);
    ui.radioButton_polyline->setChecked(false);

    ui.label_Img->setFixedSize(LABEL_IMAGE_WIDTH, LABEL_IMAGE_HEIGHT);
    ui.label_Img->setMouseTracking(true);
    setMouseTracking(true);    
}

void InspMaskEditorWidget::setImage(cv::Mat& matImage, bool bClearAll)
{
    if (bClearAll) clear();

    if (!matImage.empty())
    {
        m_hoImage = matImage.clone();
        m_imageWidth = m_hoImage.size().width;
        m_imageHeight = m_hoImage.size().height;

        displayImage(m_hoImage);
    }
}

cv::Mat InspMaskEditorWidget::getMaskImage()
{
    cv::Mat matImage(m_hoImage.rows, m_hoImage.cols, CV_8UC1, cv::Scalar(1));

    for (int i = 0; i < m_maskObjs.size(); i++)
    {
        MaskObj* obj = m_maskObjs[i];
        switch (obj->getType())
        {
        case MASK_TYPE_RECT:
        {
            RectMask* maskObj = dynamic_cast<RectMask*>(obj);
            if (maskObj && !maskObj->isEmpty())
            {
                cv::Point2f vertices[4];
                maskObj->_rect.points(vertices);

                int x = vertices[1].x;
                int y = vertices[1].y;
                int width = vertices[3].x - vertices[1].x;
                int height = vertices[3].y - vertices[1].y;

                rectangle(matImage, vertices[1], vertices[3], cv::Scalar(0), -1);
            }
        }
        break;
        case MASK_TYPE_CIRCLE:
        {
            CircleMask* maskObj = dynamic_cast<CircleMask*>(obj);
            if (maskObj && !maskObj->isEmpty())
            {
                circle(matImage, maskObj->_center, maskObj->_radius, cv::Scalar(0), -1);
            }
        }
        break;
        case MASK_TYPE_POLYLINE:
        {
            PolyLineMask* maskObj = dynamic_cast<PolyLineMask*>(obj);
            if (maskObj && !maskObj->isEmpty() && maskObj->_polyPts.size() > 2)
            {
                cv::Point *points = new cv::Point[maskObj->_polyPts.size()];
                for (int i = 0; i < maskObj->_polyPts.size(); i++)
                {
                    points[i].x = maskObj->_polyPts[i].x;
                    points[i].y = maskObj->_polyPts[i].y;
                }

                const cv::Point* pt[1] = { points };
                int npt[1] = { maskObj->_polyPts.size() };

                polylines(matImage, pt, npt, 1, 1, cv::Scalar(0));
                fillPoly(matImage, pt, npt, 1, cv::Scalar(0), 8);

                delete points;
            }
        }
        break;
        case MASK_TYPE_NULL:
            break;
        default:
            break;
        }
    }

    return matImage;
}

void InspMaskEditorWidget::mouseMoveEvent(QMouseEvent * event)
{
    double mouseX = event->x(), mouseY = event->y();

    double motionX = 0, motionY = 0;
    motionX = mouseX - m_startX;
    motionY = mouseY - m_startY;

    if (m_mouseLeftPressed)
    {
        switch (m_stateView)
        {
        case MODE_VIEW_SELECT:
        {
            if (m_nSelectIndex > -1)
            {
                //if (findSelect(mouseX, mouseY) == m_nSelectIndex)
                {
                    cv::Point2f moveDist;
                    moveDist.x += convertToImgX(mouseX - m_preMoveX, true);
                    moveDist.y += convertToImgY(mouseY - m_preMoveY, true);

                    auto obj = m_maskObjs[m_nSelectIndex];
                    obj->Move(moveDist, m_imageWidth, m_imageHeight);

                    repaintAll();
                }
            }
        }
        break;       
        case MODE_VIEW_ADD_MASK:
        {
            if ((0 != (int)motionX) || (0 != (int)motionY))
            {
                if ((int)motionX >= 0 && (int)motionY >= 0)
                {
                    double imageStartPosX = convertToImgX(m_startX);
                    double imageStartPosY = convertToImgY(m_startY);
                    if (imageStartPosX > 0 && imageStartPosX < m_imageWidth && imageStartPosY > 0 && imageStartPosY < m_imageHeight)
                    {
                        cv::Rect select;
                        select.x = convertToImgX(m_startX);
                        select.y = convertToImgY(m_startY);
                        select.width = convertToImgX(motionX, true);
                        select.height = convertToImgY(motionY, true);

                        if (select.width > 10 && (select.x + select.width) < m_imageWidth && select.height > 10 && (select.y + select.height) < m_imageHeight)
                        {
                            MaskType maskType = getMaskType();
                            switch (maskType)
                            {
                            case MASK_TYPE_RECT:
                            {
                                cv::Point2f slCenter;
                                cv::Size2f slSize;
                                slCenter.x = select.x + select.width / 2;
                                slCenter.y = select.y + select.height / 2;
                                slSize.width = select.width;
                                slSize.height = select.height;
                                m_maskRectCur._rect = cv::RotatedRect(slCenter, slSize, 0);
                            }
                            break;
                            case MASK_TYPE_CIRCLE:
                            {
                                cv::Point2f maskCenter;
                                maskCenter.x = select.x + select.width / 2;
                                maskCenter.y = select.y + select.height / 2;

                                double dRadius = select.width / 2 < select.height / 2 ? select.width / 2 : select.height / 2;

                                m_maskCircleCur._center = maskCenter;
                                m_maskCircleCur._radius = dRadius;
                            }
                            break;
                            case MASK_TYPE_POLYLINE:
                                break;
                            case MASK_TYPE_NULL:
                                break;
                            default:
                                break;
                            }

                            repaintAll();
                        }
                    }
                }
            }
            break;
        }
        case MODE_VIEW_NONE:
            break;
        default:
            break;
        }
    }
    else if (m_mouseRightPressed)
    {
    }
    else
    {
        switch (m_stateView)
        {
        case MODE_VIEW_SELECT:
        {           
        }
        break;
        case MODE_VIEW_ADD_MASK:
        {
            MaskType maskType = getMaskType();
            if (MASK_TYPE_POLYLINE == maskType)
            {

            }
        }
        break;
        case MODE_VIEW_NONE:
            break;
        default:
            break;
        }
    }

    m_preMoveX = mouseX;
    m_preMoveY = mouseY;
}

void InspMaskEditorWidget::mousePressEvent(QMouseEvent * event)
{
    if (Qt::LeftButton == event->buttons())
    {
        m_mouseLeftPressed = true;
        m_startX = event->x();
        m_startY = event->y();
        m_preMoveX = m_startX;
        m_preMoveY = m_startY;

        switch (m_stateView)
        {
        case MODE_VIEW_SELECT:
        {
            int nIndex = findSelect(m_startX, m_startY);
            if (nIndex > -1)
            {
                m_nSelectIndex = nIndex;
                ui.comboBox_maskIndex->setCurrentIndex(m_nSelectIndex + 1);
            }
            else
            {
                m_nSelectIndex = -1;
                ui.comboBox_maskIndex->setCurrentIndex(0);
                unSelectMask();
            }
        }
        break;
        case MODE_VIEW_ADD_MASK:
        {
            MaskType maskType = getMaskType();
            if (MASK_TYPE_POLYLINE == maskType)
            {
                double imageStartPosX = convertToImgX(m_startX);
                double imageStartPosY = convertToImgY(m_startY);

                m_maskPolyLineCur._polyPts.push_back(cv::Point2f(imageStartPosX, imageStartPosY));

                repaintAll();
            }
        }
        break;
        case MODE_VIEW_NONE:
            break;
        default:
            break;
        }
    }
    else if (Qt::RightButton == event->buttons())
    {
        if (MODE_VIEW_ADD_MASK == m_stateView)
        {
            MaskType maskType = getMaskType();
            if (MASK_TYPE_POLYLINE == maskType)
            {
                if (!m_maskPolyLineCur.isEmpty())
                {
                    PolyLineMask* mask = new PolyLineMask();
                    mask->_polyPts.assign(m_maskPolyLineCur._polyPts.begin(), m_maskPolyLineCur._polyPts.end());

                    m_maskObjs.push_back(mask);
                    ui.comboBox_maskIndex->addItem(QString("%1").arg(m_maskObjs.size()));

                    m_maskPolyLineCur.clear();
                }
            }
        }

        m_mouseRightPressed = true;
        setViewState(MODE_VIEW_SELECT);
        m_nSelectIndex = -1;
        ui.comboBox_maskIndex->setCurrentIndex(0);
        unSelectMask();
    }
}

void InspMaskEditorWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() & Qt::LeftButton)
    {
        switch (m_stateView)
        {
        case MODE_VIEW_SELECT:
        {
            if (m_nSelectIndex > -1)
            {               
            }
        }
        break;      
        case MODE_VIEW_ADD_MASK:
        {
            MaskType maskType = getMaskType();
            switch (maskType)
            {
            case MASK_TYPE_RECT:
            {
                if (!m_maskRectCur.isEmpty())
                {
                    RectMask* mask = new RectMask();
                    mask->_rect = m_maskRectCur._rect;

                    m_maskObjs.push_back(mask);
                    ui.comboBox_maskIndex->addItem(QString("%1").arg(m_maskObjs.size()));

                    m_maskRectCur.clear();

                    repaintAll();
                }
            }
            break;
            case MASK_TYPE_CIRCLE:
            {
                if (!m_maskCircleCur.isEmpty())
                {
                    CircleMask* mask = new CircleMask();
                    mask->_center = m_maskCircleCur._center;
                    mask->_radius = m_maskCircleCur._radius;

                    m_maskObjs.push_back(mask);
                    ui.comboBox_maskIndex->addItem(QString("%1").arg(m_maskObjs.size()));

                    m_maskCircleCur.clear();

                    repaintAll();
                }
            }
            break;
            case MASK_TYPE_POLYLINE:
                break;
            case MASK_TYPE_NULL:
                break;
            default:
                break;
            }
        }
        break;
        case MODE_VIEW_NONE:
            break;
        default:
            break;
        }
    }
    else if (event->button() & Qt::RightButton)
    {
    }

    m_mouseLeftPressed = false;
    m_mouseRightPressed = false;
}

void InspMaskEditorWidget::wheelEvent(QWheelEvent * event)
{
    switch (m_stateView)
    {
    case MODE_VIEW_NONE:
        break;
    default:
        break;
    }

    //QWidget::wheelEvent(event);
}

void InspMaskEditorWidget::A_Transform(cv::Mat& src, cv::Mat& dst, int dx, int dy)
{
    CV_Assert(src.depth() == CV_8U);//CV_Assert（）若括号中的表达式值为false，则返回一个错误信息。  
    const int rows = src.rows;
    const int cols = src.cols;
    dst.create(rows, cols, src.type());

    //dst.row(i).setTo(Scalar(255));
    //dst.col(j).setTo(Scalar(255));

    dst.setTo(cv::Scalar(0, 0, 0));

    cv::Vec3b *p;   //定义一个存放3通道的容器指针p  
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            p = dst.ptr<cv::Vec3b>(i);//指向行数的容器p  
            int x = j - dx;
            int y = i - dy;
            if (x>0 && y>0 && x < cols&&y < rows)//平移后的像素坐标在原图像的行数和列数内  
            {
                p[i, j] = src.ptr<cv::Vec3b>(y)[x];//平移后的图像（i,j)对应于原图像的（y,x)  
            }
        }
    }

}

void InspMaskEditorWidget::setViewState(int state)
{
    m_stateView = state;

    switch (m_stateView)
    {
    case MODE_VIEW_SELECT:
        setCursor(Qt::ArrowCursor);
        break;
    case MODE_VIEW_ADD_SELECT:
    case MODE_VIEW_ADD_MASK:
        setCursor(Qt::CrossCursor);
        break;
    case MODE_VIEW_NONE:
    default:
        setCursor(Qt::ArrowCursor);
        break;
    }
}

void InspMaskEditorWidget::displayImage(cv::Mat& image)
{
    QRect rect = ui.label_Img->geometry();
    rect.setX(0); rect.setY(0);
    rect.setWidth(LABEL_IMAGE_WIDTH); rect.setHeight(LABEL_IMAGE_HEIGHT);

    cv::Mat mat, matMoved;
    double fScaleW = rect.width()*1.0 / image.size().width;
    double fScaleH = rect.height()*1.0 / image.size().height;

    double fScale = fScaleH < fScaleW ? fScaleH : fScaleW;

    if (!image.empty())
    {
        cv::resize(image, mat, cv::Size(image.size().width*fScale, image.size().height*fScale), (0, 0), (0, 0), 3);

        if (mat.type() == CV_8UC3)
        {
            cvtColor(mat, mat, CV_BGR2RGB);
        }
        else if (mat.type() == CV_8UC1)
        {
            cvtColor(mat, mat, CV_GRAY2RGB);
        }

        cv::Mat matDisp = cv::Mat::zeros(cv::Size(rect.width(), rect.height()), mat.type());
        if (mat.cols < mat.rows)
        {
            cv::Mat outImg;
            outImg = matDisp(cv::Rect((mat.rows - mat.cols) / 2, 0, mat.cols, mat.rows));
            mat.copyTo(outImg);
        }
        else if (mat.cols > mat.rows)
        {
            cv::Mat outImg;
            outImg = matDisp(cv::Rect(0, (mat.cols - mat.rows) / 2, mat.cols, mat.rows));
            mat.copyTo(outImg);
        }
        else
        {
            mat.copyTo(matDisp);
        }

        //A_Transform(mat, matMoved, rect.x(), rect.y());
        //QImage image = QImage((uchar*)matMoved.data, matMoved.cols, matMoved.rows, ToInt(matMoved.step), QImage::Format_RGB888);

        QImage image = QImage((uchar*)matDisp.data, matDisp.cols, matDisp.rows, ToInt(matDisp.step), QImage::Format_RGB888);
        ui.label_Img->setPixmap(QPixmap::fromImage(image));
    }
}

void InspMaskEditorWidget::repaintAll()
{
    cv::Mat matImage = m_hoImage.clone();   

    if (!m_maskRectCur.isEmpty())
    {
        cv::Point2f vertices[4];
        m_maskRectCur._rect.points(vertices);

        int x = vertices[1].x;
        int y = vertices[1].y;
        int width = vertices[3].x - vertices[1].x;
        int height = vertices[3].y - vertices[1].y;


        cv::Mat matRect = matImage(cv::Rect(x, y, width, height));
        //rectangle(matRect, vertices[1], vertices[3], Scalar(0,0,255, 100), -1);
        cv::Mat imgLayer(height, width, matImage.type(), cv::Scalar(0, 0, 255));

        double alpha = 0.3;
        addWeighted(matRect, alpha, imgLayer, 1 - alpha, 0, matRect);
    }

    if (!m_maskCircleCur.isEmpty())
    {
        int x = m_maskCircleCur._center.x - m_maskCircleCur._radius;
        int y = m_maskCircleCur._center.y - m_maskCircleCur._radius;
        int width = m_maskCircleCur._radius * 2;
        int height = m_maskCircleCur._radius * 2;


        cv::Mat matRect = matImage(cv::Rect(x, y, width, height));
        //rectangle(matRect, vertices[1], vertices[3], Scalar(0,0,255, 100), -1);
        cv::Mat imgLayer = matRect.clone();
        circle(imgLayer, cv::Point2f(m_maskCircleCur._radius, m_maskCircleCur._radius), m_maskCircleCur._radius, cv::Scalar(0, 0, 255), -1);

        double alpha = 0.3;
        addWeighted(matRect, alpha, imgLayer, 1 - alpha, 0, matRect);
    }

    if (!m_maskPolyLineCur.isEmpty() && m_maskPolyLineCur._polyPts.size() > 0)
    {
        cv::Point *points = new cv::Point[m_maskPolyLineCur._polyPts.size()];
        for (int i = 0; i < m_maskPolyLineCur._polyPts.size(); i++)
        {
            points[i].x = m_maskPolyLineCur._polyPts[i].x;
            points[i].y = m_maskPolyLineCur._polyPts[i].y;
        }

        const cv::Point* pt[1] = { points };
        int npt[1] = { m_maskPolyLineCur._polyPts.size() };

        cv::Mat imgLayer = matImage.clone();

        polylines(imgLayer, pt, npt, 1, 1, cv::Scalar(0, 0, 255));
        fillPoly(imgLayer, pt, npt, 1, cv::Scalar(0, 0, 255), 8);

        delete points;

        double alpha = 0.3;
        addWeighted(matImage, alpha, imgLayer, 1 - alpha, 0, matImage);
    }

    for (int i = 0; i < m_maskObjs.size(); i++)
    {
        MaskObj* obj = m_maskObjs[i];
        switch (obj->getType())
        {
        case MASK_TYPE_RECT:
        {
            RectMask* maskObj = dynamic_cast<RectMask*>(obj);
            if (maskObj && !maskObj->isEmpty())
            {
                cv::Point2f vertices[4];
                maskObj->_rect.points(vertices);

                int x = vertices[1].x;
                int y = vertices[1].y;
                int width = vertices[3].x - vertices[1].x;
                int height = vertices[3].y - vertices[1].y;


                cv::Mat matRect = matImage(cv::Rect(x, y, width, height));
                //rectangle(matRect, vertices[1], vertices[3], Scalar(0,0,255, 100), -1);
                cv::Mat imgLayer(height, width, matImage.type(), maskObj->IsSelect() ? cv::Scalar(128, 128, 255) : cv::Scalar(0, 0, 255));

                double alpha = 0.3;
                addWeighted(matRect, alpha, imgLayer, 1 - alpha, 0, matRect);
            }
        }
        break;
        case MASK_TYPE_CIRCLE:
        {
            CircleMask* maskObj = dynamic_cast<CircleMask*>(obj);
            if (maskObj && !maskObj->isEmpty())
            {
                int x = maskObj->_center.x - maskObj->_radius;
                int y = maskObj->_center.y - maskObj->_radius;
                int width = maskObj->_radius * 2;
                int height = maskObj->_radius * 2;


                cv::Mat matRect = matImage(cv::Rect(x, y, width, height));
                //rectangle(matRect, vertices[1], vertices[3], Scalar(0,0,255, 100), -1);
                cv::Mat imgLayer = matRect.clone();
                circle(imgLayer, cv::Point2f(maskObj->_radius, maskObj->_radius), maskObj->_radius, maskObj->IsSelect() ? cv::Scalar(128, 128, 255) : cv::Scalar(0, 0, 255), -1);

                double alpha = 0.3;
                addWeighted(matRect, alpha, imgLayer, 1 - alpha, 0, matRect);
            }
        }
        break;
        case MASK_TYPE_POLYLINE:
        {
            PolyLineMask* maskObj = dynamic_cast<PolyLineMask*>(obj);
            if (maskObj && !maskObj->isEmpty() && maskObj->_polyPts.size() > 2)
            {
                cv::Point *points = new cv::Point[maskObj->_polyPts.size()];
                for (int i = 0; i < maskObj->_polyPts.size(); i++)
                {
                    points[i].x = maskObj->_polyPts[i].x;
                    points[i].y = maskObj->_polyPts[i].y;
                }

                const cv::Point* pt[1] = { points };
                int npt[1] = { maskObj->_polyPts.size() };

                cv::Mat imgLayer = matImage.clone();

                polylines(imgLayer, pt, npt, 1, 1, maskObj->IsSelect() ? cv::Scalar(128, 128, 255) : cv::Scalar(0, 0, 255));
                fillPoly(imgLayer, pt, npt, 1, maskObj->IsSelect() ? cv::Scalar(128, 128, 255) : cv::Scalar(0, 0, 255), 8);

                delete points;

                double alpha = 0.3;
                addWeighted(matImage, alpha, imgLayer, 1 - alpha, 0, matImage);
            }
        }
        break;
        case MASK_TYPE_NULL:
            break;
        default:
            break;
        }
    }
   
    displayImage(matImage);
}

double InspMaskEditorWidget::convertToImgX(double dMouseValue, bool bZero)
{
    QRect rect = ui.label_Img->geometry();
    double fScaleW = m_windowWidth*1.0 / m_imageWidth;
    double fScaleH = m_windowHeight*1.0 / m_imageHeight;

    double fScale = fScaleH < fScaleW ? fScaleH : fScaleW;

    if (bZero)
    {
        return (dMouseValue) / fScale;
    }
    else
    {
        if (m_imageWidth < m_imageHeight)
        {
            return (dMouseValue - rect.x() - (m_windowWidth - m_imageWidth*fScale) / 2) / fScale;
        }
        else
        {
            return (dMouseValue - rect.x()) / fScale;
        }
    }
}

double InspMaskEditorWidget::convertToImgY(double dMouseValue, bool bZero)
{
    QRect rect = ui.label_Img->geometry();
    double fScaleW = m_windowWidth*1.0 / m_imageWidth;
    double fScaleH = m_windowHeight*1.0 / m_imageHeight;

    double fScale = fScaleH < fScaleW ? fScaleH : fScaleW;

    if (bZero)
    {
        return (dMouseValue) / fScale;
    }
    else
    {
        if (m_imageWidth > m_imageHeight)
        {
            return (dMouseValue - rect.y() - (m_windowHeight - m_imageHeight*fScale) / 2) / fScale;
        }
        else
        {
            return (dMouseValue - rect.y()) / fScale;
        }
    }
}


void InspMaskEditorWidget::addImageText(cv::Mat image, cv::Point ptPos, QString szText)
{
    if (m_hoImage.empty()) return;

    const int ImageWidth = 800;
    double dScaleFactor = (double)(m_imageWidth + m_imageHeight) / ImageWidth;


    cv::String text = szText.toStdString();

    cv::Point ptPos1;
    ptPos1.x = ptPos.x - 10 * dScaleFactor;
    ptPos1.y = ptPos.y + 10 * dScaleFactor;
    cv::Point ptPos2;
    ptPos2.x = ptPos.x + 10 * dScaleFactor * text.length();
    ptPos2.y = ptPos.y - 20 * dScaleFactor;
    rectangle(image, ptPos1, ptPos2, cv::Scalar(125, 125, 125), -1);
    rectangle(image, ptPos1, ptPos2, cv::Scalar(200, 200, 200), 1);

    double fontScale = dScaleFactor*0.5f;
    cv::putText(image, text, ptPos, CV_FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(0, 255, 255), 1);
}

int InspMaskEditorWidget::findSelect(double mousePosX, double mousePosY)
{
    int nIndex = -1;

    double posX = convertToImgX(mousePosX);
    double posY = convertToImgY(mousePosY);

    cv::Point2f pt = cv::Point2f(posX, posY);

    for (int i = 0; i < m_maskObjs.size(); i++)
    {
        auto obj = m_maskObjs[i];
        if (obj->IsPointIn(pt))
        {
            nIndex = i;
            break;
        }
    }
   
    return nIndex;
}

MaskType InspMaskEditorWidget::getMaskType()
{
    MaskType maskType = MASK_TYPE_NULL;
    if (ui.radioButton_rect->isChecked())
    {
        maskType = MASK_TYPE_RECT;
    }
    else if (ui.radioButton_circle->isChecked())
    {
        maskType = MASK_TYPE_CIRCLE;
    }
    else if (ui.radioButton_polyline->isChecked())
    {
        maskType = MASK_TYPE_POLYLINE;
    }

    return maskType;
}

void InspMaskEditorWidget::unSelectMask()
{
    for (int i = 0; i < m_maskObjs.size(); i++)
    {
        m_maskObjs.at(i)->setSelect(false);
    }
    //ui.comboBox_maskIndex->setCurrentIndex(0);
}

void InspMaskEditorWidget::clear()
{
    ui.comboBox_maskIndex->clear();
    ui.comboBox_maskIndex->addItem(QString("%1").arg(QStringLiteral("请选择区域")));

    for (int i = 0; i < m_maskObjs.size(); i++)
    {
        delete m_maskObjs[i];
    }
    m_maskObjs.clear();

    m_nSelectIndex = -1;

    m_hoImage.release();
}

void InspMaskEditorWidget::onMaskIndexChanged(int iIndex)
{
    int nMaskIndex = ui.comboBox_maskIndex->currentIndex();

    if (nMaskIndex >= 1 && nMaskIndex <= m_maskObjs.size())
    {
        unSelectMask();
        m_maskObjs.at(nMaskIndex - 1)->setSelect(true);        
    }
    repaintAll();
}

void InspMaskEditorWidget::onAddMask()
{
    setViewState(MODE_VIEW_ADD_MASK);
    ui.comboBox_maskIndex->setCurrentIndex(0);
    unSelectMask();
}

void InspMaskEditorWidget::onEditDone()
{
    hide();
}

void InspMaskEditorWidget::onDeleteMask()
{
    int nMaskIndex = ui.comboBox_maskIndex->currentIndex();

    if (nMaskIndex >= 1 && nMaskIndex <= m_maskObjs.size())
    {
        delete m_maskObjs[nMaskIndex - 1];
        m_maskObjs.erase(m_maskObjs.begin() + nMaskIndex - 1);

        ui.comboBox_maskIndex->removeItem(ui.comboBox_maskIndex->count() - 1);
        ui.comboBox_maskIndex->setCurrentIndex(0);

        unSelectMask();       
    }
}