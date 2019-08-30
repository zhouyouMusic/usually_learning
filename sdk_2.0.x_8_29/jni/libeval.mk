include $(CLEAR_VARS)

ifeq ($(TARGET_ARCH_ABI), $(filter $(TARGET_ARCH_ABI), armeabi armeabi-v7a))
    CORE_LIB = corelib
endif
ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
    CORE_LIB = corelib64
endif
ifeq ($(TARGET_ARCH_ABI),x86)
    CORE_LIB = i686
endif
ifeq ($(TARGET_ARCH_ABI),x86_64)
    CORE_LIB = x86_64
endif

LOCAL_MODULE    := kaldi-base
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/kaldi-base.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE    := kaldi-cudamatrix
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/kaldi-cudamatrix.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE    := kaldi-decoder
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/kaldi-decoder.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE    := kaldi-feat
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/kaldi-feat.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE    := kaldi-fstext
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/kaldi-fstext.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE    := kaldi-hmm
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/kaldi-hmm.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE    := kaldi-lat
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/kaldi-lat.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE    := kaldi-matrix
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/kaldi-matrix.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE    := kaldi-nnet3
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/kaldi-nnet3.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE    := kaldi-online2
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/kaldi-online2.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE    := kaldi-transform
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/kaldi-transform.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE    := kaldi-tree
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/kaldi-tree.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE    := kaldi-util
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/kaldi-util.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE    := libfst
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/libfst.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE    := libopenblas
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/libopenblas.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE    := libre2
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/libre2.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE    := libsphinxbase
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/libsphinxbase.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE    := yy-decoder
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/yy-decoder.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE    := yy-nnet3
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/yy-nnet3.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE    := yy-scorer
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/yy-scorer.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE    := yy-utils
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/yy-utils.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE    := yy-vad
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/yy-vad.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE    := yy-feat
LOCAL_SRC_FILES  := $(LOCAL_PATH)/third/native_corelib/android/$(CORE_LIB)/yy-feat.a
include $(PREBUILT_STATIC_LIBRARY)