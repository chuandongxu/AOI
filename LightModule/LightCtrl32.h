#ifndef _LIGHT_CTRL_32_H_
#define _LIGHT_CTRL_32_H_

#ifdef LIGHTCTRL_EXPORTS
#define LIGHTCTRL_API __declspec(dllexport)
#else
#define LIGHTCTRL_API __declspec(dllimport)
#endif

typedef void * HLIGHT;
extern "C"
{
	LIGHTCTRL_API HLIGHT openLigh(int comport,int bound);
	LIGHTCTRL_API void setLightValue(HLIGHT dev,int ch,int val);
	LIGHTCTRL_API bool close(HLIGHT dev);
}

#endif