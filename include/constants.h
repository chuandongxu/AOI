/* \internal
*---------------------------------------------------------------------------
*                     Novo Graphics Technology Ltd
*					R&D Software Technology Group
*                      (c) Copyright 2018 NFG
*
* All rights reserved. This source code contains confidential, trade secret
* material of ASMPT. Any attempt or participation in deciphering, decoding,
* reverse engineering or in any way altering the source code is strictly
* prohibited, unless the prior written consent of Novo Graphics is obtained.
*
* Module       : constants.h
* Programmer   : Xiao Sheng Guang
* Created      : 10 Jan 2018
* Description  : This file contained constant values for the AOI project.
* Remark	   : Reference to vision library.
*--------------------------------------------------------------------------
* History :
* YYYYMMDD     AUTHOR       NOTES
* 20180110	   Xiao SG  Initial version. 
*--------------------------------------------------------------------------
*/

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <string>

#define AT                                          __FILE__, __LINE__
const float MIL_TO_UM =                             ( 25.4f );
const float UM_TO_MIL =                             ( 1.f / MIL_TO_UM );
const float INCH_TO_UM =                            ( 25.4f * 1000.f);
const float UM_TO_INCH =                            ( 1.f / INCH_TO_UM );
const float MM_TO_UM =                              ( 1000.f );
const float UM_TO_MM =                              ( 1.f / MM_TO_UM );
const float CM_TO_UM =                              (1000000.f);
const float UM_TO_CM  =                             (1.f / CM_TO_UM );
const float ONE_HUNDRED_PERCENT =                   (100.f);
const std::string DEFAULT_PROJECT =                 "./data/DefaultProject.aoi";
const std::string PROJECT_EXT =                     ".aoi";
const int DLP_IMG_COUNT =                           12;

const int OK  =                                     0;
const int NOK =                                     -1;

enum VISION_VIEW_MODE {
    MODE_VIEW_NONE              = 1,
    MODE_VIEW_SELECT            = 2,
    MODE_VIEW_SELECT_ROI        = 3,
    MODE_VIEW_MOVE              = 4,
    MODE_VIEW_SET_FIDUCIAL_MARK = 5,
    MODE_VIEW_EDIT_SRCH_WINDOW  = 6,
    MODE_VIEW_EDIT_INSP_WINDOW  = 7,
};

enum GRAY_WEIGHT_METHOD
{
	EM_MODE_PT_THRESHOLD = 0,
	EM_MODE_ONE_THRESHOLD,
	EM_MODE_TWO_THRESHOLD
};


#endif //_CONSTANTS_H_