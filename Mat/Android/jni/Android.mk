
LOCAL_PATH := $(call my-dir)
SOLUTION_PATH := $(LOCAL_PATH)/../../../

include $(CLEAR_VARS)
LOCAL_MODULE     := libMat
EXCLUDE_FILES    := 
MY_PREFIX        := $(LOCAL_PATH)
SRC_Mat          :=  $(wildcard $(SOLUTION_PATH)Mat/*.cpp               )          
ALL_SOURCES      := $(SRC_Mat)


EXCLUDE_SRCS     := $(foreach filex, $(EXCLUDE_FILES), $(MY_PREFIX)/../$(filex))
EXCLUDE_SRCS 	 += 
MY_SOURCES       := $(filter-out $(EXCLUDE_SRCS), $(ALL_SOURCES))  
LOCAL_SRC_FILES  := $(MY_SOURCES:$(MY_PREFIX)/%=%)
LOCAL_CFLAGS     := -Wno-write-strings -fopenmp -ffast-math
LOCAL_CXXFLAGS   := -fopenmp -ffast-math
$(warning "TARGET_ARCH_ABI = $(TARGET_ARCH_ABI)")
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_CFLAGS    += -mfloat-abi=softfp -mfpu=neon -march=armv7
else ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
LOCAL_CFLAGS    += -march=armv8-a
endif
LOCAL_C_INCLUDES := $(SOLUTION_PATH)Mat/ 
LOCAL_STATIC_LIBRARIES := 
LOCAL_SHARED_LIBRARIES := 
LOCAL_LDLIBS := -llog -fopenmp

include $(BUILD_SHARED_LIBRARY)