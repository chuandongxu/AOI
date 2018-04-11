#include "../DataModule/DataUtils.h"
#include <iostream>
#include <iomanip>

template<class T>
void printfMat ( const cv::Mat &mat, int nPrecision = 1)
{
    std::cout << std::fixed << std::setprecision ( nPrecision );
    for ( short row = 0; row < mat.rows; ++ row )
    {
        for ( short col = 0; col < mat.cols; ++ col )
        {
            std::cout << mat.at<T>(row, col) << " ";
        }
        std::cout << std::endl;
    }
}

template<typename _Tp>
static inline cv::Point2f warpPoint(const cv::Mat &matWarp, const cv::Point2f &ptInput) {
    cv::Mat matPoint = cv::Mat::zeros(3, 1, matWarp.type());
    matPoint.at<_Tp>(0, 0) = ptInput.x;
    matPoint.at<_Tp>(1, 0) = ptInput.y;
    matPoint.at<_Tp>(2, 0) = 1.f;
    cv::Mat matResultImg = matWarp * matPoint;
    return cv::Point_<_Tp>(ToFloat(matResultImg.at<_Tp>(0, 0)), ToFloat(matResultImg.at<_Tp>(1, 0)));
}

void TestIsNumber()
{
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl << "TEST DataUtils::isNumber function";
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl;

    static const std::string strArrayTestCase[]{
        "+",
        "-",
        "0",
        "3",
        "9",
        "a",
        "99a9",
        "9909",
        "",
        "-3.14",
        "+32768",
        "-32768",
    };

    for ( const auto &strTestCase : strArrayTestCase ) {
        bool bIsNumber = DataUtils::isNumber ( strTestCase );
        std::cout << strTestCase << ( bIsNumber ? " is a number" : " is not a number" ) << std::endl; 
    }
}

void TestParseCad()
{
    std::string strCadFile("./data/MyTestCAD.cad");
    VectorOfCadData vecCadData;
    std::string strErrorMsg;

    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl << "TEST DataUtils::parseCAD case 1";
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl;
    {
        std::vector<CAD_DATA_COLUMNS> vecColumns;
        for ( int i = CAD_DATA_COLUMNS::BEGIN; i < CAD_DATA_COLUMNS::END; ++ i )
            vecColumns.push_back ( static_cast<CAD_DATA_COLUMNS> ( i ) );
        int nResult = DataUtils::parseCAD ( strCadFile, vecColumns, vecCadData, strErrorMsg );
        if ( nResult != 0 ) {
            std::cout << "Failed to parse CAD, error " << strErrorMsg << std::endl;
        }else {
            std::cout << "Success to parse CAD, device count " << vecCadData.size() << std::endl;
        }
    }

    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl << "TEST DataUtils::parseCAD case 2";
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl;
    {
        std::vector<CAD_DATA_COLUMNS> vecColumns;
        vecColumns.push_back ( CAD_DATA_COLUMNS::NAME );
        vecColumns.push_back ( CAD_DATA_COLUMNS::X );
        vecColumns.push_back ( CAD_DATA_COLUMNS::Y );
        vecColumns.push_back ( CAD_DATA_COLUMNS::ANGLE );
        vecColumns.push_back ( CAD_DATA_COLUMNS::TOP_BOTTOM );

        vecColumns.push_back ( CAD_DATA_COLUMNS::WIDTH );
        vecColumns.push_back ( CAD_DATA_COLUMNS::LENGTH );
        vecColumns.push_back ( CAD_DATA_COLUMNS::PLACEMENT );
        vecColumns.push_back ( CAD_DATA_COLUMNS::BOARD_NO );
        vecColumns.push_back ( CAD_DATA_COLUMNS::PIN_COUNT );

        int nResult = DataUtils::parseCAD ( strCadFile, vecColumns, vecCadData, strErrorMsg );
        if ( nResult != 0 ) {
            std::cout << "Failed to parse CAD, error " << strErrorMsg << std::endl;
        }else {
            std::cout << "Success to parse CAD, device count " << vecCadData.size() << std::endl;
        }
    }

    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl << "TEST DataUtils::parseCAD case 3";
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl;
    {
        std::vector<CAD_DATA_COLUMNS> vecColumns;        
        vecColumns.push_back ( CAD_DATA_COLUMNS::X );
        vecColumns.push_back ( CAD_DATA_COLUMNS::NAME );    //change sequence with X to test error handle
        vecColumns.push_back ( CAD_DATA_COLUMNS::Y );
        vecColumns.push_back ( CAD_DATA_COLUMNS::ANGLE );
        vecColumns.push_back ( CAD_DATA_COLUMNS::TOP_BOTTOM );

        vecColumns.push_back ( CAD_DATA_COLUMNS::WIDTH );
        vecColumns.push_back ( CAD_DATA_COLUMNS::LENGTH );
        vecColumns.push_back ( CAD_DATA_COLUMNS::PLACEMENT );
        vecColumns.push_back ( CAD_DATA_COLUMNS::BOARD_NO );
        vecColumns.push_back ( CAD_DATA_COLUMNS::PIN_COUNT );

        int nResult = DataUtils::parseCAD ( strCadFile, vecColumns, vecCadData, strErrorMsg );
        if ( nResult != 0 ) {
            std::cout << "Failed to parse CAD, error " << strErrorMsg << std::endl;
        }else {
            std::cout << "Success to parse CAD, device count " << vecCadData.size() << std::endl;
        }
    }
}

