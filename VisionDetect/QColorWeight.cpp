#include "QColorWeight.h"
#include "qdebug.h"

#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"

#include "DataStoreAPI.h"
#include "VisionAPI.h"
#include "qcustomplot.h"
#include "../Common/SystemData.h"
#include "../Common/eos.h"
#include "../include/IVisionUI.h"
#include "../include/IdDefine.h"
#include "../Common/ModuleMgr.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#define ToInt(value)        (static_cast<int>(value))
#define ToFloat(param)      (static_cast<float>(param))

using namespace NFG::AOI;
using namespace AOI;

#if defined(CreateWindow) // If Win32 defines "CreateWindow":
#undef CreateWindow       //   Undefine it to avoid conflict
#endif                    //   with the line display method.

const int IMG_DISPLAY_WIDTH = 200;
const int IMG_DISPLAY_HEIGHT = 150;

class QColorWeight;
class ColorScene : public QGraphicsScene
{
public:
	explicit ColorScene(QObject *parent = 0);
	void setColorWeight(QColorWeight* colorWeight) { m_colorWeight = colorWeight; }

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
private:
	QColorWeight* m_colorWeight;
};

ColorScene::ColorScene(QObject *parent)
:QGraphicsScene(parent)
{
	clearFocus();
}

void ColorScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	//qDebug() << event->scenePos().x() << " " << event->scenePos().y();
	if (m_colorWeight)
	{
		m_colorWeight->setColorImagePos(cv::Point(event->scenePos().x(), event->scenePos().y()));
	}
	QGraphicsScene::mousePressEvent(event);
}

QColorWeight::QColorWeight(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    setWindowFlags(Qt::WindowStaysOnTopHint);

	initUI();
	initData();

    setFixedSize(size());

    QEos::Attach(EVENT_COLOR_WIDGET_STATE, this, SLOT(onColorWidgetState(const QVariantList &)));
}

QColorWeight::~QColorWeight()
{
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

	if (m_grayImgScene)
	{
		delete m_grayImgScene;
		m_grayImgScene = NULL;
	}

	if (m_colorImgScene)
	{
		delete m_colorImgScene;
		m_colorImgScene = NULL;
	}
}

//void QColorWeight::closeEvent(QCloseEvent *e){
//	//qDebug() << "关闭事件";
//	//e->ignore();
//
//	this->hide();
//}

void QColorWeight::setImage(const cv::Mat& img)
{
    if (img.empty()) return;
	if (img.type() != CV_8UC3) return;

	m_matSrcImage = img;   

	displaySourceImg();
}

void QColorWeight::setGrayParams(const GrayWeightParams& grayParams)
{
	ui.comboBox_selectMode->setCurrentIndex(grayParams.enMethod);
	ui.horizontalSlider_R->setValue(grayParams.nRScale);   
	ui.horizontalSlider_G->setValue(grayParams.nGScale);
	ui.horizontalSlider_B->setValue(grayParams.nBScale);
    ui.lineEdit_R->setText(QString::number(grayParams.nRScale));
    ui.lineEdit_G->setText(QString::number(grayParams.nGScale));
    ui.lineEdit_B->setText(QString::number(grayParams.nBScale));
	ui.horizontalSlider_grayLeft->setValue(grayParams.nThreshold1);
	ui.horizontalSlider_grayRight->setValue(grayParams.nThreshold2);
}

GrayWeightParams QColorWeight::getGrayParams() const
{
	GrayWeightParams grayParams;

	int nIndex = ui.comboBox_selectMode->currentIndex();	
	grayParams.enMethod = static_cast<GRAY_WEIGHT_METHOD>(nIndex);

    grayParams.bEnableR = ui.checkBox_R->isChecked();
    grayParams.bEnableG = ui.checkBox_G->isChecked();
    grayParams.bEnableB = ui.checkBox_B->isChecked();
	grayParams.nRScale = ui.horizontalSlider_R->value();
	grayParams.nGScale = ui.horizontalSlider_G->value();
	grayParams.nBScale = ui.horizontalSlider_B->value();
	grayParams.nThreshold1 = ui.horizontalSlider_grayLeft->value();
	grayParams.nThreshold2 = ui.horizontalSlider_grayRight->value();

	return grayParams;
}

cv::Mat QColorWeight::generateGrayImage(cv::Point ptPos)
{
	//cv::Mat matGray(m_matSrcImage.size(), CV_8UC3);
	ui.tabWidget->setCurrentIndex(0);
	m_grayGenPt = ptPos;

	m_maskMat.release();
	generateGrayPlot();
	return m_maskMat;
}

void QColorWeight::setColorParams(const ColorSpaceParams& colorParams)
{
	ui.horizontalSlider_Rn->setValue(colorParams.nRThreshold);
	ui.horizontalSlider_Tn->setValue(colorParams.nTThreshold);
}

ColorSpaceParams QColorWeight::getColorParams() const
{
	ColorSpaceParams colorParams;

	colorParams.nRThreshold = ui.horizontalSlider_Rn->value();
	colorParams.nTThreshold = ui.horizontalSlider_Tn->value();

	return colorParams;
}

cv::Mat QColorWeight::generateColorImage(cv::Point ptPos)
{
	//cv::Mat matColor(m_matSrcImage.size(), CV_8UC3);
	ui.tabWidget->setCurrentIndex(1);
	m_colorGenPt = ptPos;

	m_maskMat.release();
	generateColorPlot();
	return m_maskMat;
}

void QColorWeight::setColorImagePos(cv::Point ptMousePos)
{
    if (m_bSetColor) return;

	ui.tabWidget->setCurrentIndex(1);
	m_colorGenPt.x = ptMousePos.x * m_matSrcImage.size().width  / IMG_DISPLAY_WIDTH;
	m_colorGenPt.y = ptMousePos.y * m_matSrcImage.size().height / IMG_DISPLAY_HEIGHT;

	generateColorPlot();
}

void QColorWeight::holdColorImage(cv::Vec3b color, int nRn, int nTn)
{   
    ui.tabWidget->setCurrentIndex(1);

    m_bSetColor = true;
    m_color = color;
    ui.horizontalSlider_Rn->setValue(nRn);
    ui.horizontalSlider_Tn->setValue(nTn);
    
    generateColorPlot();
}

void QColorWeight::releaseColorImage()
{
    m_bSetColor = false;
}

void  QColorWeight::getColorParams(int& nRn, int& nTn)
{
    nRn = ui.horizontalSlider_Rn->value();
    nTn = ui.horizontalSlider_Tn->value();
}

