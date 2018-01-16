#ifndef DATA_STORE_STRUCT_H
#define DATA_STORE_STRUCT_H

#include "Common/BaseTypes.h"

namespace NFG
{
namespace AOI
{
namespace Engine
{

const Int32 OK              = 0;

struct Alignment {
    Int64               Id;
    Int32               recordID;
    Int32               algorithm;
    double              tmplPosX;
    double              tmplPosY;
    double              tmplWidth;
    double              tmplHeight;
    double              srchWinWidth;
    double              srchWinHeight;
    std::vector<Int32>  vecLight;
};
using AlignmentVector = std::vector<Alignment>;

struct Board {
    Board() : x(0.f), y(0.f), width(0.f), height(0.f), angle(0.f), isBottom(false) {}
    Int64               Id;
    String              name;
    String              abbr;   // Abbreviation
    float               x;
    float               y;
    float               width;
    float               height;
    float               angle;
    BOOL                isBottom;
};
using BoardVector = std::vector<Board>;

struct Device {
    Device() :
        Id(1),
        boardId(1),
        name(""),
        schematicName(""),
        x(0.f),
        y(0.f),
        angle(0.f),
        isBottom(false),
        type(""),
        group(""),
        isPlaced(true),
        pinCount(0) {}
    Int64               Id;
    Int64               boardId;
    String              name;
    String              schematicName;
    float               x;
    float               y;
    float               width;
    float               height;
    float               angle;
    BOOL                isBottom;
    String              type;
    String              group;
    BOOL                isPlaced;
    Int32               pinCount;
};
using DeviceVector = std::vector<Device>;

struct Window {
    enum class Usage {
        UNDEFINED = -1,
        ALIGNMENT,
        HEIGHT_BASE,
        HEIGHT_MEASURE,
        INSP_LEAD,
        INSP_CHIP,
        INSP_CONTOUR,
        INSP_HOLE,
    };
    Window() :
        Id(0),
        name("Default window"),
        x(0),
        y(0),
        width(0),
        height(0),
        angle(0),
        usage(Usage::UNDEFINED),
        recordID(-1) {}
    Int64               Id;
    String              name;
    float               x;
    float               y;
    float               width;
    float               height;
    float               angle;
    Usage               usage;
    Int32               recordID;
    String              xmlParams;
};
using WindowVector = std::vector<Window>;

struct WindowGroup {
    WindowGroup() :
        Id(0),
        deviceId(0),
        name("Default Group") {}
    Int64               Id;
    Int64               deviceId;
    String              name;
    WindowVector        vecWindows;
};

}
}
}

#endif /*DATA_STORE_STRUCT_H*/