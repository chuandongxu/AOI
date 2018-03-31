#include "Common_global.h"
#include "../lib/DataStoreAPI/include/Common/BaseTypes.h"

using namespace NFG::AOI;

int COMMON_EXPORT ReadBinaryFile(String const &filePathname, Binary &data);
int COMMON_EXPORT WriteBinaryFile(String const &filePathname, Binary const &data);
String COMMON_EXPORT FormatRecordName(int nRecordId);