void QColorWeight::initUI()
{
	// Gray Level
	connect(ui.comboBox_selectMode, SIGNAL(currentIndexChanged(int)), SLOT(onGrayModeIndexChanged(int)));
	QStringList ls;
	ls << QStringLiteral("取点阀值") << QStringLiteral("设置单阀值") << QStringLiteral("设置双阀值");
	ui.comboBox_selectMode->addItems(ls);

	connect(ui.checkBox_R, SIGNAL(stateChanged(int)), SLOT(onCheckBoxEnableR(int)));
	connect(ui.checkBox_G, SIGNAL(stateChanged(int)), SLOT(onCheckBoxEnableG(int)));
	connect(ui.checkBox_B, SIGNAL(stateChanged(int)), SLOT(onCheckBoxEnableB(int)));

	connect(ui.horizontalSlider_grayLeft, SIGNAL(valueChanged(int)), SLOT(onGrayLeftSliderChanged(int)));
	connect(ui.horizontalSlider_grayRight, SIGNAL(valueChanged(int)), SLOT(onGrayRightSliderChanged(int)));

    connect(ui.checkBox_invert, SIGNAL(stateChanged(int)), SLOT(onCheckBoxInvert(int)));

	connect(ui.horizontalSlider_R, SIGNAL(valueChanged(int)), SLOT(onGrayRSliderChanged(int)));
	connect(ui.horizontalSlider_G, SIGNAL(valueChanged(int)), SLOT(onGrayGSliderChanged(int)));
	connect(ui.horizontalSlider_B, SIGNAL(valueChanged(int)), SLOT(onGrayBSliderChanged(int)));

	ui.horizontalSlider_grayLeft->setValue(0);
	ui.horizontalSlider_grayRight->setValue(255);

	ui.horizontalSlider_R->setValue(100);
	ui.horizontalSlider_G->setValue(100);
	ui.horizontalSlider_B->setValue(100);
    ui.lineEdit_R->setText(QString::number(100));
    ui.lineEdit_G->setText(QString::number(100));
    ui.lineEdit_B->setText(QString::number(100));

	m_customPlot = std::make_shared<QCustomPlot>();    
	setupDateDemo(m_customPlot, true);
    m_customPlot->setFixedSize(350, 200);
    m_plotSeneGrayWeight = std::make_shared<QGraphicsScene>();
    ui.graphicsView_grayHitChart->setScene(m_plotSeneGrayWeight.get());
    ui.graphicsView_grayHitChart->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_grayHitChart->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_grayHitChart->setFixedSize(350, 200);
    m_plotSeneGrayWeight->addPixmap(m_customPlot->grab());   

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

	connect(ui.horizontalSlider_Rn, SIGNAL(valueChanged(int)), SLOT(onColorRnSliderChanged(int)));
	connect(ui.horizontalSlider_Tn, SIGNAL(valueChanged(int)), SLOT(onColorTnSliderChanged(int)));

	ui.horizontalSlider_Rn->setValue(30);
	ui.horizontalSlider_Tn->setValue(50);

	m_maxR = 255;
	m_minR = 0;
	m_maxG = 255;
	m_minG = 0;
	m_maxB = 255;
	m_minB = 0;
	m_maxT = 255;
	m_minT = 0;

	// Color Space
	m_grayColorScene = new QGraphicsScene();
	ui.graphicsView_grayColor->setScene(m_grayColorScene);
	ui.graphicsView_grayColor->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView_grayColor->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView_grayColor->fitInView(QRectF(0, 0, 300, 300), Qt::KeepAspectRatio);    //这样就没法缩放了 
	ui.graphicsView_grayColor->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	ui.graphicsView_grayColor->setRenderHint(QPainter::Antialiasing);

	nWidth = ui.graphicsView_grayColor->geometry().width();
	nHeight = ui.graphicsView_grayColor->geometry().height();

	cv::Mat matGrayColor = generateColorTrig(nWidth, nHeight, 0, 255, 0, 255, 0, 255);
	//cvtColor(matGrayColor, matGrayColor, CV_BGR2RGB);
	QImage imageColor = QImage((uchar*)matGrayColor.data, matGrayColor.cols, matGrayColor.rows, ToInt(matGrayColor.step), QImage::Format_RGB888);
	m_grayColorScene->addPixmap(QPixmap::fromImage(imageColor));

	m_customPlotR = std::make_shared<QCustomPlot>();
    m_regenR = new QCPBars(m_customPlotR->xAxis, m_customPlotR->yAxis);
    setupDateColor(m_customPlotR, m_regenR, 0, true);
    m_customPlotR->setFixedSize(360, 90);
    m_plotSeneR = std::make_shared<QGraphicsScene>();
    ui.graphicsView_R->setScene(m_plotSeneR.get());
    ui.graphicsView_R->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_R->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_R->setFixedSize(360, 90);
    m_plotSeneR->addPixmap(m_customPlotR->grab());

	m_customPlotG = std::make_shared<QCustomPlot>();
    m_regenG = new QCPBars(m_customPlotG->xAxis, m_customPlotG->yAxis);
    setupDateColor(m_customPlotG, m_regenG, 1, true);
    m_customPlotG->setFixedSize(360, 90);
    m_plotSeneG = std::make_shared<QGraphicsScene>();
    ui.graphicsView_G->setScene(m_plotSeneG.get());
    ui.graphicsView_G->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_G->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_G->setFixedSize(360, 90);
    m_plotSeneG->addPixmap(m_customPlotG->grab());

	m_customPlotB = std::make_shared<QCustomPlot>();
    m_regenB = new QCPBars(m_customPlotB->xAxis, m_customPlotB->yAxis);
    setupDateColor(m_customPlotB, m_regenB, 2, true);
    m_customPlotB->setFixedSize(360, 90);
    m_plotSeneB = std::make_shared<QGraphicsScene>();
    ui.graphicsView_B->setScene(m_plotSeneB.get());
    ui.graphicsView_B->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_B->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_B->setFixedSize(360, 90);
    m_plotSeneB->addPixmap(m_customPlotB->grab());

	m_customPlotGray = std::make_shared<QCustomPlot>();	
    m_regenGray = new QCPBars(m_customPlotGray->xAxis, m_customPlotGray->yAxis);
    setupDateColor(m_customPlotGray, m_regenGray, 3, true);
    m_customPlotGray->setFixedSize(360, 90);
    m_plotSeneGray = std::make_shared<QGraphicsScene>();
    ui.graphicsView_Gray->setScene(m_plotSeneGray.get());
    ui.graphicsView_Gray->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_Gray->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.graphicsView_Gray->setFixedSize(360, 90);
    m_plotSeneGray->addPixmap(m_customPlotGray->grab());

	// Image Display
	m_sourceImgScene = new QGraphicsScene();
	ui.graphicsView_sourceImg->setScene(m_sourceImgScene);
	ui.graphicsView_sourceImg->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView_sourceImg->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView_sourceImg->fitInView(QRectF(0, 0, IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT), Qt::KeepAspectRatio);    //这样就没法缩放了 
	ui.graphicsView_sourceImg->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	ui.graphicsView_sourceImg->setRenderHint(QPainter::Antialiasing);

	m_grayImgScene = new QGraphicsScene();
	ui.graphicsView_grayImg->setScene(m_grayImgScene);
	ui.graphicsView_grayImg->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView_grayImg->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView_grayImg->fitInView(QRectF(0, 0, IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT), Qt::KeepAspectRatio);    //这样就没法缩放了 
	ui.graphicsView_grayImg->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	ui.graphicsView_grayImg->setRenderHint(QPainter::Antialiasing);

	m_colorImgScene = new ColorScene();
	m_colorImgScene->setColorWeight(this);
	ui.graphicsView_ColorImg->setScene(m_colorImgScene);
	ui.graphicsView_ColorImg->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView_ColorImg->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView_ColorImg->fitInView(QRectF(0, 0, IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT), Qt::KeepAspectRatio);    //这样就没法缩放了 
	ui.graphicsView_ColorImg->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	ui.graphicsView_ColorImg->setRenderHint(QPainter::Antialiasing);	
}