void TestParseCAD_1()
{
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl << "TEST DataUtils::parseCAD and readPackageSize.";
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl;

    std::vector<CAD_DATA_COLUMNS> vecColumns;
    vecColumns.push_back(CAD_DATA_COLUMNS::NAME);    //change sequence with X to test error handle
    vecColumns.push_back(CAD_DATA_COLUMNS::X);    
    vecColumns.push_back(CAD_DATA_COLUMNS::Y);
    vecColumns.push_back(CAD_DATA_COLUMNS::ANGLE);
    vecColumns.push_back(CAD_DATA_COLUMNS::TOP_BOTTOM);
    vecColumns.push_back(CAD_DATA_COLUMNS::TYPE);
    vecColumns.push_back(CAD_DATA_COLUMNS::GROUP);
    vecColumns.push_back(CAD_DATA_COLUMNS::PLACEMENT);
    vecColumns.push_back(CAD_DATA_COLUMNS::BOARD_NO);

    VectorOfCadData vecCadData;
    std::string strErrorMsg;
    int nResult = DataUtils::parseCAD ( "./data/NN_DemoBoard.aoi.txt", vecColumns, vecCadData, strErrorMsg );
    if ( nResult != 0 ) {
        std::cout << "Failed to parse CAD, error " << strErrorMsg << std::endl;
        return;
    }
    std::cout << "Success to parse CAD" << std::endl;

    bool bCadWithWidthLength = false;
    for ( const auto &column : vecColumns ) {
        if ( column == CAD_DATA_COLUMNS::WIDTH ) {
            bCadWithWidthLength = true;
            break;
        }
    }

    MapGroupPackageSize mapGroupPackageSize;
    if ( ! bCadWithWidthLength ) {
        nResult = DataUtils::readPackageSize ( "./data/PackageSize.csv", mapGroupPackageSize, strErrorMsg );
        if ( nResult != 0 ) {
            std::cout << "Failed to read package size, error " << strErrorMsg << std::endl;
            return;
        }
    }

    char chArray[100];
    for ( auto &cadData : vecCadData ) {
        if ( ! bCadWithWidthLength ) {
            std::string strOutputPackage, strOutputType;
            PackageSize packageSize;
            if ( DataUtils::decodePackageAndType ( cadData.type, cadData.group, strOutputPackage, strOutputType ) == 0 ) {
                if ( mapGroupPackageSize.find(strOutputPackage) != mapGroupPackageSize.end() ) {
                    auto mapTypePackageSize = mapGroupPackageSize[strOutputPackage];
                    if ( mapTypePackageSize.find(strOutputType) != mapTypePackageSize.end() ) {
                        packageSize = mapTypePackageSize[strOutputType];
                        _snprintf_s ( chArray, sizeof ( chArray ), "(%.2f, %.2f)", packageSize.width, packageSize.length );
                        std::cout << " Device \"" << cadData.name << "\" type \"" << cadData.type << "\" " << " package size " << chArray << std::endl;
                    }else {
                        std::cout << "Cannot find device type \"" << cadData.type << "\" size in package" << std::endl;
                    }
                }else {
                    std::cout << "Cannot find package type \"" << strOutputPackage << "\" size in package" << std::endl;
                }
            }else {
                std::cout << "Cannot decode device type \"" << cadData.type << "\"" << std::endl;
            }
        }        
    }
}

