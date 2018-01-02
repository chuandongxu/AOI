////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 SafeNet, Inc. All rights reserved.
//
// Dog(R) is a registered trademark of SafeNet, Inc. 
//
//
////////////////////////////////////////////////////////////////////
#if !defined(__DOG_API_CPP_H__)
#define __DOG_API_CPP_H__


#include <iterator>
#include <string>
#include <vector>
#include <typeinfo>

#if !defined(__DOG_API_H__)
// DO NOT ALTER THIS PATH
// OTHERWISE YOU BLOW UP THE CD INSTALLATION
#include "dog_api.h"
#endif // __DOG_API_H__


#if !defined(DOGCPP_DECL)
#define DOGCPP_DECL
#endif // DOG_DECL

#ifdef __GNUC__
#define DEPRECATED(func) func __attribute__ ((deprecated))
#elif (_MSC_VER >= 1400)
#define DEPRECATED(func) __declspec(deprecated) func
#else
//#pragma message("WARNING: DEPRECATED is not defined for this compiler")
#define DEPRECATED(func) func
#endif

////////////////////////////////////////////////////////////////////
// Foreward declarations
////////////////////////////////////////////////////////////////////

struct DOGCPP_DECL CDogFeature;
struct DOGCPP_DECL CDogInfo;
struct DOGCPP_DECL CDogTime;
struct DOGCPP_DECL CDogHandle;

class DOGCPP_DECL CDogBase;
class DOGCPP_DECL CDog;
class DOGCPP_DECL CDogFile;


////////////////////////////////////////////////////////////////////
// struct CDogFeature
////////////////////////////////////////////////////////////////////
struct DOGCPP_DECL CDogFeature
{
	// Construction/Destruction
public:
	explicit CDogFeature(dog_u32_t ulFeature);
	~CDogFeature();

	// Attributes
public:
	enum
	{
		optDefault                          = 0,
	};

protected:
	dog_feature_t                          m_ulFeature;

	// Operators
public:
	operator dog_feature_t() const;

	// Implementation
public:
	dog_u32_t feature() const;
	dog_u32_t featureId() const;
	static CDogFeature defaultFeature();
	static CDogFeature fromFeature(dog_u32_t ulFeature);

	bool isDefault() const;
	std::string toString() const;
};


////////////////////////////////////////////////////////////////////
// struct CDogInfo
////////////////////////////////////////////////////////////////////
struct DOGCPP_DECL CDogInfo
{
	// Construction/Destruction
public:
	CDogInfo();
	~CDogInfo();

	// Attributes
public:
	char*                                   m_pszInfo;

	// Operators
public:
	operator const char*() const;

	// Implementation
public:
	void clear();
	const char* getInfo() const;
};


////////////////////////////////////////////////////////////////////
// struct CDogTime
////////////////////////////////////////////////////////////////////
struct DOGCPP_DECL CDogTime
{
	// Constrution/Destruction
public:
	CDogTime(dog_time_t time = 0);
	CDogTime(unsigned int nYear, unsigned int nMonth, unsigned int nDay,
		unsigned int nHour, unsigned int nMinute, unsigned int nSecond);
	~CDogTime();

	// Attributes
protected:
	dog_time_t                             m_time;

	// Operators
public:
	operator dog_time_t() const;

	// Implementation
public:
	unsigned int year() const;
	unsigned int month() const;
	unsigned int day() const;
	unsigned int hour() const;
	unsigned int minute() const;
	unsigned int second() const;
	dog_time_t time() const;
};


////////////////////////////////////////////////////////////////////
// struct CDogVersion
////////////////////////////////////////////////////////////////////
struct DOGCPP_DECL CDogVersion
{
	// Constrution/Destruction
public:
	CDogVersion();
	CDogVersion(const CDogVersion& version);
	CDogVersion(unsigned int nMajorVersion, unsigned int nMinorVersion,
		unsigned int nServerBuild, unsigned int nBuildNumber);
	~CDogVersion();

	// Attributes
protected:
	unsigned int m_nMajorVersion;
	unsigned int m_nMinorVersion;
	unsigned int m_nServerBuild;
	unsigned int m_nBuildNumber;

	// Operators
public:
	CDogVersion& operator=(const CDogVersion& version);
	bool operator==(const CDogVersion& version) const;
	bool operator!=(const CDogVersion& version) const;

	// Implementation
public:
	unsigned int majorVersion() const;
	unsigned int minorVersion() const;
	unsigned int serverBuild() const;
	unsigned int buildNumber() const;
};


////////////////////////////////////////////////////////////////////
// struct CDogHandle
////////////////////////////////////////////////////////////////////
struct DOGCPP_DECL CDogHandle
{
	// Construction/Destruction
public:
	CDogHandle();
	~CDogHandle();

	// Attributes
public:
	dog_u32_t                              m_ulIndex;
	dog_u32_t                              m_ulCount;
	dog_u32_t                              m_ulAltered;

	// Operators
public:
	bool operator==(const CDogHandle& other) const;
	bool operator!=(const CDogHandle& other) const;

	// Implementation
public:
	void clear();
	bool isNull() const;
};


