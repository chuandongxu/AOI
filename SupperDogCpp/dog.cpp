////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 SafeNet, Inc. All rights reserved.
//
// Dog(R) is a registered trademark of SafeNet, Inc. 
//
//
////////////////////////////////////////////////////////////////////
#include "dog_api_cpp_.h"


////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Construction/Destruction
////////////////////////////////////////////////////////////////////

CDog::CDog()
{
}

////////////////////////////////////////////////////////////////////
//! Copy constructor.
////////////////////////////////////////////////////////////////////
CDog::CDog(const CDog& other)
    : CDogBase(other)
{
}

////////////////////////////////////////////////////////////////////
//! Copy constructor.
////////////////////////////////////////////////////////////////////
CDog::CDog(const CDogBase& other)
    : CDogBase(other)
{
}

////////////////////////////////////////////////////////////////////
//! Constructs a new object and links it to a private key.
////////////////////////////////////////////////////////////////////
CDog::CDog(const CDogFeature& feature)
    : CDogBase(feature.feature())
{
}

////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Assignment operator
////////////////////////////////////////////////////////////////////
CDog& CDog::operator=(const CDogBase& other)
{
    dynamic_cast<CDogBase&>(*this) = other;
    return *this;
}

////////////////////////////////////////////////////////////////////
// 
////////////////////////////////////////////////////////////////////
CDog& CDog::operator=(const CDog& other)
{
    dynamic_cast<CDogBase&>(*this) = other;
    return *this;
}