void TestGetFrameFromCombinedImageSub(
    int nBigImgWidth,
    int nBigImgHeight,
    int nFrameImgWidth,
    int nFrameImgHeight,
    int nOverlapX,
    int nOverlapY,
    int nSelectPtX,
    int nSelectPtY)
{
    int nFrameX, nFrameY, nPtInFrameX, nPtInFrameY;
    DataUtils::getFrameFromCombinedImage (
        nBigImgWidth,
        nBigImgHeight,
        nFrameImgWidth,
        nFrameImgHeight,
        nOverlapX,
        nOverlapY,
        nSelectPtX,
        nSelectPtY,
        nFrameX,
        nFrameY,
        nPtInFrameX,
        nPtInFrameY,
        Vision::PR_SCAN_IMAGE_DIR::RIGHT_TO_LEFT);
    std::cout << "Input: " << std::endl;
    std::cout << "nBigImgWidth " << nBigImgWidth << ", nBigImgHeight " << nBigImgHeight << ", nFrameImgWidth " << nFrameImgWidth << ", nFrameImgHeight " << nFrameImgHeight << std::endl;
    std::cout << "nOverlapX " << nOverlapX << ", nOverlapY " << nOverlapY << ", nSelectPtX " << nSelectPtX << ", nSelectPtY " << nSelectPtY << std::endl;
    std::cout << "Result: " << std::endl;
    std::cout << "nFrameX " << nFrameX << ", nFrameY " << nFrameY << ", nPtInFrameX " << nPtInFrameX << ", nPtInFrameY " << nPtInFrameY << std::endl;

    int nCombinedImgPtX, nCombinedImgPtY;
    DataUtils::getCombinedImagePosFromFramePos (
        nBigImgWidth,
        nBigImgHeight,
        nFrameImgWidth,
        nFrameImgHeight,
        nOverlapX,
        nOverlapY,
        nFrameX,
        nFrameY,
        nPtInFrameX,
        nPtInFrameY,
        nCombinedImgPtX,
        nCombinedImgPtY,
        Vision::PR_SCAN_IMAGE_DIR::RIGHT_TO_LEFT);
    std::cout << "nCombinedImgPtX " << nCombinedImgPtX << ", nCombinedImgPtY " << nCombinedImgPtY << std::endl;
}

