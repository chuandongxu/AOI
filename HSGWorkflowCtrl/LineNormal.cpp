#include "LineNormal.h"
#include <QFile>
#include <QXmlstreamReader>
#include <QApplication>

QLineNormal::QLineNormal()
{
}

QLineNormal::QLineNormal(const QString &evFile)
{
	this->load(evFile);
}


QLineNormal::~QLineNormal(void)
{
}


void QLineNormal::load(const QString &evFile)
{
	m_map.clear();

	QString path = QApplication::applicationDirPath();
	path += "/config/" + evFile;

	QFile file(path);
	if(file.open(QIODevice::ReadOnly))
	{
		QXmlStreamReader reader(&file);

		while(!reader.atEnd())
		{
			reader.readNext();
			if(reader.isStartElement())
			{
				QString name = reader.name().toString();
				if("item" == name)
				{
					QXmlStreamAttributes attrs = reader.attributes();
					double x = attrs.value("x").toDouble();
					double y = attrs.value("y").toDouble();
					m_map[x] = y;
				}
			}
		}
	}
}

double QLineNormal::value(double x)
{
	iterator first,last;
	if(findRange(x,first,last))
	{
		double k = (last.value() - first.value())/(last.key() - first.key());
		double y = k *(x- first.key()) + first.value();

		return y;
	}

	return 0;
}

bool QLineNormal::findRange(double x,iterator &first,iterator &last)
{
	QMap<double,double>::iterator it = m_map.begin();
	for(; it != m_map.end(); ++it)
	{
		if(it.key() > x)
		{
			if(it != m_map.begin())
			{
				first = it-1;
				last = it;

				return true;
			}
		}
	}

	return false;
}