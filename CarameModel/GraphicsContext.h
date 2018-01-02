#pragma once

#include <QMap>
#include <QString>
#include <QVariant>
#include <QLabel>

/// <summary>
/// Graphical mode for the output  (see dev_set_*)
/// </summary>    
const QString GC_COLOR	   = "Color";
const QString GC_COLORED   = "Colored";
const QString GC_LINEWIDTH = "LineWidth";
const QString GC_DRAWMODE  = "DrawMode";
const QString GC_SHAPE     = "Shape";
const QString GC_LUT       = "Lut";
const QString GC_PAINT     = "Paint";
const QString GC_LINESTYLE = "LineStyle";

/****
   这个类用于设置显示对象的一些属性， 如颜色等
/****/
class GraphicsContext
{

private :
	QMap<QString, QString> graphicalSettings;
	// private IEnumerator iterator;

public	:
	/// <summary> 
	/// Backup of the last graphical context applied to the window.
	/// </summary>
	QMap<QString, QString>		stateOfSettings;

public:
	GraphicsContext();
	GraphicsContext(QMap<QString, QString> settings);
	~GraphicsContext(void);
	void initial();

public:
   
  void applyContext(QLabel* hv_window, QMap<QString, QString> cContext);
  void setColorAttribute(QString val);
  void setDrawModeAttribute(QString val);

public:
//  void addValue(QString key, int val);
  void addValue(QString key, QString val);
//  void addValue(QString key, HTuple val);

public:
	 void clear();
// 	 GraphicsContext copy();
// 	 QVariant getGraphicsAttribute(QString key);
     QMap<QString, QString> copyContextList();
};




