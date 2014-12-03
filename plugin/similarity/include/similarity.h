#ifndef _SIMILARITY_H_
#define _SIMILARITY_H_


#include <stdint.h>


#define FAIL_EXTERNAL_LIBRARY_CALL "Fail to call external library"


enum {
    SIM_SUCCESS = 0,
    SIM_FAIL_FILE_IO = -1,
    SIM_FAIL_MEM_ALLOC = -2,
    SIM_FAIL_LIBRARY_CALL = -3
};


/* The exported interface to interact with this plugin. */
/* The function pointer types. */
typedef int8_t (*func_SimInit) ();
typedef int8_t (*func_SimDeinit) ();
typedef int8_t (*func_SimGetHash) (char*, uint32_t, char**, uint32_t*);
typedef int8_t (*func_SimCompareHashPair) (char*, uint32_t, char*, uint32_t, uint8_t*);

/* The integrated structure to store exported functions. */
typedef struct _PLUGIN_SIMILARITY_T {
    void *hdle_Lib;
    func_SimInit Init;
    func_SimDeinit Deinit;
    func_SimGetHash GetHash;
    func_SimCompareHashPair CompareHashPair;
} PLUGIN_SIMILARITY;

/* The function name symbols. */
#define SYM_SIM_INIT                "SimInit"
#define SYM_SIM_DEINIT              "SimDeinit"
#define SYM_SIM_GET_HASH            "SimGetHash"
#define SYM_SIM_COMPARE_HASH_PAIR   "SimCompareHashPair"


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
