#include "TableCaliDataStorage.h"

#include <qtextstream.h>
#include <QApplication>

#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/ICamera.h"
#include "../include/constants.h"

const QString _TBCDataFile = QString("TBCData");

TBCDataStorageFile::TBCDataStorageFile(const QString &filepath, const QString filesuffix)
    :m_bOpened(false), m_filePath(filepath), m_fileSuffix(filesuffix)
{
    this->open(_TBCDataFile);
}

TBCDataStorageFile::~TBCDataStorageFile(void)
{
    this->close();
}

void TBCDataStorageFile::open(const QString & file)
{
    QString path;
    if (m_fileSuffix.isEmpty()) path = m_filePath + "/" + file + ".txt";
    else path = m_filePath + "/" + file + "." + m_fileSuffix;

    m_pFile = new QFile(path);
    if (m_pFile)
    {
        m_bOpened = m_pFile->open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text);
    }
}

void TBCDataStorageFile::close()
{
    if (m_pFile && m_bOpened)
    {
        m_pFile->close();
        m_bOpened = false;
    }
}

void TBCDataStorageFile::reOpen(bool bBackUp)
{
    if (bBackUp)
    {
        QFile::remove(m_pFile->fileName() + ".bak");
        m_pFile->copy(m_pFile->fileName() + ".bak");
    }

    if (m_pFile->exists()) m_pFile->remove();

    this->close();
    this->open(_TBCDataFile);
}

void TBCDataStorageFile::append(const QString & str)
{
    if (m_pFile)
    {
        //QByteArray bytesStr = QCryptographicHash::hash(str.toLocal8Bit(), QCryptographicHash::Md5);
        //QString md5 = bytesStr.toHex();

        /*QByteArray bytesStr;
        bytesStr.append(str);
        QByteArray bytes64 = bytesStr.toBase64();*/

        QTextStream stream(m_pFile);
        stream << str;
        //stream << "\r\n";
    }
}

void TBCDataStorageFile::appendNewLine()
{
    if (m_pFile)
    {
        QTextStream stream(m_pFile);
        stream << "\r\n";
    }
}

TBCDataStorageFile & TBCDataStorageFile::operator<<(int data)
{
    this->append(QString::number(data));
    return *this;
}

TBCDataStorageFile & TBCDataStorageFile::operator<<(double data)
{
    this->append(QString("%0").arg(data));
    return *this;
}

TBCDataStorageFile & TBCDataStorageFile::operator<<(const QString & str)
{
    this->append(str);
    return *this;
}

TBCDataStorageFile & TBCDataStorageFile::operator<<(TBCDataStorageFile::optType  opt)
{
    if (opt == sNewLine || opt == sEndLine)
    {
        this->appendNewLine();
    }
    return *this;
}

QStringList TBCDataStorageFile::readLines()
{
    QStringList strList;

    QTextStream in(m_pFile);
    in.seek(0);
   
    while (!in.atEnd()) 
    {
        QString line = in.readLine();
        if (!line.isEmpty() && !line.isNull()) strList.push_back(line);       
    }

    return strList;
}

//------------------------------------------------------------
TableCaliDataStorage * TableCaliDataStorage::_instance_ = NULL;
bool TableCaliDataStorage::_destoryed_ = false;
//------------------------------------------------------------

TableCaliDataStorage::TableCaliDataStorage(QObject *parent)
    : QObject(parent)
{
    QString path = QApplication::applicationDirPath();
    path += "/3D/data/";

    m_fileStorage = std::make_shared<TBCDataStorageFile>(path);

    m_nScreenWidth = 0;
    m_nScreenHeight = 0;
}

TableCaliDataStorage::~TableCaliDataStorage()
{
}

TableCaliDataStorage * TableCaliDataStorage::instance()
{
    if (!_instance_)
    {
        if (!_destoryed_)
        {
            _instance_ = new TableCaliDataStorage;
            atexit(&TableCaliDataStorage::desstory);
        }
    }

    return _instance_;
}

void TableCaliDataStorage::desstory()
{
    if (_instance_)
    {
        delete _instance_;
        _instance_ = NULL;
        _destoryed_ = true;
    }
}

void TableCaliDataStorage::setFrame(Vision::VectorOfVectorOfPoint2f& vecVecFrameCtr)
{
    m_vecVecFrameCtr = vecVecFrameCtr;
}

void TableCaliDataStorage::setData(int nIndex, Vision::VectorOfVectorOfFloat& vecVecFrameDLP)
{
    if (nIndex < 0 || nIndex >= _DLP_MAX_NUM) return;

    m_vecVecFrameDLP[nIndex] = vecVecFrameDLP;
}

void TableCaliDataStorage::saveData()
{
    m_fileStorage->reOpen(true);

    int nRow = m_vecVecFrameDLP[0].size();
    int nCol = nRow > 0 ? m_vecVecFrameDLP[0][0].size() : 0;

    (*m_fileStorage) << QStringLiteral("total_rows") << "\t" << QStringLiteral("total_cols") << TBCDataStorageFile::sEndLine;
    (*m_fileStorage) << nRow << "\t" << nCol << TBCDataStorageFile::sEndLine;

    (*m_fileStorage) << QStringLiteral("row") << "\t" << QStringLiteral("col") << "\t"
                     << QStringLiteral("frameX") << "\t" << QStringLiteral("frameY") << "\t"
                     << QStringLiteral("DLP_offset_1") << "\t" << QStringLiteral("DLP_offset_2") << "\t"
                     << QStringLiteral("DLP_offset_3") << "\t" << QStringLiteral("DLP_offset_4") << TBCDataStorageFile::sEndLine;

    for (int row = 0; row < nRow; ++row)
    {
        for (int col = 0; col < nCol; ++col)
        {
            double frameX = m_vecVecFrameCtr[row][col].x;
            double frameY = m_vecVecFrameCtr[row][col].y;

            (*m_fileStorage) << row + 1 << "\t" << col + 1 << "\t"
                            << frameX << "\t" << frameY << "\t"
                            << m_vecVecFrameDLP[0][row][col] << "\t" << m_vecVecFrameDLP[1][row][col] << "\t"
                            << m_vecVecFrameDLP[2][row][col] << "\t" << m_vecVecFrameDLP[3][row][col] << "\t" << TBCDataStorageFile::sEndLine;
        }
    }
}

