LOCAL_PATH := $(call my-dir)/..

include libopus.mk
ifeq ($(USE_NATIVE), 1)
include libeval.mk
endif
include $(CLEAR_VARS)

LOCAL_MODULE    := libskegn

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include $(LOCAL_PATH)/src $(LOCAL_PATH)/src/egn \
                    $(LOCAL_PATH)/third $(LOCAL_PATH)/third/libogg-1.3.3/include

LOCAL_SRC_FILES  := src/sdk/skegn.c \
					src/sdk/skegn_jni.c \
					src/egn/sgn_engine.c \
					src/egn/sgn_event.c \
					src/egn/lib/sgn_buf.c \
					src/egn/lib/sgn_msg_queue.c \
					src/egn/lib/sgn_secureconf.c \
					src/egn/lib/sgn_uuid.c \
					src/egn/lib/sgn_dbg.c \
					src/egn/lib/sgn_sha1.c \
					src/egn/lib/sgn_hmac.c \
					src/egn/lib/sgn_sound_intensity.c \
					src/egn/cloud/sgn_opus_enc.c \
					src/egn/platform/sgn_common.c \
					src/egn/platform/sgn_android.c \
					third/mongoose/mongoose.c \
				   	third/cJSON/cJSON.c \
				   	third/xxtea/sgn_secure_code.c \
				   	third/libogg-1.3.3/src/bitwise.c \
				   	third/libogg-1.3.3/src/framing.c

#LOCAL_SHARED_LIBRARIES += opustool


LOCAL_LDLIBS 	 := -lz	-pthread -llog -pie -fPIE -Wl,--gc-sections -shared
LOCAL_CFLAGS     := -D MG_WEBSOCKET_PING_INTERVAL_SECONDS=3 -pie -fPIE
LOCAL_CPPFLAGS   := -std=c++11
LOCAL_STATIC_LIBRARIES += libopus

ifeq ($(USE_SSL), 1)
LOCAL_LDLIBS += -lssl -lcrypto
ifeq ($(TARGET_ARCH_ABI),armeabi)
	LOCAL_LDLIBS += -L ../third/openssl/libs/android/armv6-vfp/  -lssl  -lcrypto
endif
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
	LOCAL_LDLIBS += -L ../third/openssl/libs/android/armv7a-neon -lssl  -lcrypto
endif
ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
	LOCAL_LDLIBS += -L ../third/openssl/libs/android/aarch64 -lssl  -lcrypto
endif
ifeq ($(TARGET_ARCH_ABI),x86)
	LOCAL_LDLIBS += -L ../third/openssl/libs/android/i686 -lssl  -lcrypto
endif
ifeq ($(TARGET_ARCH_ABI),x86_64)
	LOCAL_LDLIBS += -L ../third/openssl/libs/android/x86_64 -lssl  -lcrypto
endif
ifeq ($(TARGET_ARCH_ABI),mips)
	LOCAL_LDLIBS += -L ../third/openssl/libs/android/mips -lssl  -lcrypto
endif
ifeq ($(TARGET_ARCH_ABI),mips64)
	LOCAL_LDLIBS += -L ../third/openssl/libs/android/mips64 -lssl  -lcrypto
endif
endif


ifeq ($(USE_NATIVE), 1)
LOCAL_CFLAGS     += -D USE_NATIVE -D USE_NATIVE_EVAL -D USE_NATIVE_ALI -D USE_NATIVE_REC -D USE_NATIVE_VAD
ifneq ($(TARGET_ARCH_ABI), $(filter $(TARGET_ARCH_ABI), mips mips64))
LOCAL_SRC_FILES  += src/egn/lib/sgn_instance.c \
                   	src/egn/lib/sgn_file.c \
                   	src/egn/native/sgn_native_ali_module.c \
                   	src/egn/native/sgn_native_eval_module.c \
                   	src/egn/native/sgn_native_open_module.c \
                   	src/egn/native/sgn_native_rec_module.c \
                   	src/egn/native/sgn_native_vad_module.c \
                   	src/egn/native/sgn_native.c \
					src/egn/native/sgn_native_event.c \
                   	src/egn/native/sgn_auth.c 
LOCAL_WHOLE_STATIC_LIBRARIES += kaldi-base kaldi-cudamatrix kaldi-decoder kaldi-feat kaldi-fstext \
						  kaldi-hmm kaldi-lat kaldi-matrix kaldi-nnet3 kaldi-online2 kaldi-transform \
						  kaldi-tree kaldi-util libfst libopenblas libre2 libsphinxbase yy-decoder \
						  yy-nnet3 yy-scorer yy-utils yy-vad yy-feat gnustl_static 
endif
endif

ifeq ($(USE_NATIVE), 2)
LOCAL_SRC_FILES  += src/egn/lib/sgn_instance.c \
                   	src/egn/lib/sgn_file.c \
                   	src/egn/native/sgn_native_ali_module.c \
                   	src/egn/native/sgn_native_eval_module.c \
                   	src/egn/native/sgn_native_open_module.c \
                   	src/egn/native/sgn_native_rec_module.c \
                   	src/egn/native/sgn_native.c \
                   	src/egn/native/sgn_auth.c 

LOCAL_LDLIBS += -frtti -fPIE -pie -shared -Wl,-whole-archive			
ifeq ($(TARGET_ARCH_ABI),armeabi)
    LOCAL_LDLIBS += $(shell find $(LOCAL_PATH)/third/native_corelib/android/corelib -name \*.a)
	LOCAL_LDLIBS += ../third/native_corelib/android/libstdc++.a 
endif
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
    LOCAL_LDLIBS += $(shell find $(LOCAL_PATH)/third/native_corelib/android/corelib -name \*.a)
	LOCAL_LDLIBS += ../third/native_corelib/android/libstdc++.a
endif
ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
    LOCAL_LDLIBS += $(shell find $(LOCAL_PATH)/third/native_corelib/android/corelib64 -name \*.a)
	LOCAL_STATIC_LIBRARIES += gnustl_static
endif
ifeq ($(TARGET_ARCH_ABI),x86)
    LOCAL_LDLIBS += $(shell find $(LOCAL_PATH)/third/native_corelib/android/i686 -name \*.a)
	LOCAL_STATIC_LIBRARIES += gnustl_static
endif
ifeq ($(TARGET_ARCH_ABI),x86_64)
    LOCAL_LDLIBS += $(shell find $(LOCAL_PATH)/third/native_corelib/android/x86_64 -name \*.a)
	LOCAL_STATIC_LIBRARIES += gnustl_static
endif
LOCAL_LDLIBS += -Wl,-no-whole-archive -Wl,--gc-sections 
endif

include $(BUILD_SHARED_LIBRARY)
