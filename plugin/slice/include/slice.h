#ifndef _SLICE_H_
#define _SLICE_H_


#include <stdint.h>
#include <glib.h>
#include "data.h"


/* The exported interface to interact with this plugin. */
/* The function pointer types. */
typedef int8_t (*func_SlcInit) ();
typedef int8_t (*func_SlcDeinit) ();
typedef int8_t (*func_SlcGetFileSlice) (char*, uint16_t, GPtrArray**);

/* The integrated structure to store exported functions. */
typedef struct _PLUGIN_SLICE_T {
    void *hdle_Lib;
    func_SlcInit Init;
    func_SlcDeinit Deinit;
    func_SlcGetFileSlice GetFileSlice;
} PLUGIN_SLICE;

/* The function name symbols. */
#define SYM_SLC_INIT                "SlcInit"
#define SYM_SLC_DEINIT              "SlcDeinit"
#define SYM_SLC_GET_FILE_SLICE      "SlcGetFileSlice"


/**
 * This function initializes the file slicing plugin.
 *
 * @return status code
 */
int8_t
SlcInit();


/**
 * This function releases the resources allocated by file slicing plugin.
 *
 * @return status code
 */
int8_t
SlcDeinit();


/**
 * This function splits the given file into slices and returns an array
 * of SLICE structures to locate these slices.
 * 
 * @param szPathFile    The absoluate pathname of the given file.
 * @param usSizeSlc     The size of the to be created slice.
 * @param p_aSlc        The pointer to the GPtrArray which is to be filled
 *                      with SLICE structures.
 *
 * @return status code
 */
int8_t
SlcGetFileSlice(char *szPathFile, uint16_t usSizeSlc, GPtrArray **p_aSlc);


#endif
