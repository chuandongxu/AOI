#ifndef _CAN_H_
#define _CAN_H_

#define ICAN_RESULT_NUM 4

enum ENUM_RESULT_STATUS
{
	STATUS_OK_PART = 0,             //PASS
	STATUS_NG_PART,					 //NG 
	STATUS_ALARM,
	STATUS_ERROR,
};

typedef struct
{
	bool bEndCycle;
	//int nTestResult; // 0 = OK , 1 = NG, 2 = error
	ENUM_RESULT_STATUS emStatus;
	float fPressureValue;
	float fLeakValue;
}ICAN_VALUE;

typedef struct
{
	bool bEndTotalCycle;
	ICAN_VALUE stRsts[ICAN_RESULT_NUM];	
}ICAN_RESULT;

class ICAN
{
public:
	virtual bool isConnect(int nStation) = 0;

	virtual void startCycle(int nStation, int nDevice = -1) = 0;
	virtual void stopCycle(int nStation) = 0;
	virtual bool getResult(int nStation, ICAN_RESULT& result) = 0;

	/* CAN light interface */
	virtual void getCycleStatistics(int nStation, int& nTotalCycles, int& nFailCycles, int& nSuccessCycles) = 0;
	virtual void getCommStatistics(int nStation, int& nTransmited, int& nReceived, int& nErrors) = 0;
	virtual void clearResult(int nStation) = 0;
	virtual void startAutoZeroPressure(int nStation, float fDumpTime) = 0;
};


#endif