void QColorWeight::initData()
{
	clearGrayData();
	clearColorData(0);
	clearColorData(1);
	clearColorData(2);
	clearColorData(3);

	m_nGrayLevelThreshold1 = 0;
	m_nGrayLevelThreshold2 = 255;
    ui.comboBox_selectMode->setCurrentIndex(1);

    m_bSetColor = false;
}

std::string QColorWeight::getJsonFormattedParams() const
{
    GrayWeightParams stGrayParams = getGrayParams();
	ColorSpaceParams stColorParams = getColorParams();

	QJsonObject json;
	json.insert("Method", stGrayParams.enMethod);
    json["EnableB"] = stGrayParams.bEnableB;    
    json["EnableG"] = stGrayParams.bEnableG;
    json["EnableR"] = stGrayParams.bEnableR;
	json.insert("GrayScaleB", stGrayParams.nBScale);
	json.insert("GrayScaleG", stGrayParams.nGScale);
	json.insert("GrayScaleR", stGrayParams.nRScale);
	json.insert("GrayThreshold1", stGrayParams.nThreshold1);
	json.insert("GrayThreshold2", stGrayParams.nThreshold2);

	json.insert("ColorRThreshold", stColorParams.nRThreshold);
	json.insert("ColorTThreshold", stColorParams.nTThreshold);
    json.insert("PickPointX", m_colorGenPt.x);
	json.insert("PickPointY", m_colorGenPt.y);

    json["Invert"] = ui.checkBox_invert->isChecked();

	QJsonDocument document;
	document.setObject(json);
	QByteArray byteArray = document.toJson(QJsonDocument::Compact);
    return std::string(byteArray);
}

void QColorWeight::setJsonFormattedParams(const std::string &jsonParams)
{
    GrayWeightParams stGrayParams;
    ColorSpaceParams stColorParams;

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(jsonParams.c_str(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        QMessageBox::critical(this, QStringLiteral("Color Weight"), QStringLiteral("Json data is invalid."), QStringLiteral("Quit"));
        return;
    }

    if (! parse_doucment.isObject())
        return;

    QJsonObject obj = parse_doucment.object();

    stGrayParams.enMethod = static_cast<GRAY_WEIGHT_METHOD>(obj["Method"].toInt());
    stGrayParams.bEnableB = obj["EnableB"].toBool();
    stGrayParams.bEnableG = obj["EnableG"].toBool();
    stGrayParams.bEnableR = obj["EnableR"].toBool();
    stGrayParams.nBScale = obj["GrayScaleB"].toInt();
    stGrayParams.nGScale = obj["GrayScaleG"].toInt();
    stGrayParams.nRScale = obj["GrayScaleR"].toInt();
    stGrayParams.nThreshold1 = obj["GrayThreshold1"].toInt();
    stGrayParams.nThreshold2 = obj["GrayThreshold2"].toInt();

    stColorParams.nRThreshold = obj["ColorRThreshold"].toInt();
    stColorParams.nTThreshold = obj["ColorTThreshold"].toInt();
    m_colorGenPt.x = obj["PickPointX"].toInt();
	m_colorGenPt.y = obj["PickPointY"].toInt();

    ui.checkBox_invert->setChecked(obj["Invert"].toBool());

    setGrayParams(stGrayParams);
    setColorParams(stColorParams);

    if (GRAY_WEIGHT_METHOD::EM_MODE_ONE_THRESHOLD == stGrayParams.enMethod || GRAY_WEIGHT_METHOD::EM_MODE_TWO_THRESHOLD == stGrayParams.enMethod) {
        ui.tabWidget->setCurrentIndex(0);
        ui.comboBox_selectMode->setCurrentIndex(stGrayParams.enMethod);
        generateGrayPlot();
    }
    else {
        ui.tabWidget->setCurrentIndex(1);
        generateColorPlot();
    }
}

void QColorWeight::setupDateDemo(std::shared_ptr<QCustomPlot> customPlot, bool bCreate)
{
    if (bCreate)
    {
        // set locale to english, so we get english month names:
        customPlot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));
    }

    customPlot->clearGraphs();

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
        keys.push_back(m_nGrayLevelThreshold1 - 1);
        keys.push_back(m_nGrayLevelThreshold1);
        keys.push_back(m_nGrayLevelThreshold1 + 1);
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

    if (bCreate)
    {
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

        // show legend:
        customPlot->legend->setVisible(false);
        customPlot->axisRect()->setupFullAxesBox();
    }

	// set axis ranges to show all data:
	customPlot->xAxis->setRange(0, 255);
	customPlot->yAxis->setRange(0, nMaxValue);

	// zoom out a bit:
	customPlot->yAxis->scaleRange(1.1, customPlot->yAxis->range().center());
	customPlot->xAxis->scaleRange(1.0, customPlot->xAxis->range().center());
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

	nGrayValue += pixel[2] * (nValueR / 100.0) * (bUsedR ? 1 : 0);
	nGrayValue += pixel[1] * (nValueG / 100.0) * (bUsedG ? 1 : 0);
	nGrayValue += pixel[0] * (nValueB / 100.0) * (bUsedB ? 1 : 0);

	if (nGrayValue > 255) nGrayValue = 255;
	return nGrayValue;
}

cv::Mat QColorWeight::_transformColorImage(const cv::Mat &matInput)
{
    if (matInput.type() != CV_8UC3)
        return matInput;

    std::vector<cv::Mat> vecChannels;
    cv::split(matInput, vecChannels);

    int nScaleR = ui.horizontalSlider_R->value();
	int nScaleG = ui.horizontalSlider_G->value();
	int nScaleB = ui.horizontalSlider_B->value();
    bool bEnableR = ui.checkBox_R->isChecked();
	bool bEnableG = ui.checkBox_G->isChecked();
	bool bEnableB = ui.checkBox_B->isChecked();

    float fRatioR = bEnableR ? nScaleR / 100.0f : 0.f;
    float fRatioG = bEnableG ? nScaleG / 100.0f : 0.f;
    float fRatioB = bEnableB ? nScaleB / 100.0f : 0.f;

    vecChannels[0] = vecChannels[0] * fRatioB;
    vecChannels[1] = vecChannels[1] * fRatioG;
    vecChannels[2] = vecChannels[2] * fRatioR;

    cv::Mat matResult;
    cv::merge(vecChannels, matResult);
    return matResult;
}

cv::Mat QColorWeight::_convertToGrayImage()
{
    cv::Mat matResult;
    cv::cvtColor(m_matSrcTransform, matResult, CV_BGR2GRAY);
    return matResult;
}

