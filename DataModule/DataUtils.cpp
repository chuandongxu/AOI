#include "DataUtils.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <math.h>
#include "../include/constants.h"

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

/*static*/ float DataUtils::toUm(float fInput, CAD_DATA_UNIT enDataUnit) {
    float fResult = 0;
    switch (enDataUnit)
    {
    case UM:
        fResult = fInput;
        break;
    case MM:
        fResult = fInput * MM_TO_UM;
        break;
    case CM:
        fResult = fInput * CM_TO_UM;
        break;
    case MIL:
        fResult = fInput * MIL_TO_UM;
        break;
    case INCH:
        fResult = fInput * INCH_TO_UM;
        break;
    default:
        break;
    }
    return fResult;
}

/*static*/ std::vector<std::string> DataUtils::splitString(const std::string &s, char delim) {
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string strItem;
    while (std::getline(ss, strItem, delim))
        elems.push_back(strItem);

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
            strErrorMsg = "Line " + std::to_string ( nLineNumber) + " data columns count " + std::to_string ( vecData.size() ) 
                + " is not following the input data columns " + std::to_string ( vecColumns.size() ) + ".";
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
                cadData.x = ToFloat ( std::atof ( vecData[index].c_str() ) ) ;
                break;
            case CAD_DATA_COLUMNS::Y:
                if ( ! isNumber ( vecData[index] ) ) {
                    strErrorMsg = "Line " + std::to_string ( nLineNumber) + " column " + std::to_string ( index ) + " map to Y, but data \"" + vecData[index] + "\" is not number.";
                    return -1;
                }
                cadData.y = ToFloat ( std::atof ( vecData[index].c_str() ) );
                break;
            case CAD_DATA_COLUMNS::ANGLE:
                if ( ! isNumber ( vecData[index] ) ) {
                    strErrorMsg = "Line " + std::to_string ( nLineNumber) + " column " + std::to_string ( index ) + " map to ANGLE, but data \"" + vecData[index] + "\" is not number.";
                    return -1;
                }
                cadData.angle = ToFloat ( std::atof ( vecData[index].c_str () ) );
                break;
            case CAD_DATA_COLUMNS::TOP_BOTTOM:
                if ( vecData[index].length() != 1 ) {
                    strErrorMsg = "Line " + std::to_string ( nLineNumber) + " column " + std::to_string ( index ) + " map to TOP_BOTTOM, but data \"" + vecData[index] + "\" is not T or B.";
                    return -1;
                }
                cadData.isBottom = vecData[index] == "B";
                break;
            case CAD_DATA_COLUMNS::TYPE:
                if ( vecData[index].length() < 2 ) {
                    strErrorMsg = "Line " + std::to_string ( nLineNumber) + " column " + std::to_string ( index ) + " map to TYPE, but data \"" + vecData[index] + "\" is not valid.";
                    return -1;
                }
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
                cadData.width = ToFloat ( std::atof ( vecData[index].c_str() ) );
                break;
            case CAD_DATA_COLUMNS::LENGTH:
                if ( ! isNumber ( vecData[index] ) ) {
                    strErrorMsg = "Line " + std::to_string ( nLineNumber) + " column " + std::to_string ( index ) + " map to LENGTH, but data \"" + vecData[index] + "\" is not number.";
                    return -1;
                }
                cadData.length = ToFloat ( std::atof ( vecData[index].c_str() ) );
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
        float fWidth  = ToFloat ( std::atof ( vecData[2].c_str() ) );
        float fLength = ToFloat ( std::atof ( vecData[3].c_str() ) );
        mapGroupPackageSize[strGroup][strType] = PackageSize{fWidth, fLength};
        ++ nLineNumber;
    }
    return 0;
}

/*static*/ int DataUtils::findDigitString(const std::string &strInput, std::string &strOutput, int nStartPos ) {
    auto posStart = strInput.find_first_of(DigitNumber, nStartPos);
    auto posEnd = strInput.find_first_not_of(DigitNumber, posStart);
    if (posEnd > posStart) {
        strOutput = strInput.substr ( posStart, posEnd - posStart );
        return 0;
    }
    strOutput.assign("");
    return -1;
}

