//�û�ʹ�õ�ͷ�ļ�
#pragma once

#define LA_AXIS_NUM				8
#define LA_WORK_AXIS_NUM		6
#define LA_MACHINE_AXIS_NUM   5

#define AXIS_LIMIT_NONE       0       //��������
#define AXIS_LIMIT_MAX_VEL    1       //������ٶ�����
#define AXIS_LIMIT_MAX_ACC    2       //�������ٶ�����
#define AXIS_LIMIT_MAX_DV     4       //������ٶ�����������

#define KIN_MSG_BUFFER_SIZE   32

//��������ϵ�¹켣�Ƿ������ٶ�ģʽ
enum EWorkLimitMode
{
	WORK_LIMIT_INVALID=0,		//������
	WORK_LIMIT_VALID,			//������Ч
};

//���õ��ٶȶ������
enum EVelSettingDef
{
	NORMAL_DEF_VEL=0,			//����Ϊ������ϵ������ĺϳ��ٶ�
	NUM_DEF_VEL,				//��NUMϵͳ�Ĺ�����
	CUT_DEF_VEL,				//�ٶ�Ϊ�����ٶ�
};

//���õļ��ٶȶ������
enum EAccSettingDef
{
	NORMAL_DEF_ACC=0,             //���뼴���
	LONG_AXIS_ACC,                //��������ٶ�
};

//��������
enum EMachineMode
{
	NORMAL_THREE_AXIS=0,		//��׼�������ģʽ
	MULTI_AXES,					//��������ģʽ
	FIVE_AXIS,					//�������ģʽ��������ϵΪ������������ϵΪ��
	FIVE_AXIS_WORK,				//�������ģʽ����������ϵΪ����������ϵΪ��
	ROBOT,
};

//ǰհ�����ṹ��
struct TLookAheadParameter
{
	int lookAheadNum;					//ǰհ����
	double time;						//ʱ�䳣��
	double radiusRatio;					//�������Ƶ��ڲ���
	double vMax[LA_AXIS_NUM];			//���������ٶ�
	double aMax[LA_AXIS_NUM];			//����������ٶ�
	double DVMax[LA_AXIS_NUM];			//���������ٶȱ仯������ʱ�䳣���ڣ�
	double scale[LA_AXIS_NUM];			//��������嵱��
	short axisRelation[LA_AXIS_NUM];	//����������ڲ�����Ķ�Ӧ��ϵ
	char machineCfgFileName[128];		//���������ļ���
};

//////////////////////////////////////
typedef struct
{
	short RobotType;
	short reserved1;

	short KinParUse[18];
	double KinPar[18];
	short KinLimitUse[12];
	double KinLimitMin[12];
	double KinLimitMax[12];
	double KinLimitMinShift[12];
	double KinLimitMaxShift[12];

	short AxisUse[8];
	char AxisPosSignSwitch[8];
	double AxisPosOffset[8];

	short CartUnitUse[6];
	char CartPosKCSSignSwitch[6];
	short reserved2[3];
	double CartPosKCSOffset[6];
}RC_KIN_CONFIG;

typedef struct
{
	char Error;
	short ErrorID;
	char Message[129];
}RC_ERROR_INTERFACE;

typedef struct  
{
	short ErrorID;
	char Message[129];
	char LogTime[32];
	long InternalID;
}RC_MSG_BUFFER_ELEMENT;

typedef struct  
{
	short LastMsgIndex;
	RC_MSG_BUFFER_ELEMENT MsgElement[KIN_MSG_BUFFER_SIZE];
	long LastMsgID;
}RC_MSG_BUFFER;

//��ת�᷶Χ����
struct TRotationAxisRange
{
	int primaryAxisRangeOn;              //��һ��ת���޶���Χ�Ƿ���Ч��0������Ч��1����Ч
	int slaveAxisRangeOn;                //�ڶ���ת���޶���Χ�Ƿ���Ч��0������Ч��1����Ч
	double maxPrimaryAngle;              //��һ��ת�����ֵ
	double minPrimaryAngle;              //��һ��ת����Сֵ
	double maxSlaveAngle;                //�ڶ���ת�����ֵ
	double minSlaveAgnle;                //�ڶ���ת����Сֵ
};

//ѡ�����
enum EGroupSelect
{
	Continuous=0,
	Group_1,
	Group_2,
};

struct TPos
{
	double machinePos[LA_MACHINE_AXIS_NUM];
	double workPos[LA_WORK_AXIS_NUM];
};

//�ٶȹ滮ģʽ
enum EVelMode
{
	T_CURVE=0,
	S_CURVE,
	S_CURVE_NEW,                  //���ݼӼ��ٶȡ������ٶȽ���S�����ٶ�ǰհ��2015.11.16

	VEL_MODE_MAX=0x10000,         //ȷ������Ϊ4Byte
};

