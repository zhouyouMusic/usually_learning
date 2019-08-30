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
//

/** @file audio-align-scorer-export.h
 *  @brief Function prototypes for the reading aloud scorer
 *
 *  @author Frank Phillips
 */

#ifndef __YY_AUDIO_ALIGN_SCORER_EXPORT_H__
#define __YY_AUDIO_ALIGN_SCORER_EXPORT_H__

#ifdef __cplusplus
extern "C" {
#endif

/// AudioAlignScorer structure type for read aloud score
typedef struct AudioAlignScorer AudioAlignScorer;

/** @brief Creating new AudioAlignScorer object
 *
 *  @param req_header Request header
 */
__attribute ((visibility("default"))) AudioAlignScorer* AudioAlignScorerNew(char const *req_header);

/** @brief Initialization after creating new AudioAlignScorer object
 *
 *  @param aas Pointer to AudioAlignScorer object
 *  @param ref_text Reference text
 */
__attribute ((visibility("default"))) void AudioAlignScorerStart(AudioAlignScorer *aas);
__attribute ((visibility("default"))) void AudioAlignScorerStartNewReferenceAudio(AudioAlignScorer *aas, char const *url);

/** @brief Process audio in a streaming way
 *
 *  @param aas Pointer to AudioAlignScorer object
 *  @param buf A buffer which holds raw audio data
 *  @param buf_size Tell the size of the buffer, how many data actually reside in the buffer
 */
__attribute ((visibility("default"))) int AudioAlignScorerAppend(AudioAlignScorer *aas,
                         const short *buf,
                         size_t buf_size);

/** @brief Flush the last piece of data and some extra computation to get the result
 *
 *  @param aas Pointer to AudioAlignScorer object
 */
__attribute ((visibility("default"))) void AudioAlignScorerEnd(AudioAlignScorer *aas);

/** @brief Get output in JSON format
 *
 *  @param aas Pointer to AudioAlignScorer object
 */
__attribute ((visibility("default"))) char const* AudioAlignScorerGetOutput(AudioAlignScorer *aas);

/** @brief Destroy a AudioAlignScorer object when the job is finished
 *  
 *  @param aas Pointer to AudioAlignScorer object
 */
__attribute ((visibility("default"))) void AudioAlignScorerDestroy(AudioAlignScorer* aas);


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __YY_AUDIO_ALIGNER_SCORER_H__ */

