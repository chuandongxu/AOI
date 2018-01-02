////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 SafeNet, Inc. All rights reserved.
//
// Dog(R) is a registered trademark of SafeNet, Inc. 
//
//
////////////////////////////////////////////////////////////////////
#include "dog_api_cpp_.h"

////////////////////////////////////////////////////////////////////
//! \class CDogBase dog_api_cpp.h
//! \brief Pure virtual base class for all other public key classes.
//!
//! The CDogBase class implements the mechanism for accessing a
//! private CDog class providing the methods for accessing a
//! SuperDog CDog key.
//! CDogBase and all derived classes are only wrappers and pass
//! their calls to this private object.
//! To maintain thread safety this is done via a mapping class,
//! which provides methods for locking the access to the private key 
//! object. The private class itself hold a reference counter and
//! only goes out of scope when the last reference to it is gone
//! out of scope too.
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Construction/Destruction
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//! Constructs an empty object to which an existing key can
//! be attached later.
//! \sa                         operator=
//! \sa                         isValid
////////////////////////////////////////////////////////////////////
CDogBase::CDogBase()
{
    // Ensure that the CDogMap singleton is constructed before any other possible static CDogBase object
    // And if it's constructed before, it's also destroyed after.
    CDogMap::map(); 
}

////////////////////////////////////////////////////////////////////
//! Copy constructor. Adds a reference to \a other's private
//! key.
//!
//! \sa                         isValid
////////////////////////////////////////////////////////////////////
CDogBase::CDogBase(const CDogBase& other)
{
    *this = other;

    // Ensure that the CDogMap singleton is constructed before any other possible static CDogBase object
    // And if it's constructed before, it's also destroyed after.
    CDogMap::map();     
}

////////////////////////////////////////////////////////////////////
//! Constructs a new object and links it to a private key.
////////////////////////////////////////////////////////////////////
CDogBase::CDogBase(dog_feature_t feature)
{
    DIAG_VERIFY(construct(feature));
    DIAG_ASSERT(isValid());
    
    // Ensure that the CDogMap singleton is constructed before any other possible static CDogBase object
    // And if it's constructed before, it's also destroyed after.
    CDogMap::map();     
}

CDogBase::~CDogBase()
{
    DIAG_VERIFY(release());
}

////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//! Overloaded assignment operator.
////////////////////////////////////////////////////////////////////
CDogBase& CDogBase::operator=(const CDogBase& other)
{
    if (this != &other)
        addRef(other);

    return *this;
}

////////////////////////////////////////////////////////////////////
//! Overloaded equal operator.
//!
//! \return                     \a true if the object points to the
//!                             same private key object as \a other,
//!                             \a false otherwise.
////////////////////////////////////////////////////////////////////
bool CDogBase::operator==(const CDogBase& other) const
{
    // lock access
    CDogMap map;
 
    return (this == &other) ||
           (isValid() && 
            other.isValid() &&
            (other.handle() == m_handle));
}

////////////////////////////////////////////////////////////////////
//! Overloaded not equal operator.
//!
//! \sa                         operator==
////////////////////////////////////////////////////////////////////
bool CDogBase::operator!=(const CDogBase& other) const
{
    return !(*this == other);
}

////////////////////////////////////////////////////////////////////
//! Adds a reference to the private key via the map and links 
//! this object to the private key referenced by \a other.
//!
//! \param other                Reference to another CDogBase.
//!
//! \return                     \a true on success, \a false
//!                             otherwise.
//!
//! \sa                         CDogBase::construct
//!                             CDogBase::release
////////////////////////////////////////////////////////////////////
bool CDogBase::addRef(const CDogBase& other)
{
    // lock access
    CDogMap map;

    // release attached key if present
    if (!release())
    {
        DIAG_ASSERT(!"Release failed");
        return false;
    }

    DIAG_ASSERT(m_handle.isNull());
    m_handle.clear();

    if (!other.isValid())
    {
        DIAG_ASSERT(!"Other not valid");
        return false;
    }

    CDogHandle handle = other.handle();

    // add a reference
    if (!map.addRef(handle))
    {
        DIAG_ASSERT(!"addRef failed");
        return false;
    }

    m_handle = handle;
    return true;
}

////////////////////////////////////////////////////////////////////
//! Creates a new private key and initializes it with \a feature.
//!
//! \return                     \a true on success, \a false
//!                             otherwhise.
//!
//! \sa                         CDogBase::addRef
//!                             CDogBase::release
////////////////////////////////////////////////////////////////////
bool CDogBase::construct(dog_feature_t feature)
{
    if (!release())
    {
        DIAG_ASSERT(!"release failed");
        return false;
    }

    return CDogMap::createKey(feature, m_handle);
}

////////////////////////////////////////////////////////////////////
//! Detaches the private key from the object by releasing it.
//! Use this method to detach the object from a key.
//!
//! \return                     \a true on success, \a false 
//!                             otherwhise.
////////////////////////////////////////////////////////////////////
bool CDogBase::dispose()
{
    if (!isValid())
        return true;

    bool bResult = release();
    DIAG_ASSERT(m_handle.isNull());

    return bResult;
}

////////////////////////////////////////////////////////////////////
//! Returns the handle.
////////////////////////////////////////////////////////////////////
CDogHandle CDogBase::handle() const
{
    return m_handle;
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
dog_u32_t CDogBase::hashCode() const
{
    CDogMap map;

    if (!isValid()) 
        return static_cast<dog_u32_t>(-1);
    
    CDogImpl* pKey = map.getKey(m_handle);
    DIAG_ASSERT(NULL != pKey);
    
    return (NULL == pKey) ?  
            static_cast<dog_u32_t>(-1) :
            pKey->handle();
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
bool CDogBase::isKindOf(const std::type_info& info) const
{
    return (0 != (typeid(*this) == info));
}

////////////////////////////////////////////////////////////////////
//! Used to determine if the object is logged in a SuperDog CDog key.
//!
//! \return                     \a true if logged in, \a false
//!                             otherwise.
////////////////////////////////////////////////////////////////////
bool CDogBase::isLoggedIn() const
{
    CDogMap map;

    if (!isValid())
        return false;
    
    CDogImpl* pKey = map.getKey(m_handle);
    DIAG_ASSERT(NULL != pKey);
    
    return (NULL != pKey) && pKey->isLoggedIn();
}

////////////////////////////////////////////////////////////////////
//! Checks whether the object is valid, means it is attached to
//! to a private key.
//!
//! \return                     \a true if a SuperDog CDog key can be
//!                             accessed via this object, \a false
//!                             otherwise.
////////////////////////////////////////////////////////////////////
bool CDogBase::isValid() const
{
    if (m_handle.isNull())
        return false;

    CDogMap map;
    CDogImpl* pKey = map.getKey(m_handle);

    if (NULL == pKey)
        return false;

    synchronize();

    return !pKey->isAltered(m_handle);
}

////////////////////////////////////////////////////////////////////
//! Releases the private key this instance points to and 
//! invalidates the object.
//!
//! \return                     \a true on success, \a false
//!                             otherwhise.
////////////////////////////////////////////////////////////////////
bool CDogBase::release()
{
    return (m_handle.isNull() || CDogMap::release(m_handle));
}

////////////////////////////////////////////////////////////////////
//! Called before any call to the private class' methods
//! for synchronizing states etc.
//!
//! \note                       This implementation does nothing
//!                             by default. Derived classes must
//!                             implement it to stay in sync. with
//!                             the private class when needed.
////////////////////////////////////////////////////////////////////
void CDogBase::synchronize() const
{
    // this function intentionally left blank
}
