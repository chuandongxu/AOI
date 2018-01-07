#ifndef _WORK_FLOW_DEFINE_H_
#define _WORK_FLOW_DEFINE_H_

/*
//ȫ����Ϣ���壬����Ϣ���ڵײ��⵽�豸������ֹͣ����λ�Ȱ�ť��
//����ʱ����淢��EOS��Ϣ�ļ�ֵ
#define NOTIFY_USER_BTN   "user-btn"

//ȫ���û��¼���ť����,���ڵײ�ģ��ͽ���ģ��ʹ��EOS����ʱ������,
//��NOTIFY_USER_BTN���ʹ��
#define BTN_USER_START  "UserStartBtn"
#define BTN_USER_STOP   "UserStopBtn"
#define BTN_USER_RESET  "UserRestBtn"
#define BTN_SELF_SOURCE  "UserSelfSourceBtn"       //*��ȫ����
#define BTN_USER_EMSTOP  "UserEmStopBtn"           //*��ͣ��ť

*/
//*****************************************************************************************
//                ����״̬��ض���
//*****************************************************************************************
//���ߵ���������ť��ֵ
#define OUT_LINE_RUN     "OutLineStart"
//���ߵ���ֹͣ��ť��ֵ
#define OUT_LINE_STOP    "OutLineStop"
//���ߵ��Ը�λ��ť��ֵ
#define OUT_LINE_REST    "OutLineReset"
//���ߵ��Լ�ͣ��ť��ֵ
#define OUT_LINE_EMSTOP  "OutLineEmStop"
//���ߵ��԰�ȫ�ſ���
#define OUT_LINE_SAFE	 "OutLineSafe"

//*********************************************************************************************
//����ϵͳ�����ٶ�
#define RUN_FULL_SPEED  4
#define RUN_HIG_SPEED   3
#define RUN_MID_SPEED   2
#define RUN_LOW_SPEED   1

//�������ñ�־��ֵ�������ֵ��������
//ϵͳ�����б������ܼ��������������Ϣ�ġ�
#define CHECK_STA_ENABLE1 "checkStaEnable1"
#define CHECK_STA_ENABLE2 "checkStaEnable2"
#define CHECK_STA_ENABLE3 "checkStaEnable3"
#define CHECK_STA_ENABLE4 "checkStaEnable4"
#define CHECK_STA_ENABLE5 "checkStaEnable5"
#define CHECK_STA_ENABLE6 "checkStaEnable6"
#define CHECK_STA_ENABLE  "checkStaEnable%0"

//*********************************************************************************************
//�����״̬֪ͨ��ֵ���¼�����������ģ��֪ͨ����ģ��ײ�״̬���������½���
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