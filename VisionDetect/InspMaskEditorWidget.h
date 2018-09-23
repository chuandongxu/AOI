#pragma once

#include <QWidget>
#include "ui_InspMaskEditorWidget.h"

#include "VisionAPI.h"

using namespace AOI;

enum MaskType
{
    MASK_TYPE_RECT,
    MASK_TYPE_CIRCLE,
    MASK_TYPE_POLYLINE,
    MASK_TYPE_NULL
};

class MaskObj
{
public:
    MaskObj()
    {
        m_bSelect = false;
    }
    ~MaskObj(){}
public:
    MaskType getType(){ return m_emType; }
    void setType(MaskType type){ m_emType = type; }
    virtual bool IsRect(){ return MASK_TYPE_RECT == m_emType; }
    virtual bool IsCircle(){ return MASK_TYPE_CIRCLE == m_emType; }
    virtual bool IsPolyLine(){ return MASK_TYPE_POLYLINE == m_emType; }

    virtual bool IsPointIn(const cv::Point2f& pt) = 0;
    virtual void Move(const cv::Point2f& dist, double imgWidth, double imgHeight) = 0;

    void setSelect(bool bSelect){ m_bSelect = bSelect; }
    bool IsSelect(){ return m_bSelect; }

private:
    MaskType m_emType;
    bool m_bSelect;
};

class RectMask : public MaskObj
{
public:
    RectMask()
    {
        setType(MASK_TYPE_RECT);
        _rect.size.width = 0;
        _rect.size.height = 0;
    }
    ~RectMask(){}

    bool isEmpty(){ return !(_rect.size.width > 1 || _rect.size.height > 1); }
    void clear(){ _rect.size.width = 0; _rect.size.height = 0; }
    virtual bool IsPointIn(const cv::Point2f& pt) override;
    virtual void Move(const cv::Point2f& dist, double imgWidth, double imgHeight) override;

public:
    cv::RotatedRect _rect;
};

class CircleMask : public MaskObj
{
public:
    CircleMask(){ setType(MASK_TYPE_CIRCLE); _radius = 0; }
    ~CircleMask(){}

    bool isEmpty(){ return _radius <= 0; }
    void clear(){ _radius = 0; }
    virtual bool IsPointIn(const cv::Point2f& pt) override;
    virtual void Move(const cv::Point2f& dist, double imgWidth, double imgHeight) override;

public:
    cv::Point2f _center;
    double         _radius;
};

class PolyLineMask : public MaskObj
{
public:
    PolyLineMask(){ setType(MASK_TYPE_POLYLINE); }
    ~PolyLineMask(){}

    bool isEmpty(){ return _polyPts.size() == 0; }
    void clear(){ _polyPts.clear(); }
    virtual bool IsPointIn(const cv::Point2f& pt) override;
    virtual void Move(const cv::Point2f& dist, double imgWidth, double imgHeight) override;

public:
    std::vector<cv::Point2f> _polyPts;
};

class InspMaskEditorWidget : public QWidget
{
    Q_OBJECT

public:
    InspMaskEditorWidget(QWidget *parent = Q_NULLPTR);
    ~InspMaskEditorWidget();

protected:
    void closeEvent(QCloseEvent *e);

public:
    void setImage(cv::Mat& matImage, bool bClearAll = false);
    void repaintAll();
    void clear();  

    cv::Mat getMaskMat();
    void setMaskMat(cv::Mat& matMask);   

protected:
    void mouseMoveEvent(QMouseEvent * event);
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent * event);

protected slots:
    void onMaskIndexChanged(int iState);
    void onAddMask();
    void onDeleteMask();

    void onEditDone();

private:
    void initUI();
    void A_Transform(cv::Mat& src, cv::Mat& dst, int dx, int dy);
    void setViewState(int state);
    void displayImage(cv::Mat& image);
    double convertToImgX(double dMouseValue, bool bZero = false);
    double convertToImgY(double dMouseValue, bool bZero = false);
    void addImageText(cv::Mat image, cv::Point ptPos, QString szText);

private:
    int findSelect(double mousePosX, double mousePosY);    

    MaskType getMaskType();
    void unSelectMask();

    void enablePanel(bool bEnable);

private:
    int    m_stateView;
    bool   m_mouseLeftPressed;
    bool   m_mouseRightPressed;
    double m_startX, m_startY;
    double m_preMoveX, m_preMoveY;

    int   m_windowWidth;
    int   m_windowHeight;
    int   m_imageWidth;
    int   m_imageHeight;

private:
    Ui::InspMaskEditorWidget ui;
    cv::Mat    m_hoImage;
    cv::Mat m_maskMat;

    int m_nSelectIndex;

    std::vector<MaskObj*> m_maskObjs;
    RectMask m_maskRectCur;
    CircleMask m_maskCircleCur;
    PolyLineMask m_maskPolyLineCur;
};
