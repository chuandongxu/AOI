#include "QColorWeight.h"

#include "qdebug.h"

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "qcustomplot.h"

#define ToInt(value)        (static_cast<int>(value))
#define ToFloat(param)      (static_cast<float>(param))

QColorWeight::QColorWeight(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	initUI();
	initData();
}

QColorWeight::~QColorWeight()
{
	if (m_customPlot)
	{
		delete m_customPlot;
		m_customPlot = NULL;
	}

	if (m_grayScene)
	{
		delete m_grayScene;
		m_grayScene = NULL;
	}

	if (m_grayColorScene)
	{
		delete m_grayColorScene;
		m_grayColorScene = NULL;
	}
}

void QColorWeight::initUI()
{
	// Gray Level
	connect(ui.comboBox_selectMode, SIGNAL(currentIndexChanged(int)), SLOT(onGrayModeIndexChanged(int)));
	QStringList ls;
	ls << QStringLiteral("手动设置阀值") << QStringLiteral("设置单阀值") << QStringLiteral("设置双阀值");
	ui.comboBox_selectMode->addItems(ls);

	connect(ui.checkBox_R, SIGNAL(stateChanged(int)), SLOT(onCheckBoxEnableR(int)));
	connect(ui.checkBox_G, SIGNAL(stateChanged(int)), SLOT(onCheckBoxEnableG(int)));
	connect(ui.checkBox_B, SIGNAL(stateChanged(int)), SLOT(onCheckBoxEnableB(int)));

	connect(ui.horizontalSlider_grayLeft, SIGNAL(valueChanged(int)), SLOT(onGrayLeftSliderChanged(int)));
	connect(ui.horizontalSlider_grayRight, SIGNAL(valueChanged(int)), SLOT(onGrayRightSliderChanged(int)));

	connect(ui.horizontalSlider_R, SIGNAL(valueChanged(int)), SLOT(onGrayRSliderChanged(int)));
	connect(ui.horizontalSlider_G, SIGNAL(valueChanged(int)), SLOT(onGrayGSliderChanged(int)));
	connect(ui.horizontalSlider_B, SIGNAL(valueChanged(int)), SLOT(onGrayBSliderChanged(int)));

	ui.horizontalSlider_grayLeft->setValue(100);
	ui.horizontalSlider_grayRight->setValue(200);

	ui.horizontalSlider_R->setValue(33);
	ui.horizontalSlider_G->setValue(33);
	ui.horizontalSlider_B->setValue(33);

	m_customPlot = new QCustomPlot(this);
	setupDateDemo(m_customPlot);
	ui.dockWidget_grayHitChart->setWidget(m_customPlot);

	m_grayScene = new QGraphicsScene();
	ui.graphicsView_gray->setScene(m_grayScene);
	ui.graphicsView_gray->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView_gray->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	int nWidth = ui.graphicsView_gray->geometry().width();
	int nHeight = ui.graphicsView_gray->geometry().height();
	
	cv::Mat mat(cv::Size(nWidth, nHeight), CV_8UC1);
	for (int x = 0; x < mat.cols; x++)
	{
		uchar grayValue = 255 * ((double)x / mat.cols);;
		for (int y = 0; y < mat.rows; y++)
		{
			uchar& pixel = mat.at<uchar>(y, x);
			pixel = grayValue;
		}
	}
	cvtColor(mat, mat, CV_GRAY2RGB);
	QImage image = QImage((uchar*)mat.data, mat.cols, mat.rows, ToInt(mat.step), QImage::Format_RGB888);
	m_grayScene->addPixmap(QPixmap::fromImage(image));

	// Color Space
	m_grayColorScene = new QGraphicsScene();
	ui.graphicsView_grayColor->setScene(m_grayColorScene);
	ui.graphicsView_grayColor->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView_grayColor->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	nWidth = ui.graphicsView_grayColor->geometry().width();
	nHeight = ui.graphicsView_grayColor->geometry().height();
	//ui.graphicsView_grayColor->setFixedWidth(nWidth);
	//ui.graphicsView_grayColor->setFixedHeight(nHeight);	

	cv::Mat matGrayColor = generateColorTrig(nWidth, nHeight, 0, 255, 0, 255, 0, 255);
	/*cv::Mat matGrayColor(cv::Size(nWidth, nHeight), CV_8UC3);
	for (int y = 0; y < matGrayColor.rows; y++)
	{
	for (int x = 0; x < matGrayColor.cols; x++)
	{
	int greenEdge = (nWidth / qAbs(qSin(2*M_PI - qAtan2(nHeight - y, x) - M_PI/3))) * qSin(M_PI/3);
	int buleEdge = (nWidth / qAbs(qSin(2*M_PI - qAtan2(nHeight - y, nWidth - x) - M_PI/3))) * qSin(M_PI/3);

	int greenValue = 255 * (1 - (qSqrt(qPow(x - 0, 2) + qPow(y - nHeight, 2)) / greenEdge));
	int buleValue = 255 * (1 - (qSqrt(qPow(x - nWidth, 2) + qPow(y - nHeight, 2)) / buleEdge));
	int redValue = 255 * (1 - (y / nHeight));

	if (greenValue < 0 || greenValue > 255 || buleValue < 0 || buleValue > 255 || redValue < 0 || redValue > 255)
	{
	greenValue = 255;
	buleValue = 255;
	redValue = 255;
	}

	cv::Vec3b& pixel = matGrayColor.at<cv::Vec3b>(y, x);
	pixel[0] = redValue;
	pixel[1] = greenValue;
	pixel[2] = buleValue;
	}
	}*/
	//cvtColor(matGrayColor, matGrayColor, CV_BGR2RGB);
	QImage imageColor = QImage((uchar*)matGrayColor.data, matGrayColor.cols, matGrayColor.rows, ToInt(matGrayColor.step), QImage::Format_RGB888);
	m_grayColorScene->addPixmap(QPixmap::fromImage(imageColor));
}

