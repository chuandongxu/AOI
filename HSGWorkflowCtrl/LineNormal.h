#pragma once
#include <QMap>

class QLineNormal
{
	typedef QMap<double,double> QNormalMap;
	typedef QNormalMap::iterator iterator;
public:
	QLineNormal();
	QLineNormal(const QString &evFile);
	~QLineNormal(void);

	void load(const QString &evFile);
	double value(double in);

protected:
	bool findRange(double x,iterator &first,iterator &last);

protected:
	QNormalMap m_map; 
};