void QColorWeight::generateGrayPlot()
{
	if (m_matSrcTransform.empty()) return;

	clearGrayData();

    cv::Mat matGrayLocal = _convertToGrayImage();

	for (int y = 0; y < m_matSrcImage.rows; y++)
		for (int x = 0; x < m_matSrcImage.cols; x++)
			incrementGrayValue(matGrayLocal.at<uchar>(y,x));

    setupDateDemo(m_customPlot);
    m_customPlot->setFixedSize(350, 200);
    m_plotSeneGrayWeight->clear();
    m_plotSeneGrayWeight->addPixmap(m_customPlot->grab());
    ui.graphicsView_grayHitChart->update();

	displayGrayImg();
    if (ui.checkBox_relatedToColorWeight->isChecked()) generateColorPlot();
}

void QColorWeight::setupDateColor(std::shared_ptr<QCustomPlot> customPlot, QCPBars *regen, int nColorIndex, bool bCreate)
{
	// create empty bar chart objects:
	//QCPBars *regen = new QCPBars(customPlot->xAxis, customPlot->yAxis);
	
    QPen pen;
    if (bCreate)
    {
        // set names and colors:       
        pen.setWidthF(1.2);
        regen->setName("Regenerative");
        pen.setColor(QColor(150, 222, 0));
        regen->setPen(pen);
        regen->setBrush(QColor(150, 222, 0, 70));
        regen->setStackingGap(10);
        regen->setWidth(3);

        // set locale to english, so we get english month names:
        customPlot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));
    }	

	// prepare x axis with country labels:
	QVector<double> ticks;	
	QVector<double> regenData;

	int nRangeXMin = 0;
	int nRangeXMax = 255;

	// generate random walk data:
	QVector<double> keys; QVector<double> values;
	QColor color; QString label;
	if (0 == nColorIndex)
	{
		color = QColor(255, 0, 0, 255);
		label = "Red Level";
		nRangeXMin = m_minR;
		nRangeXMax = m_maxR;

		for (int i = 0; i < m_colorRHitDatas.size(); ++i)
		{
			keys.push_back(i);
			values.push_back(getColorValue(0, i));
		}
	}
	else if (1 == nColorIndex)
	{
		color = QColor(0, 255, 0, 255);
		label = "Green Level";
		nRangeXMin = m_minG;
		nRangeXMax = m_maxG;

		for (int i = 0; i < m_colorGHitDatas.size(); ++i)
		{
			keys.push_back(i);
			values.push_back(getColorValue(1, i));
		}
	}
	else if (2 == nColorIndex)
	{
		color = QColor(0, 0, 255, 255);
		label = "Blue Level";
		nRangeXMin = m_minB;
		nRangeXMax = m_maxB;

		for (int i = 0; i < m_colorBHitDatas.size(); ++i)
		{
			keys.push_back(i);
			values.push_back(getColorValue(2, i));
		}
	}
	else if (3 == nColorIndex)
	{
		color = QColor(125, 125, 125, 255);
		label = "Gray Level";
		nRangeXMin = m_minT;
		nRangeXMax = m_maxT;

		for (int i = 0; i < m_colorGrayHitDatas.size(); ++i)
		{
			keys.push_back(i);
			values.push_back(getColorValue(3, i));
		}
	}

    customPlot->clearGraphs();
  
    // Plot line
	customPlot->addGraph();
	//QPen pen;
	pen.setColor(color);
	customPlot->graph()->setLineStyle(QCPGraph::lsLine);
	customPlot->graph()->setPen(pen);
	//customPlot->graph()->setBrush(QBrush(QColor(color.red()/2, color.green()/2, color.blue()/2, 200)));	
	customPlot->graph()->setData(keys, values);

	// Bar Chart
	int nStep = 5;
	for (int i = 0; i <= 255; i += nStep)
	{
		ticks.push_back(i);
		int nCount = 0;
		for (int j = i; j < i + nStep; j++)
		{
			if (j < values.size())
				nCount += values[j];
		}
		nCount /= nStep;
		regenData.push_back(nCount);
	}
	regen->setData(ticks, regenData);

	int nMaxValue = 0;
	for (int i = 0; i < values.size(); i++)
	{
		if (values[i] > nMaxValue) nMaxValue = values[i];
	}

	// Min Value
	keys.clear(); values.clear();
	keys << nRangeXMin;
	values << nMaxValue;
	customPlot->addGraph();
	//QPen pen;
	pen.setColor(QColor(1, 92, 191, 200));
	customPlot->graph()->setLineStyle(QCPGraph::lsImpulse);
	customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
	customPlot->graph()->setPen(pen);	
	customPlot->graph()->setData(keys, values);

	// Max Value
	keys.clear(); values.clear();
	keys << nRangeXMax;
	values << nMaxValue;
	customPlot->addGraph();
	customPlot->graph()->setLineStyle(QCPGraph::lsImpulse);
	customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
	customPlot->graph()->setPen(pen);
	customPlot->graph()->setData(keys, values);	

    if (bCreate)
    {
        // set axis labels:
        customPlot->xAxis->setLabel(label);
        customPlot->yAxis->setLabel("Pixel count");

        // make top and right axes visible but without ticks and labels:
        customPlot->xAxis->setVisible(true);
        customPlot->yAxis->setVisible(true);
        customPlot->xAxis->setTicks(true);
        customPlot->yAxis->setTicks(false);
        customPlot->xAxis->setTickLabels(true);
        customPlot->yAxis->setTickLabels(false);       

        // show legend:
        customPlot->legend->setVisible(false);
        customPlot->axisRect()->setupFullAxesBox();
    }

    // set axis ranges to show all data:
    customPlot->xAxis->setRange(0, 255);
    customPlot->yAxis->setRange(0, nMaxValue);

    // zoom out a bit:
    customPlot->yAxis->scaleRange(1.2, customPlot->yAxis->range().center());
    customPlot->xAxis->scaleRange(1.0, customPlot->xAxis->range().center());
}

void QColorWeight::generateColorPlot()
{
    if (m_matSrcImage.empty()) return;

    clearColorData(0);
    clearColorData(1);
    clearColorData(2);
    clearColorData(3);

    unsigned char grayValueB, grayValueG, grayValueR;
    for (int y = 0; y < m_matSrcImage.rows; y++) {
        for (int x = 0; x < m_matSrcImage.cols; x++) {
            cv::Vec3b& pixel = m_matSrcImage.at<cv::Vec3b>(y, x);

            grayValueB = pixel[0];
            grayValueG = pixel[1];
            grayValueR = pixel[2];

            int nGrayValue = calcGrayValue(cv::Scalar(grayValueB, grayValueG, grayValueR));

            incrementColorValue(0, grayValueR);
            incrementColorValue(1, grayValueG);
            incrementColorValue(2, grayValueB);
            incrementColorValue(3, nGrayValue);
        }
    }

    setupDateColor(m_customPlotR, m_regenR, 0);
    m_customPlotR->setFixedSize(360, 90);
    m_plotSeneR->clear();    
    m_plotSeneR->addPixmap(m_customPlotR->grab());
    ui.graphicsView_R->update();

    setupDateColor(m_customPlotG, m_regenG, 1);
    m_customPlotG->setFixedSize(360, 90);
    m_plotSeneG->clear();   
    m_plotSeneG->addPixmap(m_customPlotG->grab());
    ui.graphicsView_G->update();

    setupDateColor(m_customPlotB, m_regenB, 2);
    m_customPlotB->setFixedSize(360, 90);
    m_plotSeneB->clear();   
    m_plotSeneB->addPixmap(m_customPlotB->grab());
    ui.graphicsView_B->update();

    setupDateColor(m_customPlotGray, m_regenGray, 3);
    m_customPlotGray->setFixedSize(360, 90);
    m_plotSeneGray->clear();    
    m_plotSeneGray->addPixmap(m_customPlotGray->grab());  
    ui.graphicsView_Gray->update();

    displayColorImg();
}