//////////////////////////////////
typedef enum{OPT_OFF, OPT_ON}OptimizeState;
typedef enum{NO_OPT, OPT_BLENDING, OPT_CIRCLEFITTING, OPT_CUBICSPLINE, OPT_BSPLINE}OptimizeMethod;

typedef enum{INIT_ERROR=1,		//û�н��в�����ʼ��
			PASSWORD_ERROR,		//����������ڹ̸��˶�����ƽ̨������
			INDATA_ERROR,		//�������ݴ��󣨼��Բ�������Ƿ���ȷ��
			PRE_PROCESS_ERROR,	//
			TOOL_RADIUS_COMPENSATE_ERROR_INOUT,		//���߰뾶�������󣺽���/����������������Բ��
			TOOL_RADIUS_COMPENSATE_ERROR_NOCROSS,	//���߰뾶�����������ݲ������޷����㽻��
			USERDATA_ERROR,
}ErrorID;

//�켣�Ż������ṹ��
typedef struct OptimizeParamUser 
{
	OptimizeState usePathOptimize;	//�Ƿ�ʹ��·���Ż���OPT_OFF:��ʹ��	OPT_ON:ʹ��

	float tolerance;				//����(suggest: rough:0.1, pre-finish:0.05, finish:0.01)

	OptimizeMethod optimizeMethod;	//ѡ�������Ż���ʽ

	OptimizeState keepLargeArc;		//�Ƿ�����Բ����OPT_OFF���������� OPT_ON������

	float blendingMinError;			//blending����С�趨���

	float blendingMaxAngle;			//blending�����Ƕ����ƣ������߶������Ƕȴ��ڸýǶ�ʱ������blending����λ���ȣ�

}TOptimizeParamUser;

struct TErrorInfo 
{
	ErrorID errorID;		//�����(INIT_ERROR:δ��ʼ��������PRE_PROCESS_ERROR:Ԥ����ģ�����
							//TOOL_RADIUS_COMPENSATE_ERROR:���߰뾶��������)
	long errorRowNum;		//�����к�
};

struct TPreStartPos 
{
 double Pos[LA_AXIS_NUM];
};

typedef struct  
{
	unsigned short doType;
	unsigned short doMask;
	unsigned short doValue;
	unsigned short delayTime;
	short fifo;
}TBufIoDelayData;

typedef struct  
{
	short doType;
	short doIndex;
	unsigned short highLevelTime;
	unsigned short lowLevelTime;
	long pulseNum;
	short firstLevel;
	short fifo;
}TBufDoBitPulseData;

typedef struct  
{
	short channel;
	short daValue;
	short fifo;
}TBufDaData;

typedef struct  
{
	short channel;
	short fifo;
	short source;
	double laserPower;
	double ratio;
	double minPower;
	double maxPower;
	short tableId;
}TBufLaserData;

typedef struct  
{
	short axis;
	double deltaPos;
	short fifo;
	short smoothFlag;
	short accPercent;
	short decPercent;
}TBufGearData;

typedef struct  
{
	short axis;
	double pos;
	double vel;
	double acc;
	short modal;
	short fifo;
}TBufMoveData;

typedef struct
{
	long segNum;
	short fifo;
}TBufSegNumData;

typedef struct  
{
	short crdAxis;
	short masterIndex;
	short masterType;
	short fifo;
} TBufFollowMasterData;

typedef struct  
{
	long masterPos;
	long pad;
	short fifo;
} TBufFollowEventCrossData;

typedef struct  
{
	short triggerIndex;
	long triggerOffset;
	long pad;
	short fifo;
} TBufFollowEventTriggerData;

typedef struct
{
	long masterSegment;
	long slaveSegment;
	long masterFrameWidth;
	short fifo;
}TBufFollowStartData;

typedef struct
{
	long width;
	short fifo;

}TBufFollowNextData;

typedef struct
{
	double vel;
	double acc;
	short smoothPercent;
	short fifo;
}TBufFollowReturnData;

typedef struct  
{
	short index;
	short enable;
	short fifo;
}TBufSmartCutterData;

typedef struct
{
	short eventIndex;
	short count;
	short fifo;
}TBufEventData;

//Look ahead Ex
GT_API GT_SetupLookAheadCrd(short crd,EMachineMode machineMode);
GT_API GT_SetFollowAxisParaLa(short crd, int *pAxisLimitMode, double *pVmax, double *pAmax, double *pDVmax);
GT_API GT_SetVelDefineModeLa(short crd,EVelSettingDef velDefMode);
GT_API GT_SetAxisLimitModeLa(short crd,int *pAxisLimitMode);
GT_API GT_SetWorkLimitModeLa(short crd,EWorkLimitMode workLimitMode);
GT_API GT_SetAxisVelValidModeLa(short crd,int velValidAxis);
GT_API GT_SetVelSmoothMode(int crd,int smoothMode);
GT_API GT_InitLookAheadEx(short crd,TLookAheadParameter *pLookAheadPara,short fifo=0, short motionMode =0,TPreStartPos *pPreStartPos =NULL);
GT_API GT_PrintLACmdLa(short crd,int printFlag, int clearFile);
GT_API GT_InitLookAheadPara(short crd,long lookAheadNum,double time,double radiusRatio,double scale,short fifo=0);

