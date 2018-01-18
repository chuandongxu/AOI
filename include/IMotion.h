#ifndef _MOTION_H_
#define _MOTION_H_

#define MOTION_STATE_PE 0x00001
#define MOTION_STATE_ORG 0x00010
#define MOTION_STATE_EML 0x00001
#define MOTION_STATE_ALM 0x00001
#define MOTION_STATE_EMG 0x00001

#define HOME_DIR_PDR  1
#define HOME_DIR_NDR  -1

class IMotion
{
public:
	enum STATE
	{
		STATE_NO_INIT,
		STATE_PE,
		STATE_ORG,
		STATE_EML,
		STATE_ALM,
		STATE_EMG
	};

	enum TRIGGER
	{
		TRIGGER_ALL,
		TRIGGER_DLP,
		TRIGGER_DLP1,
		TRIGGER_DLP2,
		TRIGGER_DLP3,
		TRIGGER_DLP4,
		TRIGGER_LIGHT
	};
public:
	// General Functions:
	virtual bool init() = 0;
	virtual void unInit() = 0;
	virtual bool reset() = 0;
	virtual void clearAllError() = 0;
	virtual bool IsPowerError() = 0;

	// IO Functions:
	virtual bool setDOs(QVector<int>& nPorts, int iState) = 0;
	virtual bool setDO(int nPort, int iState) = 0;// nPort 0 : n-1
	virtual bool getDO(int nPort, int &iState) = 0;
	virtual bool getDI(int nPort, int &iState) = 0;

	// 3D Functions:
	virtual bool triggerCapturing(TRIGGER emTrig, bool bWaitDone, bool bClearSetupConfig = false) = 0;

	// Motor Functions:
	virtual bool enable(int AxisID) = 0;
	virtual bool disable(int AxisID) = 0;
	virtual bool enableAllAxis() = 0;
	virtual bool isEnabled(int AxisID) = 0;
	virtual bool IsError(int AxisID) = 0;
	virtual bool IsEMStopError(int AxisID) = 0;
	virtual bool IsLimit(int AxisID) = 0;

	virtual bool home(int AxisID, bool bSyn) = 0;
	virtual bool move(int AxisID, int nProfile, double dDist, bool bSyn) = 0;
	virtual bool moveTo(int AxisID, int nProfile, double dPos, bool bSyn) = 0;

	virtual bool movePos(int nPointTable, bool bSyn) = 0;
	virtual bool moveToPos(int nPointTable, bool bSyn) = 0;

	virtual bool isHomed(int AxisID) = 0;
	virtual bool isMoveDone(int AxisID) = 0;
	virtual bool stopMove(int AxisID) = 0;
	virtual bool EmStop(int AxisID) = 0;

	virtual bool getCurrentPos(int AxisID, double *pos) = 0;

	//virtual bool AxisAbsoluteMove_SafeCheck(int nAxisCount, int* nAxisID, int* nProfile, bool* bIsSafetyCheck, bool* bIsSignalInversion, bool bSynchronize) = 0;
	//virtual bool isHomed(int nAxis,int *axis) = 0;
	//virtual bool isZSafe(int AxisID) = 0;

	////virtual bool movStepper(int nAxis, int *axis, int *dirs, bool bSynchronize) = 0;
	//
	//virtual bool getState(int &nAxis,int *axis,STATE *state) = 0;
	//virtual bool isMoveDone(int nAxis) = 0;
	//virtual bool stopMove(int nAxis) = 0;
	//
	//virtual bool getCurrentPos(int nAxis,int *axis, double *pos) = 0;
	//
	//virtual bool setDO(int nChange,int iState) = 0;      
	//virtual bool getDI(int nChange,int &iState) = 0;      
	//virtual bool getDO(int nChange,int &iState) = 0;  

	//virtual bool setExtDO(int nChange,int iState) = 0;      
	//virtual bool getExtDI(int nChange,int &iState) = 0;      
	//virtual bool getExtDO(int nChange,int &iState) = 0; 

	////获取温度AI输入
	//virtual bool getExtAI(int nChange,double &val) = 0;
	//virtual bool getExtAIs(int n,int *change,double *val) = 0;

 //   /// Special Functions: 
	////virtual bool initModule(int nStations, int nSteppers) = 0;
	//virtual bool addPressure(int nStationID,bool isSpecialMaterial) = 0;
	//virtual bool releasePressure(int nStationID) = 0;
	//virtual bool releasePressuretoPreP(int nStationID) = 0;

	//virtual bool addStep(int nStation) = 0;
	//virtual bool releaseStep(int nStation) = 0;
	//virtual bool stopStep(int nStation) = 0;
	//virtual bool requestInputLock() = 0;
	//virtual void releaseInputLock() = 0;
	//virtual void setStationStart(int nStation) = 0;
	//virtual bool queryAllStationStart() = 0;
	//virtual void releaseAllStationStart() = 0;
};


#endif