void QColorWeight::initData()
{
	clearData();

	m_nGrayLevelThreshold1 = 0;
	m_nGrayLevelThreshold2 = 0;
}

void QColorWeight::setupDateDemo(QCustomPlot *customPlot)
{	
	// set locale to english, so we get english month names:
	customPlot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));

	
	customPlot->addGraph();
	QPen pen;
	pen.setColor(QColor(0, 0, 255, 200));
	customPlot->graph()->setLineStyle(QCPGraph::lsLine);
	customPlot->graph()->setPen(pen);
	customPlot->graph()->setBrush(QBrush(QColor(0, 0, 150, 200)));

	// generate random walk data:
	QVector<double> keys; QVector<double> values;
	for (int i = 0; i < m_grayHitDatas.size(); ++i)
	{
		keys.push_back(i);
		values.push_back(getGrayValue(i));
	}
	customPlot->graph()->setData(keys, values);

	int nMaxValue = 0;
	for (int i = 0; i < values.size(); i++)
	{
		if (values[i] > nMaxValue) nMaxValue = values[i];
	}

	if (m_nGrayLevelThreshold1 > 0 && m_nGrayLevelThreshold1 < 255)
	{
		customPlot->addGraph();
		QPen pen;
		pen.setColor(QColor(255, 0, 0, 200));
		customPlot->graph()->setLineStyle(QCPGraph::lsLine);
		customPlot->graph()->setPen(pen);
		customPlot->graph()->setBrush(QBrush(QColor(255, 0, 0, 200)));

		// generate random walk data:
		QVector<double> keys; QVector<double> values;
		keys.push_back(m_nGrayLevelThreshold1-1);
		keys.push_back(m_nGrayLevelThreshold1);
		keys.push_back(m_nGrayLevelThreshold1+1);
		values.push_back(nMaxValue);
		values.push_back(nMaxValue);
		values.push_back(nMaxValue);
		customPlot->graph()->setData(keys, values);
	}

	if (m_nGrayLevelThreshold2 > 0 && m_nGrayLevelThreshold2 < 255)
	{
		customPlot->addGraph();
		QPen pen;
		pen.setColor(QColor(255, 0, 0, 200));
		customPlot->graph()->setLineStyle(QCPGraph::lsLine);
		customPlot->graph()->setPen(pen);
		customPlot->graph()->setBrush(QBrush(QColor(255, 0, 0, 200)));

		// generate random walk data:
		QVector<double> keys; QVector<double> values;
		keys.push_back(m_nGrayLevelThreshold2 - 1);
		keys.push_back(m_nGrayLevelThreshold2);
		keys.push_back(m_nGrayLevelThreshold2 + 1);
		values.push_back(nMaxValue);
		values.push_back(nMaxValue);
		values.push_back(nMaxValue);
		customPlot->graph()->setData(keys, values);
	}

	//// configure bottom axis to show date and time instead of number:
	//customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
	//customPlot->xAxis->setDateTimeFormat("MMMM\nyyyy");
	//// set a more compact font size for bottom and left axis tick labels:
	//customPlot->xAxis->setTickLabelFont(QFont(QFont().family(), 8));
	//customPlot->yAxis->setTickLabelFont(QFont(QFont().family(), 8));
	//// set a fixed tick-step to one tick per month:
	//customPlot->xAxis->setAutoTickStep(false);
	//customPlot->xAxis->setTickStep(2628000); // one month in seconds
	//customPlot->xAxis->setSubTickCount(3);
	//// apply manual tick and tick label for left axis:
	//customPlot->yAxis->setAutoTicks(false);
	//customPlot->yAxis->setAutoTickLabels(false);
	//customPlot->yAxis->setTickVector(QVector<double>() << 5 << 55);
	//customPlot->yAxis->setTickVectorLabels(QVector<QString>() << "Not so\nhigh" << "Very\nhigh");

	// set axis labels:
	customPlot->xAxis->setLabel("Gray Level");
	customPlot->yAxis->setLabel("Pixel count");

	// make top and right axes visible but without ticks and labels:
	customPlot->xAxis->setVisible(true);
	customPlot->yAxis->setVisible(true);
	customPlot->xAxis->setTicks(true);
	customPlot->yAxis->setTicks(false);
	customPlot->xAxis->setTickLabels(true);
	customPlot->yAxis->setTickLabels(false);

	// set axis ranges to show all data:
	customPlot->xAxis->setRange(0, 255);
	customPlot->yAxis->setRange(0, nMaxValue);

	// zoom out a bit:
	customPlot->yAxis->scaleRange(1.1, customPlot->yAxis->range().center());
	customPlot->xAxis->scaleRange(1.0, customPlot->xAxis->range().center());

	// show legend:
	customPlot->legend->setVisible(false);

	customPlot->axisRect()->setupFullAxesBox();
}

