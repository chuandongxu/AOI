////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 SafeNet, Inc. All rights reserved.
//
// Dog(R) is a registered trademark of SafeNet, Inc. 
//
//
////////////////////////////////////////////////////////////////////
#if !defined(__DOG_API_CPP_PRIVATE_H__)
#define __DOG_API_CPP_PRIVATE_H__


#include "dog_api_cpp.h"
#include <sstream>

////////////////////////////////////////////////////////////////////
// Diagnostics support
////////////////////////////////////////////////////////////////////
#include "dogdiag.h"

////////////////////////////////////////////////////////////////////
// foreward declarations
////////////////////////////////////////////////////////////////////
struct CDogMapEntry;
struct CDogLock;
class CDogMapImpl;
class CDogMap;
class CDogImpl;


////////////////////////////////////////////////////////////////////
// struct CDogLock
////////////////////////////////////////////////////////////////////

#if defined(WIN32) || defined(WIN64)
#define DOG_HL_TARGET_WINDOWS
#endif // WIN32 || WIN64

////////////////////////////////////////////////////////////////////
// Windows
////////////////////////////////////////////////////////////////////
#if defined(DOG_HL_TARGET_WINDOWS)

// exclude rarely used window stuff
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct CDogLock
{
	// Construction/Destruction
public:
	CDogLock();
	~CDogLock();

	// Attributes
protected:
	CRITICAL_SECTION                        m_critLock;
	bool                                    m_bInit;

	// Implementation
public:
	bool isInit() const;
	bool lock();
	bool unlock();
};

#else
////////////////////////////////////////////////////////////////////
// Unix/Mac
////////////////////////////////////////////////////////////////////
#include <pthread.h>

struct CDogLock
{
	// Construction/Destruction
public:
	CDogLock();
	~CDogLock();

	// Attributes
protected:
	pthread_mutexattr_t                     m_attrRecursive;
	pthread_mutex_t                         m_mutex;
	bool                                    m_bInit;

	// Implementation
public:
	bool isInit() const;
	bool lock();
	bool unlock();
};

#endif // DOG_HL_TARGET_WINDOWS


////////////////////////////////////////////////////////////////////
// struct CDogMapEntry
////////////////////////////////////////////////////////////////////
struct CDogMapEntry
{
	// Construction/Destruction
public:
	CDogMapEntry();
	~CDogMapEntry();

	// Attributes
public:
	dog_u32_t                              m_ulCount;
	CDogImpl*                              m_pKey;
};


////////////////////////////////////////////////////////////////////
// class CDogMapImpl
////////////////////////////////////////////////////////////////////
class CDogMapImpl
{
	// Construction/Destruction
public:
	CDogMapImpl();
	~CDogMapImpl();

	// Attributes
public:
	static const dog_u32_t                 m_ulInitCapacity;
	std::vector<struct CDogMapEntry>       m_map;
	CDogLock                               m_lock;

	// Implementation
public:
	bool addRef(CDogHandle& handle) const;
	bool createKey(dog_u32_t feature, CDogHandle& handle);
	dog_u32_t findKey(const CDogImpl* pKey);
	CDogImpl* getKey(const CDogHandle& handle) const;
	bool lockMap() const;
	bool release(CDogHandle& handle) const;
	bool removeKey(const CDogImpl* pKey);
	bool unlockMap() const;
};


////////////////////////////////////////////////////////////////////
// class CDogMap
////////////////////////////////////////////////////////////////////
class CDogMap
{
	// Construction/Destruction
public:
	CDogMap();
	~CDogMap();

	// Attributes
protected:
	bool                                    m_bLocked;

	// Implementation
public:
	static bool addRef(CDogHandle& handle);
	static bool createKey(dog_u32_t feature, CDogHandle& handle);
	static dog_u32_t findKey(const CDogImpl* pKey);
	CDogImpl* getKey(const CDogHandle& handle) const;
	static bool release(CDogHandle& handle);
	static bool removeKey(const CDogImpl* pKey);

protected:
	friend class CDogBase;
	static CDogMapImpl& map();
};


////////////////////////////////////////////////////////////////////
// helper macros
////////////////////////////////////////////////////////////////////
#define DOG_PROLOGUE(handle) \
	CDogMap _map; \
	DIAG_ASSERT(isValid()); \
	if (!isValid()) \
	return DOG_INVALID_OBJECT; \
	CDogImpl* _pKey = _map.getKey((handle)); \
	DIAG_ASSERT(NULL != _pKey); \
	if (NULL == _pKey) \
	return DOG_INVALID_OBJECT; \

#define DOG_KEYPTR                         _pKey


////////////////////////////////////////////////////////////////////
// class CDogImpl
////////////////////////////////////////////////////////////////////
class CDogImpl
{
	// Construction/Destruction
public:
	CDogImpl(dog_feature_t feature, CDogHandle& handle);

protected:
	virtual ~CDogImpl();

	// Attributes
protected:
	enum
	{
		stIdle                              = 0,
		stActive                            = 1
	};

	dog_handle_t                           m_handle;
	dog_feature_t                          m_feature;
	dog_u32_t                              m_ulState;
	dog_u32_t                              m_ulRefCount;
	dog_u32_t                              m_ulAltered;

	// Implementation
public:
	dog_u32_t altered() const;
	dogStatus decrypt(unsigned char* pData, dog_size_t ulSize) const;
	dogStatus encrypt(unsigned char* pData, dog_size_t ulSize) const;
	dog_feature_t feature() const;
	dogStatus getFileSize(dog_fileid_t fileId, dog_size_t& ulSize) const;
	static dogStatus getInfo(const char* pszQuery, const char* pszFormat, dog_vendor_code_t vendorCode, CDogInfo& info);
	dogStatus getTime(dog_time_t& time) const;
	dogStatus getSessionInfo(const char* pszFormat, CDogInfo& info) const;
	static dogStatus getVersion(dog_vendor_code_t vendorCode, CDogVersion& version);
	dog_handle_t handle() const;
	bool isAltered(const CDogHandle& handle) const;
	bool isLoggedIn() const;
	dogStatus login(dog_vendor_code_t vendorCode, const char* pszScope );
	dogStatus logout(bool bFinal = false);
	dogStatus readFile(dog_fileid_t fileId,
		dog_size_t ulOffset,
		unsigned char* pData,
		dog_size_t ulSize) const;
	dogStatus setIdleTime(dog_u16_t nIdleTime) const;
	std::string toString() const;
	dogStatus writeFile(dog_fileid_t fileId,
		dog_size_t ulOffset,
		const unsigned char* pData,
		dog_size_t ulSize) const;
	static dogStatus update(const char* pszUpdate, CDogInfo& acknowledge);

public:
	bool addRef(CDogHandle& handle);
	bool release();
};


////////////////////////////////////////////////////////////////////
// class CDogBase64
////////////////////////////////////////////////////////////////////
class CDogBase64
{
	// Construction/Destruction
public:
	CDogBase64();
	~CDogBase64();

	// Attributes
protected:
	static const char                       m_rAlphabet[];
	static const char                       m_cPad;
	static const size_t                     m_ulMaxLineLength;

	// Implementation
public:
	static void decode(const std::string& source,
		std::vector<unsigned char>& dest) throw();
	static void encode(const std::vector<unsigned char>& source,
		std::string& dest) throw();

protected:
	int decodeChar(const int nChar) const;
};


#endif // !__DOG_API_CPP_PRIVATE_H__
