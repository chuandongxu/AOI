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
*------------------------------------------------------------------------
*/

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <string>

#define AT											__FILE__, __LINE__
const double MIL_TO_UM	=							( 25.4 );
const double UM_TO_MIL	=							( 1 / MIL_TO_UM );
const std::string DEFAULT_PROJECT =                 "./data/DefaultProject.aoi";
const std::string PROJECT_EXT =                           ".aoi";

enum VISION_VIEW_MODE {
    MODE_VIEW_NONE              = 1,
    MODE_VIEW_SELECT            = 2,
    MODE_VIEW_SELECT_ROI        = 3,
    MODE_VIEW_MOVE              = 4,
    MODE_VIEW_SET_FIDUCIAL_MARK = 5,
};

#endif //_CONSTANTS_H_