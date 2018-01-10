#include "DataUtils.h"
#include <fstream>
#include <sstream>

DataUtils::DataUtils()
{
}

DataUtils::~DataUtils()
{
}

/*static*/ std::vector<std::string> DataUtils::splitString ( const std::string &s, char delim )
{
    std::vector<std::string> elems;
    std::stringstream ss ( s );
    std::string strItem;
    while( std::getline ( ss, strItem, delim ) ) {
        elems.push_back ( strItem );
    }
    return elems;
}

/*static*/ int DataUtils::parseCAD(const std::string &strFilePath, const std::vector<CAD_DATA_COLUMNS> &vecColumns, VectorOfCadData &vecCadData, std::string &strErrorMsg)
{
    vecCadData.clear();
    std::ifstream fs(strFilePath, std::ifstream::in);
    if ( ! fs.is_open() ) {
        strErrorMsg = "Failed to open file " + strFilePath;
        return -1;
    }
    std::string strLine;
    int nLineNumber = 1;
    while ( std::getline ( fs, strLine, '\r' ) ) {
        auto vecData = splitString ( strLine, '\t' );
        if ( vecData.size() != vecColumns.size() ) {
            strErrorMsg = "Line " + std::to_string ( nLineNumber) + " data columns is not following the input data columns.";
            return -1;
        }
        CadData cadData;
        for ( size_t index = 0; index < vecColumns.size(); ++ index ) {
            auto column = vecColumns[index];
            switch( column )
            {
            case CAD_DATA_COLUMNS::NAME:
                cadData.name = vecData[index];
                break;
            case CAD_DATA_COLUMNS::X:
                cadData.x = std::atof ( vecData[index].c_str() );
                break;
            case CAD_DATA_COLUMNS::Y:
                cadData.y = std::atof ( vecData[index].c_str() );
                break;
            case CAD_DATA_COLUMNS::ANGLE:
                cadData.angle = std::atof ( vecData[index].c_str () );
                break;
            case CAD_DATA_COLUMNS::TOP_BOTTOM:
                cadData.bBottom = vecData[index] == "B";
                break;
            case CAD_DATA_COLUMNS::TYPE:
                cadData.type = vecData[index];
                break;
            case CAD_DATA_COLUMNS::GROUP:
                cadData.group = vecData[index];
                break;
            case CAD_DATA_COLUMNS::WIDTH:
                cadData.width = std::atof ( vecData[index].c_str() );
                break;
            case CAD_DATA_COLUMNS::LENGTH:
                cadData.length = std::atof ( vecData[index].c_str() );
                break;
            case CAD_DATA_COLUMNS::PLACEMENT:
                cadData.placement = vecData[index] == "Placement";
                break;
            case CAD_DATA_COLUMNS::BOARD_NO:
                cadData.boardNo = std::atoi ( vecData[index].c_str () );
                break;
            default:
                break;
            }            
        }
        vecCadData.push_back ( cadData );
        ++ nLineNumber;
    }
    fs.close();
    return 0;
}

/*static*/ int DataUtils::readPackageSize(const std::string &strFilePath, MapGroupPackageSize &mapGroupPackageSize, std::string &strErrorMsg) {
    std::ifstream fs(strFilePath, std::ifstream::in);
    if ( ! fs.is_open() ) {
        strErrorMsg = "Failed to open file " + strFilePath;
        return -1;
    }
    std::string strLine;
    std::getline ( fs, strLine, '\n' ); //The first line is the data column name.
    int nLineNumber = 1;
    while ( std::getline ( fs, strLine, '\n' ) ) {
        auto vecData = splitString ( strLine, ',' );
        if ( vecData.size() != PackageSizeDataColumnCount ) {
            strErrorMsg = "Line " + std::to_string ( nLineNumber) + " data columns is not correct.";
            return -1;
        }
        std::string strGroup = vecData[0];
        std::string strType = vecData[1];
        float fLength = std::atof ( vecData[2].c_str() );
        float fWidth = std::atof ( vecData[3].c_str() );
        mapGroupPackageSize[strGroup][strType] = PackageSize{fLength, fWidth};
        ++ nLineNumber;
    }
    return 0;
}

/*static*/ int DataUtils::findDigitString(const std::string &strInput, std::string &strOutput, int nStartPos ) {
    auto posStart = strInput.find_first_of(DigitNumber, nStartPos);
    auto posEnd = strInput.find_first_not_of(DigitNumber, posStart);
    if (posEnd > posStart) {
        strOutput = strInput.substr(posStart, posEnd - posStart);
        return 0;
    }
    strOutput.assign("");
    return -1;
}

/*static*/ int DataUtils::decodePackageAndType(const std::string &strPackageType, const std::string &strInputGroup, std::string &strOutputPackage, std::string &strOutputType) {
    if ( strPackageType.size() < 4 )
        return -1;

    if ( strInputGroup == "R" || strInputGroup == "C" ) {
        strOutputType = "RC";
        return findDigitString ( strPackageType, strOutputType );
    }

    //D for Diode.
    if ( strInputGroup == "D" ) {
        auto posStartNumber = strPackageType.find_first_of ( DigitNumber );
        const std::string strLocalType = strPackageType.substr( posStartNumber );
        if ( strLocalType == "D" ) {
            strOutputType = "RC";
            return findDigitString ( strPackageType, strOutputType );
        } 
    }

    std::string strHead = strPackageType.substr(3);
    if ( strHead == "SOT" )
        return findDigitString ( strPackageType, strOutputType, 3 );

    return -1;
}