/*static*/ int DataUtils::decodePackageAndType(const std::string &strPackageType, const std::string &strInputGroup, std::string &strOutputPackage, std::string &strOutputType) {
    if ( strPackageType.size() < 4 )
        return -1;

    if ( strInputGroup == "R" || strInputGroup == "C" ) {
        strOutputPackage = "RC";
        auto result = findDigitString ( strPackageType, strOutputType );
        if ( result == 0 && '0' == strOutputType[0] )
            strOutputType = strOutputType.substr(1);    //Remove 0 in the beginning
        return result;
    }

    //D for Diode.
    if ( strInputGroup == "D" ) {
        auto posStartNumber = strPackageType.find_first_of ( DigitNumber );
        const std::string strLocalType = strPackageType.substr ( 0, posStartNumber );
        if ( "D" == strLocalType ) {
            strOutputPackage = strLocalType;
            auto result = findDigitString ( strPackageType, strOutputType );
            if ( result == 0 && '0' == strOutputType[0] )
                strOutputType = strOutputType.substr(1);    //Remove 0 in the beginning
            return result;
        } 
    }

    std::string strHead;

    //The head type length is 2.
    strHead = strPackageType.substr(0, 2);
    if( "YC" == strHead ||
        "TQ" == strHead ||
        "VQ" == strHead ||
        "TO" == strHead ||
        "FG" == strHead ||
        "FF" == strHead )
    {
        strOutputPackage = strHead;
        return findDigitString ( strPackageType, strOutputType, 2 );
    }else
    if ( "DO" == strHead ) {
        strOutputPackage = strHead;
        auto posStart = strPackageType.find_first_of(DigitNumber, 3);
        strOutputType = strPackageType.substr ( posStart, strPackageType.length() - posStart );
        return 0;
    }

    //The head type length is 3.
    strHead = strPackageType.substr(0, 3);
    if( "SOT" == strHead ||
        "SOD" == strHead ||
        "SOL" == strHead ||
        "QFP" == strHead ||
        "QFN" == strHead ||
        "BGA" == strHead ||
        "DIP" == strHead
        )
    {
        strOutputPackage = strHead;
        return findDigitString ( strPackageType, strOutputType, 3 );
    }else
    if ( "LED" == strHead ) {
        strOutputPackage = strHead;
        auto result = findDigitString(strPackageType, strOutputType);
        if (result == 0 && '0' == strOutputType[0])
            strOutputType = strOutputType.substr(1);    //Remove 0 in the beginning
        return result;
    }

    //The head type length is 4.
    strHead = strPackageType.substr(0, 4);
    if( "SOIC" == strHead ||
        "MSOP" == strHead ||
        "SSOP" == strHead ||
        "LQFP" == strHead ||
        "TQFP" == strHead ||
        "VQFP" == strHead ||
        "MQFP" == strHead ||
        "PBGA" == strHead ||
        "FBGA" == strHead ||
        "PLCC" == strHead ||
        "TSOP" == strHead )
    {
        strOutputPackage = strHead;
        return findDigitString ( strPackageType, strOutputType, 4 );
    }
    else
    if ( "HEAD" == strHead ) {
        strOutputPackage = strHead;
        strOutputType = strPackageType.substr ( 5, strPackageType.length() - 5 );
        return 0;
    }

    if ( strPackageType.size() < 5 )
        return -1;

    //The head type length is 5.
    strHead = strPackageType.substr(0, 5);
    if( "TSSOP" == strHead )
    {
        strOutputPackage = strHead;
        return findDigitString ( strPackageType, strOutputType, 5 );
    }else
    if ( "D-PAK" == strHead ) {
        strOutputPackage = strHead;
        strOutputType = strPackageType.substr ( 6, strPackageType.length() - 6 );
        return 0;
    }

    if ( strPackageType.size() < 6 )
        return -1;
    strHead = strPackageType.substr(0, 6);
    if ( "PCIBUS" == strHead ) {
        strOutputPackage = strHead;
        return findDigitString ( strPackageType, strOutputType, 5 );
    }
    return -1;
}

