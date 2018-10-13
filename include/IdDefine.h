#ifndef _ID_DEFINE_H_
#define _ID_DEFINE_H_

#include "workflowDefine.h"

//定义模块ID
#define UI_MODEL  1001
#define CTRL_MODEL 1003
#define MOTION_MODEL 1004
#define CAMERA_MODEL 1005
#define DATA_MODEL 1007
#define DLP_MODEL 1006
#define VISION_MODEL 1008
#define VISION_TEST_MODEL 1009
#define LIGHT_MODEL 1010
#define BARCODE_MODEL 1011
#define WORKHOLDER_MODEL 1012
#define TOWERLIGHT_MODEL 1013
#define FUNCMGR_MODULE 88001

#define AXIS_MOTOR_NUM		8

//定义轴ID
#define AXIS_MOTOR_X            "AXIS_MOTOR_X"
#define AXIS_MOTOR_Y            "AXIS_MOTOR_Y"
#define AXIS_MOTOR_Z            "AXIS_MOTOR_Z"
#define AXIS_MOTOR_TRACK_WIDTH  "AXIS_MOTOR_TRACK_WIDTH"
#define AXIS_MOTOR_TRACKING     "AXIS_MOTOR_TRACKING"

/* ************* 定义AI ********************************* */
//#define CHECK_AI_1     0
//#define CHECK_AI_2     1
//#define CHECK_AI_3     2

/* ************* 定义I/O ********************************* */
////输入模块1
#define DI_TRACK_ARRIVED          "DI_TRACK_ARRIVED"
#define DI_TRACK_READY  		  "DI_TRACK_READY"
#define DI_TRACK_STOP			  "DI_TRACK_STOP"
#define DI_TRACK_DELIVERED		  "DI_TRACK_DELIVERED"

#define DI_IM_STOP				  "DI_IM_STOP"
#define DI_START				  "DI_START"
#define DI_RESET				  "DI_RESET"
#define DI_STOP					  "DI_STOP"
#define DI_SAFE_DOOR			  "DI_SAFE_DOOR"

/////输出模块1
#define DO_CAMERA_TRIGGER1         "DO_CAMERA_TRIGGER1"
#define DO_CAMERA_TRIGGER2         "DO_CAMERA_TRIGGER2"

#define DO_TRACK_CYLINDER		   "DO_TRACK_CYLINDER"

#define DO_RED_LIGHT               "DO_RED_LIGHT"
#define DO_YELLOW_LIGHT            "DO_YELLOW_LIGHT"
#define DO_GREEN_LIGHT             "DO_GREEN_LIGHT"
#define DO_BUZZER                  "DO_BUZZER"

#define DO_START                   "DO_START"
#define DO_STOP                    "DO_STOP"
#define DO_RESET                   "DO_RESET"
#define DO_Z_BRAKE                 "DO_Z_BRAKE"
#define DO_STEPPER_RUN             "DO_STEPPER_RUN"

#endif