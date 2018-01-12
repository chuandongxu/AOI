#include "ErrorCode.h"
#include <qsqlquery>
#include <qvariant>
#include <QMessageBox> 
#include <qapplication.h>
#include "track.h"

QErrorCode::QErrorCode(void)
{
}

QErrorCode::~QErrorCode(void)
{
}

const QString QErrorCode::getErrorString(unsigned int error)
{
	if(m_errorMap.contains(error))
	{
		return m_errorMap[error];
	}

	return "";
}

void QErrorCode::init()
{
	m_errorMap[ERROR_NO_ERROR]				= QStringLiteral("");
	m_errorMap[ERROR_START_UP]				= QStringLiteral("系统启动。");
	m_errorMap[ERROR_SHUTDOWN]				= QStringLiteral("系统退出。");
	m_errorMap[ERROR_CVS_OPEN_ALM]          = QStringLiteral("测量结果记录失败，请检查配置路劲是否正确，是否有写权限");
}

void QErrorCode::addErrorMap(unsigned int id,const QString &descr)
{
	m_errorMap[id] = descr;
}