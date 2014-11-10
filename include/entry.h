#ifndef _ENTRY_H_
#define _ENTRY_H_


#define OPT_LONG_HELP               "help"
#define OPT_LONG_PATH_SAMPLE_SET    "input"
#define OPT_LONG_PATH_PATTERN       "output"
#define OPT_LONG_BLK_COUNT          "blkcount"   
#define OPT_LONG_BLK_SIZE           "blksize"
#define OPT_LONG_PARALLELITY        "parallelity"
#define OPT_LONG_SIMILARITY         "similarity"

#define OPT_HELP                    'h'
#define OPT_PATH_SAMPLE_SET         'i'
#define OPT_PATH_PATTERN            'o'
#define OPT_BLK_COUNT               'c'
#define OPT_BLK_SIZE                's'
#define OPT_PARALLELITY             'p'
#define OPT_SIMILARITY              't'

#define BUF_SIZE_LARGE              1024
#define BUF_SIZE_SMALL              64

#define EARLY_RETURN(rc)            rc = -1;        \
                                    print_usage();  \
                                    goto EXIT;

#endif
