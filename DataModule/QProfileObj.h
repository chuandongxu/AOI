#pragma once

#include <QObject>
#include <QVector>
#include <QImage>

#include "opencv/cv.h"

const QString g_szMeasureName[] = { QStringLiteral("线与线的夹角"), QStringLiteral("线与线的距离"), QStringLiteral("点到线的距离"),
                                    QStringLiteral("点到点的距离"), QStringLiteral("点的高度"), QStringLiteral("圆到圆的距离"),
                                    QStringLiteral("圆到直线的距离"), QStringLiteral("圆到点的距离"), QStringLiteral("圆弧半径"),
                                    QStringLiteral("区域面积"), QStringLiteral("曲线长度"), QStringLiteral("未知类型") };

enum MeasureType
{
    MEASURE_TYPE_LNTOLNANGLE = 0,
    MEASURE_TYPE_LNTOLN_DIST,
    MEASURE_TYPE_PTTOLN_DIST,
    MEASURE_TYPE_PTTOPT_DIST,
    MEASURE_TYPE_PT_HEIGHT,
    MEASURE_TYPE_CRTOCR_DIST,
    MEASURE_TYPE_CRTOLN_DIST,
    MEASURE_TYPE_CRTOPT_DIST,
    MEASURE_TYPE_CR_RADIUS,
    MEASURE_TYPE_AREA,
    MEASURE_TYPE_PRF_LENGTH,
    MEASURE_TYPE_NULL
};

enum MeasureDataType
{
    MEASURE_DATA_TYPE_LINE = 0,
    MEASURE_DATA_TYPE_POINT,
    MEASURE_DATA_TYPE_ARC,
    MEASURE_DATA_TYPE_NULL
};

class QProfMeasureObj 
{
public:
    QProfMeasureObj(MeasureType emType)
        :m_emType(emType)
    {
        m_szName = g_szMeasureName[(int)m_emType];
    }
    ~QProfMeasureObj(){}

public:
    MeasureType getType(){ return m_emType; }
    QString    getName(){ return m_szName; }
    virtual double getValue() = 0;
    virtual QString getValueStr() = 0;
    virtual void setValue(double dValue) = 0;

    int getRangeNum(){ return m_ptRanges.size(); }
    cv::Point getRange(int i)
    {
        if (i >= 0 && i < m_ptRanges.size())
        {
            return m_ptRanges[i];
        }

        return cv::Point();
    }
    MeasureDataType getRangeType(int i)
    {
        if (i >= 0 && i < m_ptRangeTypes.size())
        {
            return m_ptRangeTypes[i];
        }

        return MEASURE_DATA_TYPE_LINE;
    }
    cv::Point getRangeValue(int i)
    {
        if (i >= 0 && i < m_ptRangeValues.size())
        {
            return m_ptRangeValues[i];
        }

        return cv::Point();
    }
    void setRangeValue(int i, cv::Point ptValue)
    {
        if (i >= 0 && i < m_ptRangeValues.size())
        {
            m_ptRangeValues[i] = ptValue;
        }        
    }

    void addRange(cv::Point pt, MeasureDataType dataType, cv::Point ptValue = cv::Point(0, 0)){ m_ptRanges.push_back(pt); m_ptRangeTypes.push_back(dataType); m_ptRangeValues.push_back(ptValue); }
    void clearRanges(){ m_ptRanges.clear(); m_ptRangeTypes.clear(); m_ptRangeValues.clear(); }

private:
    MeasureType m_emType;
    QString m_szName;
    QVector<cv::Point> m_ptRanges;
    QVector<MeasureDataType> m_ptRangeTypes;
    QVector<cv::Point> m_ptRangeValues;
};

class QProfMeasLnToLnAngle : public QProfMeasureObj
{
public:
    QProfMeasLnToLnAngle()
        :QProfMeasureObj(MEASURE_TYPE_LNTOLNANGLE)
    {
    }
    ~QProfMeasLnToLnAngle(){}

