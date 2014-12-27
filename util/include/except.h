#ifndef _EXCEPT_H_
#define _EXCEPT_H_


#define SUCCESS             (0)   /* The task bundling external API finishes successfully. */
#define FAIL_FILE_IO        (-10) /* The general exception regarding file IO. */
#define FAIL_MEM_ALLOC      (-20) /* The general exception regarding memory allocation. */
#define FAIL_MISC           (-30) /* The exception regarding miscellaneous utility tasks. */
#define FAIL_EXT_LIB_CALL   (FAIL_MISC - 1) /* Fail to call external shared library. */
#define FAIL_PLUGIN_RESOLVE (FAIL_MISC - 2) /* Fail to resolve cluster module. */
#define FAIL_OPT_PARSE      (FAIL_MISC - 3) /* Fail to parse command line option. */
#define FAIL_CONF_PARSE     (FAIL_MISC - 4) /* Fail to parse cluster configuration. */
#define FAIL_FILE_FORMAT    (FAIL_MISC - 5) /* Fail to resolve the given file. */


#endif