bool TableCaliDataStorage::loadData()
{
    m_vecVecFrameCtr.clear();
    for (int i = 0; i < _DLP_MAX_NUM; i++)
        m_vecVecFrameDLP[i].clear();

    QStringList lines = m_fileStorage->readLines();

    int nTotalRow = 0, nTotalCol = 0;

    bool bReadHeader = false;
    bool bReadData = false;
    for each(QString line in lines)
    {
        if (line.trimmed().isEmpty()) continue;  
        if (line.contains(QStringLiteral("total_rows")))
        {
            bReadHeader = true;
            continue;
        }
        else if (line.contains(QStringLiteral("frameX")))
        {
            bReadData = true;
            continue;
        }

        QStringList strs = line.trimmed().split("\t");
        if (bReadHeader && !bReadData)
        {
            nTotalRow = strs[0].toInt();
            nTotalCol = strs[1].toInt();   

            m_vecVecFrameCtr = Vision::VectorOfVectorOfPoint2f(nTotalRow, Vision::VectorOfPoint2f(nTotalCol, cv::Point2f(0.0, 0.0)));
            for (int i = 0; i < _DLP_MAX_NUM; i++)
                m_vecVecFrameDLP[i] = Vision::VectorOfVectorOfFloat(nTotalRow, Vision::VectorOfFloat(nTotalCol, float(0.0)));
        }
        else if (bReadData)
        {
            int nRow = strs[0].toInt() - 1;
            int nCol = strs[1].toInt() - 1;
            float fFrameX = strs[2].toFloat();
            float fFrameY = strs[3].toFloat();
            float fOffset1 = strs[4].toFloat();
            float fOffset2 = strs[5].toFloat();
            float fOffset3 = strs[6].toFloat();
            float fOffset4 = strs[7].toFloat();

            m_vecVecFrameCtr[nRow][nCol] = cv::Point2f(fFrameX, fFrameY);
            m_vecVecFrameDLP[0][nRow][nCol] = fOffset1;
            m_vecVecFrameDLP[1][nRow][nCol] = fOffset2;
            m_vecVecFrameDLP[2][nRow][nCol] = fOffset3;
            m_vecVecFrameDLP[3][nRow][nCol] = fOffset4;
        }  
    }

    auto pCamera = getModule<ICamera>(CAMERA_MODEL);
    int nImageWidth = 0, nImageHeight = 0;
    pCamera->getCameraScreenSize(nImageWidth, nImageHeight);

    m_nScreenWidth = nImageWidth;
    m_nScreenHeight = nImageHeight;

    return true;
}

bool TableCaliDataStorage::getFrameOffsetByPixel(cv::Point2f& pt, float* fOffsetValue)
{
    double dResolutionX = System->getSysParam("CAM_RESOLUTION_X").toDouble();
    double dResolutionY = System->getSysParam("CAM_RESOLUTION_Y").toDouble();    

    double dTopLeftCtrX = m_vecVecFrameCtr[0][0].x;
    double dTopLeftCtrY = m_vecVecFrameCtr[0][0].y;

    cv::Point2f ptUm;
    ptUm.x = dTopLeftCtrX + (pt.x - m_nScreenWidth/2) * dResolutionX * UM_TO_MM;
    ptUm.y = dTopLeftCtrY + (pt.y - m_nScreenHeight/2) * dResolutionY * UM_TO_MM;

    return getFrameOffsetByMm(ptUm, fOffsetValue);
}

bool TableCaliDataStorage::getFrameOffsetByUm(cv::Point2f& pt, float* fOffsetValue)
{
    pt.x *= UM_TO_MM;
    pt.y *= UM_TO_MM;
    return getFrameOffsetByMm(pt, fOffsetValue);
}

bool TableCaliDataStorage::getFrameOffsetByMm(cv::Point2f& pt, float* fOffsetValue)
{
    Vision::PR_CALC_FRAME_VALUE_CMD stCmd;
    Vision::PR_CALC_FRAME_VALUE_RPY stRpy;

    stCmd.vecVecRefFrameCenters = m_vecVecFrameCtr;
    stCmd.ptTargetFrameCenter.x = pt.x;
    stCmd.ptTargetFrameCenter.y = pt.y;

    for (int i = 0; i < _DLP_MAX_NUM; i++)
    {
        stCmd.vecVecRefFrameValues = m_vecVecFrameDLP[i];
        if (Vision::VisionStatus::OK == Vision::PR_CalcFrameValue(&stCmd, &stRpy)) {
            fOffsetValue[i] = stRpy.fResult;
        }
        else {
            System->setTrackInfo(QString(QStringLiteral("PR_CalcFrameValue error!")));
            return false;
        }
    }

    return true;
}
