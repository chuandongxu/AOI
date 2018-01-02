////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 SafeNet, Inc. All rights reserved.
//
// Dog(R) is a registered trademark of SafeNet, Inc. 
//
//
////////////////////////////////////////////////////////////////////
#include "dog_api_cpp_.h"


////////////////////////////////////////////////////////////////////
// Construction/Destruction
////////////////////////////////////////////////////////////////////

CDogImpl::CDogImpl(dog_feature_t feature, CDogHandle& handle)
    : m_handle(0),
      m_feature(feature),
      m_ulState(stIdle),
      m_ulRefCount(0),
      m_ulAltered(0)
{
    addRef(handle);
}

CDogImpl::~CDogImpl()
{
    DIAG_ASSERT(0 == m_ulRefCount);

    logout(true);
    CDogMap::removeKey(this);
}

////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//! Increments the internal reference counter and updates
//! the \a handle's \a CDogHandle::m_ulAltered field.
////////////////////////////////////////////////////////////////////
bool CDogImpl::addRef(CDogHandle& handle)
{
    handle.m_ulAltered = m_ulAltered;
    return (0 < ++m_ulRefCount);
}

////////////////////////////////////////////////////////////////////
//! Returns the current altered counter.
////////////////////////////////////////////////////////////////////
dog_u32_t CDogImpl::altered() const
{
    return m_ulAltered;
}

////////////////////////////////////////////////////////////////////
//! Wrapper for the \a dog_decrypt API function.
////////////////////////////////////////////////////////////////////
dogStatus CDogImpl::decrypt(unsigned char* pData, 
                              dog_size_t ulSize) const
{
    return isLoggedIn() ? 
                dog_decrypt(m_handle, pData, ulSize) : 
                DOG_INV_HND;
}

////////////////////////////////////////////////////////////////////
//! Wrapper for the \a dog_encrypt API function.
////////////////////////////////////////////////////////////////////
dogStatus CDogImpl::encrypt(unsigned char* pData, 
                              dog_size_t ulSize) const
{
    return isLoggedIn() ? 
                dog_encrypt(m_handle, pData, ulSize) : 
                DOG_INV_HND;
}

////////////////////////////////////////////////////////////////////
//! Returns the feature this object represents.
////////////////////////////////////////////////////////////////////
dog_feature_t CDogImpl::feature() const
{
    DIAG_ASSERT(0 < m_ulRefCount);

    CDogFeature feature(m_feature);
    return feature;
}

////////////////////////////////////////////////////////////////////
//! Wrapper for the \a dog_get_size API function.
////////////////////////////////////////////////////////////////////
dogStatus CDogImpl::getFileSize(dog_fileid_t fileId, 
                                  dog_size_t& ulSize) const
{
    return isLoggedIn() ?
                dog_get_size(m_handle, fileId, &ulSize) : 
                DOG_INV_HND;
}

////////////////////////////////////////////////////////////////////
//! Wrapper for the \a dog_get_rtc API function.
////////////////////////////////////////////////////////////////////
dogStatus CDogImpl::getTime(dog_time_t& time) const
{
    return isLoggedIn() ? 
                dog_get_time(m_handle, &time) : 
                DOG_INV_HND;
}

////////////////////////////////////////////////////////////////////
//! Wrapper  for the \a dog_get_sessioninfo API function.
////////////////////////////////////////////////////////////////////
dogStatus CDogImpl::getSessionInfo(const char* pszFormat, 
                                     CDogInfo& info) const
{
    if (NULL == pszFormat)
        return DOG_INVALID_PARAMETER;

    info.clear();

    return isLoggedIn() ? 
                dog_get_sessioninfo(m_handle, 
                                     const_cast<char*>(pszFormat),
                                     &info.m_pszInfo) :
                DOG_INV_HND;
}

////////////////////////////////////////////////////////////////////
//! Wrapper  for the \a dog_get_info API function.
////////////////////////////////////////////////////////////////////
dogStatus CDogImpl::getInfo(const char* pszQuery,
                              const char* pszFormat,
                              dog_vendor_code_t vendorCode,
                              CDogInfo& info)
{
    info.clear();

    return pszQuery && pszFormat ?
        dog_get_info(const_cast<char*>(pszQuery),
                      const_cast<char*>(pszFormat),
                      vendorCode, 
                      &info.m_pszInfo) :
        DOG_INVALID_PARAMETER;
}


////////////////////////////////////////////////////////////////////
//! Wrapper for the \a dog_get_version API function.
////////////////////////////////////////////////////////////////////
dogStatus CDogImpl::getVersion(dog_vendor_code_t vendorCode,
                                 CDogVersion& version)
{
    unsigned int nMajor = 0;
    unsigned int nMinor = 0;
    unsigned int nServer = 0;
    unsigned int nBuildNo = 0;
    
    dogStatus status = dog_get_version(&nMajor, 
                                         &nMinor, 
                                         &nServer, 
                                         &nBuildNo, 
                                         vendorCode);

    version = DOG_SUCCEEDED(status) ? 
        CDogVersion(nMajor, nMinor, nServer, nBuildNo) :
        CDogVersion();

    return status;
}

