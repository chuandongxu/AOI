#include "QRunSettingWidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QGridLayout>
#include <QScrollArea>
#include <QDrag>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QPainter>

#include "../Common/SystemData.h"
#include "../Common/ModuleMgr.h"
#include "../include/IdDefine.h"
#include "../include/IVision.h"
#include "../include/ICamera.h"
#include "../include/IData.h"
#include "../Common/eos.h"

#include "../DataModule/QDetectObj.h"

#include "CryptLib.h"

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

QRunSettingWidget::QRunSettingWidget(DataCtrl* pCtrl, QWidget *parent)
	:m_pCtrl(pCtrl), QWidget(parent)
{
	ui.setupUi(this);

	QEos::Attach(EVENT_OBJ_STATE, this, SLOT(onObjEvent(const QVariantList &)));

	setAcceptDrops(true);
	
	QStringList ls;
	ls << QStringLiteral("Baslar相机采集") << QStringLiteral("Dalsa采集卡采集") << QStringLiteral("手动导入图片");
	ui.comboBox_runType->addItems(ls);
	int nCaptureMode = System->getParam("camera_capture_mode").toInt();
	ui.comboBox_runType->setCurrentIndex(nCaptureMode);
	m_pCtrl->setRunType(nCaptureMode);
	ui.comboBox_runType->setEnabled(false);
	connect(ui.comboBox_runType, SIGNAL(currentIndexChanged(int)), SLOT(onRunTypeIndexChanged(int)));

	connect(ui.comboBox_runMode, SIGNAL(currentIndexChanged(int)), SLOT(onRunModeIndexChanged(int)));
	ls.clear();
	ls << QStringLiteral("单次测量") << QStringLiteral("循环测试");
	ui.comboBox_runMode->addItems(ls);
	ui.comboBox_runMode->setCurrentIndex(0);

	ui.tableView_objList->setModel(&m_model);
	ui.tableView_objList->horizontalHeader()->setStretchLastSection(true);
	ui.tableView_objList->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableView_objList->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableView_objList->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(ui.tableView_objList, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotRowDoubleClicked(const QModelIndex &)));


	//QPalette palette;
	//palette.setColor(QPalette::Background, QColor(200, 200, 200, 70));
	//ui.tabWidget->setPalette(palette);
	//ui.tabWidget->setAutoFillBackground(true);

	connect(ui.pushButton_3DDetectOpen, SIGNAL(clicked()), SLOT(on3DDetectOpen()));

	connect(ui.pushButton_cellEditor, SIGNAL(clicked()), SLOT(onCellEditor()));

	connect(ui.pushButton_profileEditor, SIGNAL(clicked()), SLOT(onProfileEditor()));

	loadTmps();
	loadObjs();

	initObjList();

	QString user;
	int level = 0;
	System->getUser(user, level);
	if (USER_LEVEL_MANAGER > level)
	{
		ui.pushButton_cellEditor->setEnabled(false);
		ui.pushButton_profileEditor->setEnabled(false);
	}
}

QRunSettingWidget::~QRunSettingWidget()
{

}

void QRunSettingWidget::slotRowDoubleClicked(const QModelIndex& index)
{
	//QModelIndex index = ui.tableView_objList->currentIndex();
	if (index.isValid())
	{
		int row = index.row();

		QEos::Notify(EVENT_OBJ_STATE, 0, RUN_OBJ_EDITOR, row + 1);
	}
}

void QRunSettingWidget::onObjEvent(const QVariantList &data)
{
	int iBoard = data[0].toInt();
	int iEvent = data[1].toInt();
	if (iEvent != RUN_OBJ_CHANGE)return;

	initObjList();
}

void QRunSettingWidget::initObjList()
{
	m_model.clear();

	QStringList ls;
	ls << QStringLiteral("元件") << QStringLiteral("类型") << QStringLiteral("位置") << QStringLiteral("操作");
	m_model.setHorizontalHeaderLabels(ls);

	ui.tableView_objList->setColumnWidth(0, 70);
	ui.tableView_objList->setColumnWidth(1, 70);
	ui.tableView_objList->setColumnWidth(2, 160);
	ui.tableView_objList->setColumnWidth(3, 40);

	IData * pData = getModule<IData>(DATA_MODEL);
	if (pData)
	{
		for (int j = 0; j < pData->getObjNum(EM_DATA_TYPE_OBJ); j++)
		{
			QDetectObj* pObj = pData->getObj(j, EM_DATA_TYPE_OBJ);
			if (pObj)
			{
				int nr = m_model.rowCount();
				m_model.insertRow(nr);

				m_model.setData(m_model.index(nr, 0), pObj->getName());
				QString szType;
				switch (pObj->getType().toInt())
				{
				case 0:
					szType = QStringLiteral("标准组件");
					break;
				case 1:
					szType = QStringLiteral("管脚器件");
					break;
				case 2:
					szType = QStringLiteral("IC芯片");
					break;
				case 3:
					szType = QStringLiteral("LED元件");
					break;
				default:
					break;
				}
				m_model.setData(m_model.index(nr, 1), szType);
				m_model.setData(m_model.index(nr, 2), QStringLiteral("x = %1, y = %2").arg(pObj->getX()).arg(pObj->getY()));
				m_model.setData(m_model.index(nr, 3), QStringLiteral("编辑"));	
				m_model.item(nr, 3)->setForeground(QBrush(QColor(183, 71, 42)));
				//m_model.item(nr, 3)->setBackground(QBrush(QColor(0, 0, 220)));
			}		
		}
	}
}


