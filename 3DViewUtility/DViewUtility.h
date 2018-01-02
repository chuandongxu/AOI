#pragma once

#include "dviewutility_global.h"

#include <QImage>

class ViewEditor;
class QGLWidget;
class DVIEWUTILITY_EXPORT DViewUtility
{
public:
    DViewUtility();
	~DViewUtility();

	void show();
	void hide();
	bool isShown();
	void changeToMesh();
	void loadFile(bool invert, int nSizeX, int nSizeY, QVector<double>& xValues, QVector<double>& yValues, QVector<double>& zValues);

	void previousROIDisplay();
	void prepareROIDisplay(QImage& texture, bool invert);
	QGLWidget* getQGLWidget();
private:
	ViewEditor *m_pInstance;	

};
