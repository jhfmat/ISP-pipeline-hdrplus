#ifndef __MY_LOG_H__
#define __MY_LOG_H__
#include <stdio.h>
#include "Basicdef.h"
#define  LOG_TAG    "Mat================="
#if defined(_WIN32)
#include <windows.h>
#include "NEONToSSE.h"
#elif  defined __linux__

#else
#include <jni.h>
#include "arm_neon.h"
#include "android/log.h"
#endif

#if defined(_WIN32)
#define  LOGI(...) { printf(__VA_ARGS__); printf("\n"); }
#elif  defined __linux__
#define  LOGI(...) { printf(__VA_ARGS__); printf("\n"); }
#else 
#define  LOGI(...) {  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__); } 
#endif


#if defined(_WIN32)
#define  LOGE(...) { printf("error\t") printf(__VA_ARGS__); printf("\n"); }
#elif  defined __linux__
#define  LOGE(...) { printf("error\t") printf(__VA_ARGS__); printf("\n"); }
#else
#define  LOGE(...) {__android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__;}
#endif


#ifdef USE_NEON
#define print_Vf(VV) printf("%f %f %f %f\n",VV[0],VV[1],VV[2],VV[3]);
#define print_V4(VV) printf("%x %x %x %x\n",VV[0],VV[1],VV[2],VV[3]);
#define print_V8(VV) printf("%x %x %x %x %x %x %x %x\n",VV[0],VV[1],VV[2],VV[3],VV[4],VV[5],VV[6],VV[7]);
#define print_V16(VV) printf("%x %x %x %x %x %x %x %x\n",VV[0],VV[1],VV[2],VV[3],VV[4],VV[5],VV[6],VV[7]); \
    printf("   %x %x %x %x %x %x %x %x\n",VV[0+8],VV[1+8],VV[2+8],VV[3+8],VV[4+8],VV[5+8],VV[6+8],VV[7+8]);
#endif

#endif
