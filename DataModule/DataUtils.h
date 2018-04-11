#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "VisionAPI.h"

using namespace AOI;

enum CAD_DATA_COLUMNS {
    BEGIN,
    NAME = BEGIN,
    X,
    Y,
    ANGLE,
    TOP_BOTTOM,
    TYPE,
    GROUP,
    WIDTH,
    LENGTH,
    PLACEMENT,
    BOARD_NO,
    PIN_COUNT,
    END
};

enum CAD_DATA_UNIT {
    UM,
    MM,
    CM,
    MIL,
    INCH
};

static const std::string CAD_DATA_COLUMN_NAMES[] {
    "Name",
    "X",
    "Y",
    "Angle",
    "TopBottom",
    "Type",
    "Group",
    "Width",
    "Length",
    "Placement",
    "BoardNo",
    "PinCount",
};

static_assert ( sizeof(CAD_DATA_COLUMN_NAMES) /sizeof(std::string) == CAD_DATA_COLUMNS::END, "Column count not match with names" );

struct CadData {
    CadData() :
        name(""),
        boardNo(0),
        pinCount(0) {}
    std::string     name;
    float           x;
    float           y;
    float           width;
    float           length;
    float           angle;
    bool            isBottom;
    std::string     type;
    std::string     group;
    bool            placement;
    int             boardNo;
    int             pinCount;
};
using VectorOfCadData = std::vector<CadData>;

struct PackageSize {
    PackageSize() :
        length(5),
        width(5) {}
    PackageSize(float Width, float Length ) :
        length(Length),
        width(Width) {}

    float length;
    float width;
};

template <typename Tp> inline int   ToInt32(Tp param) { return static_cast<int>(param); }
template <typename Tp> inline short ToInt16(Tp param) { return static_cast<short>(param); }
template <typename Tp> inline float ToFloat(Tp param) { return static_cast<float>(param); }

using MapTypePackageSize = std::unordered_map<std::string, PackageSize>;
using MapGroupPackageSize = std::unordered_map<std::string, MapTypePackageSize>;
const int PackageSizeDataColumnCount = 4;
const std::string DigitNumber = "0123456789";

class DataUtils
{
    DataUtils();
    ~DataUtils();
public:
    template<typename _Tp>
    static inline std::vector<std::vector<_Tp>> matToVector(const cv::Mat &matInputImg) {
        std::vector<std::vector<_Tp>> vecVecArray(matInputImg.rows, std::vector<_Tp>(matInputImg.cols, 0));
        if (matInputImg.isContinuous()) {
            for (int row = 0; row < matInputImg.rows; ++ row) {
                int nRowStart = row * matInputImg.cols;
                vecVecArray[row].assign((_Tp *)matInputImg.datastart + nRowStart, (_Tp *)matInputImg.datastart + nRowStart + matInputImg.cols);
            }
        }
        else {
            for (int row = 0; row < matInputImg.rows; ++ row) {
                vecVecArray[row].assign((_Tp*)matInputImg.ptr<uchar>(row), (_Tp*)matInputImg.ptr<uchar>(row) + matInputImg.cols);
            }
        }
        return vecVecArray;
    }

    static bool isNumber(std::string const& n);
    static float toUm(float fInput, CAD_DATA_UNIT enDataUnit);
    static std::vector<std::string> splitString ( const std::string &s, char delim );
    static int parseCAD(const std::string &strFilePath, const std::vector<CAD_DATA_COLUMNS> &vecColumns, VectorOfCadData &vecCadData, std::string &strErrorMsg);
    static int readPackageSize(const std::string &strFilePath, MapGroupPackageSize &mapGroupPackageSize, std::string &strErrorMsg);
    static int findDigitString(const std::string &strInput, std::string &strOutput, int nStartPos = 0 );
    static int decodePackageAndType(const std::string &strPackageType, const std::string &strInputGroup, std::string &strOutputPackage, std::string &strOutputType);
    static int getFrameFromCombinedImage(
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
        Vision::PR_SCAN_IMAGE_DIR enDir);

    static int getCombinedImagePosFromFramePos(
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
        Vision::PR_SCAN_IMAGE_DIR enDir);

    static int alignWithTwoPoints(
        cv::Point2f  ptCadPos1,
        cv::Point2f  ptCadPos2,
        cv::Point2f  ptTargetPos1,
        cv::Point2f  ptTargetPos2,
        float       &fRotationInRadian,
        float       &Tx,
        float       &Ty,
        float       &fScale);

    static int assignFrames(
        float                            left,
        float                            top,
        float                            right,
        float                            bottom,
        float                            fovWidth,
        float                            fovHeight,
        Vision::VectorOfVectorOfPoint2f &vecVecFrameCtr);

    static bool isWindowInFrame(
        const cv::Point2f &ptWindowCtr,
        float              winWidth,
        float              winHeight,
        const cv::Point2f &ptFrameCtr,
        float              fovWidth,
        float              fovHeight);

    static cv::Rect convertWindowToFrameRect(
        const cv::Point2f &ptWindowCtr,
        float              winWidth,
        float              winHeight,
        const cv::Point2f &ptFrameCtr,
        int                imageWidth,
        int                imageHeight,
        float              fResolutionX,
        float              fResolutionY);
};