void QRunSettingWidget::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
		if (event->source() == this) {
			event->setDropAction(Qt::MoveAction);
			//event->accept();
		}
		else {
			event->acceptProposedAction();
		}
	}
	else {
		event->ignore();
	}
}

void QRunSettingWidget::dragMoveEvent(QDragMoveEvent *event)
{
	if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
		if (event->source() == this) {
			event->setDropAction(Qt::MoveAction);
			event->accept();
		}
		else {
			event->acceptProposedAction();
		}
	}
	else {
		event->ignore();
	}
}

void QRunSettingWidget::dropEvent(QDropEvent *event)
{
	if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
		QByteArray itemData = event->mimeData()->data("application/x-dnditemdata");
		QDataStream dataStream(&itemData, QIODevice::ReadOnly);

		QPixmap pixmap;
		QPoint offset;
		dataStream >> pixmap >> offset;

		QLabel *newIcon = new QLabel(this);
		newIcon->setPixmap(pixmap);
		newIcon->move(event->pos() - offset);
		newIcon->show();
		newIcon->setAttribute(Qt::WA_DeleteOnClose);

		if (event->source() == this) {
			event->setDropAction(Qt::MoveAction);
			event->accept();
		}
		else {
			event->acceptProposedAction();
		}
	}
	else {
		event->ignore();
	}
}

//! [1]
void QRunSettingWidget::mousePressEvent(QMouseEvent *event)
{
	QLabel *child = qobject_cast<QLabel*>(childAt(event->pos()));
	if (!child)
		return;

	QPixmap pixmap = *child->pixmap();

	QByteArray itemData;
	QDataStream dataStream(&itemData, QIODevice::WriteOnly);
	dataStream << pixmap << QPoint(event->pos() - child->pos() - ui.toolBox->pos()) << child->objectName();
	//! [1]

	//! [2]
	QMimeData *mimeData = new QMimeData;
	mimeData->setData("application/x-dnditemdata", itemData);
	//! [2]

	//! [3]
	QDrag *drag = new QDrag(this);
	drag->setMimeData(mimeData);
	drag->setPixmap(pixmap);
	drag->setHotSpot(event->pos() - child->pos() - ui.toolBox->pos());
	//! [3]

	QPixmap tempPixmap = pixmap;
	QPainter painter;
	painter.begin(&tempPixmap);
	painter.fillRect(pixmap.rect(), QColor(0, 0, 127, 127));
	painter.end();

	child->setPixmap(tempPixmap);

	if (drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction) == Qt::MoveAction) {
		child->close();
	}
	else {
		child->show();
		child->setPixmap(pixmap);
	}
}

void QRunSettingWidget::loadTmps()
{
	loadTemplate(ui.page, 0);
	loadTemplate(ui.page_1, 1);
	loadTemplate(ui.page_2, 2);
	loadTemplate(ui.page_3, 3);
}

void QRunSettingWidget::loadObjs()
{
	//IData

}

