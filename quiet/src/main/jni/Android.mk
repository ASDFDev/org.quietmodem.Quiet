LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := complex-prebuilt
LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/libcomplex.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := quiet-prebuilt
LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/libquiet.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := liquid-prebuilt
LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/libliquid.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := fec-prebuilt
LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/libfec.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := jansson-prebuilt
LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/libjansson.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := quiet-jni
LOCAL_SRC_FILES := system.c profiles.c transmitter.c receiver.c opensl.c
LOCAL_CFLAGS := -fpic -std=c99 -g -Wall -Wextra -O2
LOCAL_LDFLAGS := -fpie -lOpenSLES -llog -lm
LOCAL_SHARED_LIBRARIES := quiet-prebuilt liquid-prebuilt fec-prebuilt jansson-prebuilt complex-prebuilt
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include/$(TARGET_ARCH_ABI)
include $(BUILD_SHARED_LIBRARY)