int QColorWeight::calcGrayValue(cv::Scalar& pixel)
{
	int nValueR = ui.horizontalSlider_R->value();
	int nValueG = ui.horizontalSlider_G->value();
	int nValueB = ui.horizontalSlider_B->value();

	bool bUsedR = ui.checkBox_R->isChecked();
	bool bUsedG = ui.checkBox_G->isChecked();
	bool bUsedB = ui.checkBox_B->isChecked();

	int nGrayValue = 0;

	nGrayValue += pixel[0] * (nValueR / 100.0) * (bUsedR ? 1 : 0);
	nGrayValue += pixel[1] * (nValueG / 100.0) * (bUsedG ? 1 : 0);
	nGrayValue += pixel[2] * (nValueB / 100.0) * (bUsedB ? 1 : 0);

	return nGrayValue;
}

void QColorWeight::generateGrayPlot()
{
	if (m_imageMat.empty()) return;

	clearData();

	unsigned char grayValueB, grayValueG, grayValueR;
	for (int y = 0; y < m_imageMat.rows; y++)
	{
		for (int x = 0; x < m_imageMat.cols; x++)
		{
			cv::Vec3b& pixel = m_imageMat.at<cv::Vec3b>(y, x);
		
			grayValueR = pixel[0];
			grayValueG = pixel[1];
			grayValueB = pixel[2];

			int nGrayValue = calcGrayValue(cv::Scalar(grayValueR, grayValueG, grayValueB));
			if (nGrayValue > 255) nGrayValue = 255;

			incrementGrayValue(nGrayValue);
		}
	}

	if (m_customPlot)
	{
		delete m_customPlot;
		m_customPlot = new QCustomPlot(this);
		setupDateDemo(m_customPlot);
		ui.dockWidget_grayHitChart->setWidget(m_customPlot);
	}
}

void QColorWeight::clearData()
{
	m_grayHitDatas.clear();
	for (int i = 0; i < 255; i++)
	{
		m_grayHitDatas.insert(i, 0);
	}
}

int QColorWeight::getGrayValue(int nGrayLevel)
{
	if (nGrayLevel < 0 || nGrayLevel > 255) return 0;

	return m_grayHitDatas.value(nGrayLevel);
}

void QColorWeight::setGrayValue(int nGrayLevel, int nValue)
{
	if (nGrayLevel < 0 || nGrayLevel > 255) return;

	m_grayHitDatas[nGrayLevel] = nValue;
}

int QColorWeight::incrementGrayValue(int nGrayLevel)
{
	if (nGrayLevel < 0 || nGrayLevel > 255) return 0;

	m_grayHitDatas[nGrayLevel] = m_grayHitDatas[nGrayLevel] + 1;

	return m_grayHitDatas[nGrayLevel];
}

void QColorWeight::setImage(cv::Mat& img)
{
	if (img.type() != CV_8UC3) return;

	m_imageMat = img;	
}

