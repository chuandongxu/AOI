#pragma once
#include <string>
#include <vector>
#include <unordered_map>

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
        Length(5),
        Width(5) {}
    PackageSize(float Length, float Width) :
        Length(Length),
        Width(Width) {}

    float Length;
    float Width;
};

using MapTypePackageSize = std::unordered_map<std::string, PackageSize>;
using MapGroupPackageSize = std::unordered_map<std::string, MapTypePackageSize>;
const int PackageSizeDataColumnCount = 4;
const std::string DigitNumber = "0123456789";

class DataUtils
{
    DataUtils();
    ~DataUtils();
public:
    static bool isNumber(std::string const& n);
    static std::vector<std::string> splitString ( const std::string &s, char delim );
    static int parseCAD(const std::string &strFilePath, const std::vector<CAD_DATA_COLUMNS> &vecColumns, VectorOfCadData &vecCadData, std::string &strErrorMsg);
    static int readPackageSize(const std::string &strFilePath, MapGroupPackageSize &mapGroupPackageSize, std::string &strErrorMsg);
    static int findDigitString(const std::string &strInput, std::string &strOutput, int nStartPos = 0 );
    static int decodePackageAndType(const std::string &strPackageType, const std::string &strInputGroup, std::string &strOutputPackage, std::string &strOutputType);
};

