#include "ViewEditor.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>

ViewEditor::ViewEditor(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	init();

	QVBoxLayout* pVBoxLayout = new QVBoxLayout(ui.frame);
	m_pViewWidget = new ViewWidget();
	pVBoxLayout->addWidget(m_pViewWidget);	

	m_pViewSetting = new QViewSetting();

	m_bShown = false;
}

ViewEditor::~ViewEditor()
{
	if (m_pViewWidget)
	{
		delete m_pViewWidget;
		m_pViewWidget = NULL;
	}	

	if (m_pViewSetting)
	{
		delete m_pViewSetting;
		m_pViewSetting = NULL;
	}
}

void ViewEditor::loadFile(bool invert, int nSizeX, int nSizeY, QVector<Vector>& values)
{
	if (m_pViewWidget)
	{
		m_pViewWidget->loadFile(invert, nSizeX, nSizeY, values);
	}
}

void ViewEditor::changeToMesh()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->changeMeshTriView(false);
	}
}

void ViewEditor::previousROIDisplay()
{	
}

void ViewEditor::prepareROIDisplay(QImage& texture, bool invert)
{
	m_pViewWidget->topView();
	m_pViewWidget->changeMeshTriView(false);
	m_pViewWidget->meshTexture(texture, invert);
	m_pViewWidget->rotateView();
}

QGLWidget* ViewEditor::getQGLWidget()
{
	return m_pViewWidget;
}

void ViewEditor::closeEvent(QCloseEvent *e){
	//qDebug() << "关闭事件";
	e->ignore();

	this->hide();
	m_pViewWidget->hideView();

	m_pViewSetting->hide();

	m_bShown = false;
}

void ViewEditor::init()
{
	setAttribute(Qt::WA_DeleteOnClose);

	createActions();
	createMenus();
	createToolBars();
	createStatusBar();	

	setUnifiedTitleAndToolBarOnMac(true);
}