////////////////////////////////////////////////////////////////////
//! Decrypts the specified data using an AES key.
//!
//! \param pData                Pointer to the first byte to be
//!                             decrypted.
//! \param ulSize               The size of the buffer to be
//!                             decrypted.
//!
//! \return                     A \a dogStatus status code.
//!
//! \sa                         CDogImpl::decrypt
////////////////////////////////////////////////////////////////////
dogStatus CDog::decrypt(unsigned char* pData, 
                          dog_size_t ulSize) const
{
    DOG_PROLOGUE(m_handle);
    return DOG_KEYPTR->decrypt(pData, ulSize);
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
dogStatus CDog::decrypt(const char*) const
{
    return DOG_INVALID_PARAMETER;
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
dogStatus CDog::decrypt(std::string& data) const
{
    std::vector<unsigned char> vector;
    CDogBase64::decode(data, vector);

    dogStatus nStatus = decrypt(&vector[0], 
                                 static_cast<dog_size_t>(vector.size()));
    if (DOG_STATUS_OK != nStatus)
        return nStatus;

    data.resize(0);
    for (std::vector<unsigned char>::const_iterator it = vector.begin();
         (vector.end() != it) && (0 != *it);
         it++)
        data += *it;

    return DOG_STATUS_OK;
}

////////////////////////////////////////////////////////////////////
//! Encrypts the specified data using an AES key.
//!
//! \param pData                Pointer to the first byte to be
//!                             encrypted.
//! \param ulSize               The size of the buffer to be
//!                             encrypted.
//!
//! \return                     A \a dogStatus status code.
//!
//! \sa                         CDogImpl::encrypt
////////////////////////////////////////////////////////////////////
dogStatus CDog::encrypt(unsigned char* pData, 
                          dog_size_t ulSize) const
{
    DOG_PROLOGUE(m_handle);
    return DOG_KEYPTR->encrypt(pData, ulSize);
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
dogStatus CDog::encrypt(const char*) const
{
    return DOG_INVALID_PARAMETER;
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
dogStatus CDog::encrypt(std::string& data) const
{
    std::vector<unsigned char> vector;
    std::copy(data.begin(), data.end(), std::back_inserter(vector));
    if (DOG_MIN_BLOCK_SIZE > vector.size())
        vector.resize(DOG_MIN_BLOCK_SIZE, 0);

    dogStatus nStatus= encrypt(&vector[0], 
                                static_cast<dog_size_t>(vector.size()));
    if (DOG_STATUS_OK != nStatus)
        return nStatus;

    CDogBase64::encode(vector, data);
    return DOG_STATUS_OK;
}

////////////////////////////////////////////////////////////////////
//! Returns the feature identifier for the this CDog object.
//!
//! \return                     A CDogFeature instance.
//!
//! \sa                         CDogImpl::feature
////////////////////////////////////////////////////////////////////
CDogFeature CDog::feature() const
{
    DIAG_ASSERT(isValid());

    // lock access
    CDogMap map;
    CDogImpl* pKey = map.getKey(m_handle);

    DIAG_ASSERT(NULL != pKey);
    if (NULL != pKey)
        return CDogFeature(pKey->feature());
    
    return CDogFeature::defaultFeature();
}

////////////////////////////////////////////////////////////////////
//! Returns an instance of the CDogFile class. The instance
//! references the same key as the instance of the CDog class.
//!
//! \return                     An instance of the CDogFile class.
////////////////////////////////////////////////////////////////////
CDogFile CDog::getFile() const
{
    return getFile(CDogFile::fileReadWrite);
}

////////////////////////////////////////////////////////////////////
//! Returns an instance of the CDogFile class. The instance
//! references the same key as the instance of the CDog class.
//!
//! \return                     An instance of the CDogFile class.
////////////////////////////////////////////////////////////////////
CDogFile CDog::getFile(dog_fileid_t fileId) const
{
    // lock access
    CDogMap map;

    DIAG_ASSERT(isValid());

    CDogFile file(fileId, (isValid() ? *this : CDog()));
    DIAG_ASSERT(isValid() ? file.isValid() : true);

    return file;
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
dogStatus CDog::getInfo(const char* pszQuery, 
                          const char* pszFormat, 
                          dog_vendor_code_t vendorCode, 
                          std::string& info)
{
    info.resize(0);
    CDogInfo _info;

    dogStatus status = CDogImpl::getInfo(pszQuery, 
                                           pszFormat, 
                                           vendorCode,
                                           _info);

    if (DOG_SUCCEEDED(status) && (NULL != _info.getInfo()))
        info = _info;

    return status; 
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
dogStatus CDog::getInfo(const char* pszQuery, 
                          const std::string& format, 
                          dog_vendor_code_t vendorCode, 
                          std::string& info)
{
    return CDog::getInfo(pszQuery,
                          format.c_str(),
                          vendorCode,
                          info);
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
dogStatus CDog::getInfo(const std::string& query, 
                          const char* pszFormat, 
                          dog_vendor_code_t vendorCode, 
                          std::string& info)
{
    return CDog::getInfo(query.c_str(),
                          pszFormat,
                          vendorCode,
                          info);
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
dogStatus CDog::getInfo(const std::string& query, 
                          const std::string& format, 
                          dog_vendor_code_t vendorCode, 
                          std::string& info)
{
    return CDog::getInfo(query.c_str(),
                          format.c_str(),
                          vendorCode,
                          info);
}


////////////////////////////////////////////////////////////////////
//! Returns the date and time of the CDog's time.
//!
//! \param time                 A reference to a CDogTime 
//!                             receiving the CDog's time.
//!
//! \return                     A \a dogStatus status code.
//!
//! \sa                         Dogimpl::getTime
////////////////////////////////////////////////////////////////////
dogStatus CDog::getTime(CDogTime& time) const
{
    DOG_PROLOGUE(m_handle);
    
    dog_time_t tc = 0;
    dogStatus status = DOG_KEYPTR->getTime(tc);

    if (DOG_SUCCEEDED(status) || DOG_TIME_ERR == status)
    {
        CDogTime _time(tc);
        time = _time;
    }

    return status;
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
dogStatus CDog::getSessionInfo(const char* pszFormat, 
                                 CDogInfo& info) const
{
    DOG_PROLOGUE(m_handle);
    return DOG_KEYPTR->getSessionInfo(pszFormat, info);
}

////////////////////////////////////////////////////////////////////
//! Returns information about the session.
////////////////////////////////////////////////////////////////////
dogStatus CDog::getSessionInfo(const std::string& format, 
                                 std::string& info) const
{ 
    info.resize(0);
    CDogInfo _info;
    
    dogStatus status = getSessionInfo(format.c_str(), _info);
    if (DOG_SUCCEEDED(status) && (NULL != _info.getInfo()))
        info = _info;
    
    return status; 
}

////////////////////////////////////////////////////////////////////
//! Returns the dog version
////////////////////////////////////////////////////////////////////
dogStatus CDog::getVersion(dog_vendor_code_t vendorCode,
                             CDogVersion& version)
{
    return CDogImpl::getVersion(vendorCode, version);
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
std::string CDog::keyInfo()
{ 
    return std::string(DOG_KEYINFO); 
}

////////////////////////////////////////////////////////////////////
//! Performs a login to SuperDog
//!
//! \param  scope               scope parameter
//! \param  vendorCode          Pointer to the vendor code.
//!
//! \return                     A \a dogStatus status code.
//!
//! \sa                         CDogImpl::loginScope
////////////////////////////////////////////////////////////////////
dogStatus CDog::login(dog_vendor_code_t vendorCode, 
                        const std::string& scope)
{
    return login(vendorCode, scope.c_str());
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
dogStatus CDog::login(dog_vendor_code_t vendorCode, 
                        const char* pszScope /* = 0*/)
{
    DOG_PROLOGUE(m_handle);
    return DOG_KEYPTR->login(vendorCode, pszScope);
}

////////////////////////////////////////////////////////////////////
//! Performs a logout from SuperDog.
//!
//! \return                     A \a dogStatus status code.
//!
//! \sa                         CDogImpl::logout
////////////////////////////////////////////////////////////////////
dogStatus CDog::logout()
{
    DOG_PROLOGUE(m_handle);
    dogStatus status = DOG_KEYPTR->logout();

    // keep the logout counter current
    synchronize();
    return status;
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
std::string CDog::sessionInfo()
{ 
    return std::string(DOG_SESSIONINFO); 
}

////////////////////////////////////////////////////////////////////
//! Synchronizes the logout counter with the private class' logout
//! counter. Usually called by isValid
////////////////////////////////////////////////////////////////////
void CDog::synchronize() const
{
    CDogMap map;
    CDogImpl* pKey = map.getKey(m_handle);

    DIAG_ASSERT(NULL != pKey);
    if (NULL != pKey)
    {
        // keep the logout counter current
        CDogHandle& handle = const_cast<CDogHandle&>(m_handle);
        handle.m_ulAltered = pKey->altered();
    }
}

////////////////////////////////////////////////////////////////////
//! Returns the login handle as a human readable string.
//!
//! \sa                         CDogImpl::toString
////////////////////////////////////////////////////////////////////
std::string CDog::toString() const
{
    CDogMap map;

    DIAG_ASSERT(isValid());
    if (!isValid())
        return std::string();

    CDogImpl* pKey = map.getKey(m_handle);
    DIAG_ASSERT(NULL != pKey);

    return (NULL == pKey) ? std::string() : pKey->toString();
}

////////////////////////////////////////////////////////////////////
//! Performs SuperDog update operation.
////////////////////////////////////////////////////////////////////
dogStatus CDog::update(const char* pszUpdate, 
                         CDogInfo& acknowledge)
{
    return CDogImpl::update(pszUpdate, acknowledge);
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
dogStatus CDog::update(const char* pszUpdate, 
                         std::string& acknowledge)
{
    acknowledge.resize(0);
    CDogInfo ack;
    
    dogStatus status = CDog::update(pszUpdate, ack);
    if (DOG_SUCCEEDED(status) && (NULL != ack.getInfo()))
        acknowledge = ack;

    return status; 
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
dogStatus CDog::update(const std::string& update, 
                         std::string& acknowledge)
{ 
    return CDog::update(update.c_str(), acknowledge);
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
std::string CDog::updateInfo()
{ 
    return std::string(DOG_UPDATEINFO); 
}

 
