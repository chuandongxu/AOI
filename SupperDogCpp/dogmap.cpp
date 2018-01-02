////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 SafeNet, Inc. All rights reserved.
//
// Dog(R) is a registered trademark of SafeNet, Inc. 
//
//
////////////////////////////////////////////////////////////////////
#include "dog_api_cpp_.h"


////////////////////////////////////////////////////////////////////
// struct CDogMapEntry
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Construction/Destruction
////////////////////////////////////////////////////////////////////

CDogMapEntry::CDogMapEntry()
    : m_ulCount(0),
      m_pKey(NULL)
{
}

CDogMapEntry::~CDogMapEntry()
{
}


////////////////////////////////////////////////////////////////////
// class CDogMapImpl
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Initialization
////////////////////////////////////////////////////////////////////

const dog_u32_t CDogMapImpl::m_ulInitCapacity = 0xFFFF;

////////////////////////////////////////////////////////////////////
// Construction/Destruction
////////////////////////////////////////////////////////////////////
CDogMapImpl::CDogMapImpl()
{
    DIAG_TRACE("CDogMapImpl::CDogMapImpl\n");

    DIAG_ASSERT(m_lock.isInit());

    // reserve a certain amount by default.
    m_map.reserve(m_ulInitCapacity);

    // zero index is reserved for invalid handles
    // so store a dummy entry there.
    CDogMapEntry dummy;
    m_map.push_back(dummy);
}

CDogMapImpl::~CDogMapImpl()
{
    DIAG_TRACE("CDogMapImpl::~CDogMapImpl\n");

    for (std::vector<CDogMapEntry>::iterator it = m_map.begin();
         it != m_map.end(); 
         it++)
    {
        DIAG_ASSERT(it->m_pKey == NULL);
        while (it->m_pKey)
            it->m_pKey->release();
    }
}

////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
bool CDogMapImpl::addRef(CDogHandle& handle) const
{
    DIAG_ASSERT(!handle.isNull());

    if (handle.isNull() || !lockMap())
        return false;

    bool bResult = false;
    if (m_map.size() > handle.m_ulIndex)
    {
        const CDogMapEntry& entry = m_map[handle.m_ulIndex];
        if ((entry.m_ulCount == handle.m_ulCount) &&
            (NULL != entry.m_pKey))
            bResult = entry.m_pKey->addRef(handle);
    }

    DIAG_VERIFY(unlockMap());
    return bResult;
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
bool CDogMapImpl::createKey(dog_u32_t feature, 
                             CDogHandle& handle)
{
    handle.clear();
    DIAG_ASSERT(handle.isNull());

    if (!lockMap())
        return false;

    CDogImpl* pKey = DIAG_NEW CDogImpl(feature, handle);

    for (dog_u32_t ulIndex = 1; m_map.size() > ulIndex; ulIndex++)
    {
        if (NULL == m_map[ulIndex].m_pKey)  
        {
            handle.m_ulIndex = ulIndex;
            handle.m_ulCount = m_map[ulIndex].m_ulCount;
            break;
        }
    }

    if (0 == handle.m_ulIndex)
    {
        m_map.push_back(CDogMapEntry());
        handle.m_ulIndex = static_cast<dog_u32_t>(m_map.size() - 1);
    }

    m_map[handle.m_ulIndex].m_pKey = pKey;
    
    DIAG_VERIFY(unlockMap());   
    return true;
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
dog_u32_t CDogMapImpl::findKey(const CDogImpl* pKey)
{
    dog_u32_t ulFound = 0;

    DIAG_ASSERT(NULL != pKey);
    if ((NULL == pKey) || !lockMap())
        return ulFound;

    for (dog_u32_t ulIndex = 1; m_map.size() > ulIndex; ulIndex++)
    {
        if (m_map[ulIndex].m_pKey == pKey)
        {
            ulFound = ulIndex;
            break;
        }
    }
    
    DIAG_VERIFY(unlockMap());
    return ulFound;
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
CDogImpl* CDogMapImpl::getKey(const CDogHandle& handle) const
{
    DIAG_ASSERT(!handle.isNull());

    if (handle.isNull() || (m_map.size() <= handle.m_ulIndex))
        return NULL;

    const CDogMapEntry& entry = m_map[handle.m_ulIndex];
    return (entry.m_ulCount == handle.m_ulCount) ? 
                entry.m_pKey : NULL;
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
bool CDogMapImpl::lockMap() const
{
    CDogMapImpl* pMap = const_cast<CDogMapImpl*>(this);
    return pMap->m_lock.lock();
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
bool CDogMapImpl::release(CDogHandle& handle) const
{
    DIAG_ASSERT(!handle.isNull());

    if (handle.isNull() || !lockMap())
        return false;

    bool bResult = false;

    DIAG_ASSERT(m_map.size() > handle.m_ulIndex);
    if (m_map.size() > handle.m_ulIndex)
    {
        const CDogMapEntry& entry = m_map[handle.m_ulIndex];

        if (entry.m_ulCount == handle.m_ulCount)
        {
            DIAG_ASSERT(NULL != entry.m_pKey);
            bResult = (NULL == entry.m_pKey) ? 
                        true : entry.m_pKey->release();
        }
    }
    
    if (bResult)
        handle.clear();

    DIAG_VERIFY(unlockMap());
    return bResult;
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
bool CDogMapImpl::removeKey(const CDogImpl* pKey)
{
    if (NULL == pKey)
        return true;

    for (std::vector<CDogMapEntry>::iterator it = m_map.begin();
         it != m_map.end(); 
         it++)
    {
        if (it->m_pKey == pKey)
        {
            it->m_ulCount++;
            it->m_pKey = NULL;
            return true;
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
bool CDogMapImpl::unlockMap() const
{
    CDogMapImpl* pMap = const_cast<CDogMapImpl*>(this);
    return pMap->m_lock.unlock();
}


////////////////////////////////////////////////////////////////////
// class CDogMap
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Initialization
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
// Construction/Destruction
////////////////////////////////////////////////////////////////////

CDogMap::CDogMap()
    : m_bLocked(false)
{
    m_bLocked = map().lockMap();
    DIAG_ASSERT(m_bLocked);
}

CDogMap::~CDogMap()
{
    if (m_bLocked)
        DIAG_VERIFY(map().unlockMap());
}

////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
bool CDogMap::addRef(CDogHandle& handle)
{
    return map().addRef(handle);
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
bool CDogMap::createKey(dog_u32_t feature, CDogHandle& handle)
{
    return map().createKey(feature, handle);
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
dog_u32_t CDogMap::findKey(const CDogImpl* pKey)
{
    return map().findKey(pKey);
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
CDogImpl* CDogMap::getKey(const CDogHandle& handle) const
{
    DIAG_ASSERT(m_bLocked);
    return m_bLocked ? map().getKey(handle) : NULL;
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
CDogMapImpl& CDogMap::map()
{
    static CDogMapImpl _theMap; // It's a Meyer's singleton. The constructor is called at the first call.
    return _theMap;
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
bool CDogMap::release(CDogHandle& handle)
{
    return map().release(handle);
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
bool CDogMap::removeKey(const CDogImpl* pKey)
{
    return map().removeKey(pKey);
}
