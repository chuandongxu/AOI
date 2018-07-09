#pragma once

#include <QObject>
#include <qstring.h>
#include <qfile.h>
#include <qmutex.h>
#include <qstringlist.h>

#include <memory>

#include "VisionAPI.h"

using namespace AOI;

#define _DLP_MAX_NUM    4

class TBCDataStorageFile
{
public:
    enum optType{ sNewLine, sEndLine };
public:
    TBCDataStorageFile(const QString &filepath, const QString filesuffix = "txt");
    ~TBCDataStorageFile(void);

    void reOpen(bool bBackUp = true);

    void append(const QString & str);
    void appendNewLine();

    TBCDataStorageFile & operator<<(int data);
    TBCDataStorageFile & operator<<(double data);
    TBCDataStorageFile & operator<<(const QString & str);
    TBCDataStorageFile & operator<<(optType opt);

    QStringList readLines();

protected:
    void open(const QString & file);
    void close();

private:
    QFile * m_pFile;
    bool m_bOpened;
    QString m_filePath;
    QString m_fileSuffix;
};

class TableCaliDataStorage : public QObject
{
    Q_OBJECT

public:
    TableCaliDataStorage(QObject *parent = NULL);
    ~TableCaliDataStorage();

    static TableCaliDataStorage * instance();

    void setFrame(Vision::VectorOfVectorOfPoint2f& vecVecFrameCtr);
    void setData(int nIndex, Vision::VectorOfVectorOfFloat& vecVecFrameDLP);
    void saveData();


    bool loadData();
    bool getFrameOffsetByPixel(cv::Point2f& pt, float* fOffsetValue);// fOffsetValue[4] values   
    bool getFrameOffsetByUm(cv::Point2f& pt, float* fOffsetValue);// fOffsetValue[4] values
    bool getFrameOffsetByMm(cv::Point2f& pt, float* fOffsetValue);// fOffsetValue[4] values

private:
    static void desstory();

private:
    Vision::VectorOfVectorOfPoint2f m_vecVecFrameCtr;
    Vision::VectorOfVectorOfFloat m_vecVecFrameDLP[_DLP_MAX_NUM];

private:
    std::shared_ptr<TBCDataStorageFile> m_fileStorage;
    int m_nScreenWidth;
    int m_nScreenHeight;

private:
    static TableCaliDataStorage * _instance_;
    static bool _destoryed_;
};

#define TableCalData TableCaliDataStorage::instance()
