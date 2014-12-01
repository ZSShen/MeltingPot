#ifndef _SIMILARITY_H_
#define _SIMILARITY_H_


#include <stdint.h>


#define FAIL_MEM_ALLOC_HASH_STRING      "Fail to allocate buffer for hash string"
#define FAIL_EXTERNAL_LIBRARY_CALL      "Fail to call the library for similarity computation"


enum {
    SIM_SUCCESS = 0,
    SIM_FAIL_FILE_IO = -1,
    SIM_FAIL_MEM_ALLOC = -2,
    SIM_FAIL_LIBRARY_CALL = -3
};


/**
 * This function initializes the similarity comparison plugin.
 *
 * @return status code
 */
int8_t
SimInit();


/**
 * This function releases the resources allocated by similarity comparison plugin.
 *
 * @return status code
 */
int8_t
SimDeinit();


/**
 * This function computes the hash string derived from the input binary sequence.
 *
 * @param szBin         The pointer to the buffer storing binary sequence.
 * @param uiLenBuf      The length of the binary sequence.
 * @param p_szHash      The pointer to the pointer which points to the result 
 *                      hash string.
 * @param p_uiLenHash   The pointer to the varialbe which should be updated 
 *                      with the length of the hash string.
 * 
 * @return status code
 */
int8_t
SimGetHash(char *szBin, uint32_t uiLenBuf, char **p_szHash, uint32_t *p_uiLenHash);


/**
 * This function computes the similarity score for a given hash pair.
 *
 * @param szHashSrc     The pointer to the source hash string.
 * @param uiLenSrc      The length of the source hash string.
 * @param szHashTge     The pointer to the target hash string.
 * @param uiLenTge      The length of the target hash string.
 * @param p_ucSim       The pointer to the variable which should be updated
 *                      with the similarity score.
 * 
 * @return status code
 */
int8_t
SimCompareHashPair(char *szHashSrc, uint32_t uiLenSrc, 
                   char *szHashTge, uint32_t uiLenTge, uint8_t *p_ucSim);


#endif
