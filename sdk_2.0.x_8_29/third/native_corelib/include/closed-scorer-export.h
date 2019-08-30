/********************************************************************
 * Copyright 2017- Author: Frank Phillips All Rights Reserved.
 * NOTICE:  All information contained herein is, and remains
 * the property of the author. The intellectual and technical concepts 
 * contained herein are proprietary to the author. Dissemination of 
 * this information or reproduction of this material is strictly 
 * forbidden unless prior written permission is obtained from the author.
 */

// Copyright 2017- Author: Frank Phillips All Rights Reserved.
// can not be copied and/or distributed without the express
// permission of Frank Phillips
//
//

/** @file closed-scorer.h
 *  @brief Function prototypes for the reading aloud scorer
 *
 *  @author Frank Phillips
 */

#ifndef __YY_CLOSED_SCORER_H__
#define __YY_CLOSED_SCORER_H__

#ifdef __cplusplus
extern "C" {
#endif

/// MinimumEntropyScorer structure type for read aloud score
typedef struct MinimumEntropyScorer MinimumEntropyScorer;

/** @brief Creating new MinimumEntropyScorer object
 *
 *  @param req_header Request header
 */
__attribute ((visibility("default"))) MinimumEntropyScorer* MinimumEntropyScorerNew(char const *req_header);

/** @brief Initialization after creating new MinimumEntropyScorer object
 *
 *  @param mes Pointer to MinimumEntropyScorer object
 *  @param ref_text Reference text
 */
__attribute ((visibility("default"))) void MinimumEntropyScorerStart(MinimumEntropyScorer *mins);
__attribute ((visibility("default"))) void MinimumEntropyScorerStartNewPrompt(MinimumEntropyScorer *mins, char const *ref_text);

/** @brief Process audio in a streaming way
 *
 *  @param mins Pointer to MinimumEntropyScorer object
 *  @param buf A buffer which holds raw audio data
 *  @param buf_size Tell the size of the buffer, how many data actually reside in the buffer
 */
__attribute ((visibility("default"))) int MinimumEntropyScorerAppend(MinimumEntropyScorer *mins,
                         const short *buf,
                         size_t buf_size);


/** @brief Flush the last piece of data and some extra computation to get the result
 *
 *  @param mins Pointer to MinimumEntropyScorer object
 */
__attribute ((visibility("default"))) void MinimumEntropyScorerEnd(MinimumEntropyScorer *mins);

/** @brief Get output in JSON format
 *
 *  @param mins Pointer to MinimumEntropyScorer object
 */
__attribute ((visibility("default"))) char const* MinimumEntropyScorerGetOutput(MinimumEntropyScorer *mins);

/** @brief Destroy a MinimumEntropyScorer object when the job is finished
 *  
 *  @param mins Pointer to MinimumEntropyScorer object
 */
__attribute ((visibility("default"))) void MinimumEntropyScorerDestroy(MinimumEntropyScorer* mins);


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __YY_CLOSED_SCORER_H__ */