////////////////////////////////////////////////////////////////////
//! Returns the handle for the feature.
////////////////////////////////////////////////////////////////////
dog_handle_t CDogImpl::handle() const
{
    return isLoggedIn() ? m_handle : 0;
}

////////////////////////////////////////////////////////////////////
//! Used to check if a logout occured.
////////////////////////////////////////////////////////////////////
bool CDogImpl::isAltered(const CDogHandle& handle) const
{
    DIAG_ASSERT(0 < m_ulRefCount);

    return (0 >= m_ulRefCount) ||
           (m_ulAltered != handle.m_ulAltered);
}

////////////////////////////////////////////////////////////////////
//! Used to check if the object is in the login state.
////////////////////////////////////////////////////////////////////
bool CDogImpl::isLoggedIn() const
{
    DIAG_ASSERT(0 < m_ulRefCount);
    DIAG_ASSERT(0 < CDogMap::findKey(this));
    
    return (0 < m_ulRefCount) && (stActive == m_ulState);
}



////////////////////////////////////////////////////////////////////
//! Wrapper for the \a dog_login_scope API function.
////////////////////////////////////////////////////////////////////
dogStatus CDogImpl::login(dog_vendor_code_t vendorCode,  const char* pszScope)
{
    DIAG_ASSERT(0 < m_ulRefCount);
    if (0 >= m_ulRefCount)
        return DOG_INVALID_OBJECT;

    if (isLoggedIn())
        return DOG_ALREADY_LOGGED_IN;

	 
    dogStatus status = pszScope ? 
        dog_login_scope(m_feature, const_cast<char*>(pszScope), vendorCode, &m_handle) :
        dog_login(m_feature, vendorCode, &m_handle);

    if (DOG_SUCCEEDED(status))
        m_ulState = stActive;

    return status;
}

////////////////////////////////////////////////////////////////////
//! Wrapper for the \a dog_logout API function.
////////////////////////////////////////////////////////////////////
dogStatus CDogImpl::logout(bool bFinal /* = false */)
{
    // final call from dtor
    // Ref. counter shall be zero.
    // Perform logout when still active.
    if (bFinal)
    {
        DIAG_ASSERT(0 == m_ulRefCount);
        if (stActive == m_ulState)
        {
            m_ulState = stIdle;
            dog_logout(m_handle);
        }

        // nothing left to do.
        return DOG_STATUS_OK;
    }

    DIAG_ASSERT(0 < m_ulRefCount);
    if (0 >= m_ulRefCount)
        return DOG_INVALID_OBJECT;

    if (!isLoggedIn())
        return DOG_ALREADY_LOGGED_OUT;

    dogStatus status = dog_logout(m_handle);

    if (DOG_SUCCEEDED(status))
    {
        // alter the state and update 
        // the logout counter.
        m_ulState = stIdle;
        m_ulAltered++;
    }

    return status;
}

////////////////////////////////////////////////////////////////////
//! Releases the the object by decrementing its reference counter.
//! If the reference counter drops below one the object will be
//! destroyed.
////////////////////////////////////////////////////////////////////
bool CDogImpl::release()
{
    DIAG_ASSERT(0 < m_ulRefCount);
    if (0 == m_ulRefCount)
        return false;

    if (0 == --m_ulRefCount)
        // gone...
        delete this;

    return true;
}

////////////////////////////////////////////////////////////////////
//! Wrapper for the \a dog_read API function.
////////////////////////////////////////////////////////////////////
dogStatus CDogImpl::readFile(dog_fileid_t fileId,
                               dog_size_t ulOffset,
                               unsigned char* pData, 
                               dog_size_t ulSize) const
{
    return isLoggedIn() ?
                dog_read(m_handle, 
                         fileId, 
                         ulOffset, 
                         ulSize, 
                         pData) :
                DOG_INV_HND;
}


////////////////////////////////////////////////////////////////////
//! Returns the handle as a human readable string.
////////////////////////////////////////////////////////////////////
std::string CDogImpl::toString() const
{
    if (!isLoggedIn())
        return std::string();

    std::ostringstream stream;
    stream << m_handle;

    return std::string(stream.str());
}

////////////////////////////////////////////////////////////////////
//! Wrapper for the \a dog_write API function.
////////////////////////////////////////////////////////////////////
dogStatus CDogImpl::writeFile(dog_fileid_t fileId,
                                dog_size_t ulOffset,
                                const unsigned char* pData, 
                                dog_size_t ulSize) const
{
    if (!isLoggedIn())
        return DOG_INV_HND;

    return isLoggedIn() ?
                dog_write(m_handle, 
                           fileId, 
                           ulOffset, 
                           ulSize, 
                           const_cast<unsigned char*>(pData)) :
                DOG_INV_HND;
}

////////////////////////////////////////////////////////////////////
//! Wrapper for the \a dog_update API function.
////////////////////////////////////////////////////////////////////
dogStatus CDogImpl::update(const char* pszUpdate, 
                             CDogInfo& acknowledge)
{
    if (NULL == pszUpdate)
        return DOG_INVALID_PARAMETER;

    acknowledge.clear();

    return dog_update(const_cast<char*>(pszUpdate), 
                       &acknowledge.m_pszInfo);
}
