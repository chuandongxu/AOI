#ifndef _AOI_CRYPT_LIB_H_
#define _AOI_CRYPT_LIB_H_

#ifdef DLLEXPORT
    #define VisionAPI   __declspec(dllexport)
#else
    #define VisionAPI   __declspec(dllimport)
#endif

#include <string>

namespace AOI
{
namespace Crypt
{

//Return 0 means success, other wise is fail.
VisionAPI int EncryptFileNfg(const std::string &strInputFilePath, const std::string &strOutputFilePath);
VisionAPI int DecryptFileNfg(const std::string &strInputFilePath, const std::string &strOutputFilePath);
VisionAPI std::string GetErrorMsg();

}
}

#endif _AOI_CRYPT_LIB_H_