void TestGetFrameFromCombinedImage()
{
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl << "TEST getFrameFromCombinedImage case 1";
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl;
    {
        int nBigImgWidth    = 17421 * 2;
        int nBigImgHeight   = 12368 * 2;
        int nFrameImgWidth  = 2032;
        int nFrameImgHeight = 2032;
        int nOverlapX       = 102;
        int nOverlapY       = 140;
        int nSelectPtX      = 820 * 2;
        int nSelectPtY      = 10613 * 2;
        TestGetFrameFromCombinedImageSub ( nBigImgWidth,
            nBigImgHeight,
            nFrameImgWidth,
            nFrameImgHeight,
            nOverlapX,
            nOverlapY,
            nSelectPtX,
            nSelectPtY);
    }

    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl << "TEST getFrameFromCombinedImage case 2";
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl;
    {
        int nBigImgWidth    = 17421 * 2;
        int nBigImgHeight   = 12368 * 2;
        int nFrameImgWidth  = 2032;
        int nFrameImgHeight = 2032;
        int nOverlapX       = 102;
        int nOverlapY       = 140;
        int nSelectPtX      = 16631 * 2;
        int nSelectPtY      = 11949 * 2;
        TestGetFrameFromCombinedImageSub ( nBigImgWidth,
            nBigImgHeight,
            nFrameImgWidth,
            nFrameImgHeight,
            nOverlapX,
            nOverlapY,
            nSelectPtX,
            nSelectPtY);
    }
}

void TestAlignWithTwoPoints() {
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl << "TEST alignWithTwoPoints case 1";
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl;
    float fRotationInRadian, Tx, Ty, fScale;
    {
        cv::Point2f ptCadPos1(100, 100);
        cv::Point2f ptCadPos2(200, 200);
        cv::Point2f ptTargetPos1(105, 105);
        cv::Point2f ptTargetPos2(195, 195);
        cv::Mat matTransform;
        DataUtils::alignWithTwoPoints ( ptCadPos1, ptCadPos2, ptTargetPos1, ptTargetPos2, fRotationInRadian, Tx, Ty, fScale );
        std::cout << "RotationInRadian " << fRotationInRadian << ", Tx " << Tx << ", Ty " << Ty << ", Scale " << fScale << std::endl;
    }

    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl << "TEST alignWithTwoPoints case 1";
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl;
    {
        cv::Point2f ptCadPos1(100, 100);
        cv::Point2f ptCadPos2(200, 200);
        cv::Point2f ptTargetPos1(115, 100);
        cv::Point2f ptTargetPos2(195, 200);
        cv::Mat matTransform;
        DataUtils::alignWithTwoPoints ( ptCadPos1, ptCadPos2, ptTargetPos1, ptTargetPos2, fRotationInRadian, Tx, Ty, fScale );
        //printfMat<float> ( matTransform, 2 );
        std::cout << "RotationInRadian " << fRotationInRadian << ", Tx " << Tx << ", Ty " << Ty << ", Scale " << fScale << std::endl;

        cv::Point2f ptCtr( ptCadPos1.x / 2.f + ptCadPos2.x / 2.f,  ptCadPos1.y / 2.f + ptCadPos2.y / 2.f );
        double fDegree = fRotationInRadian * 180. / CV_PI;
        auto matRotation = cv::getRotationMatrix2D ( ptCtr, fDegree, fScale );
        matRotation.at<double>(0, 2) += Tx;
        matRotation.at<double>(1, 2) += Ty;
        auto ptWrapResult1 = warpPoint<double> ( matRotation, ptCadPos1 );
        auto ptWrapResult2 = warpPoint<double> ( matRotation, ptCadPos2 );
        std::cout << "WrapResult1 " << ptWrapResult1 << ", WrapResult2 " << ptWrapResult2 << std::endl;
    }
}

void TestAssignFrames() {
    auto printResult = [](const Vision::VectorOfVectorOfPoint2f &vecVecFrameCtr) {
        std::cout << "Rows: " << vecVecFrameCtr.size() << " Cols: " << vecVecFrameCtr[0].size() << std::endl;
        for (const auto &vecFrameCtr : vecVecFrameCtr) {
            for (const auto &ptFrameCtr : vecFrameCtr)
                std::cout << ptFrameCtr << " ";
            std::cout << std::endl;
        }
    };

    Vision::VectorOfVectorOfPoint2f vecVecFrameCtr;

    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl << "TEST assignFrames case 1";
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl;
    
    DataUtils::assignFrames(200, 500, 600, 300, 50, 50, vecVecFrameCtr);
    printResult(vecVecFrameCtr);

    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl << "TEST assignFrames case 2";
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl;
    
    DataUtils::assignFrames(12435, 13224, 22678, 6543, 600, 600, vecVecFrameCtr);
    printResult(vecVecFrameCtr);
}