cv::Mat QColorWeight::generateGrayImage(cv::Point2f ptPos)
{
	cv::Mat matGray(m_imageMat.size(), CV_8UC3);

	generateGrayPlot();

	return matGray;
}

cv::Mat QColorWeight::generateColorImage(cv::Point2f ptPos)
{
	cv::Mat matColor(m_imageMat.size(), CV_8UC3);

	return matColor;
}

void QColorWeight::onGrayModeIndexChanged(int iIndex)
{
	int nIndex = ui.comboBox_selectMode->currentIndex();
}

void QColorWeight::onCheckBoxEnableR(int iState)
{
	if (iState == Qt::Checked)
	{		
	}
	else
	{		
	}

	generateGrayPlot();
}

void QColorWeight::onCheckBoxEnableG(int iState)
{
	if (iState == Qt::Checked)
	{
	}
	else
	{
	}

	generateGrayPlot();
}

void QColorWeight::onCheckBoxEnableB(int iState)
{
	if (iState == Qt::Checked)
	{
	}
	else
	{
	}

	generateGrayPlot();
}

void QColorWeight::onGrayLeftSliderChanged(int i)
{
	QString str = QString::number(i);
	ui.lineEdit_grayLeft->setText(str);

	m_nGrayLevelThreshold1 = i;

	generateGrayPlot();
}

void QColorWeight::onGrayRightSliderChanged(int i)
{
	QString str = QString::number(i);
	ui.lineEdit_grayRight->setText(str);

	m_nGrayLevelThreshold2 = i;

	generateGrayPlot();
}

void QColorWeight::onGrayRSliderChanged(int i)
{
	QString str = QString::number(i);
	ui.lineEdit_R->setText(str);

	generateGrayPlot();
}

void QColorWeight::onGrayGSliderChanged(int i)
{
	QString str = QString::number(i);
	ui.lineEdit_G->setText(str);

	generateGrayPlot();
}

void QColorWeight::onGrayBSliderChanged(int i)
{
	QString str = QString::number(i);
	ui.lineEdit_B->setText(str);

	generateGrayPlot();
}

