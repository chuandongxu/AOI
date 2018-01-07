#ifndef _WORK_FLOW_DEFINE_H_
#define _WORK_FLOW_DEFINE_H_

/*
//全局消息定义，此消息用于底层检测到设备启动，停止，复位等按钮有
//动作时向界面发送EOS消息的键值
#define NOTIFY_USER_BTN   "user-btn"

//全局用户事件按钮定义,用于底层模块和界面模块使用EOS交互时的数据,
//和NOTIFY_USER_BTN配合使用
#define BTN_USER_START  "UserStartBtn"
#define BTN_USER_STOP   "UserStopBtn"
#define BTN_USER_RESET  "UserRestBtn"
#define BTN_SELF_SOURCE  "UserSelfSourceBtn"       //*安全开关
#define BTN_USER_EMSTOP  "UserEmStopBtn"           //*急停按钮

*/
//*****************************************************************************************
//                离线状态相关定义
//*****************************************************************************************
//离线调试启动按钮键值
#define OUT_LINE_RUN     "OutLineStart"
//离线调试停止按钮键值
#define OUT_LINE_STOP    "OutLineStop"
//离线调试复位按钮键值
#define OUT_LINE_REST    "OutLineReset"
//离线调试急停按钮键值
#define OUT_LINE_EMSTOP  "OutLineEmStop"
//离线调试安全门开关
#define OUT_LINE_SAFE	 "OutLineSafe"

//*********************************************************************************************
//定义系统运行速度
#define RUN_FULL_SPEED  4
#define RUN_HIG_SPEED   3
#define RUN_MID_SPEED   2
#define RUN_LOW_SPEED   1

//定义启用标志键值，这个键值是用来在
//系统参数中保存气密检测器启用配置信息的。
#define CHECK_STA_ENABLE1 "checkStaEnable1"
#define CHECK_STA_ENABLE2 "checkStaEnable2"
#define CHECK_STA_ENABLE3 "checkStaEnable3"
#define CHECK_STA_ENABLE4 "checkStaEnable4"
#define CHECK_STA_ENABLE5 "checkStaEnable5"
#define CHECK_STA_ENABLE6 "checkStaEnable6"
#define CHECK_STA_ENABLE  "checkStaEnable%0"

//*********************************************************************************************
//检测器状态通知键值和事件，工艺流程模块通知界面模块底层状态，进而更新界面
#define EVENT_CHECK_STATE "eventCheckState"
#define STATION_STATE_WAIT_START          100
#define STATION_STATE_CAPTURING           101
#define STATION_STATE_GENERATE_GRAY       102
#define STATION_STATE_CALCULATE_3D        103
#define STATION_STATE_MATCH_POSITION      104
#define STATION_STATE_CALCULATE_HEIGHT    105
#define STATION_STATE_RESOULT             106

#define EVENT_RESULT_DISPLAY "eventResultDisplay"
#define STATION_RESULT_DISPLAY		      107
#define STATION_RESULT_DISPLAY_PROF		  108
#define STATION_RESULT_DISPLAY_CLEAR	  109
#define STATION_RESULT_DISPLAY_CLEAR_DATA 110
#define STATION_RESULT_IMAGE_DISPLAY	  111

#define EVENT_GOHOME_STATE   "goHomeState"
#define GOHOME_STATE_OK                   120
#define GOHOME_STATE_NG                   121

#define EVENT_RUN_STATE      "runState"
#define RUN_STATE_RUNING                  130
#define RUN_STATE_STOP                    131

#define EVENT_GOHOMEING_STATE    "goHomeIng"            
#define GOHOMEING_STATE_OK                140
#define GOHOMEING_STATE_NG                141

#define EVENT_TAB_STATE      "tabState"
#define RUN_TAB_RUNING					  150
#define RUN_TAB_SETTING                   151

#define EVENT_OBJ_STATE		 "objState"
#define RUN_OBJ_EDITOR					 160
#define RUN_OBJ_CHANGE                   161
#define RUN_OBJ_PROFILE					 162
#define RUN_OBJ_PROFILE_EDIT			 163

#define EVENT_UI_STATE		 "eventUIState"
#define RUN_UI_STATE_SETTING			 170


#define EVENT_BARCODE_CHANGE "barCodeChange"
#define STATION_STATE_BARCODE             180

#define EVENT_AI_STATE			"AIStateChange"
#define EVENT_CHANGE_USER		"changeUserLevel"

#endif