/*static*/ int DataUtils::getFrameFromCombinedImage(
    int nBigImgWidth,
    int nBigImgHeight,
    int nFrameImgWidth,
    int nFrameImgHeight,
    int nOverlapX,
    int nOverlapY,
    int nSelectPtX,
    int nSelectPtY,
    int &nFrameX,
    int &nFrameY,
    int &nPtInFrameX,
    int &nPtInFrameY,
    Vision::PR_SCAN_IMAGE_DIR enDir)
{
    if (Vision::PR_SCAN_IMAGE_DIR::RIGHT_TO_LEFT == enDir) {
        //X frame start from right to left.
        nFrameX = (nBigImgWidth - nSelectPtX) / (nFrameImgWidth - nOverlapX);
        nFrameY = nSelectPtY / (nFrameImgHeight - nOverlapY);
        nPtInFrameX = nFrameImgWidth - ((nBigImgWidth - nSelectPtX) - nFrameX * (nFrameImgWidth - nOverlapX));
        nPtInFrameY = nSelectPtY - nFrameY * (nFrameImgHeight - nOverlapY);
    }
    else {
        if (nSelectPtX < nFrameImgWidth)
            nFrameX = 0;
        else
            nFrameX = 1 + (nSelectPtX - nFrameImgWidth)  / (nFrameImgWidth  - nOverlapX);

        if (nSelectPtY < nFrameImgHeight)
            nFrameY = 0;
        else
            nFrameY = 1 + (nSelectPtY - nFrameImgHeight) / (nFrameImgHeight - nOverlapY);
        nPtInFrameX = nSelectPtX - nFrameX * (nFrameImgWidth - nOverlapX);
        nPtInFrameY = nSelectPtY - nFrameY * (nFrameImgHeight - nOverlapY);
    }

    return 0;
}

//Frame X sequence is from right to left.
/*static*/ int DataUtils::getCombinedImagePosFromFramePos(
        int nBigImgWidth,
        int nBigImgHeight,
        int nFrameImgWidth,
        int nFrameImgHeight,
        int nOverlapX,
        int nOverlapY,
        int nFrameX,
        int nFrameY,
        int nPtInFrameX,
        int nPtInFrameY,
        int &nCombinedImgPtX,
        int &nCombinedImgPtY,
        Vision::PR_SCAN_IMAGE_DIR enDir)
{
    if ( Vision::PR_SCAN_IMAGE_DIR::RIGHT_TO_LEFT == enDir ) {
        nCombinedImgPtX = nBigImgWidth - ( nFrameX * ( nFrameImgWidth - nOverlapX ) + ( nFrameImgWidth- nPtInFrameX ) );
        nCombinedImgPtY = nPtInFrameY + nFrameY * ( nFrameImgHeight - nOverlapY );
    }else {
        nCombinedImgPtX = nPtInFrameX + nFrameX * ( nFrameImgWidth  - nOverlapX );
        nCombinedImgPtY = nPtInFrameY + nFrameY * ( nFrameImgHeight - nOverlapY );
    }
    return 0;
}

/*****************************************
matTransform = [ cos(a) -sina(a) Tx ]
               [ sin(a)  cos(a)  Ty ]
******************************************
/*static*/ int DataUtils::alignWithTwoPoints(
        cv::Point2f  ptCadPos1,
        cv::Point2f  ptCadPos2,
        cv::Point2f  ptTargetPos1,
        cv::Point2f  ptTargetPos2,
        float       &fRotationInRadian,
        float       &Tx,
        float       &Ty,
        float       &fScale )
{
    float fLengthCad = sqrt ( ( ptCadPos1.x - ptCadPos2.x ) * ( ptCadPos1.x - ptCadPos2.x ) + ( ptCadPos1.y - ptCadPos2.y ) * ( ptCadPos1.y - ptCadPos2.y ) );
    float fLengthTarget = sqrt ( ( ptTargetPos1.x - ptTargetPos2.x ) * ( ptTargetPos1.x - ptTargetPos2.x ) + ( ptTargetPos1.y - ptTargetPos2.y ) * ( ptTargetPos1.y - ptTargetPos2.y ) );
    fScale = fLengthTarget / fLengthCad;

    float fAngle1 = atan2 ( ( ptTargetPos1.y - ptTargetPos2.y ), ( ptTargetPos1.x - ptTargetPos2.x ) );
    float fAngle2 = atan2 ( ( ptCadPos1.y - ptCadPos2.y ), ( ptCadPos1.x - ptCadPos2.x ) );
    fRotationInRadian = fAngle1 - fAngle2;
    fRotationInRadian = -fRotationInRadian; //Because coordinate of vision system is reversed with normal coordinate.
    Tx = ( ptTargetPos1.x + ptTargetPos2.x ) / 2.f - ( ptCadPos1.x + ptCadPos2.x ) / 2.f;
    Ty = ( ptTargetPos1.y + ptTargetPos2.y ) / 2.f - ( ptCadPos1.y + ptCadPos2.y ) / 2.f;
    return 0;
}