////////////////////////////////////////////////////////////////////
// class CDogBase
////////////////////////////////////////////////////////////////////
class DOGCPP_DECL CDogBase
{
	// Construction/Destruction
protected:
	CDogBase();
	explicit CDogBase(const CDogBase& other);
	explicit CDogBase(dog_feature_t feature);
	virtual ~CDogBase();

	// Attributes
protected:
	CDogHandle                             m_handle;

	// Operators
public:
	CDogBase& operator=(const CDogBase& other);
	virtual bool operator==(const CDogBase& other) const;
	bool operator!=(const CDogBase& other) const;

	// Overrides
public:
	virtual dog_u32_t hashCode() const;
	virtual bool isValid() const;
	virtual std::string toString() const = 0;

protected:
	virtual bool addRef(const CDogBase& other);
	virtual bool construct(dog_feature_t feature);
	virtual bool release();
	virtual void synchronize() const;

	// Implementation
public:
	bool dispose();
	bool isKindOf(const std::type_info& info) const;
	bool isLoggedIn() const;

protected:
	CDogHandle handle() const;
};


////////////////////////////////////////////////////////////////////
// SuperDog status codes
////////////////////////////////////////////////////////////////////
typedef dog_status_t dogStatus;


////////////////////////////////////////////////////////////////////
// Helpers
////////////////////////////////////////////////////////////////////
template<typename _Type>
__inline bool DOG_SUCCEEDED(_Type status)
{ return DOG_STATUS_OK == static_cast<dogStatus>(status); }


////////////////////////////////////////////////////////////////////
// class CDog
////////////////////////////////////////////////////////////////////
class DOGCPP_DECL CDog : public CDogBase
{
	// Construction/Destruction
public:
	CDog();
	CDog(const CDog& other);
	explicit CDog(const CDogBase& other);
	explicit CDog(const CDogFeature& feature);

	// Operators
public:
	CDog& operator=(const CDogBase& other);
	CDog& operator=(const CDog& other);

	// Overrides
public:
	virtual std::string toString() const;

protected:
	virtual void synchronize() const;

	// Implementation
public:
	dogStatus decrypt(unsigned char* pData, dog_size_t ulSize) const;
	dogStatus decrypt(const char* szData) const;
	dogStatus decrypt(std::string& data) const;
	dogStatus encrypt(unsigned char* pData, dog_size_t ulSize) const;
	dogStatus encrypt(const char* szData) const;
	dogStatus encrypt(std::string& data) const;
	CDogFeature feature() const;
	CDogFile getFile() const;
	CDogFile getFile(dog_fileid_t fileId) const;
	static dogStatus getInfo(const char* pszQuery, const char* pszFormat, dog_vendor_code_t vendorCode, std::string& info);
	static dogStatus getInfo(const char* pszQuery, const std::string& format, dog_vendor_code_t vendorCode, std::string& info);
	static dogStatus getInfo(const std::string& query, const char* pszFormat, dog_vendor_code_t vendorCode, std::string& info);
	static dogStatus getInfo(const std::string& query, const std::string& format, dog_vendor_code_t vendorCode, std::string& info);


	dogStatus getTime(CDogTime& time) const;
	dogStatus getSessionInfo(const char* pszFormat, CDogInfo& info) const;
	dogStatus getSessionInfo(const std::string& format, std::string& info) const;
	static dogStatus getVersion(dog_vendor_code_t vendorCode, CDogVersion& version);
	dogStatus login(dog_vendor_code_t vendorCode, const char* pszScope = 0);
	dogStatus login(dog_vendor_code_t vendorCode, const std::string& scope );
	dogStatus logout();
	static std::string keyInfo();
	static std::string sessionInfo();
	static dogStatus update(const char* pszUpdate, CDogInfo& acknowledge);
	static dogStatus update(const char* pszUpdate, std::string& acknowledge);
	static dogStatus update(const std::string& update, std::string& acknowledge);
	static std::string updateInfo();
};



////////////////////////////////////////////////////////////////////
// CDog template helper functions.
// Provided for your convenience.
////////////////////////////////////////////////////////////////////
#if !defined(DOGCPP_NO_TEMPLATES)

////////////////////////////////////////////////////////////////////
//! Decrypts the provided data
////////////////////////////////////////////////////////////////////
template<typename _Class, typename _Type>
__inline dogStatus DogDecrypt(const _Class& dog, _Type* pData)
{
	return (NULL == pData) ?
DOG_INVALID_PARAMETER :
	dog.decrypt(reinterpret_cast<unsigned char*>(pData),
		sizeof(_Type));

}

////////////////////////////////////////////////////////////////////
//! Decrypts the data provided through a std::vector collection
////////////////////////////////////////////////////////////////////
template<typename _Class, typename _Type>
__inline dogStatus DogDecrypt(const _Class& dog,
							  std::vector<_Type>& vector)
{
	return vector.empty() ?
DOG_STATUS_OK :
	dog.decrypt(reinterpret_cast<unsigned char*>(&vector[0]),
		static_cast<dog_size_t>(vector.size() * sizeof(_Type)));
}

