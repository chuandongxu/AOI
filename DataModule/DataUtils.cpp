#include "DataUtils.h"
#include <fstream>
#include <sstream>
#include <cctype>

DataUtils::DataUtils()
{
}

DataUtils::~DataUtils()
{
}

/*static*/ bool DataUtils::isNumber(std::string const& n)
{
    if ( n.size() <= 0 )
        return false;

    int nStartIndex = 0;
    if ( std::isdigit( n[0] ) )
        nStartIndex = 0;
    else if ( n[0] == '-' || n[0] == '+' )
        nStartIndex = 1;
    else
        return false;

    bool bWithDigit = false;
    for (std::string::size_type i = nStartIndex; i < n.size(); ++ i) {
        if ( ! std::isdigit ( n[i] ) && n[i] != '.')
            return false;
        else
            bWithDigit = true;
    }

    return bWithDigit;
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

    char chChangeLine = '\r';
    {
        char chArray[100];
        fs.read ( chArray, sizeof ( chArray ) );
        std::string strTemp(chArray);
        auto nPosOfCR = strTemp.find_first_of('\r');
        auto nPosOfLF = strTemp.find_first_of('\n');
        if ( nPosOfLF < nPosOfCR  )
            chChangeLine = '\n';
        fs.seekg (0, fs.beg);
    }
    
    int nLineNumber = 1;
    
    std::string strLine;
    while ( std::getline ( fs, strLine, chChangeLine ) ) {
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
                if ( ! isNumber ( vecData[index] ) ) {
                    strErrorMsg = "Line " + std::to_string ( nLineNumber) + " column " + std::to_string ( index ) + " map to X, but data \"" + vecData[index] + "\" is not number.";
                    return -1;
                }
                cadData.x = std::atof ( vecData[index].c_str() );
                break;
            case CAD_DATA_COLUMNS::Y:
                if ( ! isNumber ( vecData[index] ) ) {
                    strErrorMsg = "Line " + std::to_string ( nLineNumber) + " column " + std::to_string ( index ) + " map to Y, but data \"" + vecData[index] + "\" is not number.";
                    return -1;
                }
                cadData.y = std::atof ( vecData[index].c_str() );
                break;
            case CAD_DATA_COLUMNS::ANGLE:
                if ( ! isNumber ( vecData[index] ) ) {
                    strErrorMsg = "Line " + std::to_string ( nLineNumber) + " column " + std::to_string ( index ) + " map to ANGLE, but data \"" + vecData[index] + "\" is not number.";
                    return -1;
                }
                cadData.angle = std::atof ( vecData[index].c_str () );
                break;
            case CAD_DATA_COLUMNS::TOP_BOTTOM:
                cadData.isBottom = vecData[index] == "B";
                break;
            case CAD_DATA_COLUMNS::TYPE:
                cadData.type = vecData[index];
                break;
            case CAD_DATA_COLUMNS::GROUP:
                cadData.group = vecData[index];
                break;
            case CAD_DATA_COLUMNS::WIDTH:
                if ( ! isNumber ( vecData[index] ) ) {
                    strErrorMsg = "Line " + std::to_string ( nLineNumber) + " column " + std::to_string ( index ) + " map to WIDTH, but data \"" + vecData[index] + "\" is not number.";
                    return -1;
                }
                cadData.width = std::atof ( vecData[index].c_str() );
                break;
            case CAD_DATA_COLUMNS::LENGTH:
                if ( ! isNumber ( vecData[index] ) ) {
                    strErrorMsg = "Line " + std::to_string ( nLineNumber) + " column " + std::to_string ( index ) + " map to LENGTH, but data \"" + vecData[index] + "\" is not number.";
                    return -1;
                }
                cadData.length = std::atof ( vecData[index].c_str() );
                break;
            case CAD_DATA_COLUMNS::PLACEMENT:
                cadData.placement = vecData[index] == "Placement";
                break;
            case CAD_DATA_COLUMNS::BOARD_NO:
                if ( ! isNumber ( vecData[index] ) ) {
                    strErrorMsg = "Line " + std::to_string ( nLineNumber) + " column " + std::to_string ( index ) + " map to BOARD NO, but data \"" + vecData[index] + "\" is not number.";
                    return -1;
                }
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

    if ( vecCadData.empty() ) {
        strErrorMsg = "Cannot read out CAD data from the file " + strFilePath + ".";
        return -1;
    }
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