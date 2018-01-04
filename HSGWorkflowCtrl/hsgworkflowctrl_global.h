#ifndef HSGWORKFLOWCTRL_GLOBAL_H
#define HSGWORKFLOWCTRL_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef HSGWORKFLOWCTRL_LIB
# define HSGWORKFLOWCTRL_EXPORT Q_DECL_EXPORT
#else
# define HSGWORKFLOWCTRL_EXPORT Q_DECL_IMPORT
#endif

//#define STATION_COUNT 2

#define PARAM_STATION_ID			"param-station-id"
#define PARAM_STATION_START_IO		"param-station-start-io"
#define PARAM_STATION_IN_AXIS		"param-station-in-axis"
#define PARAM_STATION_TOP_AXIS		"param-station-top-axis"
#define PARAM_STATION_CHECK_POS     "param-station-check-pos-profile"
#define PARAM_STATION_BACK_POS      "param-station-back-pos-profile"
#define PARAM_STATION_CHECK_TYPE     "param-station-chek-type-profile"
#define PARAM_STATION_OK_LIGHT      "param-station-ok-light"
#define PARAM_STATION_NG_LIGHT      "param-station-ng-light"
#define PARAM_STATION_AI            "param-station-ai"


#define SAFE_DOOR_KEY  "safe-door"

//错误代码
#define ERROR_STATION_SAFE_GRATING_ALRM 0x15010009
#define MSG_STTATION_SAFE_GRATING_ALRM  QStringLiteral("有工位触发安全光栅，请检查设备，确保安全后运行设备")

#define ERROR_SAFE_DOOR       0x18010002
#define MSG_SAFE_DOOR       QStringLiteral("安全们打开,设备已停止运行")

#define ERROR_MOTION_POS_WARRING   0x18010008 
#define MSG_MOTION_POS_WARRING   QStringLiteral("运动轴控制错误")

#define ERROR_ATHU_NORIGHT_WARRING   0x38010001 
#define MSG_ATH_NORIGHT_WARRING   QStringLiteral("系统并没有授权验证，请联系厂家解决")

#define ERROR_MOTOR_ALM        0x58010003
#define MSG_MOTOR_ALM       QStringLiteral("有轴处于告警状态，请检查驱动器")

#define ERROR_ZHOME_ALM        0x58010004
#define MSG_ZHOME_ALM       QStringLiteral("压合轴回零失败，请检查电源是否开启，运动控制部分是否正常")

#define ERROR_XHOME_ALM        0x58010005
#define MSG_XHOME_ALM       QStringLiteral("进轴回零失败，请检查电源是否开启，运动控制部分是否正常")

#define ERROR_SHOME_ALM        0x58010006
#define MSG_SHOME_ALM       QStringLiteral("密封轴回零失败，请检查电源是否开启，运动控制部分是否正常")

#define ERROR_HOME_MOTION_ALM        0x58010007
#define MSG_HOME_NOMOTION_ALM       QStringLiteral("回零失败，运动控制系统加载异常")

#endif // HSGWORKFLOWCTRL_GLOBAL_H