void QRunSettingWidget::loadTemplate(QWidget* pWidget, int nType)
{
	IData * pData = getModule<IData>(DATA_MODEL);
	if (!pData) return;

	QString path = QApplication::applicationDirPath();
	int nImageWidth = 60, nImageHeight = 60;

	QGridLayout* pGridLayout = new QGridLayout(pWidget);
	pGridLayout->setAlignment(Qt::AlignTop);

	int nObjIndex = 0;
	for (int i = 0; i < pData->getObjNum(EM_DATA_TYPE_TMP); i++)
	{
		QDetectObj* pObj = pData->getObj(i, EM_DATA_TYPE_TMP);
		if (pObj && (pObj->getType().toInt() == nType))
		{
			QLabel *boatIcon = new QLabel();

			QString fileName = pObj->getBitmap();
			if (fileName.isEmpty())
			{
				fileName += "/image/DataImage/R.png";
			}
			QPixmap pixmap = QPixmap(path + fileName);
			if (pixmap.width() > nImageWidth)
			{
				pixmap = pixmap.scaled(QSize(nImageWidth, nImageHeight));
			}
			boatIcon->setPixmap(pixmap);
			boatIcon->move(0, 0);
			//boatIcon->show();
			boatIcon->setAttribute(Qt::WA_DeleteOnClose);
			boatIcon->setObjectName(QString("%1-%2").arg(nType).arg(pObj->getID()));

			pGridLayout->addWidget(boatIcon, nObjIndex / 4, nObjIndex % 4, 1, 1);

			nObjIndex += 1;
		}
	}
}

void QRunSettingWidget::onRunTypeIndexChanged(int iIndex)
{
	int nRunType = ui.comboBox_runType->currentIndex();		
}

void QRunSettingWidget::onRunModeIndexChanged(int iIndex)
{
	int nRunMode = ui.comboBox_runMode->currentIndex();
	m_pCtrl->setRunMode(nRunMode);
}

void QRunSettingWidget::on3DDetectOpen()
{
	ICamera* pCam = getModule<ICamera>(CAMERA_MODEL);
	if (!pCam) return;

	IVision* pVision = getModule<IVision>(VISION_MODEL);
	if (!pVision) return;

	QString path = QApplication::applicationDirPath();
	path += "/capture/";

	QString filePath = QFileDialog::getExistingDirectory(this, QStringLiteral("打开图片文件夹"), path);

	if (!filePath.isEmpty())
	{
		cv::Mat matGray;
		if (convertToGrayImage(filePath, matGray))
		{
			pCam->setImage(matGray);
			pVision->loadImage(matGray);

			ui.lineEdit_3DDetectFile->setText(QString("%1").arg(filePath + "/"));
		}
		else
		{
			ui.lineEdit_3DDetectFile->setText("");
		}

		//ui.lineEdit_3DDetectFile->setText(QString("%1").arg(filePath + "/"));

		//std::string strImageFile = QString("%1").arg(filePath + "/" + "01.bmp").toStdString();
		//cv::Mat mat = cv::imread(strImageFile, cv::IMREAD_GRAYSCALE);
		//m_pView->setImage(mat);
	}
	else
	{
		ui.lineEdit_3DDetectFile->setText("");
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请选择文件夹"));
	}
}

void QRunSettingWidget::onCellEditor()
{
	QEos::Notify(EVENT_OBJ_STATE, 0, RUN_OBJ_EDITOR, 0);
}

void QRunSettingWidget::onProfileEditor()
{
	QEos::Notify(EVENT_OBJ_STATE, 0, RUN_OBJ_PROFILE, 0);
}

bool QRunSettingWidget::convertToGrayImage(QString& szFilePath, cv::Mat &matGray)
{
	IVision* pVision = getModule<IVision>(VISION_MODEL);
	if (!pVision) return false;

	//判断路径是否存在
	QDir dir(szFilePath);
	if (!dir.exists())
	{
		return false;
	}
	dir.setFilter(QDir::Files | QDir::NoSymLinks);
	QFileInfoList list = dir.entryInfoList();

	int file_count = list.count();
	if (file_count <= 0)
	{
		return false;
	}

	QVector<cv::Mat> imageMats;
	for (int i = 0; i < file_count; i++)
	{
		QFileInfo file_info = list.at(i);
		QString suffix = file_info.suffix();
		if (QString::compare(suffix, QString("ent"), Qt::CaseInsensitive) == 0)
		{
			QString absolute_file_path = file_info.absoluteFilePath();
			QString nameDecrypt = file_info.absolutePath() + "/" + file_info.baseName() + ".bmp";
			AOI::Crypt::DecryptFileNfg(absolute_file_path.toStdString(), nameDecrypt.toStdString());

			cv::Mat mat = cv::imread(nameDecrypt.toStdString(), cv::IMREAD_GRAYSCALE);
			imageMats.push_back(mat);

			QFile::remove(nameDecrypt);
		}
		else if (QString::compare(suffix, QString("bmp"), Qt::CaseInsensitive) == 0)
		{
			QString absolute_file_path = file_info.absoluteFilePath();
			cv::Mat mat = cv::imread(absolute_file_path.toStdString(), cv::IMREAD_GRAYSCALE);
			imageMats.push_back(mat);
		}
		else
		{
			return false;
		}
	}

	pVision->generateGrayImage(imageMats, matGray);

	return true;
}

