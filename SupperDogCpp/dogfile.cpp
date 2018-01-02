////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 SafeNet, Inc. All rights reserved.
//
// Dog(R) is a registered trademark of SafeNet, Inc.  
//
//
////////////////////////////////////////////////////////////////////
#include "dog_api_cpp_.h"


////////////////////////////////////////////////////////////////////
//! \class CDogFile dog_api_cpp.h
//! \brief CDogFile enables file operations on SuperDog.
//!
/////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Construction/Destruction
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//! Initializes the object.
////////////////////////////////////////////////////////////////////
CDogFile::CDogFile()
{
}

////////////////////////////////////////////////////////////////////
//! Initializes the object to use the file \a fileId of the SuperDog
//! specified by \a other.
////////////////////////////////////////////////////////////////////
CDogFile::CDogFile(dog_fileid_t fileId, const CDog& other)
    : CDogBase(other)
{
    init(fileId);
}

////////////////////////////////////////////////////////////////////
//! Copy constructor.
////////////////////////////////////////////////////////////////////
CDogFile::CDogFile(const CDogFile& other)
    : CDogBase(other)
{
    init(other.getFileId());
}

CDogFile::~CDogFile()
{
    DIAG_VERIFY(release());
}

////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//! Assignment operator.
////////////////////////////////////////////////////////////////////
CDogFile& CDogFile::operator=(const CDogFile& other)
{
    if (this != &other)
    {
        dynamic_cast<CDogBase&>(*this) =
            dynamic_cast<const CDogBase&>(other);
    
        init(other.getFileId());
    }

    return *this;
}

////////////////////////////////////////////////////////////////////
//! Determines whether the string \a szString can be written
////////////////////////////////////////////////////////////////////
bool CDogFile::canWriteString(const char* szString)
{
    return (NULL == szString) ? true : 
                                canWriteString(std::string(szString));
}

////////////////////////////////////////////////////////////////////
//! Determines whether the string \a string can be written
////////////////////////////////////////////////////////////////////
bool CDogFile::canWriteString(const std::string& string)
{
    return maxStringLength() >= string.length();
}

////////////////////////////////////////////////////////////////////
//! Returns the file identifier used for read/write operations on
//! SuperDog.
////////////////////////////////////////////////////////////////////
dog_fileid_t CDogFile::getFileId() const
{
    DIAG_ASSERT(isValid());
    return m_fileId;
}

////////////////////////////////////////////////////////////////////
//! Returns the current file position.
//!
//! \sa                         setFilePos
////////////////////////////////////////////////////////////////////
dog_size_t CDogFile::getFilePos() const
{
    DIAG_ASSERT(isValid());
    return m_ulFilePos;
}

////////////////////////////////////////////////////////////////////
//! Returns the file position for the passed string.
//!
//! \sa                         setFilePos
////////////////////////////////////////////////////////////////////
dog_size_t CDogFile::getFilePosFromString(const char* szString)
{
    return getFilePosFromString(std::string((NULL == szString) ? 
                                                "" : szString));
}

////////////////////////////////////////////////////////////////////
//! Returns the file position for the passed string.
//!
//! \sa                         setFilePos
////////////////////////////////////////////////////////////////////
dog_size_t CDogFile::getFilePosFromString(const std::string& string)
{
    return canWriteString(string) ? 
                static_cast<dog_size_t>(string.length() + sizeof(unsigned char)) : 
                0;
}

