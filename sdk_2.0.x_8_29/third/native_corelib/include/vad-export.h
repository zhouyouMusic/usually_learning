/********************************************************************
 * Copyright 2017- Author: Frank Phillips All Rights Reserved.
 * NOTICE:  All information contained herein is, and remains
 * the property of the author. The intellectual and technical concepts
 * contained herein are proprietary to the author. Dissemination of
 * this information or reproduction of this material is strictly
 * forbidden unless prior written permission is obtained from the author.
 */

// Copyright 2017 Frank Phillips
// can not be copied and/or distributed without the express
// permission of Frank Phillips
//
/** @file vad-export.h
 *  @brief Function prototypes for the VAD
 *
 *  @author Frank Phillips
 */

#ifndef __YY_VAD_EXPORT_H__
#define __YY_VAD_EXPORT_H__

#ifdef __cplusplus
extern "C" {
#endif

enum VadState {
    kVadStateSilence = 0,    /* 未开始说话 */
    kVadStateSpeaking,       /* 正在说话 */
    kVadStateEnd             /* 说话结束 */
};

struct EndpointConfig {
    float minimum_speech_length;   /* 参考时长(s)：该音频时长在 参考时长 后才开始检测 停止说话 状态 */
    float trailing_silence;        /* 检测精度(s)：停止说话后多长时间返回 停止说话 状态值 */
};

/* 判断状态变化的时间窗口大小可配置
struct vad *vad_new(const char *cfg);
int vad_delete(struct vad *vad);
int vad_start(struct vad *vad, char *param);
enum vad_state vad_feed(struct vad *vad, const void *data, int size);
int vad_stop(struct vad *vad);
*/

/// Vad structure type
typedef struct Vad Vad;

/** @brief VadNew produce a new Vad
 *
 */
__attribute ((visibility("default"))) Vad* VadNew();

__attribute ((visibility("default"))) void VadSetEndpointConfig(struct Vad *vad,struct EndpointConfig config);

__attribute ((visibility("default"))) enum VadState VadGetState(Vad *vad);

/** @brief Initialization after creating new Vad object
 *
 *  @param vad Pointer to Vad object
 *  @param waittime_end seconds
 */
__attribute ((visibility("default"))) void VadStart(Vad *vad);

/** @brief Process audio in a streaming way
 *
 *  @param vad Pointer to Vad object
 *  @param buf A buffer which holvad raw audio data
 *  @param buf_size Tell the size of the buffer, how many data actually reside in the buffer
 */
__attribute ((visibility("default"))) enum VadState VadAppend(Vad *vad, const short *buf, int buf_size);

/**
 * @brief Number of frames ready
 *
 */
__attribute ((visibility("default"))) int VadNumFramesReady(Vad *vad);

/**
 *
 * Label frame as speech or silence
 * 0 means silence 1 means speech
 */
__attribute ((visibility("default"))) int VadLabelFrame(Vad *vad, int frame);

/** @brief Flush the last piece of data and some extra computation to get the result
 *
 *  @param vad Pointer to Vad object
 */
__attribute ((visibility("default"))) void VadEnd(Vad *vad);

/** @brief Get output in JSON format
 *
 *  @param vad Pointer to Vad object
 */
__attribute ((visibility("default"))) char const* VadGetOutput(Vad *vad);

/** @brief Destroy a Vad object when the job is finished
 *
 *  @param vad Pointer to Vad object
 */
__attribute ((visibility("default"))) void VadDestroy(Vad* vad);


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __YY_VAD_EXPORT_H__ */