GT_API GT_LnXYEx(short crd,double x,double y,double synVel,double synAcc,long segNum,short override2,short fifo=0);
GT_API GT_LnXYG0Ex(short crd,double x,double y,double synVel,double synAcc,long segNum,short override2,short fifo=0);

GT_API GT_LnXYZEx(short crd,double x,double y,double z,double synVel,double synAcc,long segNum,short override2,short fifo=0);
GT_API GT_LnXYZG0Ex(short crd,double x,double y,double z,double synVel,double synAcc,long segNum,short override2,short fifo=0);

GT_API GT_LnXYZAEx(short crd,double x,double y,double z,double a,double synVel,double synAcc,long segNum,short override2,short fifo=0);
GT_API GT_LnXYZAG0Ex(short crd,double x,double y,double z,double a,double synVel,double synAcc,long segNum,short override2,short fifo=0);

GT_API GT_LnXYZACEx(short crd,double *pPos,short posMask,double synVel,double synAcc,long segNum,short override2,short fifo);
GT_API GT_LnXYZACG0Ex(short crd,double *pPos,short posMask,double synVel,double synAcc,long segNum,short override2,short fifo);

GT_API GT_LnXYZACUVWEx(short crd,double *pPos,short posMask,double synVel,double synAcc,long segNum,short override2,short fifo);
GT_API GT_LnXYZACUVWG0Ex(short crd,double *pPos,short posMask,double synVel,double synAcc,long segNum,short override2,short fifo);

GT_API GT_ArcXYREx(short crd,double x,double y,double radius,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo=0);
GT_API GT_ArcYZREx(short crd,double y,double z,double radius,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo=0);
GT_API GT_ArcZXREx(short crd,double z,double x,double radius,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo=0);

GT_API GT_ArcXYCEx(short crd,double x,double y,double xCenter,double yCenter,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo=0);
GT_API GT_ArcYZCEx(short crd,double y,double z,double yCenter,double zCenter,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo=0);
GT_API GT_ArcZXCEx(short crd,double z,double x,double zCenter,double xCenter,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo=0);

GT_API GT_ArcXYZEx(short crd,double x,double y,double z,double interX,double interY,double interZ,double synVel,double synAcc,long segNum,short override2,short fifo=0);

GT_API GT_HelixXYRZEx(short crd,double x,double y,double z,double radius,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo=0);
GT_API GT_HelixXYCZEx(short crd,double x,double y,double z,double xCenter,double yCenter,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo=0);

GT_API GT_BufDelayEx(short crd,unsigned short delayTime,short fifo=0);
GT_API GT_BufGearEx(short crd,short gearAxis,double deltaPos,short fifo=0);
GT_API GT_BufGearPercentEx(short crd,short gearAxis,double deltaPos,short accPercent,short decPercent,short fifo=0);
GT_API GT_BufMoveEx(short crd,short moveAxis,double pos,double vel,double acc,short modal,short fifo=0);
GT_API GT_BufIOEx(short crd,unsigned short doType,unsigned short doMask,unsigned short doValue,short fifo=0);
GT_API GT_BufDAEx(short crd,short chn,short daValue,short fifo=0);
GT_API GT_SetUserSegNumEx(short crd,long segNum,short fifo=0);

//Add By lin.ga 20150330
GT_API GT_BufLaserOnEx(short crd,short fifo=0,short channel=0);
GT_API GT_BufLaserOffEx(short crd,short fifo=0,short channel=0);
GT_API GT_BufLaserPrfCmdEx(short crd,double laserPower,short fifo=0,short channel=0);
GT_API GT_BufLaserFollowRatioEx(short crd,double ratio,double minPower,double maxPower,short fifo=0,short channel=0);
GT_API GT_BufLaserFollowModeEx(short crd,short source =0,short fifo=0,short channel=0);
GT_API GT_BufLaserFollowSplineEx(short crd,short tableId,double minPower,double maxPower,short fifo=0,short channel=0);
GT_API GT_BufLaserFollowOffEx(short crd,short fifo=0,short channel=0);

GT_API GT_BufDisableDoBitPulseEx(short crd,short doType,short doIndex,short fifo);
GT_API GT_BufEnableDoBitPulseEx(short crd,short doType,short doIndex,unsigned short highLevelTime,unsigned short lowLevelTime,long pulseNum,short firstLevel,short fifo);

