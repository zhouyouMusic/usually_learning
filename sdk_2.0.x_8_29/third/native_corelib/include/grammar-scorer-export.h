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
/** @file grammar-scorer.h
 *  @brief Function prototypes for the reading aloud scorer
 *
 *  @author Frank Phillips
 */

#ifndef __YY_GRAMMAR_SCORER_H__
#define __YY_GRAMMAR_SCORER_H__

#ifdef __cplusplus
extern "C" {
#endif

/// GrammarScorer structure type for JSGF recognition
typedef struct GrammarScorer GrammarScorer;

__attribute ((visibility("default"))) GrammarScorer* GrammarScorerNew();

/** @brief Initialization after creating new GrammarScorer object
 *
 *  @param scorer Pointer to GrammarScorer object
 *  @param jsgf_grammar A string which defines a JSGF grammar
 *         for JSGF reference, see http://www.w3.org/TR/jsgf/
 *  @return return value 0 means OK, 1 means JSGF syntax error
 */
__attribute ((visibility("default"))) int GrammarScorerStart(GrammarScorer *scorer, char const *jsgf_grammar);

/**
 * @brief Single choice
 * @param scorer Pointer to GrammarScorer object
 * @param choices e.g. (may i help you) | (what can i do for you) | (may i offer some help to you) | (please do me a favor);
 */
__attribute ((visibility("default"))) int GrammarScorerStartSingleChoice(GrammarScorer *scorer, char const *choices);

/** @brief Process audio in a streaming way
 *
 *  @param scorer Pointer to GrammarScorer object
 *  @param buf A buffer which holds raw audio data
 *  @param buf_size Tell the size of the buffer, how many data actually reside in the buffer
 */
__attribute ((visibility("default"))) int GrammarScorerAppend(GrammarScorer *scorer,
                         const short *buf,
                         size_t buf_size
                         );

/** @brief Flush the last piece of data and some extra computation to get the result
 *
 *  @param scorer Pointer to GrammarScorer object
 */
__attribute ((visibility("default"))) void GrammarScorerEnd(GrammarScorer *scorer);

/** @brief Get output in JSON format
 *
 *  @param scorer Pointer to GrammarScorer object
 */
__attribute ((visibility("default"))) char const* GrammarScorerGetOutput(GrammarScorer *scorer);

/** @brief Destroy a GrammarScorer object when the job is finished
 *  
 *  @param scorer Pointer to GrammarScorer object
 */
__attribute ((visibility("default"))) void GrammarScorerDestroy(GrammarScorer* scorer);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __YY_GRAMMAR_SCORER_H__ */

