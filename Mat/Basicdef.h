#ifndef __TYPE_DEF_H__
#define __TYPE_DEF_H__
/**************************************
UNIX   _unix
or _unix_
Linux   _linux
or _linux_
Windows32       _WIN32
Windows64       _WIN64
IOS              __APPLE__
android        __ANDROID__
***************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cstdint>
#include "math.h"
#include <omp.h>
#include "TimeStatistic.h"
#define PI 3.14159

///////////////////////////////diff platform use/////////////////////////////
#ifdef USE_SSE
#include <mmintrin.h> //MMX  
#include <xmmintrin.h> //SSE(include mmintrin.h)  
#include <emmintrin.h> //SSE2(include xmmintrin.h)  
#include <pmmintrin.h> //SSE3(include emmintrin.h)  
#include <tmmintrin.h> //SSSE3(include pmmintrin.h)  
#include <smmintrin.h>//SSE4.1(include tmmintrin.h)  
#include <nmmintrin.h>//SSE4.2(include smmintrin.h)  
#include <wmmintrin.h>//AES(include nmmintrin.h)  
#include <immintrin.h>//AVX(include wmmintrin.h)  
#endif
#ifndef _WIN32
#include<sys/time.h>
#include<sys/resource.h>
#include <sys/types.h>
#include <sched.h>
#include <unistd.h>
#include <sys/syscall.h>  
#define gettid() syscall(__NR_gettid)  
#endif
///////////////////////Android system use ////////////////////////////
#ifdef __ANDROID__
#ifndef __aarch64__
#define PRELOAD(pdata) __asm volatile ("PLD [%0,#256] ": : "r" (pdata))
#define PRESTR(pdata) 
#else
//#define PRELOAD(pdata) __asm volatile ("PRFM PLDL1STRM, [%0,#256] ": : "r" (pdata))
#define PRELOAD(pdata) __asm volatile ("PRFM PLDL1KEEP, [%0,#128] ": : "r" (pdata))
#define PRESTR(pdata) __asm volatile ("PRFM PSTL1KEEP, [%0,#128] ": : "r" (pdata))
#endif//#ifndef ARM_V7
# define barrier() __asm volatile("": : :"memory") 
#endif

//////////////////////linux cannot use/////////////////////////
#ifndef __linux__
#define USE_NEON
#define  USE_OPT
#endif

#ifdef _WIN32
#define ATTR_ALIGN(n)  __declspec(align(n))
#else
#define ATTR_ALIGN(n)  __attribute__((aligned(n)))
#endif
//////////////////////global use start Algorithm optimization definition/////////////////////////
#define USE_HOMEDIFFmin 
//#define USE_JHFDIFFminFeature 
//#define USE_JHFDIFFminBayer2YC4 
//#define USE_JHFE 
#define USE_JHFDIFFminFusion 
#define USE_JHFDIFFminhomegraph 
#define USE_JHFDIFFminNLM 
#define USE_JHFHDROPT
#define USE_JHFDIFFE 
#define USE_OMP


#define THREAD_FUNC_IMPL_9(RET, CLASS, FUNC, T0, P0, T1, P1, T2, P2, T3, P3, T4, P4, T5, P5, T6, P6, T7, P7, T8, P8) \
typedef struct tagThreadParam##FUNC \
{ \
	CLASS *pConfig; \
    T0 P0; T1 P1; T2 P2; T3 P3; T4 P4; T5 P5; T6 P6; T7 P7; T8 P8;\
    int nStart; \
    int nEnd; \
}ThreadParam##FUNC; \
void* ThreadFunc##FUNC(void* pParam) \
{ \
	ThreadParam##FUNC *p = (ThreadParam##FUNC*)pParam; \
    p->pConfig->FUNC##Impl(p->P0, p->P1, p->P2, p->P3, p->P4, p->P5, p->P6, p->P7, p->P8, p->nStart, p->nEnd); \
    return NULL; \
} \
RET CLASS::FUNC##Impl(T0 P0, T1 P1, T2 P2, T3 P3, T4 P4, T5 P5, T6 P6, T7 P7, T8 P8, int nStart, int nEnd) \



///////////////////////DLL¿ØÖÆÊä³ö ////////////////////////////
#if __GNUC__ >= 4
#define DLL_PUBLIC __attribute__ ((visibility("default")))
#define DLL_LOCAL  __attribute__ ((visibility("hidden")))
#else
#define DLL_PUBLIC
#define DLL_LOCAL
#endif

//////////////////////end Algorithm optimization definition/////////////////////////
typedef struct tagGlobalControl
{
	int nFrameNum;
	int nCFAPattern;
	int nAWBGain[4];
	int nCameraGain;
	int nCameraExposure;
	int nBLC;
	int nWP;
	int nBit;
	int nISPGain;
	int nDigiGain;
	int nEQGain;
	int nLENCQ;
	int nfaceNum;
	float nCCM[3][3];
	tagGlobalControl()
	{
		nFrameNum = 1;
		nCFAPattern = 0;
		nfaceNum = 0;
		nAWBGain[0] = nAWBGain[1] = nAWBGain[2] = nAWBGain[3] = 256;
		nCameraGain = 16;
		nISPGain = 128;
		nDigiGain = 128;
		nCameraExposure = 1;
		nBLC = 16;
		nBit = 16;
		nEQGain = 16;
		nLENCQ = 64;
		nCCM[0][0] = 1; nCCM[0][1] = 0; nCCM[0][2] = 0;
		nCCM[1][0] = 0; nCCM[1][1] = 1; nCCM[1][2] = 0;
		nCCM[2][0] = 0; nCCM[2][1] = 0; nCCM[2][2] = 1;
	}
}TGlobalControl;
#endif
