#include "GraphicsContext.h"


GraphicsContext::GraphicsContext(void)
{
}


GraphicsContext::~GraphicsContext(void)
{
}



 void GraphicsContext::initial()
 {

 }


 void GraphicsContext::applyContext(QLabel* hv_window, QMap<QString, QString> cContext)
 {
	 if (!hv_window) return;

	 QString key  = "";
	 QString valS = "";  // 即 value 最终要用到的类型 可以是 QString 或 int  
	 int     valI = -1;	

	 QMap<QString, QString>::const_iterator iterator;

	 try
	 {
		 //while (iterator.MoveNext())
		 for ( iterator = cContext.begin(); iterator != cContext.end(); ++iterator )
		 {
			try
			{
				key = iterator.key();

				if ( GC_COLOR == key )
				{
					valS = iterator.value();
					//@HalconCpp::SetColor(hv_window, valS.toStdString().c_str());
				} 
				else if( GC_COLORED == key)
				{
					valI = iterator.value().toInt();
					//@HalconCpp::SetColored(hv_window, valI);
				}
				else if( GC_DRAWMODE == key)
				{
					valS = iterator.value();
					//@HalconCpp::SetDraw(hv_window, valS.toStdString().c_str());

				}
				else if( GC_LINEWIDTH == key)
				{
					valI = iterator.value().toInt();
					//@HalconCpp::SetLineWidth(hv_window, valI);
				}
				else if( GC_LUT == key)
				{
					valS = iterator.value();
					//@HalconCpp::SetLut(hv_window, valS.toStdString().c_str());
				}
				else if( GC_PAINT == key )
				{
					valS = iterator.value();
					//@HalconCpp::SetPaint(hv_window, valS.toStdString().c_str());
				}
				else if( GC_SHAPE == key)
				{
					valS = iterator.value();
					//@HalconCpp::SetShape(hv_window, valS.toStdString().c_str());
				}
				else if( GC_LINESTYLE == key )
				{
					//@ valH = (HTuple)cContext[key];
					//@ window.SetLineStyle(valH);
				}
			}
			catch (...)
			{
			}
			 
		 }//while
	 }
	 catch (...)
	 {
		 // gcNotification(e.Message);
		 return;
	 }


 }

 void GraphicsContext::setColorAttribute(QString val)
 {

 }

 void GraphicsContext::setDrawModeAttribute(QString val)
 {

 }

 void GraphicsContext::addValue(QString key, QString val)
 {
	 if (graphicalSettings.contains(key))
	 {
		 graphicalSettings[key] = val;
	 }
	 else
	 {
		 graphicalSettings.insert(key, val);
	 }
		 
 }

 void GraphicsContext::clear()
 {

 }

QMap<QString, QString> GraphicsContext::copyContextList()
{
	return graphicalSettings;
}