#ifndef _I_FUNCTION_MGR_H_
#define _I_FUNCTION_MGR_H_

#include <qstring.h>
#include <qmap.h>
#include <qvariant.h>

#define FUNCTION_MGR_ID 88001

class ICmdExec
{
public:
	virtual bool exec() = 0;

	//virtual void setParam(const QStringList &params) = 0;
	//virtual void setParam(const QMap<QString,QVariant> &params) = 0;
	virtual void setParam(const QString &name,const QVariant &val) = 0;

	//virtual QStringList getParam() = 0;
	//virtual void getParam(QMap<QString,QVariant> &params) = 0;
	virtual QVariant getParam(const QString &name) = 0;

	//virtual void setResoult(const QStringList &resoult) = 0;
	//virtual void setResoult(const QMap<QString,QVariant> &resoult) = 0;
	virtual void setResoult(const QString &name,const QVariant &val) = 0;

	//virtual QStringList getResoult() = 0;
	//virtual QVariant getResoult(const QString &name) = 0;
	virtual void getResoult(QMap<QString,QVariant> &params) = 0;
};

class QCmdExecBase : public ICmdExec
{
public:
	//virtual void setParam(const QStringList &params);
	//virtual void setParam(const QMap<QString,QVariant> &params);
	virtual void setParam(const QString &name,const QVariant &val){
		m_params[name] = val;
	};

	//virtual QStringList getParam();
	//virtual void getParam(QMap<QString,QVariant> &params);
	virtual QVariant getParam(const QString &name){
		if(m_params.contains(name))return m_params[name];
		return QVariant();
	};

	//virtual void setResoult(const QStringList &resoult);
	//virtual void setResoult(const QMap<QString,QVariant> &resoult);
	virtual void setResoult(const QString &name,const QVariant &val){
		m_resoult[name] = val;
	};

	//virtual QStringList getResoult();
	//virtual QVariant getResoult(const QString &name);
	virtual void getResoult(QMap<QString,QVariant> &params){
		params = m_params;
	}

protected:
	QMap<QString,QVariant> m_params;
	QMap<QString,QVariant> m_resoult;
};

template<typename T1>
class QCmdExec : public QCmdExecBase
{
	typedef bool (T1::*FUNC)(ICmdExec * pExec);
public:
	QCmdExec(T1 *obj,FUNC func)
		:m_pobj(obj),m_func(func)
	{}
	virtual bool exec(){
		if(m_pobj && m_func)
			return (m_pobj->*m_func)(this);

		return false;
	};
	
protected:
	T1 * m_pobj;
	FUNC m_func;
};


class IFunctionMgr
{
public:
	typedef QMap<QString,QVariant> QParams;

public:
	virtual void addCmdMaper(const QString &cmd,ICmdExec * p) = 0;
	virtual void removeCmdMaper(const QString &cmd,ICmdExec * p) = 0;

	virtual bool exec(const QString & cmdline) = 0;
	virtual bool exec(const QString & cmd,const QStringList &params,QStringList &resoult) = 0;
	virtual bool exec(const QString & cmd,const QParams &params,QParams &resoult) = 0;	
};

/*
template<typename T>
void addcmd(const QString &str,T *p){
	IFunctionMgr * p = getModule<IFunctionMgr>(FUNCTION_MGR_ID);
	if(p)p->addCmdMaper(str,p);
}
*/

#define CMD_MAPER(CMD,OBJ) {IFunctionMgr * pIFunctionMgr = getModule<IFunctionMgr>(FUNCTION_MGR_ID);\
	if(pIFunctionMgr)pIFunctionMgr->addCmdMaper(CMD,OBJ);\
}

#endif
