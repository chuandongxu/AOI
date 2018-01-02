#pragma once

#include <QMainWindow>
#include "ui_ViewEditor.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QTextEdit;
QT_END_NAMESPACE

#include "ViewWidget.h"
#include "ViewSetting.h"

class ViewEditor : public QMainWindow
{
	Q_OBJECT

public:
	ViewEditor(QWidget *parent = Q_NULLPTR);
	~ViewEditor();

public:
	void loadFile(bool invert, int nSizeX, int nSizeY, QVector<Vector>& values);
	void changeToMesh();

	void previousROIDisplay();
	void prepareROIDisplay(QImage& texture, bool invert);
	QGLWidget* getQGLWidget();


	void setShow(bool bShow) { m_bShown = bShow; }
	bool isShown(){ return m_bShown; }

protected:
	void closeEvent(QCloseEvent *e);

private slots:
	void newFile();
	void openFile();
	void saveFile();
	void saveAsFile();
	void zoomIn();
	void zoomOut();
	void moveView();
	void rotateView();
	void fullScreen();
	void frontView();
	void topView();
	void sideView();
	void showPointCloud();
	void showMeshTri();
	void surfaceCut();
	void surfaceTexture();
	void about();
	void aboutSystem();

	void viewSetting();

private:
	void init();
	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();

private:
	QMenu *fileMenu;
	QMenu *editMenu;
	QMenu *settingMenu;
	QMenu *helpMenu;

	QToolBar *fileToolBar;
	QToolBar *editToolBar;
	QAction *newAct;
	QAction *openAct;
	QAction *saveAct;
	QAction *saveAsAct;

	QAction *zoomInAct;
	QAction *zoomOutAct;
	QAction *moveAct;
	QAction *rotateAct;
	QAction *fullScreenAct;
	QAction *frontViewAct;
	QAction *topViewAct;
	QAction *sideViewAct;

	QAction *showPointCloudAct;
	QAction *showMeshTriAct;
	QAction *meshSurfaceCutAct;
	QAction *meshSurfaceTextureAct;

	QAction *aboutAct;
	QAction *aboutSystemAct;

	QAction *viewSettingAct;

private:
	Ui::ViewEditor ui;
	ViewWidget* m_pViewWidget;	
	QViewSetting* m_pViewSetting;
	bool m_bShown;
};