////////////////////////////////////////////////////////////////////
//! Encrypts the provided data
////////////////////////////////////////////////////////////////////
template<typename _Class, typename _Type>
__inline dogStatus DogEncrypt(const _Class& dog, _Type* pData)
{
	return (NULL == pData) ?
DOG_INVALID_PARAMETER :
	dog.encrypt(reinterpret_cast<unsigned char*>(pData),
		sizeof(_Type));
}

////////////////////////////////////////////////////////////////////
//! Encrypts the data provided through a std::vector collection
////////////////////////////////////////////////////////////////////
template<typename _Class, typename _Type>
__inline dogStatus DogEncrypt(const _Class& dog,
							  std::vector<_Type>& vector)
{
	return vector.empty() ?
DOG_STATUS_OK :
	dog.encrypt(reinterpret_cast<unsigned char*>(&vector[0]),
		static_cast<dog_size_t>(vector.size() * sizeof(_Type)));
}

#endif //! DOGCPP_NO_TEMPLATES


////////////////////////////////////////////////////////////////////
// class CDogFile
////////////////////////////////////////////////////////////////////
class DOGCPP_DECL CDogFile : public CDogBase
{
	// Construction/Destruction
public:
	CDogFile();
	CDogFile(const CDogFile& other);
	CDogFile(dog_fileid_t fileId, const CDog& other);
	virtual ~CDogFile();

	// Attributes
public:
	enum
	{
		fileReadWrite                       = DOG_FILEID_RW
	};

protected:
	dog_fileid_t                           m_fileId;
	dog_size_t                             m_ulFilePos;

	// Operators
public:
	CDogFile& operator=(const CDogFile& other);

	// Overrides
public:
	virtual dog_u32_t hashCode() const;
	virtual std::string toString() const;

protected:
	virtual bool release();

	// Implementation
public:
	static bool canWriteString(const char* szString);
	static bool canWriteString(const std::string& string);
	dog_fileid_t getFileId() const;
	dog_size_t getFilePos() const;
	static dog_size_t getFilePosFromString(const char* szString);
	static dog_size_t getFilePosFromString(const std::string& string);
	dogStatus getFileSize(dog_size_t& ulSize) const;
	static unsigned char maxStringLength();
	dogStatus read(unsigned char* pData, dog_size_t ulSize) const;
	dogStatus read(std::string& string) const;
	bool setFilePos(dog_size_t ulPos);
	dogStatus write(const unsigned char* pData, dog_size_t ulCount) const;
	dogStatus write(const char* szData) const;
	dogStatus write(const std::string& string) const;

protected:
	void init(dog_fileid_t fileId = 0);
};


////////////////////////////////////////////////////////////////////
// class CDogFile template helper functions.
// Provided for your convenience.
////////////////////////////////////////////////////////////////////
#if !defined(DOGCPP_NO_TEMPLATES)

////////////////////////////////////////////////////////////////////
//! Reads data from the current position.
////////////////////////////////////////////////////////////////////
template<typename _Type>
__inline dogStatus DogRead(const CDogFile& file, _Type& data)
{
	return file.read(reinterpret_cast<unsigned char*>(&data),
		sizeof(_Type));
}

////////////////////////////////////////////////////////////////////
//! Reads data from the current position and appends it to the
//! vector referenced by \c it.
//!
//! \param it                   The output iterator of the vector.
//! \param ulCount              The number of element to be read.
//!
//! \return                     A \a dogStatus status code.
////////////////////////////////////////////////////////////////////
template<typename _Container>
__inline dogStatus DogRead(const CDogFile& file,
						   std::back_insert_iterator<_Container> iter,
						   dog_size_t ulCount)
{
	if (0 == ulCount)
		return DOG_STATUS_OK;

	std::vector<typename _Container::value_type> vector(ulCount, 0);

	dogStatus status =
		file.read(reinterpret_cast<unsigned char*>(&vector[0]),
		static_cast<dog_size_t>(vector.size() * sizeof(typename _Container::value_type)));

	// if succeeded append values to
	// the end of the vector.
	if (DOG_SUCCEEDED(status))
		std::copy(vector.begin(), vector.end(), iter);

	return status;
}


////////////////////////////////////////////////////////////////////
//! Writes the data into the key.
////////////////////////////////////////////////////////////////////
template<typename _Type>
__inline dogStatus DogWrite(const CDogFile& file, const _Type& data)
{
	return file.write(reinterpret_cast<const unsigned char*>(&data),
		sizeof(_Type));
}

////////////////////////////////////////////////////////////////////
//! Writes the data into the key.
//!
//! \return                     A \a dogStatus status code.
////////////////////////////////////////////////////////////////////
template<typename _Type, typename _Iter>
__inline dogStatus DogWrite(const CDogFile& file,
							_Iter first,
							_Iter last)
{
	std::vector<_Type> vector;
	std::copy(first, last, std::back_inserter(vector));

	return file.write(reinterpret_cast<const unsigned char*>(&vector[0]),
		static_cast<dog_size_t>(vector.size() * sizeof(_Type)));
}

#endif // !DOG_NO_TEMPLATES

#endif // !__DOG_API_CPP_H__
