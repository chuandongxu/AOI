#ifndef _SINGLONE_H_
#define _SINGLONE_H_

#include <QString>

template<typename T>
class  QSinglone
{
	class QInns : public T{
	public:
		QInns(){};
		virtual ~QInns(){};
	};
protected:
	QSinglone(){m_obj = new QInns();};
	~QSinglone(){if(m_obj)delete m_obj;};

public:
	static T * getInstance();
	static void destory();

protected:
	T * m_obj;
private:
	static QSinglone<T> * _instance_;
	static bool _bDestoryed_;
};

template<typename T>
QSinglone<T> * QSinglone<T>::_instance_ = NULL;
template<typename T>
bool QSinglone<T>::_bDestoryed_ = false;

template<typename T>
T * QSinglone<T>::getInstance(){
	if(!_instance_){
		if(!_bDestoryed_)_instance_ = new QSinglone<T>();
	}
	
	if(_instance_){
		return _instance_->m_obj;
	}
	return NULL;
}

template<typename T>
void QSinglone<T>::destory()
{
	if(_instance_){
		delete _instance_;
		_instance_ = NULL;
		_bDestoryed = true;
	}
}

//-----------------------------------------------------------------------------------------
template<typename T>
class  QSingloneMgr
{
protected:
	QSingloneMgr(){};
	~QSingloneMgr(){};

public:
	static QSingloneMgr<T> * getInstance();
	static void destory();

	void addMgrObj(const QString &name,T * w){
		m_map[name] = w;
	};
	void removeMgrObj(const QString &name,T * w){
		m_map.remove(name);
	};

	T * getMgrObj(const QString &name){
		if(m_map.contans(name)){
			return m_map[name];
		}
	};

protected:
	QMap<QString,T*> m_map;

private:
	static QSingloneMgr<T> * _instance_;
	static bool _bDestoryed_;
};

template<typename T>
QSingloneMgr<T> * QSingloneMgr<T>::_instance_ = NULL;

template<typename T>
bool QSingloneMgr<T>::_bDestoryed_ = false;

template<typename T>
QSingloneMgr<T> * QSingloneMgr<T>::getInstance(){
	if(!_instance_){
		if(!_bDestoryed_)_instance_ = new QSingloneMgr<T>();
	}
	return _instance_;
}

template<typename T>
void QSingloneMgr<T>::destory()
{
	if(_instance_){
		delete _instance_;
		_instance_ = NULL;
		_bDestoryed = true;
	}
}

#endif