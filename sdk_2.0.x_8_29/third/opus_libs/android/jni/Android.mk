LOCAL_PATH := $(call my-dir)/..
include $(CLEAR_VARS)
include $(LOCAL_PATH)/celt_sources.mk
include $(LOCAL_PATH)/opus_sources.mk
include $(LOCAL_PATH)/silk_sources.mk
LOCAL_MODULE    := libopus

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include $(LOCAL_PATH)/src $(LOCAL_PATH)/silk \
                    $(LOCAL_PATH)/silk/float $(LOCAL_PATH)/celt $(OGG_DIR)/include \
                    $(LOCAL_PATH)/silk/fixed
LOCAL_SRC_FILES := $(CELT_SOURCES) $(SILK_SOURCES) $(SILK_SOURCES_FLOAT)\
                   $(OPUS_SOURCES) $(OPUS_SOURCES_FLOAT) src/repacketizer_demo.c

LOCAL_CFLAGS := -Ofast -flto -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 \
-DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 \
-DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1 -DLT_OBJDIR=".libs/" -DOPUS_BUILD -Drestrict=__restrict -DVAR_ARRAYS=1 \
-DFLOAT_APPROX=1 -DHAVE_LRINTF=1 -DHAVE_LRINT=1
LOCAL_CPPFLAGS      := -DBSD=1
LOCAL_LDLAGS   += -flto
# Note: OPUS enhanced DSP/NEON implementation is not yet compatible with arm64.
# Only add the appropriate defines for 32-bit arm architecture.
ifeq ($(TARGET_ARCH_ABI), $(filter $(TARGET_ARCH_ABI), armeabi armeabi-v7a arm64-v8a))
    LOCAL_SRC_FILES    += $(CELT_SOURCES_ARM)
    ifeq ($(TARGET_ARCH_ABI), arm64-v8a)
        LOCAL_SRC_FILES += $(CELT_SOURCES_ARM_NEON_INTR) $(SILK_SOURCES_ARM_NEON_INTR)
        LOCAL_CFLAGS    += -DOPUS_ARM_MAY_HAVE_NEON_INTR=1 -DOPUS_ARM_PRESUME_NEON_INTR=1 -DOPUS_ARM_PRESUME_AARCH64_NEON_INTR=1
    else
        #LOCAL_CFLAGS += -mfloat-abi=softfp -mfpu=neon
    endif
endif

ifeq ($(TARGET_ARCH_ABI), $(filter $(TARGET_ARCH_ABI), x86 x86_64))

    LOCAL_CFLAGS += -DOPUS_X86_MAY_HAVE_SSE=1  -DOPUS_X86_PRESUME_SSE=1 -DOPUS_X86_MAY_HAVE_SSE2=1 \
                    -DOPUS_X86_PRESUME_SSE2=1 -DCPU_INFO_BY_ASM=1
    LOCAL_SRC_FILES += $(CELT_SOURCES_SSE) $(CELT_SOURCES_SSE2)

    ifeq ($(TARGET_ARCH_ABI), x86_64)
        LOCAL_CFLAGS += -DOPUS_X86_MAY_HAVE_SSE4_1=1 -DOPUS_X86_MAY_HAVE_AVX=1 \
                        -DOPUS_HAVE_RTCD=1
        LOCAL_SRC_FILES += $(CELT_SOURCES_SSE4_1) $(SILK_SOURCES_SSE4_1)
    endif
endif

include $(BUILD_STATIC_LIBRARY)