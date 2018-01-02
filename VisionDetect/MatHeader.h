#ifndef _MAT_HEADER_
#define _MAT_HEADER_
#include <QDebug>
#include <QFile>

#define ToInt(value)                (static_cast<int>(value))

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

template<typename _tp>
inline cv::Mat intervals(_tp start, _tp interval, _tp end) {
	std::vector<_tp> vecValue;
	int nSize = ToInt((end - start) / interval);
	if (nSize <= 0) {
		static std::string msg = std::string(__FUNCTION__) + " input paramters are invalid.";
		throw std::exception(msg.c_str());
	}
	vecValue.reserve(nSize);
	_tp value = start;
	if (interval > 0) {
		while (value <= end) {
			vecValue.push_back(value);
			value += interval;
		}
	}
	else {
		while (value >= end) {
			vecValue.push_back(value);
			value += interval;
		}
	}
	//cv::Mat matResult ( vecValue ); //This have problem, because matResult share the memory with vecValue, after leave this function, the memory already released.
	return cv::Mat(vecValue).clone();
}


template<typename _tp>
inline void meshgrid(_tp xStart, _tp xInterval, _tp xEnd, _tp yStart, _tp yInterval, _tp yEnd, cv::Mat &matX, cv::Mat &matY) {
	cv::Mat matCol = intervals<_tp>(xStart, xInterval, xEnd);
	matCol = matCol.reshape(1, 1);

	cv::Mat matRow = intervals<_tp>(yStart, yInterval, yEnd);
	matX = cv::repeat(matCol, matRow.rows, 1);
	matY = cv::repeat(matRow, 1, matCol.cols);
}

#endif // _MAT_HEADER_
