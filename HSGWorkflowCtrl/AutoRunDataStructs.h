#ifndef _AOI_AUTO_RUN_DATA_STRUCTS_H_
#define _AOI_AUTO_RUN_DATA_STRUCTS_H_

#include "DataStoreAPI.h"
#include "VisionHeader.h"
using namespace NFG::AOI;
using namespace AOI;

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

struct AutoRunParams {
    AutoRunParams(int     nImgWidthPixel,
                  int     nImgHeightPixel,
                  float   fBoardLeftPos,
                  float   fBoardTopPos,
                  float   fBoardRightPos,
                  float   fBoardBtmPos,
                  float   fFrameOverlapX,
                  float   fFrameOverlapY,
                  Vision::PR_SCAN_IMAGE_DIR enScanDir) :
                  nImgWidthPixel    (nImgWidthPixel),
                  nImgHeightPixel   (nImgHeightPixel),
                  fBoardLeftPos     (fBoardLeftPos),
                  fBoardTopPos      (fBoardTopPos),
                  fBoardRightPos    (fBoardRightPos),
                  fBoardBtmPos      (fBoardBtmPos),
                  fOverlapUmX       (fFrameOverlapX),
                  fOverlapUmY       (fFrameOverlapY),
                  enScanDir         (enScanDir) {}
    int     nImgWidthPixel;
    int     nImgHeightPixel;
    float   fBoardLeftPos;
    float   fBoardTopPos;
    float   fBoardRightPos;
    float   fBoardBtmPos;
    float   fOverlapUmX;
    float   fOverlapUmY;    
    int     nGlobalBaseColorDiff;
    int     nGlobalBaseGrayDiff;
    cv::Scalar                      scalarGlobalBase;
    Vision::PR_SCAN_IMAGE_DIR       enScanDir;
};

#endif /*_AOI_AUTO_RUN_DATA_STRUCTS_H_*/