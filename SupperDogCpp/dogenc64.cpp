////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 SafeNet, Inc. All rights reserved.
//
// Dog(R) is a registered trademark of SafeNet, Inc. 
//
//
////////////////////////////////////////////////////////////////////
#include "dog_api_cpp_.h"


////////////////////////////////////////////////////////////////////
// Initialization
////////////////////////////////////////////////////////////////////
const char CDogBase64::m_rAlphabet[] = 
    { 'A', 'B', 'C', 'D', 
      'E', 'F', 'G', 'H', 
      'I', 'J', 'K', 'L', 
      'M', 'N', 'O', 'P', 
      'Q', 'R', 'S', 'T', 
      'U', 'V', 'W', 'X', 
      'Y', 'Z', 'a', 'b', 
      'c', 'd', 'e', 'f', 
      'g', 'h', 'i', 'j', 
      'k', 'l', 'm', 'n', 
      'o', 'p', 'q', 'r', 
      's', 't', 'u', 'v', 
      'w', 'x', 'y', 'z', 
      '0', '1', '2', '3', 
      '4', '5', '6', '7', 
      '8', '9', '+', '/' 
    };

const char CDogBase64::m_cPad = '=';
const size_t CDogBase64::m_ulMaxLineLength = 76;


////////////////////////////////////////////////////////////////////
// Construction/Destruction
////////////////////////////////////////////////////////////////////

CDogBase64::CDogBase64()
{
}

CDogBase64::~CDogBase64()
{
}

////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// walk the source buffer
// each four character sequence is converted to 3 bytes
// CRLFs and =, and any characters not in the encoding table
// are skipped
////////////////////////////////////////////////////////////////////
void CDogBase64::decode(const std::string& source,
                         std::vector<unsigned char>& dest) throw()
{
    dest.resize(0);
    dest.reserve(( source.length() * 4) / 3);

    std::string::const_iterator srcIt = source.begin(); 
    
    while (source.end() != srcIt)
    {
        dog_u32_t ulCurr = 0;
        int nBits = 0;

        int i;
        for (i = 0; 4 > i; i++)
        {
            if (source.end() <= srcIt)
                break;

            int nChar = CDogBase64().decodeChar(*(srcIt++));
            if (0 > nChar)
            {
                i--;
                continue;
            }
            
            ulCurr <<= 6;
            ulCurr |= (nChar & 0xFF);
            nBits += 6;
        }

        // dwCurr has the 3 bytes to write to the output buffer
        // left to right
        ulCurr <<= 24 - nBits;
        for (i = 0; (nBits / 8) > i; i++, ulCurr <<= 8)
            dest.push_back(static_cast<unsigned char>((ulCurr >> 16) & 0xFF));
    }
}

////////////////////////////////////////////////////////////////////
// returns -1 if the character is invalid
// or should be skipped
// otherwise, returns the 6-bit code for the character
// from the encoding table
////////////////////////////////////////////////////////////////////
int CDogBase64::decodeChar(const int nChar) const
{
    if (('A' <= nChar) && ('Z' >= nChar))
        return nChar - 'A' + 0; // 0 range starts at 'A'
    if (('a' <= nChar) && ('z' >= nChar))
        return nChar - 'a' + 26;    // 26 range starts at 'a'
    if (('0' <= nChar) && ('9' >= nChar))
        return nChar - '0' + 52;    // 52 range starts at '0'
    if ('+' == nChar)
        return 62;
    if ('/' == nChar)
        return 63;

    return -1;
}

////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
void CDogBase64::encode(const std::vector<unsigned char>& source,
                         std::string& dest) throw()
{
    dest.resize(0);
    
    std::vector<unsigned char>::const_iterator srcIt = source.begin();
    size_t nLen1 = (source.size() / 3) *4;
    size_t nLen2 = nLen1 / m_ulMaxLineLength;
    size_t nLen3 = 19;

    dest.reserve(nLen1 + 2 * nLen2);

    for (size_t i = 0; nLen2 >= i; i++)
    {
        if (nLen2 == i)
            nLen3 = (nLen1 % m_ulMaxLineLength) / 4;

        for (size_t j = 0; nLen3 > j; j++)
        {
            dog_u32_t ulCurr = 0;
            for (int n = 0; 3 > n; n++, ulCurr <<= 8)
                ulCurr |= *(srcIt++);

            for (int k = 0; 4 > k; k++, ulCurr <<= 6)
            {
                dog_u8_t nIndex = static_cast<dog_u8_t>(ulCurr >> 26);
                dest+= m_rAlphabet[nIndex];
            }
        }

        dest += "\r\n";
    }

    if (2 <= dest.size())
        dest.resize(dest.size() - 2);

    nLen2 = (source.size() % 3) ? (source.size() % 3 + 1) : 0;
    if (nLen2)
    {
        dog_u32_t ulCurr = 0;
        for (size_t n = 0; 3 > n; n++, ulCurr <<= 8)
        {
            if ((source.size() % 3) > n)
                ulCurr |= *(srcIt++);
        }

        for (size_t k = 0; nLen2 > k; k++, ulCurr <<= 6)
        {
            dog_u8_t nIndex = static_cast<dog_u8_t>(ulCurr>>26);
            dest += m_rAlphabet[nIndex];
        }

        nLen3 = nLen2 ? (4 - nLen2) : 0;
        for (size_t j = 0; nLen3 > j; j++)
            dest += m_cPad;
    }
}
