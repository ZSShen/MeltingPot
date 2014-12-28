#ifndef _SIMILARITY_NGRAM_H_
#define _SIMILARITY_NGRAM_H_


#define MARKER_HIT                  (1)
#define MARKER_MISS                 (-1)

#ifndef BLOOM_FILTER_WIDTH
    #define BLOOM_FILTER_WIDTH      (256)   /* Default bitvector size. */
#endif
#ifndef NGRAM_DIMENSION
    #define NGRAM_DIMENSION         (1)     /* Default is unigram matching. */
#endif


#endif