cv::Mat QColorWeight::generateColorTrig(int nWidth, int nHeight, int nMinR, int nMaxR, int nMinG, int nMaxG, int nMinB, int nMaxB)
{
	double k = double(qMax(qMax(nMaxR, nMaxG), nMaxB)) / 255;

	//double minR = double(nMinR) / k / 255;
	//double maxR = double(nMaxR) / k / 255;
	//double minG = double(nMinG) / k / 255;
	//double maxG = double(nMaxG) / k / 255;
	//double minB = double(nMinB) / k / 255;
	//double maxB = double(nMaxB) / k / 255;

	cv::Mat xy(nHeight, nWidth, CV_32FC3);
	for (int y = 0; y < xy.rows; y++)
	{
		for (int x = 0; x < xy.cols; x++)
		{
			cv::Vec3f& data = xy.at<cv::Vec3f>(y, x);
			data[0] = -1 + x * 2.0 / (xy.cols - 1);
			data[1] = 1 - y * 2.0 / (xy.rows - 1);
			data[2] = qAtan2(data[1], data[0]);
		}
	}

	int m = xy.rows;
	int n = xy.cols;

	cv::Mat image(m, n, CV_8UC3);
	//cv::Mat z = cv::Mat::zeros(m, n, CV_8UC1);
	cv::Mat R = cv::Mat::ones(m, n, CV_32FC1);
	cv::Mat G = cv::Mat::ones(m, n, CV_32FC1);
	cv::Mat B = cv::Mat::ones(m, n, CV_32FC1);

	for (int y = 0; y < xy.rows; y++)
	{
		for (int x = 0; x < xy.cols; x++)
		{
			cv::Vec3f& data = xy.at<cv::Vec3f>(y, x);
			double xData = data[0]; double yData = data[1]; double angle = data[2];
			if (xData == 0 && yData == 0)
			{
				float& r = R.at<float>(y, x);
				r = 1 * k;
				float& g = G.at<float>(y, x);
				g = 1 * k;
				float& b = B.at<float>(y, x);
				b = 1 * k;
			}
		}
	}

	for (int y = 0; y < xy.rows; y++)
	{
		for (int x = 0; x < xy.cols; x++)
		{
			cv::Vec3f& data = xy.at<cv::Vec3f>(y, x);
			double xData = data[0]; double yData = data[1]; double angle = data[2];
			
			if ((angle >= (-5.0 / 6 * M_PI)) && (angle < (-1.0 / 2 * M_PI)) && (yData > (-1.0 / 2)))
			{
				float& r = R.at<float>(y, x);
				r = (1 + yData * 2)*k;
				float& g = G.at<float>(y, x);
				g = 1 * k;
				float& b = B.at<float>(y, x);
				b = (1 + xData * 2 / qSqrt(3))*k;
			}
			else if ((angle >= (-1.0 / 2 * M_PI)) && (angle < (-1.0 / 6 * M_PI)) && (yData > (-1.0 / 2)))
			{
				float& r = R.at<float>(y, x);
				r = (1 + yData * 2)*k;
				float& g = G.at<float>(y, x);
				g = (1 - xData * 2 / qSqrt(3))*k;
				float& b = B.at<float>(y, x);
				b = 1 * k;
			}
		}
	}

	for (int y = 0; y < xy.rows; y++)
	{
		for (int x = 0; x < xy.cols; x++)
		{
			cv::Vec3f& data = xy.at<cv::Vec3f>(y, x);
			double xData = data[0]; double yData = data[1]; double angle = data[2];			

			double theta = 2 * M_PI / 3;
			double xData1 = xData*qCos(theta) + yData*qSin(theta);
			double yData1 = -xData*qSin(theta) + yData*qCos(theta);
			double angle1 = qAtan2(yData1, xData1);

			if ((angle1 >= (-5.0 / 6 * M_PI)) && (angle1 < (-1.0 / 2 * M_PI)) && (yData1 > (-1.0 / 2)))
			{
				float& r = R.at<float>(y, x);
				r = (1 + xData1 * 2 / qSqrt(3))*k;
				float& g = G.at<float>(y, x);
				g = (1 + yData1 * 2)*k;
				float& b = B.at<float>(y, x);
				b = 1 * k;
			}
			else if ((angle1 >= (-1.0 / 2 * M_PI)) && (angle1 < (-1.0 / 6 * M_PI)) && (yData1 > (-1.0 / 2)))
			{
				float& r = R.at<float>(y, x);
				r = 1 * k;
				float& g = G.at<float>(y, x);
				g = (1 + yData1 * 2)*k;
				float& b = B.at<float>(y, x);
				b = (1 - xData1 * 2 / qSqrt(3))*k;
			}
		}
	}

	for (int y = 0; y < xy.rows; y++)
	{
		for (int x = 0; x < xy.cols; x++)
		{
			cv::Vec3f& data = xy.at<cv::Vec3f>(y, x);
			double xData = data[0]; double yData = data[1]; double angle = data[2];			

			double theta = 4 * M_PI / 3;
			double xData2 = xData*qCos(theta) + yData*qSin(theta);
			double yData2 = -xData*qSin(theta) + yData*qCos(theta);
			double angle2 = qAtan2(yData2, xData2);

			if ((angle2 >= (-5.0 / 6 * M_PI)) && (angle2 < (-1.0 / 2 * M_PI)) && (yData2 > (-1.0 / 2)))
			{
				float& r = R.at<float>(y, x);
				r = 1 * k;
				float& g = G.at<float>(y, x);
				g = (1 + xData2 * 2 / qSqrt(3))*k;
				float& b = B.at<float>(y, x);
				b = (1 + yData2 * 2)*k;
			}
			else if ((angle2 >= (-1.0 / 2 * M_PI)) && (angle2 < (-1.0 / 6 * M_PI)) && (yData2 > (-1.0 / 2)))
			{
				float& r = R.at<float>(y, x);
				r = (1 - xData2 * 2 / qSqrt(3))*k;
				float& g = G.at<float>(y, x);
				g = 1 * k;
				float& b = B.at<float>(y, x);
				b = (1 + yData2 * 2)*k;
			}
		}
	}

	for (int y = 0; y < xy.rows; y++)
	{
		for (int x = 0; x < xy.cols; x++)
		{
			cv::Vec3f& data = xy.at<cv::Vec3f>(y, x);
			double xData = data[0]; double yData = data[1]; double angle = data[2];			

			//qDebug() <<xData << " " << yData << " " << angle;
			//qDebug() << R.at<float>(y, x) << " " << G.at<float>(y, x) << " " << B.at<float>(y, x);

			cv::Vec3b& pixel = image.at<cv::Vec3b>(y, x);
			pixel[0] = R.at<float>(y, x) * 255;
			pixel[1] = G.at<float>(y, x) * 255;
			pixel[2] = B.at<float>(y, x) * 255;
		}
	}

	return image;
}
