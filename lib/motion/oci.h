/** 
*---------------------------------------------------------------------------
* Module		: oci.h
* Programmer	: Googol
* Created		: 20th Apr 2015
* Description	: Open control interface 
* Remark		: 
*--------------------------------------------------------------------------
* History :
* DD-MM-YYYY   AUTHOR             NOTES
* 01-01-2016   Jia Songtao        First drafted
*------------------------------------------------------------------------
*/

#pragma once

#define OCI_API extern "C" short __declspec(dllexport)  

#define OCI_DLL_VERSION_0                        0
#define OCI_DLL_VERSION_1                        1
#define OCI_DLL_VERSION_2                        9
#define OCI_DLL_VERSION_3                        9

#define OCI_DLL_DATE_0                           1
#define OCI_DLL_DATE_1                           7
#define OCI_DLL_DATE_2                           0
#define OCI_DLL_DATE_3                           2
#define OCI_DLL_DATE_4                           2
#define OCI_DLL_DATE_5                           2

#define OC_SRV_APP_TYPE_NORMAL                  0
#define OC_SRV_APP_TYPE_GANTRY                  1

#define OC_SRV_OPEN_LOOPMODE                    0
#define OC_SRV_ONLY_VEL_LOOPMODE                1
#define OC_SRV_ONLY_POS_LOOPMODE                2
#define OC_SRV_BOTH_POS_VEL_LOOP                3



#define CONTROL_FILTER_MAX                      1

#define CONTROL_GANTRY_AXIS_MASTER              1 
#define CONTROL_GANTRY_AXIS_YAW                 2 

#define SIGNAL_TYPE_STEP                        3
#define SIGNAL_TYPE_NOTHING                     0xFF

#define WITH_VEL_CTRL_LOOP                      0
#define WITHOUT_VEL_CTRL_LOOP                   1

#define EXCITE_NOTHING                          0xFF
#define EXCITE_OPEN_LOOP                        1
#define EXCITE_VEL_CLOSE_LOOP                   2
#define EXCITE_POS_CLOSE_LOOP                   3

#define PROFILE_ON_PLANE_XY                     0
#define PROFILE_ON_PLANE_XZ                     1
#define PROFILE_ON_PLANE_YZ                     2

#define PROFILE_FILTER_WINDOW_TOTAOL            200                  
#define PROFILE_TYPE_TRAP_TYPE_I                1
#define PROFILE_TYPE_TRAP_TYPE_II               2

#define FIXPt_4_28                               0x10000000 //((unsigned long) (1<<28))
#define FIXPt_0_16                               0x10000//((unsigned long) (1<<16))

typedef struct _OCA_CTRL_GEN_
{
	short  s16Gain;
	short  s16Offset;
	double dFrq;
	long   u32RunPeriod;
	short  u16SigType;
} OCA_CTRL_GEN;


typedef struct FrcCmp
{
	short  enaFrcCmp;
	double FrcCmpFiltBwFreq;
	double FrcCmpAmp;
	double Servo_Interrupt_Freq;
}TFrcCmp;

typedef struct VelFiltII
{
	double BwFreq;
	double Servo_Interrupt_Freq;
}TVelFiltII;

typedef struct
{
	long s32LPNum0;
	long s32LPDen0;

	long s32Notchk1;
	long s32Notchk2;
	long s32Notchk3;
	double Servo_Interrupt_Freq;
} TFilt;

typedef struct Pid
{
	double kp;
	double ki;
	double kd;
	double kvff;
	double kaff;
	long   integralLimit;
	long   derivativeLimit;
	short  limit;
	short  appType;
} TPid;

typedef struct YawCtrlOCDB
{
	double k1; 
	double k2; 
	double k3; 
	long   k1Limit;
	long   k2Limit;
	short  limit;
} TYawCtrlOCDB;

//to S curve
typedef struct  
{
	double x;
	double y;
	double z;

	double vel_in;
	double acc_in;
	double jerk_in;
	double snap_in;

	double vel_out;
	double acc_out;
	double jerk_out;
	double snap_out;

	//Determined by mechanical limit
	double vlmt;
	double almt;
	double jlmt;
	double slmt;
	double Servo_Interrupt_Freq;
} TOptPrfLine;

typedef struct  
{
	short plane;
	double radius;

	double xEnd;
	double yEnd;
	double zEnd;

	double vel_in;
	double acc_in;
	double jerk_in;
	double snap_in;

	double vel_out;
	double acc_out;
	double jerk_out;
	double snap_out;

	//Determined by mechanical limit
	double vlmt;
	double almt;
	double jlmt;
	double slmt;
	double Servo_Interrupt_Freq;
} TOptPrfArc_R;

typedef struct  
{
	//Input:
	short plane;

	double xCenter;
	double yCenter;
	double zCenter;

	double xEnd;
	double yEnd;
	double zEnd;

	double vel_in;
	double acc_in;
	double jerk_in;
	double snap_in;

	//Determined by mechanical limit
	double vlmt;
	double almt;
	double jlmt;
	double slmt;

	//Output:
	double vel_out;
	double acc_out;
	double jerk_out;
	double snap_out;
	double Servo_Interrupt_Freq;
} TOptPrfArc_C;

typedef struct TSCurvePrm
{	
	//boundary:
	double vm;
	double am;
	double jm;
	double sm;

	//setting:
	double acc;
	double dec;
	double velStart;
	double vel;
	long   ldist;
	short sNum;
	short bOverLow;
	double Servo_Interrupt_Freq;
} TSCurvePrm;

typedef struct InputShape
{
	short A0;
	short A1;
	short A2;
	short count;
	short count_hf;
	double Servo_Interrupt_Freq;
}TIS;

//function prototype
OCI_API OC_SetFrcCmpFilter(TFrcCmp *pFrcCmp, TFilt *pFilt);
OCI_API OC_GetFrcCmpFilter(TFrcCmp *pFrcCmp, TFilt Filt);
OCI_API OC_SetVelFilterII( TVelFiltII *pTVelFiltII, TFilt *pFilt);
OCI_API OC_GetVelFilterII( TVelFiltII *pTVelFiltII, TFilt Filt);
OCI_API OC_SetPosLoopPid(double *pKp, double *pKi, double *pKd, TPid *pPid);
OCI_API OC_GetPosLoopPid(double Kp, double Ki, double Kd, TPid *pPid);
OCI_API OC_SetYawCtrlOCDB(double *pK1, double *pK2, double *pK3, TYawCtrlOCDB *pYawCtrlOCDB);
OCI_API OC_GetYawCtrlOCDB(double K1, double K2, double K3, TYawCtrlOCDB *pYawCtrlOCDB);

OCI_API OC_OptimizeCrdLine(TOptPrfLine *pOptPrfLine);
OCI_API OC_OptimizeCrdArc_C(TOptPrfArc_C *pOptPrfArc_C);
OCI_API OC_OptimizeCrdArc_R(TOptPrfArc_R *pOptPrfArc_R);

OCI_API  OC_SetSCurvePrm( TSCurvePrm *pPrm, short *psN1, short *psN2);
OCI_API  OC_GetSCurvePrm( TVelFiltII *pTVelFiltII, TFilt *pFilt);

OCI_API  OC_SetInputShape(double freq, double z, TIS *pIS);
