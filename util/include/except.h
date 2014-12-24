#ifndef _EXCEPT_H_
#define _EXCEPT_H_


enum {
    SUCCESS = 0,
    FAIL_FILE_IO = -1,
    FAIL_MEM_ALLOC = -2,
    FAIL_OPT_PARSE = -3,
    FAIL_CONF_PARSE = -4,
    FAIL_PLUGIN_RESOLVE = -5,
    FAIL_PLUGIN_INTERACT = -6,
    FAIL_PROCESS = -7,
    FAIL_PTN_CREATE = -8,
};


#endif
