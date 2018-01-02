////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 SafeNet, Inc. All rights reserved.
//
// Dog(R) is a registered trademark of SafeNet, Inc. 
//
//
////////////////////////////////////////////////////////////////////
#include "dog_api_cpp_.h"
#include <string.h>

////////////////////////////////////////////////////////////////////
//! \struct CDogTimeHelper
//! \brief Helper class for managing the time via the low level API.
////////////////////////////////////////////////////////////////////
struct CDogTimeHelper
{
// Construction/Destruction
public:
    CDogTimeHelper();
    ~CDogTimeHelper();

// Attributes
public:
    unsigned int                        m_nYear;
    unsigned int                        m_nMonth;
    unsigned int                        m_nDay;
    unsigned int                        m_nHour;
    unsigned int                        m_nMinute;
    unsigned int                        m_nSecond;

// Implementation
public:
    dog_time_t toTime() const;
    bool toTime(const dog_time_t time);
};


////////////////////////////////////////////////////////////////////
// struct CDogTimeHelper Implementation
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Construction/Destruction
////////////////////////////////////////////////////////////////////

CDogTimeHelper::CDogTimeHelper()
{
    memset(this, 0x00, sizeof(*this));
}

CDogTimeHelper::~CDogTimeHelper()
{
    memset(this, 0x00, sizeof(*this));
}

////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//! Converts the stored time to the \a dog_time_t type.
////////////////////////////////////////////////////////////////////
dog_time_t CDogTimeHelper::toTime() const
{
    dog_time_t time = 0;

    if (!DOG_SUCCEEDED(dog_datetime_to_dogtime(m_nDay,
                                                  m_nMonth,
                                                  m_nYear,
                                                  m_nHour,
                                                  m_nMinute,
                                                  m_nSecond,
                                                  &time)))
        time = 0;

    return time;
}

////////////////////////////////////////////////////////////////////
//! Converts the given time \a time into the internal structure.
////////////////////////////////////////////////////////////////////
bool CDogTimeHelper::toTime(dog_time_t time)
{
    if (!DOG_SUCCEEDED(dog_dogtime_to_datetime(time,
                                                  &m_nDay,
                                                  &m_nMonth,
                                                  &m_nYear,
                                                  &m_nHour,
                                                  &m_nMinute,
                                                  &m_nSecond)))
    {
        memset(this, 0x00, sizeof(*this));
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////////////
//! \struct CDogTime dogcpp_.h
//! \brief Class encapsulating SuperDog date and time.
//!
//! The CDogTime class encapsultes SuperDog time and provides 
//! methods for converting SuperDog data and time.
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Construction/Destruction
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//! Initializes the object with the passed \a time.
////////////////////////////////////////////////////////////////////
CDogTime::CDogTime(dog_time_t time /* = 0 */)
    : m_time(time)
{
}

////////////////////////////////////////////////////////////////////
//! Initializes the object with the passed data and time.
////////////////////////////////////////////////////////////////////
CDogTime::CDogTime(unsigned int nYear, unsigned int nMonth, 
                     unsigned int nDay, unsigned int nHour, 
                     unsigned int nMinute, unsigned int nSecond)
    : m_time(0)
{
    CDogTimeHelper helper;
    helper.m_nYear = nYear;
    helper.m_nMonth = nMonth;
    helper.m_nDay = nDay;
    helper.m_nHour = nHour;
    helper.m_nMinute = nMinute;
    helper.m_nSecond = nSecond;

    m_time = helper.toTime();
}

CDogTime::~CDogTime()
{
}

////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//! Special operator enabling the CDogTime object to behave like
//! a \a dog_time_t type.
////////////////////////////////////////////////////////////////////
CDogTime::operator dog_time_t() const
{
    return m_time;
}

////////////////////////////////////////////////////////////////////
//! Returns the day.
////////////////////////////////////////////////////////////////////
unsigned int CDogTime::day() const
{
    CDogTimeHelper helper;
    DIAG_VERIFY(helper.toTime(*this));
    
    return helper.m_nDay;
}

////////////////////////////////////////////////////////////////////
//! Returns the hour.
////////////////////////////////////////////////////////////////////
unsigned int CDogTime::hour() const
{
    CDogTimeHelper helper;
    DIAG_VERIFY(helper.toTime(*this));
    
    return helper.m_nHour;
}

////////////////////////////////////////////////////////////////////
//! Returns the minute.
////////////////////////////////////////////////////////////////////
unsigned int CDogTime::minute() const
{
    CDogTimeHelper helper;
    DIAG_VERIFY(helper.toTime(*this));
    
    return helper.m_nMinute;
}

////////////////////////////////////////////////////////////////////
//! Returns the month.
////////////////////////////////////////////////////////////////////
unsigned int CDogTime::month() const
{
    CDogTimeHelper helper;
    DIAG_VERIFY(helper.toTime(*this));
    
    return helper.m_nMonth;
}

////////////////////////////////////////////////////////////////////
//! Returns the second.
////////////////////////////////////////////////////////////////////
unsigned int CDogTime::second() const
{
    CDogTimeHelper helper;
    DIAG_VERIFY(helper.toTime(*this));
    
    return helper.m_nSecond;
}

////////////////////////////////////////////////////////////////////
//! Returns the stored time.
////////////////////////////////////////////////////////////////////
dog_time_t CDogTime::time() const
{
    return m_time;
}

////////////////////////////////////////////////////////////////////
//! Returns the year.
////////////////////////////////////////////////////////////////////
unsigned int CDogTime::year() const
{
    CDogTimeHelper helper;
    DIAG_VERIFY(helper.toTime(*this));
    
    return helper.m_nYear;
}

