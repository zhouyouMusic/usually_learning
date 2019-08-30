PRO_NAME := record
PRO_FLAGS :=
ifeq ($(SYS_TYPE),ANDROID)
PRO_FLAGS += -D_ANDROID_ 
TEST_CFLAGS += -L../record/src/lib_android
else

ifeq ($(SYS_TYPE),R16_LINUX)
TEST_CFLAGS += -L../record/src/lib_R16

else

TEST_CFLAGS += -L../record/src/lib

endif
endif
TEST_CFLAGS := -lasound