/*static*/ int DataUtils::assignFrames(
        float                            left,
        float                            top,
        float                            right,
        float                            bottom,
        float                            fovWidth,
        float                            fovHeight,
        Vision::VectorOfVectorOfPoint2f &vecVecFrameCtr)
{
    vecVecFrameCtr.clear();
    if (right <= left || top < bottom)
        return -1;

    int frameCountX = static_cast<int>((right - left) / fovWidth) + 1;
    int frameCountY = static_cast<int>((top - bottom) / fovHeight) + 1;
    float overlapX = 0.f, overlapY = 0.f;
    if (frameCountX > 1)
        overlapX = (frameCountX * fovWidth - (right - left)) / (frameCountX - 1);
    else
        overlapX = 0.f;
    if (frameCountY > 1)
        overlapY = (frameCountY * fovHeight - (top - bottom)) / (frameCountY - 1);
    else
        overlapY = 0.f;

    for (int row = 0; row < frameCountY; ++ row) {
        Vision::VectorOfPoint2f vecFrameCtr;
        for (int col = 0; col < frameCountX; ++ col) {
            float frameCtrX = 0.f, frameCtrY = 0.f;
            if (frameCountX > 1)
                frameCtrX = left + (col * (fovWidth - overlapX) + fovWidth / 2.f);
            else
                frameCtrX = (right + left) / 2.f;

            if (frameCountY > 1)
                frameCtrY = top - (row * (fovHeight - overlapY) + fovHeight / 2.f);
            else
                frameCtrY = (top + bottom) / 2.f;
            vecFrameCtr.emplace_back(frameCtrX, frameCtrY);
        }
        vecVecFrameCtr.push_back(vecFrameCtr);
    }
    return 0;
}

/*static*/ bool DataUtils::isWindowInFrame(
        const cv::Point2f &ptWindowCtr,
        float              winWidth,
        float              winHeight,
        const cv::Point2f &ptFrameCtr,
        float              fovWidth,
        float              fovHeight)
{
    cv::Rect2f rectFrame(ptFrameCtr.x - fovWidth / 2, ptFrameCtr.y - fovHeight / 2, fovWidth, fovHeight);
    Vision::VectorOfPoint2f vecPoints; vecPoints.reserve(4);
    vecPoints.emplace_back(ptWindowCtr.x - winWidth / 2.f, ptWindowCtr.y - winHeight / 2.f);
    vecPoints.emplace_back(ptWindowCtr.x + winWidth / 2.f, ptWindowCtr.y - winHeight / 2.f);
    vecPoints.emplace_back(ptWindowCtr.x + winWidth / 2.f, ptWindowCtr.y + winHeight / 2.f);
    vecPoints.emplace_back(ptWindowCtr.x - winWidth / 2.f, ptWindowCtr.y + winHeight / 2.f);
    for (const auto &point : vecPoints)
        if (! rectFrame.contains(point))
            return false;

    return true;
}

/*static*/ cv::Rect DataUtils::convertWindowToFrameRect(
        const cv::Point2f &ptWindowCtr,
        float              winWidth,
        float              winHeight,
        const cv::Point2f &ptFrameCtr,
        int                imageWidth,
        int                imageHeight,
        float              fResolutionX,
        float              fResolutionY
        )
{
    cv::Point2f ptRectCtr;
    ptRectCtr.x = imageWidth  / 2 + (ptWindowCtr.x - ptFrameCtr.x) / fResolutionX;
    ptRectCtr.y = imageHeight / 2 - (ptWindowCtr.y - ptFrameCtr.y) / fResolutionY; //The image Y and CAD Y positive direction is inverse.
    int winWidthPixel  = static_cast<int>(winWidth  / fResolutionX);
    int winHeightPixel = static_cast<int>(winHeight / fResolutionY);
    return cv::Rect(static_cast<int>(ptRectCtr.x) - winWidthPixel / 2, static_cast<int>(ptRectCtr.y) - winHeightPixel / 2, winWidthPixel, winHeightPixel);
}