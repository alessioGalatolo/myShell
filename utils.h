//common utilities
#ifndef DSOPP_SYNTHESIS_UTILS_H
#define DSOPP_SYNTHESIS_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define NULL_CHECK(x)\
    if((x) == NULL){\
        fprintf(stderr, "Error: null pointer\n");\
        return 0;\
    }

#define THREAD_CHECK(x)\
    if((x) != 0){return 0;}


#define FCLOSE(x)\
    if(x != NULL){if(fclose(x) == EOF) perror("fclose"); x = NULL;}

//macro to replace malloc calls, it checks requested size and malloc results.
//In case of failure prints error and does 'clean' commands, then returns 0
#define MALLOC(x, size, clean)\
    {\
        size_t s = size;\
        if((s) < 0){\
            fprintf(stderr, "Error: malloc of a negative number\n");\
            clean;\
            return 0;\
        } else {\
            if((s) == 0)\
                fprintf(stderr, "Warning: malloc of zero size\n");\
            if(((x) = malloc(s)) == NULL){\
                fprintf(stderr, "Error: malloc returned a null pointer\n");\
                clean;\
                return 0;\
            }\
        }\
    }

//macro to replace realloc calls, it checks requested size and realloc results
//In case of failure calls the commands passed in clean
#define REALLOC(x, s, clean)\
    if((s) < 0){\
        fprintf(stderr, "Error: tried to realloc a negative value\n");\
        clean;\
    } else if((s) == 0){\
        free(x);\
        (x) = NULL;\
    } else {\
        void* tmp = realloc(x, s);\
        if(tmp == NULL){\
            fprintf(stderr, "Error: realloc unable to allocate %ld memory. Returned a null pointer\n", s);\
            clean;\
        }\
        (x) = tmp;\
    }

//macro used to avoid double free on a variable
#define FREE(x)\
    if((x) != NULL){\
        free(x);\
        (x) = NULL;\
    }


#endif //DSOPP_SYNTHESIS_UTILS_H
