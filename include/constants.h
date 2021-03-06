/* \internal
*---------------------------------------------------------------------------
*                     Novo Graphics Technology Ltd
*                    R&D Software Technology Group
*                      (c) Copyright 2018 NFG
*
* All rights reserved. This source code contains confidential, trade secret
* material of NFG. Any attempt or participation in deciphering, decoding,
* reverse engineering or in any way altering the source code is strictly
* prohibited, unless the prior written consent of Novo Graphics is obtained.
*
* Module       : constants.h
* Programmer   : Xiao Sheng Guang
* Created      : 10 Jan 2018
* Description  : This file contained constant values for the AOI project.
* Remark       : Reference to vision library.
*--------------------------------------------------------------------------
* History :
* YYYYMMDD     AUTHOR       NOTES
* 20180110       Xiao SG  Initial version. 
*--------------------------------------------------------------------------
*/

#ifndef _AOI_CONSTANTS_H_
#define _AOI_CONSTANTS_H_

#include <string>

#define AT                                          __FILE__, __LINE__
const float MIL_TO_UM =                             (25.4f);
const float UM_TO_MIL =                             (1.f / MIL_TO_UM);
const float INCH_TO_UM =                            (25.4f * 1000.f);
const float UM_TO_INCH =                            (1.f / INCH_TO_UM);
const float MM_TO_UM =                              (1000.f);
const float UM_TO_MM =                              (1.f / MM_TO_UM);
const float CM_TO_UM =                              (1000000.f);
const float UM_TO_CM  =                             (1.f / CM_TO_UM);
const float ONE_HUNDRED_PERCENT =                   (100.f);
const std::string DEFAULT_PROJECT =                 "./data/DefaultProject.aoi";
const std::string PROJECT_EXT =                     ".aoi";
const int DLP_IMG_COUNT =                           12;

const int OK  =                                     0;
const int NOK =                                     -1;

enum VISION_VIEW_MODE {
    MODE_VIEW_NONE = 1,
    MODE_VIEW_SELECT_3D_ROI,
    MODE_VIEW_SELECT_INSPECT_ROI,
    MODE_VIEW_SELECT_ROI,
    MODE_VIEW_MOVE,
    MODE_VIEW_SET_FIDUCIAL_MARK,
    MODE_VIEW_EDIT_FIDUCIAL_MARK,
    MODE_VIEW_EDIT_FM_SRCH_WINDOW,
    MODE_VIEW_EDIT_INSP_WINDOW,
    MODE_VIEW_EDIT_SRCH_WINDOW,
    MODE_VIEW_SELECT_SUB_ROI,
    MODE_VIEW_EDIT_DEVICE,
};

enum GRAY_WEIGHT_METHOD
{
    EM_MODE_PT_THRESHOLD = 0,
    EM_MODE_ONE_THRESHOLD,
    EM_MODE_TWO_THRESHOLD
};

class CAPTURE_2D_IMAGE_SEQUENCE
{
public:
    enum {
        WHITE_LIGHT,
        LOW_ANGLE_LIGHT,
        RED_LIGHT,
        GREEN_LIGHT,
        BLUE_LIGHT,
        UNIFORM_LIGHT,
        TOTAL_COUNT
    };
};

class PROCESSED_IMAGE_SEQUENCE
{
public:
    enum {
        WHITE_LIGHT,
        LOW_ANGLE_LIGHT,
        SOLDER_LIGHT,
        UNIFORM_LIGHT,
        HEIGHT_GRAY,
        TOTAL_COUNT
    };
};

class CAPTURE_NUM_MODE {
public:
    enum {
        ALL_DLP_AND_LIGHT_54,
        ALL_DLP_48,
        SINGLE_DLP_12,
        SINGLE_FRAME_1,
        LIGHT_6,
        TOTAL_COUNT
    };
};

#endif //_AOI_CONSTANTS_H_