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

struct Light {
    std::vector<Int32>      vecLightIntensity;
    Int32                   expTime;
};
using LightVector = std::vector<Light>;

struct Alignment {
    Alignment() :
        Id              (0),
        isFM            (0),
        fmShape         (0),
        isFMDark        (0),
        recordID        (0),
        algorithm       (0),
        tmplPosX        (0.f),
        tmplPosY        (0.f),
        tmplWidth       (0.f),
        tmplHeight      (0.f),
        srchWinWidth    (0.f),
        srchWinHeight   (0.f),
        lightId         (0) {}
    Int64               Id;
    BOOL                isFM;
    Int32               fmShape;
    BOOL                isFMDark;
    Int32               recordID;
    Int32               algorithm;
    float               tmplPosX;
    float               tmplPosY;
    float               tmplWidth;
    float               tmplHeight;
    float               srchWinWidth;
    float               srchWinHeight;
    Int64               lightId;
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
        Id              (0),
        boardId         (1),
        name            (""),
        schematicName   (""),
        x               (0.f),
        y               (0.f),
        width           (1.f),
        height          (1.f),
        angle           (0.f),
        isBottom        (false),
        type            (""),
        group           (""),
        isPlaced        (true),
        pinCount        (0) {}
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
        FIND_LINE,
        FIND_CIRCLE,
    };
    Window() :
        Id          (0),
        deviceId    (0),
        name        ("Default window"),
        x           (0),
        y           (0),
        width       (0),
        height      (0),
        angle       (0),
        usage       (Usage::UNDEFINED),
        recordID    (-1),
        inspParams  ("{}"),
        colorParams ("{}"),
        lightId     (1) {}
    Int64               Id;
    Int64               deviceId;
    String              name;
    float               x;
    float               y;
    float               width;
    float               height;
    float               angle;
    Usage               usage;
    Int32               recordID;
    String              inspParams;
    String              colorParams;
    Int64               lightId;        //Light Id in database start from 1.
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