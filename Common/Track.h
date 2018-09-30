#ifndef _COMM_TRACK_H_
#define _COMM_TRACK_H_

/********************************************************************************
 功能简介：track信息输出模块，提供track的输出和调试信息输出功能，
           对调试信息的输出还需对象定义时给足够的支持(定义出输出)
           函数的实现部分
 作    者：wolfseek
 联系方式：email:wolfseek@163.com,qq:535789574
 版权声明：本软件使用GPL开源协议。作者不能保证此软件在各种环境下都能健壮运行。
           作者不承担应用此软件产生的任何责任。请保留作者信息的，请谨慎使用。
 创建日期：2012-5-10
 维护记录：2012-7-8 重新定义启用track功能的宏，由QT_NO_DEBUG_STREAM
                    改为ENABLE_DEBUG，使在release下也能去用此模块功能，
                    并追加LOG。
 后继完善： 暂无
 *********************************************************************************/
//#include <QString>
#include "Common_global.h"

//#ifdef _WIN32
//  #ifdef TRACK_EXPORTS 
//  #define TRACK_PORT __declspec(dllexport)
//  #else 
//  #define TRACK_PORT __declspec(dllimport)
//  #endif
//#else
//  #define TRACK_PORT
//#endif



COMMON_EXPORT void track_print(int iLevel,const char* fun,int line,const char * str,...);

class COMMON_EXPORT CFuncTrack
{
public:
    CFuncTrack(const char *fun,int line,const char *str,...);
    ~CFuncTrack();
protected:
    void* m_func;
    int m_line;
};


#define TR_INFO(str,...) track_print(4,__FILE__,__LINE__,str,__VA_ARGS__)
#define TR_WARRING(str,...) track_print(3,__FILE__,__LINE__,str,__VA_ARGS__)
#define TR_ERROR(str,...) track_print(1,__FILE__,__LINE__,str,__VA_ARGS__)
#define TR_FUNC(str,...) CFuncTrack funcTrack(__FILE__,__LINE__,str,__VA_ARGS__)
#define TR_TRACK(str,...)  track_print(5,__FILE__,__LINE__,str,__VA_ARGS__)

#endif