void ViewEditor::createActions()
{
	newAct = new QAction(QIcon("image/newFile.png"), QStringLiteral("新建"), this);
	newAct->setShortcuts(QKeySequence::New);
	newAct->setStatusTip(tr("Create a new file"));
	connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

	openAct = new QAction(QIcon("image/openFile.png"), QStringLiteral("打开..."), this);
	openAct->setShortcuts(QKeySequence::Open);
	openAct->setStatusTip(tr("Open an existing file"));
	connect(openAct, SIGNAL(triggered()), this, SLOT(openFile()));
	openAct->setEnabled(false);

	saveAct = new QAction(QIcon("image/saveFile.png"), QStringLiteral("保存"), this);
	saveAct->setShortcuts(QKeySequence::Save);
	saveAct->setStatusTip(tr("Save the document to disk"));
	connect(saveAct, SIGNAL(triggered()), this, SLOT(saveFile()));

	saveAsAct = new QAction(QIcon("image/saveAsFile.png"), QStringLiteral("另存为..."), this);
	saveAsAct->setShortcuts(QKeySequence::SaveAs);
	saveAsAct->setStatusTip(tr("Save the document under a new name"));
	connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAsFile()));

	zoomInAct = new QAction(QIcon("image/zoomOut.png"), QStringLiteral("缩小"), this);
	zoomInAct->setShortcuts(QKeySequence::ZoomIn);
	zoomInAct->setStatusTip(tr("Zoom in window"));
	connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

	zoomOutAct = new QAction(QIcon("image/zoomIn.png"), QStringLiteral("放大"), this);
	zoomOutAct->setShortcuts(QKeySequence::ZoomOut);
	zoomOutAct->setStatusTip(tr("Zoom out window"));
	connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

	moveAct = new QAction(QIcon("image/moveView.png"), QStringLiteral("平移"), this);
	moveAct->setShortcuts(QKeySequence::MoveToNextWord);
	moveAct->setStatusTip(tr("Move window"));
	connect(moveAct, SIGNAL(triggered()), this, SLOT(moveView()));

	rotateAct = new QAction(QIcon("image/rotateView.png"), QStringLiteral("旋转"), this);
	rotateAct->setShortcuts(QKeySequence::MoveToNextChar);
	rotateAct->setStatusTip(tr("Rotate window"));
	connect(rotateAct, SIGNAL(triggered()), this, SLOT(rotateView()));

	fullScreenAct = new QAction(QIcon("image/fullScreen.png"), QStringLiteral("全屏"), this);
	fullScreenAct->setShortcuts(QKeySequence::FullScreen);
	fullScreenAct->setStatusTip(tr("Full screen"));
	connect(fullScreenAct, SIGNAL(triggered()), this, SLOT(fullScreen()));

	frontViewAct = new QAction(QIcon("image/frontView.png"), QStringLiteral("前视图"), this);
	frontViewAct->setShortcuts(QKeySequence::SelectNextChar);
	frontViewAct->setStatusTip(tr("Front View"));
	connect(frontViewAct, SIGNAL(triggered()), this, SLOT(frontView()));

	topViewAct = new QAction(QIcon("image/topView.png"), QStringLiteral("俯视图"), this);
	topViewAct->setShortcuts(QKeySequence::SelectPreviousChar);
	topViewAct->setStatusTip(tr("Top View"));
	connect(topViewAct, SIGNAL(triggered()), this, SLOT(topView()));

	sideViewAct = new QAction(QIcon("image/sideView.png"), QStringLiteral("右视图"), this);
	sideViewAct->setShortcuts(QKeySequence::SelectNextWord);
	sideViewAct->setStatusTip(tr("Side View"));
	connect(sideViewAct, SIGNAL(triggered()), this, SLOT(sideView()));

	showPointCloudAct = new QAction(QIcon("image/showCloud.png"), QStringLiteral("显示点云"), this);
	showPointCloudAct->setShortcuts(QKeySequence::SelectPreviousWord);
	showPointCloudAct->setStatusTip(tr("Point Cloud"));
	connect(showPointCloudAct, SIGNAL(triggered()), this, SLOT(showPointCloud()));

	showMeshTriAct = new QAction(QIcon("image/showMesh.png"), QStringLiteral("曲面显示"), this);
	showMeshTriAct->setShortcuts(QKeySequence::SelectNextLine);
	showMeshTriAct->setStatusTip(tr("Mesh Trigger"));
	connect(showMeshTriAct, SIGNAL(triggered()), this, SLOT(showMeshTri()));

	meshSurfaceCutAct = new QAction(QIcon("image/cutSurface.png"), QStringLiteral("剖线显示"), this);
	meshSurfaceCutAct->setShortcuts(QKeySequence::SelectPreviousLine);
	meshSurfaceCutAct->setStatusTip(tr("Surface Cutting"));
	connect(meshSurfaceCutAct, SIGNAL(triggered()), this, SLOT(surfaceCut()));

	meshSurfaceTextureAct = new QAction(QIcon("image/textureMesh.png"), QStringLiteral("贴图显示"), this);
	meshSurfaceTextureAct->setShortcuts(QKeySequence::SelectNextPage);
	meshSurfaceTextureAct->setStatusTip(tr("Surface Texture"));
	connect(meshSurfaceTextureAct, SIGNAL(triggered()), this, SLOT(surfaceTexture()));

	aboutAct = new QAction(tr("&About"), this);
	aboutAct->setStatusTip(tr("Show the application's About box"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

	aboutSystemAct = new QAction(tr("About System"), this);
	aboutSystemAct->setStatusTip(tr("Show the System About box"));
	connect(aboutSystemAct, SIGNAL(triggered()), this, SLOT(aboutSystem()));

	viewSettingAct = new QAction(QIcon("image/rectangle2.png"), QStringLiteral("参数设置"), this);
	viewSettingAct->setShortcuts(QKeySequence::SelectEndOfDocument);
	viewSettingAct->setStatusTip(tr("Show the parameters setting dialog"));
	connect(viewSettingAct, SIGNAL(triggered()), this, SLOT(viewSetting()));
}

//! [implicit tr context]
void ViewEditor::createMenus()
{
	ui.mainToolBar->setVisible(false);

	fileMenu = menuBar()->addMenu(QStringLiteral("文件"));
	//! [implicit tr context]
	fileMenu->addAction(newAct);
	fileMenu->addAction(openAct);
	fileMenu->addAction(saveAct);
	fileMenu->addAction(saveAsAct);
	fileMenu->addSeparator();

	editMenu = menuBar()->addMenu(QStringLiteral("视图"));
	editMenu->addAction(zoomInAct);
	editMenu->addAction(zoomOutAct);
	editMenu->addAction(moveAct);
	editMenu->addAction(rotateAct);
	editMenu->addAction(fullScreenAct);
	editMenu->addSeparator();
	editMenu->addAction(frontViewAct);
	editMenu->addAction(topViewAct);
	editMenu->addAction(sideViewAct);

	settingMenu = menuBar()->addMenu(QStringLiteral("设置"));
	settingMenu->addAction(viewSettingAct);

	menuBar()->addSeparator();

	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(aboutAct);
	helpMenu->addAction(aboutSystemAct);
}

void ViewEditor::createToolBars()
{
	//! [0]
	fileToolBar = addToolBar(tr("File"));
	fileToolBar->addAction(newAct);
	fileToolBar->addAction(openAct);
	fileToolBar->addAction(saveAct);
	fileToolBar->addAction(saveAsAct);

	editToolBar = addToolBar(tr("Edit"));
	editToolBar->addAction(zoomInAct);
	editToolBar->addAction(zoomOutAct);
	editToolBar->addAction(moveAct);
	editToolBar->addAction(rotateAct);
	editToolBar->addAction(fullScreenAct);
	editToolBar->addSeparator();
	editToolBar->addAction(frontViewAct);
	editToolBar->addAction(topViewAct);
	editToolBar->addAction(sideViewAct);
	editToolBar->addSeparator();
	editToolBar->addAction(showPointCloudAct);
	editToolBar->addAction(showMeshTriAct);
	editToolBar->addAction(meshSurfaceCutAct);
	editToolBar->addAction(meshSurfaceTextureAct);
}

void ViewEditor::createStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
}