////////////////////////////////////////////////////////////////////
//! Returns the size of the file.
//!
//! \param ulSize               Reference receiving the file size.
//!
//! \return                     A \a dogStatus status code.
////////////////////////////////////////////////////////////////////
dogStatus CDogFile::getFileSize(dog_size_t& ulSize) const
{
    DOG_PROLOGUE(m_handle);
    return DOG_KEYPTR->getFileSize(m_fileId, ulSize);
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
dog_u32_t CDogFile::hashCode() const
{
    dog_u32_t code = CDogBase::hashCode();
    if (static_cast<dog_u32_t>(-1) != code)
    {
        code &= 0xFFFF;
        code |= m_fileId << 0x10;
    }

    return code;
}

////////////////////////////////////////////////////////////////////
//! Initializes the object.
////////////////////////////////////////////////////////////////////
void CDogFile::init(dog_fileid_t fileId /* = fileReadWrite */)
{
    m_fileId = fileId;
    m_ulFilePos = 0;
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
unsigned char CDogFile::maxStringLength()
{
    return 0xFF;
}

////////////////////////////////////////////////////////////////////
//! Reads data from the current position into a buffer.
//!
//! \param pData                Pointer to a buffer data shall be
//!                             written to.
//! \param ulSize               The size of the buffer in bytes.
//!
//! \return                     A \a dogStatus status code.
//!
//! \sa                         CDogImpl::readFile
//!                             getFilePos
//!                             setFilePos
////////////////////////////////////////////////////////////////////
dogStatus CDogFile::read(unsigned char* pData, 
                           dog_size_t ulSize) const
{
    DOG_PROLOGUE(m_handle);
    return DOG_KEYPTR->readFile(m_fileId,
                                 m_ulFilePos,
                                 pData,
                                 ulSize);
}

////////////////////////////////////////////////////////////////////
//! Reads a std::string from the data file
////////////////////////////////////////////////////////////////////
dogStatus CDogFile::read(std::string& string) const
{
    string.resize(0);

    unsigned char length = 0;
    dogStatus status = read(&length, sizeof(unsigned char));

    if (!DOG_SUCCEEDED(status))
        return status;

    if (0 < length)
    {
    
        std::vector<unsigned char> vector(length+1);
    
        status = read(&vector[0],
                      static_cast<dog_size_t>(vector.size() * sizeof(unsigned char)));
        if (!DOG_SUCCEEDED(status))
            return status;

        std::vector<unsigned char>::iterator it = vector.begin();
        
	// Note that we increment the iterator before reading the first byte,
	// therefore the length byte is skipped and will not show up 
	// in the final string
        for (++it; vector.end() != it; it++)
            string += *it;
    }

    return DOG_STATUS_OK;
}

////////////////////////////////////////////////////////////////////
//! Releases the private key this instance points to and 
//! invalidates the object.
//!
//! \return                     \a true on success, \a false
//!                             otherwhise.
//!
//! \sa                         CDogBase::release
////////////////////////////////////////////////////////////////////
bool CDogFile::release()
{
    if (!CDogBase::release())
        return false;

    m_fileId = 0;
    m_ulFilePos = 0;

    return true;
}

////////////////////////////////////////////////////////////////////
//! Sets the file position for the next read/write operation.
//!
//! \param ulPos                The new position within the file.
//!
//! \return                     \a true on success, \a false
//!                             otherwhise.
//!
//! \sa                         getFilePos
//!                             getFileSize
////////////////////////////////////////////////////////////////////
bool CDogFile::setFilePos(dog_size_t ulPos)
{
    dog_size_t ulSize = 0;
    if (!DOG_SUCCEEDED(getFileSize(ulSize)) || (ulSize <= ulPos))
        return false;

    m_ulFilePos = ulPos;
    return true;
}

////////////////////////////////////////////////////////////////////
//! Returns the file identifier as a human readable string.
////////////////////////////////////////////////////////////////////
std::string CDogFile::toString() const
{
    CDogMap map;

    DIAG_ASSERT(isValid());
    if (!isValid())
        return std::string();

    std::ostringstream stream;
    stream << m_fileId;

    return std::string(stream.str());
}

////////////////////////////////////////////////////////////////////
//! Writes data from a buffer to the key.
//!
//! \param pData                Pointer to a buffer holding the
//!                             data.
//! \param ulCount              The number of elements to be written.
//!
//! \return                     A \a dogStatus status code.
//!
//! \sa                         CDogImpl::writeFile
//!                             getFilePos
//!                             setFilePos
////////////////////////////////////////////////////////////////////
dogStatus CDogFile::write(const unsigned char* pData, 
                            dog_size_t ulCount) const
{
    DOG_PROLOGUE(m_handle);
    return DOG_KEYPTR->writeFile(m_fileId,
                                  m_ulFilePos,
                                  pData, 
                                  ulCount);
}

////////////////////////////////////////////////////////////////////
//! Writes a string into the data file
////////////////////////////////////////////////////////////////////
dogStatus CDogFile::write(const char* szData) const
{
    return write(std::string((NULL == szData) ? "" : szData));
}

////////////////////////////////////////////////////////////////////
//! Writes a std::string into the data file
////////////////////////////////////////////////////////////////////
dogStatus CDogFile::write(const std::string& string) const
{
    if (maxStringLength() < string.length())
        return DOG_INVALID_PARAMETER;

    std::vector<unsigned char> vector;
    vector.push_back(static_cast<unsigned char>(string.length()));
    
    std::copy(string.begin(), 
              string.end(), 
              std::back_inserter(vector));

    return write(&vector[0], 
                 static_cast<dog_size_t>(vector.size()));
}
