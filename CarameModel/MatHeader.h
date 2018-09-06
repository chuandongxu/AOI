#ifndef _MAT_HEADER_
#define _MAT_HEADER_
#include <QDebug>
#include <QFile>

template<class T>
void printfMat(const cv::Mat &mat)
{
    for (short row = 0; row < mat.rows; ++row)
    {
        for (short col = 0; col < mat.cols; ++col)
        {
            qDebug() << QString::number(mat.at<T>(row, col), 'g', 8);
        }
        qDebug() << ("\n");
    }
}

template<class T>
void printIntMat(const cv::Mat &mat)
{
    for (short row = 0; row < mat.rows; ++row)
    {
        for (short col = 0; col < mat.cols; ++col)
        {
            qDebug() << QString::number(mat.at<T>(row, col), 10);            
        }
        qDebug() << ("\n");
    }
}

template<class T>
void printfMatToFile(const cv::Mat &mat, QString& fileName)
{
    QFile iniFile(fileName);
    if (iniFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        QTextStream out(&iniFile);

        for (short row = 0; row < mat.rows; ++row)
        {
            for (short col = 0; col < mat.cols; ++col)
            {
                out << QString::number(mat.at<T>(row, col), 'g', 8) << "\t";
            }
            out << ("\n");
        }
    }
    iniFile.close();
}

void printf2fVectorToFile(std::vector<cv::Point2f> vecPoints, QString& fileName)
{
    QFile iniFile(fileName);
    if (iniFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        QTextStream out(&iniFile);
        for (int i = 0; i < vecPoints.size(); i++)
        {
            cv::Point2f ptVeh = vecPoints[i];

            out << QString::number(ptVeh.x, 'g', 8) << "\t" << QString::number(ptVeh.y, 'g', 8);
            out << ("\n");
        }    
    }
    iniFile.close();
}

void printf3fVectorToFile(std::vector<cv::Point3f> vecPoints, QString& fileName)
{
    QFile iniFile(fileName);
    if (iniFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        QTextStream out(&iniFile);
        for (int i = 0; i < vecPoints.size(); i++)
        {
            cv::Point3f ptVeh = vecPoints[i];

            out << QString::number(ptVeh.x, 'g', 8) << "\t" << QString::number(ptVeh.y, 'g', 8) << "\t" << QString::number(ptVeh.z, 'g', 8);
            out << ("\n");
        }
    }
    iniFile.close();
}

template<typename _Tp>
inline std::vector<std::vector<_Tp>> matToVector(const cv::Mat &matInput) {
    std::vector<std::vector<_Tp>> vecVecArray;
    if (matInput.isContinuous()) {
        for (int row = 0; row < matInput.rows; ++row) {
            std::vector<_Tp> vecRow;
            int nRowStart = row * matInput.cols;
            vecRow.assign((_Tp *)matInput.datastart + nRowStart, (_Tp *)matInput.datastart + nRowStart + matInput.cols);
            vecVecArray.push_back(vecRow);
        }
    }
    else {
        for (int row = 0; row < matInput.rows; ++row) {
            std::vector<_Tp> vecRow;
            vecRow.assign((_Tp*)matInput.ptr<uchar>(row), (_Tp*)matInput.ptr<uchar>(row) +matInput.cols);
            vecVecArray.push_back(vecRow);
        }
    }
    return vecVecArray;
}

inline cv::Mat vectorToMat(const std::vector<std::vector<double>> &vecVecArray) {
    cv::Mat matResult(vecVecArray.size(), vecVecArray[0].size(), CV_64FC1);
    int nRow = 0;
    for (const auto &vecRow : vecVecArray) {
        cv::Mat matTarget(matResult, cv::Range(nRow, nRow + 1), cv::Range::all());
        cv::Mat matSrc(cv::Size(matResult.cols, 1), CV_64FC1, (void *)vecRow.data());
        matSrc.copyTo(matTarget);
        ++nRow;
    }
    return matResult;
}

#endif // _MAT_HEADER_
