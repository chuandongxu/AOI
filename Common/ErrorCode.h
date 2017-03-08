#pragma once

#include <qobject.h>
#include <QString>
#include <QMap>
#include <qstandarditemmodel.h>
#include "Common_global.h"

//----------------------------------------------------------
//0xABBBCCCC
// A: 告警级别 
//           0：error 
//           1 :工艺流程错误
//           3：warring,
//           4:info,
//           5 : 工艺流程告警
//           6:TRACK
//           9:默认正常状态
// B: 子模块,0系统核模块
// c: 告警代码
//系统运行信息
#define ERROR_NO_ERROR              0x90000000
#define ERROR_START_UP				0x60000001
#define ERROR_SHUTDOWN              0x60000002
#define ERROR_CVS_OPEN_ALM          0x50000003

//-----------------------------------------------------------------
class COMMON_EXPORT QErrorCode
{
	typedef QMap<unsigned int,QString> QErrorMap;
public:
	QErrorCode(void);
	~QErrorCode(void);

	void init();
	const QString getErrorString(unsigned int error);

	void addErrorMap(unsigned int id,const QString &descr);

private:
	QErrorMap m_errorMap;
};

class COMMON_EXPORT QErrorModel : public QStandardItemModel
{
	Q_OBJECT
public:
	QErrorModel(){};
	
public slots:
	void onErrorInfo(const QString &data,const QString &msg,unsigned int level);
};