#ifndef _ID_DEFINE_H_
#define _ID_DEFINE_H_

#include "workflowDefine.h"

//定义模块ID
#define UI_MODEL  1001
#define CTRL_MODEL 1003
#define MOTION_MODEL 1004
#define BARCODE_MODEL 1011
#define FUNCMGR_MODULE 88001

//定义轴ID
//#define AXIS_CHECK1_TOP       1
//#define AXIS_CHECK1_IN        0

/* ************* 定义AI ********************************* */
//#define CHECK_AI_1     0
//#define CHECK_AI_2     1
//#define CHECK_AI_3     2

/* ************* 定义I/O ********************************* */
////输入模块1
#define DI_IM_STOP				  0
#define DI_RESET				  1
#define DI_START				  2
#define DI_STOP					  3
#define DI_SAFE_DOOR			  4

/////输出模块1
#define DO_YELLOW_LIGHT            0
#define DO_GREEN_LIGHT             1
#define DO_RED_LIGHT               2
#define DO_BUZZER                  3
#endif