    virtual double getValue() { return _dAngle;  }
    virtual void setValue(double dValue){ _dAngle = dValue; }
    virtual QString getValueStr()
    {
        return QString("Angle: %1 ").arg(_dAngle, 0, 'g', 3);
    }

public:
    //cv::Point_<double> _line1Pt1;
    //cv::Point_<double> _line1Pt2;
    //cv::Point_<double> _line2Pt1;
    //cv::Point_<double> _line2Pt2;
    double _dAngle;
};

class QProfMeasLnToLnDist : public QProfMeasureObj
{
public:
    QProfMeasLnToLnDist()
        :QProfMeasureObj(MEASURE_TYPE_LNTOLN_DIST)
    {
    }
    ~QProfMeasLnToLnDist(){}

    virtual double getValue() { return _dDist; }
    virtual void setValue(double dValue){ _dDist = dValue; }
    virtual QString getValueStr()
    {
        return QString("Dist: %1 mm").arg(_dDist, 0, 'g', 3);
    }

public:
    //cv::Point_<double> _line1Pt1;
    //cv::Point_<double> _line1Pt2;
    //cv::Point_<double> _line2Pt1;
    //cv::Point_<double> _line2Pt2;
    double _dDist;
};

class QProfMeasPtToLnDist : public QProfMeasureObj
{
public:
    QProfMeasPtToLnDist()
        :QProfMeasureObj(MEASURE_TYPE_PTTOLN_DIST)
    {
    }
    ~QProfMeasPtToLnDist(){}

    virtual double getValue() { return _dDist; }
    virtual void setValue(double dValue){ _dDist = dValue; }
    virtual QString getValueStr()
    {
        return QString("Dist: %1 mm").arg(_dDist, 0, 'g', 3);
    }

public:
    //cv::Point_<double> _line1Pt1;
    //cv::Point_<double> _line1Pt2;
    //cv::Point_<double> _line2Pt1;
    //cv::Point_<double> _line2Pt2;
    double _dDist;
};

class QProfMeasPtToPtDist : public QProfMeasureObj
{
public:
    QProfMeasPtToPtDist()
        :QProfMeasureObj(MEASURE_TYPE_PTTOPT_DIST)
    {
    }
    ~QProfMeasPtToPtDist(){}

    virtual double getValue() { return _dDist; }
    virtual void setValue(double dValue){ _dDist = dValue; }
    virtual QString getValueStr()
    {
        return QString("Dist: %1 mm").arg(_dDist, 0, 'g', 3);
    }

public:
    double _dDist;
};

class QProfMeasPtHeight : public QProfMeasureObj
{
public:
    QProfMeasPtHeight()
        :QProfMeasureObj(MEASURE_TYPE_PT_HEIGHT)
    {
    }
    ~QProfMeasPtHeight(){}

    virtual double getValue() { return _dHeight; }
    virtual void setValue(double dValue){ _dHeight = dValue; }
    virtual QString getValueStr()
    {
        return QString("Height: %1 mm").arg(_dHeight, 0, 'g', 3);
    }

public:
    double _dHeight;
};

class QProfMeasCirToCir : public QProfMeasureObj
{
public:
    QProfMeasCirToCir()
        :QProfMeasureObj(MEASURE_TYPE_CRTOCR_DIST)
    {
    }
    ~QProfMeasCirToCir(){}

    virtual double getValue() { return _dDist; }
    virtual void setValue(double dValue){ _dDist = dValue; }
    virtual QString getValueStr()
    {
        return QString("Dist: %1 mm").arg(_dDist, 0, 'g', 3);
    }

public:
    double _dDist;

    double _dRadius1;
    cv::Point2f _ptCirCenter1;
    double _dRadius2;
    cv::Point2f _ptCirCenter2;
};

class QProfMeasCirToLn : public QProfMeasureObj
{
public:
    QProfMeasCirToLn()
        :QProfMeasureObj(MEASURE_TYPE_CRTOLN_DIST)
    {
    }
    ~QProfMeasCirToLn(){}

    virtual double getValue() { return _dDist; }
    virtual void setValue(double dValue){ _dDist = dValue; }
    virtual QString getValueStr()
    {
        return QString("Dist: %1 mm").arg(_dDist, 0, 'g', 3);
    }

public:
    double _dDist;