void QColorWeight::clearGrayData()
{
    m_grayHitDatas.clear();
    for (int i = 0; i <= 255; i++)
    {
        m_grayHitDatas.insert(i, 0);
    }	
}

int QColorWeight::getGrayValue(int nGrayLevel)
{
	if (nGrayLevel < 0 || nGrayLevel > 255) return 0;

	return m_grayHitDatas.value(nGrayLevel);
}

int QColorWeight::incrementGrayValue(int nGrayLevel)
{
	if (nGrayLevel < 0 || nGrayLevel > 255) return 0;

	m_grayHitDatas[nGrayLevel] = m_grayHitDatas[nGrayLevel] + 1;

	return m_grayHitDatas[nGrayLevel];
}

void QColorWeight::clearColorData(int nColorIndex)
{
	if (0 == nColorIndex)
	{
		m_colorRHitDatas.clear();
		for (int i = 0; i <= 255; i++)
		{
			m_colorRHitDatas.insert(i, 0);
		}
	}
	else if (1 == nColorIndex)
	{
		m_colorGHitDatas.clear();
		for (int i = 0; i <= 255; i++)
		{
			m_colorGHitDatas.insert(i, 0);
		}
	}
	else if (2 == nColorIndex)
	{
		m_colorBHitDatas.clear();
		for (int i = 0; i <= 255; i++)
		{
			m_colorBHitDatas.insert(i, 0);
		}
	}
	else if (3 == nColorIndex)
	{
		m_colorGrayHitDatas.clear();
		for (int i = 0; i <= 255; i++)
		{
			m_colorGrayHitDatas.insert(i, 0);
		}
	}	
}

int QColorWeight::getColorValue(int nColorIndex, int nGrayLevel)
{
	if (nGrayLevel < 0 || nGrayLevel > 255) return 0;

	if (0 == nColorIndex)
	{
        return m_colorRHitDatas.value(nGrayLevel);
	}
	else if (1 == nColorIndex)
	{
		return m_colorGHitDatas.value(nGrayLevel);
	}
	else if (2 == nColorIndex)
	{
		return m_colorBHitDatas.value(nGrayLevel);
	}
	else if (3 == nColorIndex)
	{		
		return m_colorGrayHitDatas.value(nGrayLevel);
	}

	return 0;
}

int QColorWeight::incrementColorValue(int nColorIndex, int nGrayLevel)
{
	if (nGrayLevel < 0 || nGrayLevel > 255) return 0;

	if (0 == nColorIndex)
	{
        m_colorRHitDatas[nGrayLevel] = m_colorRHitDatas[nGrayLevel] + 1;
        return m_colorRHitDatas[nGrayLevel];
	}
	else if (1 == nColorIndex)
	{
		m_colorGHitDatas[nGrayLevel] = m_colorGHitDatas[nGrayLevel] + 1;
		return m_colorGHitDatas[nGrayLevel];
	}
	else if (2 == nColorIndex)
	{
		m_colorBHitDatas[nGrayLevel] = m_colorBHitDatas[nGrayLevel] + 1;
		return m_colorBHitDatas[nGrayLevel];
	}
	else if (3 == nColorIndex)
	{
		m_colorGrayHitDatas[nGrayLevel] = m_colorGrayHitDatas[nGrayLevel] + 1;
		return m_colorGrayHitDatas[nGrayLevel];
	}

	return 0;
}

void QColorWeight::displaySourceImg()
{
    if (m_matSrcImage.empty())
        return;

    m_matSrcTransform = _transformColorImage(m_matSrcImage);

	cv::Mat matSourceImg = m_matSrcTransform.clone();

	cv::cvtColor(matSourceImg, matSourceImg, CV_BGR2RGB);
	QImage img = QImage((uchar*)matSourceImg.data, matSourceImg.cols, matSourceImg.rows, ToInt(matSourceImg.step), QImage::Format_RGB888);
	m_sourceImgScene->clear();
	m_sourceImgScene->addPixmap(QPixmap::fromImage(img.scaled(QSize(IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT))));
}

void QColorWeight::displayGrayImg() {
    cv::Mat matGrayImg = m_matSrcImage.clone();
    cv::Mat maskMat = cv::Mat::zeros(matGrayImg.rows, matGrayImg.cols, CV_8UC1);

    bool bInvert = ui.checkBox_invert->isChecked();

    int nIndex = ui.comboBox_selectMode->currentIndex();
    GRAY_WEIGHT_METHOD emMode = static_cast<GRAY_WEIGHT_METHOD>(nIndex);
    if (EM_MODE_PT_THRESHOLD == emMode) {
        cv::Vec3b& pixelGet = matGrayImg.at<cv::Vec3b>(m_grayGenPt.y, m_grayGenPt.x);
        int nGrayValueGet = calcGrayValue(cv::Scalar(pixelGet[0], pixelGet[1], pixelGet[2]));

        for (int y = 0; y < matGrayImg.rows; y++) {
            for (int x = 0; x < matGrayImg.cols; x++) {
                cv::Vec3b& pixel = matGrayImg.at<cv::Vec3b>(y, x);
                uchar& mask = maskMat.at<uchar>(y, x);

                int nGrayValue = calcGrayValue(cv::Scalar(pixel[0], pixel[1], pixel[2]));
                if (bInvert ? (nGrayValue > nGrayValueGet) : (nGrayValue <= nGrayValueGet)) {
                    pixel[0] = 0;
                    pixel[1] = 0;
                    pixel[2] = 0;
                }
                else {
                    pixel[0] = nGrayValue;
                    pixel[1] = nGrayValue;
                    pixel[2] = nGrayValue;

                    mask = 1;
                }
            }
        }
    }
    else if (EM_MODE_ONE_THRESHOLD == emMode) {
        cv::Mat matGrayLocal = _convertToGrayImage();       
        cv::threshold(matGrayLocal, maskMat, m_nGrayLevelThreshold1, 255, bInvert ? cv::ThresholdTypes::THRESH_BINARY_INV : cv::ThresholdTypes::THRESH_BINARY);
        cv::cvtColor(maskMat, matGrayImg, CV_GRAY2RGB);
    }
    else if (EM_MODE_TWO_THRESHOLD == emMode) {
        Vision::PR_THRESHOLD_CMD stCmd;
        Vision::PR_THRESHOLD_RPY stRpy;
        stCmd.matInputImg = _convertToGrayImage();
        stCmd.bDoubleThreshold = true;
        stCmd.nThreshold1 = m_nGrayLevelThreshold1;
        stCmd.nThreshold2 = m_nGrayLevelThreshold2;
        stCmd.bInverseResult = bInvert;
        Vision::PR_Threshold(&stCmd, &stRpy);
        maskMat = stRpy.matResultImg;

        cv::cvtColor(maskMat, matGrayImg, CV_GRAY2RGB);
    }

    m_maskMat = maskMat;

    QImage img = QImage((uchar*)matGrayImg.data, matGrayImg.cols, matGrayImg.rows, ToInt(matGrayImg.step), QImage::Format_RGB888);
    m_grayImgScene->clear();
    m_grayImgScene->addPixmap(QPixmap::fromImage(img.scaled(QSize(IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT))));
}

