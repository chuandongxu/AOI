#ifndef _AOI_AUTO_RUN_DATA_STRUCTS_H_
#define _AOI_AUTO_RUN_DATA_STRUCTS_H_

#include "DataStoreAPI.h"
using namespace NFG::AOI;

using WindowGroupVector = std::vector<Engine::WindowGroup>;

struct DeviceInspWindow
{
    Engine::Device          device;
    Engine::WindowVector    vecUngroupedWindows;
    WindowGroupVector       vecWindowGroup;
    bool                    bInspected = false;
    bool                    bAlignmentPassed = true;
    bool                    bGood = true;
};

using DeviceInspWindowVector = std::vector<DeviceInspWindow>;

#endif /*_AOI_AUTO_RUN_DATA_STRUCTS_H_*/