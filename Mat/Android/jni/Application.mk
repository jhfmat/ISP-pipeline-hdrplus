# Build both ARMv5TE and ARMv7-A machine code.
#NDK_TOOLCHAIN_VERSION := 4.9
APP_PLATFORM := android-22
APP_ABI    := arm64-v8a
APP_OPTIM  := release  
APP_STL    := c++_static
OPT_CFLAGS := -O3 -fno-exceptions -fno-bounds-check -funroll-loops -funswitch-loops -fvisibility=default \
              -finline-functions -funsafe-loop-optimizations
              
OPT_CPPFLAGS := $(OPT_CLFAGS)  
#APP_LDFLAGS := -static-libstdc++
APP_CFLAGS  := $(APP_CFLAGS) $(OPT_CFLAGS) -frtti -std=c++11 
APP_CPPFLAGS := $(APP_CPPFLAGS) $(OPT_CPPFLAGS) 
