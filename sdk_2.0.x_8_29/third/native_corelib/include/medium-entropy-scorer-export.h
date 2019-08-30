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
/** @file medium-entropy-scorer-export.h
 *  @brief Function prototypes for the open ended scorer
 *  @author Frank Phillips
 */

#ifndef __YY_MEDIUM_ENTROPY_SCORER_EXPORT_H__
#define __YY_MEDIUM_ENTROPY_SCORER_EXPORT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    kElementarySchool,   // 0
    kMiddleSchool,       // 1
    kHighSchool,         // 2
} QuestionClass;

/*
 * 短文朗读 0
 * 短文跟读 1
 * 句子翻译 2
 * 段落翻译 3
 * 故事复述 4
 * 看图说话 5
 * 回答问题 6
 * 口头作文 7
 */
typedef enum {
    kParagraphRead,         // 0
    kParagraphFollow,       // 1
    kSentenceTranslation,   // 2
    kParagraphTranslation,  // 3
    kStoryRetell,           // 4
    kPictureTalk,           // 5
    kQuestionsAndAnswers,   // 6
    kOralEssay              // 7
} QuestionType;

/// MediumEntropyScorer structure type for open ended score
typedef struct MediumEntropyScorer MediumEntropyScorer;

/** @brief Initialize an MediumEntropyScorer object by qid
 */
__attribute ((visibility("default"))) MediumEntropyScorer* MediumEntropyScorerNew(char const *req_header);


/** @brief Initialization after creating new MediumEntropyScorer object
 *
 *  @param scorer Pointer to MediumEntropyScorer object
 */
__attribute ((visibility("default"))) void MediumEntropyScorerStart(MediumEntropyScorer *scorer);

/** @brief Process audio in a streaming way
 *
 *  @param scorer Pointer to MediumEntropyScorer object
 *  @param buf A buffer which holds raw audio data
 *  @param buf_size Tell the size of the buffer, how many data actually reside in the buffer
 */
__attribute ((visibility("default"))) int MediumEntropyScorerAppend(MediumEntropyScorer *scorer,
                         const short *buf,
                         size_t buf_size);

/** @brief Flush the last piece of data and some extra computation to get the result
 *  @param scorer Pointer to MediumEntropyScorer object
 */
__attribute ((visibility("default"))) void MediumEntropyScorerEnd(MediumEntropyScorer *scorer);

/** @brief Get output in JSON format
 *  @param scorer Pointer to MediumEntropyScorer object
 */
__attribute ((visibility("default"))) char const* MediumEntropyScorerGetOutput(MediumEntropyScorer *scorer);

/** @brief Destroy a MediumEntropyScorer object when the job is finished
 *  @param scorer Pointer to MediumEntropyScorer object
 */
__attribute ((visibility("default"))) void MediumEntropyScorerDestroy(MediumEntropyScorer* scorer);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __YY_MEDIUM_ENTROPY_SCORER_H__ */