void TestIsWindowInFrame() {
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl << "TEST isWindowInFrame case 1";
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl;

    {
        cv::Point2f ptWindowCtr(100, 100);
        float winWidth  = 20;
        float winHeight = 20;
        cv::Point2f ptFrameCtr(110, 110);
        float frameWidth  = 40;
        float frameHeight = 40;
        bool bInside = DataUtils::isWindowInFrame(ptWindowCtr, winWidth, winHeight, ptFrameCtr, frameWidth, frameHeight);
        std::cout << (bInside ? "inside" : "not inside") << std::endl;
    }

    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl << "TEST isWindowInFrame case 2";
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl;
    
    {
        cv::Point2f ptWindowCtr(110, 110);
        float winWidth  = 20;
        float winHeight = 20;
        cv::Point2f ptFrameCtr(100, 100);
        float frameWidth  = 20;
        float frameHeight = 20;
        bool bInside = DataUtils::isWindowInFrame(ptWindowCtr, winWidth, winHeight, ptFrameCtr, frameWidth, frameHeight);
        std::cout << (bInside ? "inside" : "not inside") << std::endl;
    }
}

void TestConvertWindowToFrameRect()
{
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl << "TEST convertWindowToFrameRect case 1";
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl;

    {
        cv::Point2f ptWindowCtr(100, 100);
        float winWidth  = 500;
        float winHeight = 600;
        cv::Point2f ptFrameCtr(110, 110);
        int imageWidth  = 2032;
        int imageHeight = 2032;
        float fResolutionX = 15;
        float fResolutionY = 15;
        cv::Rect rectResult = DataUtils::convertWindowToFrameRect(ptWindowCtr, winWidth, winHeight, ptFrameCtr, imageWidth, imageHeight, fResolutionX, fResolutionY);
        std::cout << "Result rect " << rectResult << std::endl;
    }

    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl << "TEST convertWindowToFrameRect case 2";
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl;

    {
        cv::Point2f ptWindowCtr(2200, 3220);
        float winWidth  = 1500;
        float winHeight = 1000;
        cv::Point2f ptFrameCtr(100, 100);
        int imageWidth  = 2032;
        int imageHeight = 2032;
        float fResolutionX = 15;
        float fResolutionY = 15;
        cv::Rect rectResult = DataUtils::convertWindowToFrameRect(ptWindowCtr, winWidth, winHeight, ptFrameCtr, imageWidth, imageHeight, fResolutionX, fResolutionY);
        std::cout << "Result rect " << rectResult << std::endl;
    }

    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl << "TEST convertWindowToFrameRect case 3";
    std::cout << std::endl << "----------------------------------------------";
    std::cout << std::endl;

    {
        cv::Point2f ptWindowCtr(200, 220);
        float winWidth  = 1500;
        float winHeight = 1000;
        cv::Point2f ptFrameCtr(5000, 5000);
        int imageWidth  = 2032;
        int imageHeight = 2032;
        float fResolutionX = 15;
        float fResolutionY = 15;
        cv::Rect rectResult = DataUtils::convertWindowToFrameRect(ptWindowCtr, winWidth, winHeight, ptFrameCtr, imageWidth, imageHeight, fResolutionX, fResolutionY);
        std::cout << "Result rect " << rectResult << std::endl;
    }
}

void TestDataUtils()
{
    TestConvertWindowToFrameRect();
    TestAssignFrames();
    TestIsNumber();
    TestParseCad();
    TestParseCAD_1();
    TestGetFrameFromCombinedImage();
    TestAlignWithTwoPoints();
    TestIsWindowInFrame();    
}