void ViewEditor::newFile()
{
	//IMap * pMap = getModule<IMap>(MAP_MODEL);
	//if (!pMap) return;

	//pMap->clearMapInfo();

	//QString mapPath = QApplication::applicationDirPath();
	//mapPath += "/map/mapData" + QDateTime::currentDateTime().toString("MMddhhmmss") + ".xml";

	//pMap->saveMapInfo(mapPath, true);
	
}

void ViewEditor::openFile()
{	
	QString path = QApplication::applicationDirPath();
	path += "/3D/data/";

	QString fileName = QFileDialog::getOpenFileName(this,
		QString("Select Data File to load"),
		path,
		"*.csv *.txt *.raw");

	if (!fileName.isEmpty())
	{
	
		if (m_pViewWidget)
		{
			m_pViewWidget->loadFile(fileName);
		}
	}
}

void ViewEditor::saveFile()
{
	/*if (!m_pCtrl->getMapPath().isEmpty())
	{
		m_pCtrl->saveMapInfo(m_pCtrl->getMapPath(), false);
	}*/
}

void ViewEditor::saveAsFile()
{
	/*QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
		m_pCtrl->getMapPath());
	if (fileName.isEmpty())
	{
		return;
	}

	if (!fileName.isEmpty())
	{
		m_pCtrl->saveMapInfo(fileName, true);
	}*/
}

void ViewEditor::zoomIn()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->zoomIn();
	}
}

void ViewEditor::zoomOut()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->zoomOut();
	}
}

void ViewEditor::moveView()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->moveView();
	}
}

void ViewEditor::rotateView()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->rotateView();
	}
}

void ViewEditor::fullScreen()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->fullScreen();
	}
}

void ViewEditor::frontView()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->frontView();
	}
}

void ViewEditor::topView()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->topView();
	}
}

void ViewEditor::sideView()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->sideView();
	}
}

void ViewEditor::showPointCloud()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->changePointCloudView();
	}
}

void ViewEditor::showMeshTri()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->changeMeshTriView();
	}
}

void ViewEditor::surfaceCut()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->csgCutMesh();

		int nLeft = this->geometry().left();
		int nTop = this->geometry().top();
		m_pViewWidget->setBasePos(nLeft, nTop);
	}	
}

void ViewEditor::surfaceTexture()
{
	if (m_pViewWidget)
	{
		m_pViewWidget->meshTexture();
	}
}

void ViewEditor::about()
{
	QMessageBox::about(this, tr("About Editor"),
		tr("The <b>Editor</b> example demonstrates how to edit or "
		"display 3D applications using system."));
}

void ViewEditor::aboutSystem()
{
	QMessageBox::about(this, tr("About Editor"),
		tr("The <b>Editor</b> example demonstrates how to display "
		"3D applications using system."));
}

void ViewEditor::viewSetting()
{
	m_pViewSetting->show();
}

