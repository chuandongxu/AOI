#include "CommonFunc.h"
#include <fstream>

int COMMON_EXPORT ReadBinaryFile(String const &filePathname, Binary &data) {
    Binary().swap(data);
    std::ifstream fs(filePathname.c_str(), std::ios_base::in | std::ios_base::binary | std::ios::ate);

    if (!fs)
        return -1;

    int nFileSize = static_cast<int> (fs.tellg());
    fs.seekg(0, std::ios::beg);
    data.reserve(static_cast<Binary::size_type>(nFileSize));   

    for (std::ifstream::char_type c; fs.get(c);)
        data.push_back(c);

    fs.close();
    return 0;
}

int COMMON_EXPORT WriteBinaryFile(String const &filePathname, Binary const &data) {
    std::ofstream fs(filePathname.c_str(), std::ios_base::out | std::ios_base::binary);
    if (!fs)
        return -1;

    fs.write(reinterpret_cast<char const *>(&data[0]), data.size());
    fs.close();
}

String COMMON_EXPORT FormatRecordName(int nRecordId) {
    char strName[100];
    _snprintf(strName, sizeof(strName), "./Vision/Record/%03d.rcd", nRecordId);
    return strName;
}