void QColorWeight::displayColorImg()
{
	cv::Mat matColorImg = m_matSrcImage.clone();

	int nRn = ui.horizontalSlider_Rn->value();
	int nTn = ui.horizontalSlider_Tn->value();
    bool bInvert = ui.checkBox_invert->isChecked();

	cv::Mat matGrayColor = generateColorRange(nRn, nTn, matColorImg, bInvert);
	QImage imageColor = QImage((uchar*)matGrayColor.data, matGrayColor.cols, matGrayColor.rows, ToInt(matGrayColor.step), QImage::Format_RGB888);
	m_grayColorScene->clear();
	m_grayColorScene->addPixmap(QPixmap::fromImage(imageColor));

	cv::cvtColor(matColorImg, matColorImg, CV_BGR2RGB);
	QImage img = QImage((uchar*)matColorImg.data, matColorImg.cols, matColorImg.rows, ToInt(matColorImg.step), QImage::Format_RGB888);
	m_colorImgScene->clear();
	m_colorImgScene->addPixmap(QPixmap::fromImage(img.scaled(QSize(IMG_DISPLAY_WIDTH, IMG_DISPLAY_HEIGHT))));
}

void QColorWeight::onGrayModeIndexChanged(int iIndex)
{
    int nIndex = ui.comboBox_selectMode->currentIndex();
    GRAY_WEIGHT_METHOD emMode = static_cast<GRAY_WEIGHT_METHOD>(nIndex);
    if (EM_MODE_PT_THRESHOLD == emMode) {
        ui.horizontalSlider_grayLeft->setVisible(false);
        ui.horizontalSlider_grayRight->setVisible(false);
    }
    else if (EM_MODE_ONE_THRESHOLD == emMode){
        ui.horizontalSlider_grayLeft->setVisible(true);
        ui.horizontalSlider_grayRight->setVisible(false);
    }
    else if (EM_MODE_TWO_THRESHOLD == emMode){
        ui.horizontalSlider_grayLeft->setVisible(true);
        ui.horizontalSlider_grayRight->setVisible(true);
    }
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

void QColorWeight::onCheckBoxInvert(int iState)
{
    if (iState == Qt::Checked)
    {
    }
    else
    {
    }

    generateGrayPlot();
}

void QColorWeight::onGrayRSliderChanged(int i)
{
	QString str = QString::number(i);
	ui.lineEdit_R->setText(str);

    displaySourceImg();
	generateGrayPlot();
}

void QColorWeight::onGrayGSliderChanged(int i)
{
	QString str = QString::number(i);
	ui.lineEdit_G->setText(str);

    displaySourceImg();
	generateGrayPlot();
}

void QColorWeight::onGrayBSliderChanged(int i)
{
	QString str = QString::number(i);
	ui.lineEdit_B->setText(str);

    displaySourceImg();
	generateGrayPlot();
}

void QColorWeight::onColorRnSliderChanged(int i)
{
	QString str = QString::number(i);
	ui.lineEdit_Rn->setText(str);

	generateColorPlot();
}

void QColorWeight::onColorTnSliderChanged(int i)
{
	QString str = QString::number(i);
	ui.lineEdit_Tn->setText(str);

	generateColorPlot();
}

cv::Mat QColorWeight::generateColorRange(int nRn, int nTn, cv::Mat& matImage, bool bInvert)
{
	if (m_matSrcImage.empty()) return cv::Mat();

    cv::Mat matImageSrc;
    if (bInvert)
    {
        matImageSrc = matImage.clone();
    }

	int m = matImage.rows;
	int n = matImage.cols;

    if (m_colorGenPt.x < 0 || m_colorGenPt.x > n)
        return cv::Mat();

    if (m_colorGenPt.y < 0 || m_colorGenPt.y > m)
        return cv::Mat();

	cv::Mat R = cv::Mat::ones(m, n, CV_8UC1);
	cv::Mat G = cv::Mat::ones(m, n, CV_8UC1);
	cv::Mat B = cv::Mat::ones(m, n, CV_8UC1);
	cv::Mat T = cv::Mat::ones(m, n, CV_8UC1);
	cv::Mat S = cv::Mat::ones(m, n, CV_8UC1);

	for (int y = 0; y < m; y++)
	{
		for (int x = 0; x < n; x++)
		{
			cv::Vec3b& pixel = matImage.at<cv::Vec3b>(y, x);

			uchar& r = R.at<uchar>(y, x);
			r = pixel[2];
			uchar& g = G.at<uchar>(y, x);
			g = pixel[1];
			uchar& b = B.at<uchar>(y, x);
			b = pixel[0];

			uchar& t = T.at<uchar>(y, x);
			t = calcGrayValue(cv::Scalar(b, g, r));

			uchar& s = S.at<uchar>(y, x);
			s = qMax(r, qMax(g, b));
		}
	}

	uchar Rt = R.at<uchar>(m_colorGenPt.y, m_colorGenPt.x);
	uchar Gt = G.at<uchar>(m_colorGenPt.y, m_colorGenPt.x);
	uchar Bt = B.at<uchar>(m_colorGenPt.y, m_colorGenPt.x);
	uchar Tt = T.at<uchar>(m_colorGenPt.y, m_colorGenPt.x);
	uchar St = S.at<uchar>(m_colorGenPt.y, m_colorGenPt.x);

    if (m_bSetColor)
    {
        Rt = m_color[0];
        Gt = m_color[1];
        Bt = m_color[2];
        Tt = calcGrayValue(cv::Scalar(Bt, Gt, Rt));
        St = qMax(Rt, qMax(Gt, Bt));
    }

	uchar maxRGB = qMax(Rt, qMax(Gt, Bt));

	uchar maxR = 0, minR = 255;
	uchar maxG = 0, minG = 255;
	uchar maxB = 0, minB = 255;
	uchar maxT = 0, minT = 255;

	cv::Mat maskMat = cv::Mat::zeros(m, n, CV_8UC1);
	for (int y = 0; y < m; y++)
	{
		for (int x = 0; x < n; x++)
		{
			cv::Vec3b& pixel = matImage.at<cv::Vec3b>(y, x);
			uchar& mask = maskMat.at<uchar>(y, x);

			uchar& r = R.at<uchar>(y, x);
			uchar& g = G.at<uchar>(y, x);
			uchar& b = B.at<uchar>(y, x);
			uchar& t = T.at<uchar>(y, x);
			uchar& s = S.at<uchar>(y, x);

			if (Rt == maxRGB)
			{
				if (((r - g) >((Rt - Gt) - nRn))
					&& ((r - b) > ((Rt - Bt) - nRn))
					&& ((r - g) < ((Rt - Bt) + nRn))
					&& ((r - b) < ((Rt - Bt) + nRn))
					&& (qAbs(t - Tt) < nTn))
				{
					mask = Vision::PR_MAX_GRAY_LEVEL;

					pixel[2] = 255;
					pixel[1] = 255;
					pixel[0] = 0;

					if (r > maxR) maxR = r;
					if (r < minR) minR = r;
					if (g > maxG) maxG = g;
					if (g < minG) minG = g;
					if (b > maxB) maxB = b;
					if (b < minB) minB = b;
					if (t > maxT) maxT = t;
					if (t < minT) minT = t;					
				}
			}
			else if (Gt == maxRGB)
			{
				if (((g - r) > ((Gt - Rt) - nRn))
					&& ((g - b) > ((Gt - Bt) - nRn))
					&& ((g - r) < ((Gt - Rt) + nRn))
					&& ((g - b) < ((Gt - Bt) + nRn))
					&& (qAbs(t - Tt) < nTn))
				{
					mask = Vision::PR_MAX_GRAY_LEVEL;

					pixel[2] = 255;
					pixel[1] = 255;
					pixel[0] = 0;

					if (r > maxR) maxR = r;
					if (r < minR) minR = r;
					if (g > maxG) maxG = g;
					if (g < minG) minG = g;
					if (b > maxB) maxB = b;
					if (b < minB) minB = b;
					if (t > maxT) maxT = t;
					if (t < minT) minT = t;
				}
			}
			else
			{
				if (((b - r) > ((Bt - Rt) - nRn))
					&& ((b - g) > ((Bt - Gt) - nRn))
					&& ((b - r) < ((Bt - Rt) + nRn))
					&& ((b - g) < ((Bt - Gt) + nRn))
					&& (qAbs(t - Tt) < nTn))
				{
					mask = Vision::PR_MAX_GRAY_LEVEL;

					pixel[2] = 255;
					pixel[1] = 255;
					pixel[0] = 0;

					if (r > maxR) maxR = r;
					if (r < minR) minR = r;
					if (g > maxG) maxG = g;
					if (g < minG) minG = g;
					if (b > maxB) maxB = b;
					if (b < minB) minB = b;
					if (t > maxT) maxT = t;
					if (t < minT) minT = t;
				}
			}
		}
	}
	m_maskMat = maskMat;
    if (bInvert)
    {
        m_maskMat = Vision::PR_MAX_GRAY_LEVEL - m_maskMat;
       
        cv::Mat imgLayer;
        cv::cvtColor(m_maskMat, imgLayer, CV_GRAY2RGB);

        std::vector<cv::Mat> vecChannels;
        cv::split(imgLayer, vecChannels);
        vecChannels[0] = 0;
        cv::merge(vecChannels, imgLayer);

        double alpha = 0.3;
        addWeighted(matImageSrc, alpha, imgLayer, 1 - alpha, 0, matImageSrc);

        matImage = matImageSrc;
    }

	m_maxR = maxR;
	m_minR = minR;
	m_maxG = maxG;
	m_minG = minG;
	m_maxB = maxB;
	m_minB = minB;
	m_maxT = maxT;
	m_minT = minT;

    if (!m_bSetColor)
    {
        double dXScale = m_matSrcImage.size().width * 1.0 / IMG_DISPLAY_WIDTH;
        double dYScale = m_matSrcImage.size().height * 1.0 / IMG_DISPLAY_HEIGHT;
        double dScale = qMax(dXScale, dYScale);

        cv::circle(matImage, m_colorGenPt, 3.5 * dScale, cv::Scalar(0, 0, 255), 1 * dScale, 8);
    }	

	int nWidth = ui.graphicsView_grayColor->geometry().width();
	int nHeight = ui.graphicsView_grayColor->geometry().height();
	return generateColorTrig(nWidth, nHeight, minR, maxR, minG, maxG, minB, maxB);
}

cv::Mat QColorWeight::generateColorTrig(int nWidth, int nHeight, int nMinR, int nMaxR, int nMinG, int nMaxG, int nMinB, int nMaxB)
{
	double k = double(qMax(qMax(nMaxR, nMaxG), nMaxB)) / 255;

	double minR = double(nMinR) / k / 255;
	double maxR = double(nMaxR) / k / 255;
	double minG = double(nMinG) / k / 255;
	double maxG = double(nMaxG) / k / 255;
	double minB = double(nMinB) / k / 255;
	double maxB = double(nMaxB) / k / 255;

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

	// Color Space Calculate

	int nGridLen = m / 2;
	int xx[2], yy1[2], yy2[2];

	// Red	
	xx[0] = (-1 + 1) * nGridLen;
	xx[1] = (1 + 1) * nGridLen;
	for (int i = 0; i < 2; i++)
	{
		yy1[i] = ((1 - minR) / 2 + 1) * nGridLen;
		yy2[i] = ((1 - maxR) / 2 + 1) * nGridLen;
	}
	cv::line(image, cv::Point(xx[0], yy1[0]), cv::Point(xx[1], yy1[1]), cv::Scalar(17, 177, 255));
	if (maxR != 1) cv::line(image, cv::Point(xx[0], yy2[0]), cv::Point(xx[1], yy2[1]), cv::Scalar(17, 177, 255));

	// Green
	xx[0] = (-1 + 1) * nGridLen;
	xx[1] = (1 + 1) * nGridLen;
	for (int i = 0; i < 2; i++)
	{
		yy1[i] = qSqrt(3)*(xx[i] - nGridLen) + minG * nGridLen;
		yy2[i] = qSqrt(3)*(xx[i] - nGridLen) + maxG * nGridLen;
	}
	cv::line(image, cv::Point(xx[0], yy1[0]), cv::Point(xx[1], yy1[1]), cv::Scalar(17, 177, 255));
	if (maxG != 1) 	cv::line(image, cv::Point(xx[0], yy2[0]), cv::Point(xx[1], yy2[1]), cv::Scalar(17, 177, 255));

	// Blue
	xx[0] = (-1 + 1) * nGridLen;
	xx[1] = (1 + 1) * nGridLen;
	for (int i = 0; i < 2; i++)
	{
		yy1[i] = -qSqrt(3)*(xx[i] - nGridLen) + minB * nGridLen;
		yy2[i] = -qSqrt(3)*(xx[i] - nGridLen) + maxB * nGridLen;
	}
	cv::line(image, cv::Point(xx[0], yy1[0]), cv::Point(xx[1], yy1[1]), cv::Scalar(17, 177, 255));
	if (maxB != 1) 	cv::line(image, cv::Point(xx[0], yy2[0]), cv::Point(xx[1], yy2[1]), cv::Scalar(17, 177, 255));

	double dMaxValues[3] = { maxR, maxG, maxB };
	QVector<int> idx;
	QVector<int> idx1;
	for (int i = 0; i < 3; i++)
	{
		if (dMaxValues[i] == 1)
		{
			idx1.push_back(i);
		}
		else
		{
			idx.push_back(i);
		}
	}
	double lValus[3][4] = { 0, (3 - minR) / 2 * nGridLen, 0, (3 - maxR) / 2 * nGridLen,
		qSqrt(3), (minG - qSqrt(3)) * nGridLen, qSqrt(3), (maxG - qSqrt(3)) * nGridLen,
		-qSqrt(3), (minB + qSqrt(3)) * nGridLen, -qSqrt(3), (maxB + qSqrt(3)) * nGridLen };

	if (idx.size() == 2)
	{
		cv::Point2f ptInt1, ptInt2, ptInt3, ptInt4;
		calcTwoLineIntersect(lValus[idx[0]][0], lValus[idx[0]][1], lValus[idx[1]][0], lValus[idx[1]][1], ptInt1);
		calcTwoLineIntersect(lValus[idx[0]][0], lValus[idx[0]][1], lValus[idx[1]][2], lValus[idx[1]][3], ptInt2);
		calcTwoLineIntersect(lValus[idx[0]][2], lValus[idx[0]][3], lValus[idx[1]][0], lValus[idx[1]][1], ptInt3);
		calcTwoLineIntersect(lValus[idx[0]][2], lValus[idx[0]][3], lValus[idx[1]][2], lValus[idx[1]][3], ptInt4);	

		cv::line(image, ptInt1, ptInt2, cv::Scalar(255, 255, 0));
		cv::line(image, ptInt2, ptInt4, cv::Scalar(255, 255, 0));
		cv::line(image, ptInt4, ptInt3, cv::Scalar(255, 255, 0));
		cv::line(image, ptInt3, ptInt1, cv::Scalar(255, 255, 0));
	}
	else if (idx.size() == 1)
	{
		cv::Point2f ptInt1, ptInt2, ptInt3, ptInt4;
		calcTwoLineIntersect(lValus[idx[0]][0], lValus[idx[0]][1], lValus[idx1[0]][0], lValus[idx1[0]][1], ptInt1);
		calcTwoLineIntersect(lValus[idx[0]][2], lValus[idx[0]][3], lValus[idx1[0]][0], lValus[idx1[0]][1], ptInt2);
		calcTwoLineIntersect(lValus[idx[0]][0], lValus[idx[0]][1], lValus[idx1[1]][0], lValus[idx1[1]][1], ptInt3);
		calcTwoLineIntersect(lValus[idx[0]][2], lValus[idx[0]][3], lValus[idx1[1]][0], lValus[idx1[1]][1], ptInt4);

		cv::line(image, ptInt1, ptInt2, cv::Scalar(255, 255, 0));
		cv::line(image, ptInt2, ptInt4, cv::Scalar(255, 255, 0));
		cv::line(image, ptInt4, ptInt3, cv::Scalar(255, 255, 0));
		cv::line(image, ptInt3, ptInt1, cv::Scalar(255, 255, 0));
	}
	else
	{
		cv::Point2f ptInt1, ptInt2, ptInt3, ptInt4;

		ptInt1.x = nGridLen;
		ptInt2.x = (-qSqrt(3) / 2 + 1) * nGridLen;
		ptInt3.x = (qSqrt(3) / 2 + 1) * nGridLen;
		ptInt4.x = nGridLen;

		ptInt1.y = 0;
		ptInt2.y = 3.0 / 2 * nGridLen;
		ptInt3.y = 3.0 / 2 * nGridLen;
		ptInt4.y = 0;

		cv::line(image, ptInt1, ptInt2, cv::Scalar(255, 255, 0));
		cv::line(image, ptInt2, ptInt3, cv::Scalar(255, 255, 0));	
		cv::line(image, ptInt3, ptInt4, cv::Scalar(255, 255, 0));
		cv::line(image, ptInt4, ptInt1, cv::Scalar(255, 255, 0));
	}

	return image;
}

bool QColorWeight::calcTwoLineIntersect(cv::Point2f pt1, cv::Point2f pt2, cv::Point2f pt3, cv::Point2f pt4, cv::Point2f& ptIntersect)
{
	Vision::PR_Line2f line1, line2;
	line1.pt1 = pt1;
	line1.pt2 = pt2;
	line2.pt1 = pt3;
	line2.pt2 = pt4;

	Vision::PR_TWO_LINE_INTERSECT_CMD  stCmd;
	stCmd.line1 = line1;
	stCmd.line2 = line2;

	Vision::PR_TWO_LINE_INTERSECT_RPY stRpy;
	Vision::VisionStatus retStatus = Vision::PR_TwoLineIntersect(&stCmd, &stRpy);
	if (retStatus == Vision::VisionStatus::OK)
	{	
		ptIntersect = stRpy.ptIntersect;
		return true;
	}
	else
	{
		System->setTrackInfo(QString("Error at PR_TwoLineIntersect, error code = %1").arg((int)retStatus));
	}

	return false;
}

bool QColorWeight::calcTwoLineIntersect(double k1, double b1, double k2, double b2, cv::Point2f& ptIntersect)
{
	if (k1 == k2)
	{
		ptIntersect.x = 0;
		ptIntersect.y = 0;
	}		
	else
	{				
		double a1 = k1, b1 = -1, c1 = b1;
		double a2 = k2, b2 = -1, c2 = b2;
		double dA = (a1*b2 - a2*b1);
		
		ptIntersect.x = (b1*c2 - b2*c1) / dA;
		ptIntersect.y = (a1*c2 - a2*c1) / dA;

		/*double A[4] = { k1, -1, k2, -1 };
		double B[2] = { -b1, -b2 };

		cv::Mat matA = cv::Mat(2, 2, CV_32FC2, A);
		cv::Mat matB = cv::Mat(2, 1, CV_32FC2, B);

		cv::Mat matA1;
		cvInvert(&matA, &matA1, CV_LU);


		cv::Mat matK = matA1 * matB;

		ptIntersect.x = matK.at<float>(0);
		ptIntersect.y = matK.at<float>(1);*/
	}

	return true;
}

void QColorWeight::onColorWidgetState(const QVariantList &data)
{
    IVisionUI *pUI = getModule<IVisionUI>(UI_MODEL);
    cv::Mat matImage = pUI->getImage();
    if (matImage.empty())
        return;    

    cv::Rect rectROI;
    int nOption = data[0].toInt();
    switch(nOption)
    {
    case CHANGE_SELECTED_ROI:
        rectROI = pUI->getSelectedROI();
        break;
    case CHANGE_SRCH_WINDOW:
        rectROI = pUI->getSrchWindow();
        break;
    default:
        break;
    }

    if (rectROI.x < 0) rectROI.x = 0;
    if (rectROI.y < 0) rectROI.y = 0;
    if ((rectROI.x + rectROI.width) > matImage.cols)
        rectROI.width = matImage.cols - rectROI.x;
    if ((rectROI.y + rectROI.height) > matImage.rows)
        rectROI.height = matImage.rows - rectROI.y;

    cv::Mat matROI(matImage, rectROI);
    setImage(matROI);

    GrayWeightParams stGrayParams = getGrayParams();
	ColorSpaceParams stColorParams = getColorParams();

    if(GRAY_WEIGHT_METHOD::EM_MODE_ONE_THRESHOLD == stGrayParams.enMethod || GRAY_WEIGHT_METHOD::EM_MODE_TWO_THRESHOLD == stGrayParams.enMethod) {
        ui.tabWidget->setCurrentIndex(0);
        generateGrayPlot();
    }
    else {
        ui.tabWidget->setCurrentIndex(1);
        generateColorPlot();
    }
}