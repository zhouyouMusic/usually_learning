include $(CLEAR_VARS)
LOCAL_MODULE    := libopus

LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/opus_libs/android/libs/$(TARGET_ARCH_ABI)/libopus.a

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/third/opus-1.3.1/include $(LOCAL_PATH)/third/openssl/include 

include $(PREBUILT_STATIC_LIBRARY)
