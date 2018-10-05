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
#define AXIS_MOTOR_X            1
#define AXIS_MOTOR_Y            2
#define AXIS_MOTOR_Z            3
#define AXIS_MOTOR_TRACK_WIDTH  4
#define AXIS_MOTOR_TRACKING     5

/* ************* 定义AI ********************************* */
//#define CHECK_AI_1     0
//#define CHECK_AI_2     1
//#define CHECK_AI_3     2

/* ************* 定义I/O ********************************* */
////输入模块1
#define DI_TRACK_ARRIVED          1
#define DI_TRACK_READY  		  2
#define DI_TRACK_STOP			  3
#define DI_TRACK_DELIVERED		  4

#define DI_IM_STOP				  5
#define DI_START				  6
#define DI_RESET				  7
#define DI_STOP					  8
#define DI_SAFE_DOOR			  0

/////输出模块1
#define DO_CAMERA_TRIGGER1         1
#define DO_CAMERA_TRIGGER2         2

#define DO_TRACK_CYLINDER		   3

#define DO_RED_LIGHT               5
#define DO_YELLOW_LIGHT            6
#define DO_GREEN_LIGHT             7
#define DO_BUZZER                  8

#define DO_START                   9
#define DO_STOP                    10
#define DO_RESET                   11
#define DO_Z_BRAKE                 12
#define DO_STEPPER_RUN             13

#define DO_TRIGGER_DLP1            0
#define DO_TRIGGER_DLP2            0
#define DO_TRIGGER_DLP3            0
#define DO_TRIGGER_DLP4		       0
#define DO_LIGHT1_CH1			   0
#define DO_LIGHT1_CH2	           0
#define DO_LIGHT1_CH3              0
#define DO_LIGHT1_CH4              0
#define DO_LIGHT2_CH1              0
#define DO_LIGHT2_CH2              0
#define DO_LIGHT2_CH3              0
#define DO_LIGHT2_CH4              0
#define DO_LIGHT1_ENABLE           0
#define DO_LIGHT2_ENABLE           0
#endif