GT_API GT_CrdDataEx(short crd,TCrdData *pCrdData,short fifo=0);
GT_API GT_GetLookAheadSegCountEx(short crd,long *pSegCount,short fifo=0);

GT_API GT_GetMotionTimeEx(short crd,double *pTime,short fifo);

GT_API GT_BufFollowMasterEx(short crd,TBufFollowMaster *pBufFollowMaster,short fifo=0);
GT_API GT_BufFollowEventCrossEx(short crd,TBufFollowEventCross *pEventCross,short fifo=0);
GT_API GT_BufFollowEventTriggerEx(short crd,TBufFollowEventTrigger *pEventTrigger,short fifo=0);
GT_API GT_BufFollowStartEx(short crd,long masterSegment,long slaveSegment,long masterFrameWidth,short fifo=0);
GT_API GT_BufFollowNextEx(short crd,long width,short fifo=0);
GT_API GT_BufFollowReturnEx(short crd,double vel,double acc,short smoothPercent,short fifo=0);

GT_API GT_BufSmartCutterOnEx(short crd,short smartCutterIndex,short fifo=0);
GT_API GT_BufSmartCutterOffEx(short crd,short smartCutterIndex,short fifo=0);

GT_API GT_BufEventOnEx(short crd,short eventIndex,short count,short fifo);
GT_API GT_BufEventOffEx(short crd,short eventIndex,short count,short fifo);
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////lin.ga 20150701 Add PathOpt Fuction/////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
GT_API GT_SetPathOptPara(short crd, TOptimizeParamUser *optPrm,short fifo);
GT_API GT_GetPathOptErrorInfo(short crd,TErrorInfo *errorInfo,short fifo);


//5 Axis
GT_API GT_CrdRTCPOn(short crd,short fifo=0);
GT_API GT_CrdRTCPOff(short crd,short fifo=0);
GT_API GT_UpdateMachineBuildingFile(short crd, int update);
GT_API GT_InitialMachineBuilding(short crd, char *pMachineCfgFileName,double *machineCoordCenter, double *workCoordCenter, double toolLength);
GT_API GT_SetRotationAxisRange(short crd, TRotationAxisRange *pRotationAxisRange);
GT_API GT_SetInverseSolutionSelectPara(short crd, EGroupSelect groupSelect, int priorAxisSet);
GT_API GT_MachineForwardTrans(short crd, double *pMachinePos, double *pWorkPos);
GT_API GT_MachineRTCPTrans(short crd, double *pInputPos, double *pMachinePos, double *pWorkPos);
GT_API GT_MachineTransformation(short crd, int posType, double *pPrePos, double *pPos, int *pPosNum, TPos **pReturnPos);
GT_API GT_SetCompToolLength(short crd, double compToolLength);
GT_API GT_SetCompWorkCoordOffset(short crd, double *pCompWorkOffset);
GT_API GT_SetNonlinearErrorControl(short crd, int enable, double nonlinearError);
GT_API GT_EnableDiscreateArc(short crd,short enable,double arcError);
GT_API GT_StartXYCMachineMode(short crd, short dir, double contactAngle, double rotationAngle, double *pTranslation, double aValue,short fifo);//����XYCģʽ
GT_API GT_EndXYCMachineMode(short crd,short fifo);                                                   //�˳�XYCģʽ
GT_API GT_SetWorkCrdPlane(short crd, short enable, double *pNormVector, double *pPoint,short fifo);    //���ò����ӹ�ƽ�����
GT_API GT_SetStartPointProcessMode(short crd, short enable, double z,short fifo);                      //Ԥ�����һ��xyc����
GT_API GT_InitialMachineBuildingEx(short crd, char *pMachineCfgFileName,double *machineCoordCenter, double *workCoordCenter, double toolLength);
GT_API GT_SetWorkCrdLaserFollowMode(short crd,short enbale,short fifo,short chn);
GT_API GT_ArcXYRACEx(short crd,double x,double y,double a,double c,double radius,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo);
GT_API GT_ArcXYCACEx(short crd,double x,double y,double a,double c,double xCenter,double yCenter,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo);
GT_API GT_ArcXYZACEx(short crd,double x,double y,double z,double a,double c,double interX,double interY,double interZ,double interA,double interC,double synVel,double synAcc,long segNum,short override2,short fifo);
GT_API GT_ArcYZRACEx(short crd,double y,double z,double a,double c,double radius,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo);
GT_API GT_ArcYZCACEx(short crd,double y,double z,double a,double c,double yCenter,double zCenter,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo);
GT_API GT_ArcZXRACEx(short crd,double z,double x,double a,double c,double radius,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo);
GT_API GT_ArcZXCACEx(short crd,double z,double x,double a,double c,double zCenter,double xCenter,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo);
