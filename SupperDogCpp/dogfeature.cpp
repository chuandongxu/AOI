////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 SafeNet, Inc. All rights reserved.
//
// Dog(R) is a registered trademark of SafeNet, Inc. 
//
//
////////////////////////////////////////////////////////////////////
#include "dog_api_cpp_.h"


////////////////////////////////////////////////////////////////////
//! \mainpage Overview
//!
//! This document describes SuperDog high layer API.
//!
//! \section __helpers__ Helper Classes
//!
//! For the sake of convenience several helper classes are 
//! implemented. These allow a more flexible access to certain
//! parameters required by the low level API.
//!
//! \subsection __DogFeature__ The CDogFeature Class
//!
//! The CDogFeature class encapsulates a feature identifier of a 
//! SuperDog feature.
//!
//! \subsection __DogTime__ The CDogTime Class
//!
//! This class encapsulates the date/time functionality provided 
//! by SuperDog.
//!
//! \subsection __DogHandle__ The CDogHandle Class
//!
//! This class is used internally by the CDogBase class. CDogHandle 
//! holds the the index into the CDog - CDogImpl map and some
//! state informations in addition. A unique usage counter of the
//! the index in the map and a logout counter is used to check if a 
//! logout occured.
//!
//! \section __key__ The Key Classes
//!
//! This section describes the classes for accessing a CDog protection
//! key. 
//! The basic concept of the implementation is to provide a private
//! class implementing all methods for accessing a CDog protection key 
//! via the low level API. The access to this class is controlled via
//! a helper mapping class maintaining a lock mechanism, which 
//! permits the access to the private class from different threads.
//! The design of the public interface is straight foreward.
//! Main concept is to provide classes, which just act as wrappers.
//! The advantage of this design is, that these classes can be
//! easily replaced (i.e. for implementing a .net assembly or a 
//! COM interface).
//! The Base of these classes is the CDogBase class.
//! The implementation of this class is pure virtual. Main purpose 
//! of it is to maintain the mapping to the private key class.
//!
//! \subsection __DogBase__ The CDogBase Class
//!
//! This is a pure virtual class. No instance shall ever directly
//! be created from this class. Purpose of this class is to provide
//! methods for creating a, attaching to and detaching from a 
//! private key object via the map object.
//!
//! \subsection __Cdog__ The CDog class
//! 
//! The CDog class represents the main public class. Through
//! this class a CDog protection key can be accessed. There
//! can be any number of instances referencing the same 
//! CDog protection key.
//!
//! In addition this class provides the methods for gaining access
//! to the CDog key's file operation class \ref CDogFile.
//!
//! \subsection __DogFile__ The CDogFile class
//!
//! The CDogFile class is used for file i/o operations on a 
//! CDog protection key. Objects of this class are instantiated via
//! the CDog class.
//!
//!
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//! \struct CDogFeature dog_api_cpp.h
//! \brief Class holding a CDog key feature id.
//!
//! The CDogFeature class holds the feature identifier of a CDog 
//! key. Furthermore methods are implemented allowing the feature
//! to be manipulated. Through the static method 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Construction/Destruction
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//! Constructs a CDogFeature object with the feature identifier
//! \a ulFeature.
////////////////////////////////////////////////////////////////////
CDogFeature::CDogFeature(dog_u32_t ulFeature)
    : m_ulFeature(ulFeature)
{
}

////////////////////////////////////////////////////////////////////
//! Destroys the object.
////////////////////////////////////////////////////////////////////
CDogFeature::~CDogFeature()
{
}

////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//!
////////////////////////////////////////////////////////////////////
CDogFeature CDogFeature::defaultFeature()
{
    return CDogFeature(DOG_DEFAULT_FID);
}

////////////////////////////////////////////////////////////////////
//! Returns the feature identifier.
////////////////////////////////////////////////////////////////////
dog_u32_t CDogFeature::feature() const
{
    return m_ulFeature;
}

////////////////////////////////////////////////////////////////////
//! Special operator enabling the CDogFeature to behave like a
//! \a dog_u32_t primitive type.
////////////////////////////////////////////////////////////////////
CDogFeature::operator dog_feature_t() const
{
    return m_ulFeature;
}

////////////////////////////////////////////////////////////////////
//! Strips the options from a classic CDog protection key feature, 
//! else does nothing.
////////////////////////////////////////////////////////////////////
dog_u32_t CDogFeature::featureId() const
{
    return m_ulFeature;
}

////////////////////////////////////////////////////////////////////
//! Creates and returns a CDogFeature instance based on a 
//! feature id
//!
//! \param ulFeature            The feature id to be used
//! \return                     A CDogFeature class 
//!                             If the feature is a program number
//!                             the default feature will be returned
////////////////////////////////////////////////////////////////////
CDogFeature CDogFeature::fromFeature(dog_u32_t ulFeature)
{
    return CDogFeature(ulFeature);
}


////////////////////////////////////////////////////////////////////
//! Returns \a true if the feature is the default feature.
////////////////////////////////////////////////////////////////////
bool CDogFeature::isDefault() const
{
    return featureId() == static_cast<dog_u32_t>(DOG_DEFAULT_FID);
}



////////////////////////////////////////////////////////////////////
//! Returns the feature identifier as a human readable string.
////////////////////////////////////////////////////////////////////
std::string CDogFeature::toString() const
{
    std::ostringstream stream;
    stream << m_ulFeature;

    return std::string(stream.str());
}