    double _dRadius;
    cv::Point2f _ptCirCenter;
};

class QProfMeasCirToPt : public QProfMeasureObj
{
public:
    QProfMeasCirToPt()
        :QProfMeasureObj(MEASURE_TYPE_CRTOPT_DIST)
    {
    }
    ~QProfMeasCirToPt(){}

    virtual double getValue() { return _dDist; }
    virtual void setValue(double dValue){ _dDist = dValue; }
    virtual QString getValueStr()
    {
        return QString("Dist: %1 mm").arg(_dDist, 0, 'g', 3);
    }

public:
    double _dDist;

    double _dRadius;
    cv::Point2f _ptCirCenter;
};

class QProfMeasCirRadius : public QProfMeasureObj
{
public:
    QProfMeasCirRadius()
        :QProfMeasureObj(MEASURE_TYPE_CR_RADIUS)
    {
    }
    ~QProfMeasCirRadius(){}

    virtual double getValue() { return _dRadius; }
    virtual void setValue(double dValue){ _dRadius = dValue; }
    virtual QString getValueStr()
    {
        return QString("Radius: %1 mm").arg(_dRadius, 0, 'g', 3);
    }

public:
    double _dRadius;
    cv::Point2f _ptCirCenter;
};

class QProfMeasProfArea : public QProfMeasureObj
{
public:
    QProfMeasProfArea()
        :QProfMeasureObj(MEASURE_TYPE_AREA)
    {
    }
    ~QProfMeasProfArea(){}

    virtual double getValue() { return _dArea; }
    virtual void setValue(double dValue){ _dArea = dValue; }
    virtual QString getValueStr()
    {
        return QString("Area: %1 mm2").arg(_dArea, 0, 'g', 3);
    }

public:
    double _dArea;    
};

class QProfMeasProfLength : public QProfMeasureObj
{
public:
    QProfMeasProfLength()
        :QProfMeasureObj(MEASURE_TYPE_PRF_LENGTH)
    {
    }
    ~QProfMeasProfLength(){}

    virtual double getValue() { return _dLength; }
    virtual void setValue(double dValue){ _dLength = dValue; }
    virtual QString getValueStr()
    {
        return QString("Length: %1 mm").arg(_dLength, 0, 'g', 3);
    }

public:
    double _dLength;    
};


class QProfileObj : public QObject
{
    Q_OBJECT

public:
    QProfileObj(int nID, QString name, QObject *parent = NULL);
    ~QProfileObj();

    QProfileObj(const QProfileObj &other);//复制构造函数 
    QProfileObj & operator = (const QProfileObj &other);//赋值符 

private:
    void deepCopy(const QProfileObj &other);

public:
    QString getName(){ return m_objName; }
    void setName(QString& name){ m_objName = name; }
    int        getID(){ return m_nID; }

    void setCutingPosn(double posX1, double posY1, double posX2, double posY2);
    void getCutingPosn(double& posX1, double& posY1, double& posX2, double& posY2);
    void set3DImage(QImage& img){ m_img3D = img; }
    QImage& get3dImage(){ return m_img3D; }
    void setGrayImage(cv::Mat& img){ m_imgGray = img; }
    cv::Mat& getGrayImage(){ return m_imgGray; }

    void setProfData(QVector<cv::Point2d>& profDatas){ m_profDatas = profDatas; }
    QVector<cv::Point2d>& getProfData(){ return m_profDatas; }

    void addMeasObj(QProfMeasureObj* pObj);
    QProfMeasureObj* getMeasObj(int nIndex);
    void removeMeasObj(int nIndex);
    int getMeasObjNum();
    void clearMeasObjs();

    void setOffset(cv::Point2d ptOffset);

private:
    int m_nID;
    QString m_objName;    

    double m_posX1;
    double m_posY1;
    double m_posX2;
    double m_posY2;

    QImage m_img3D;
    cv::Mat m_imgGray;
    QVector<cv::Point2d> m_profDatas;

    QVector<QProfMeasureObj*> m_profMeasObjs;
};
