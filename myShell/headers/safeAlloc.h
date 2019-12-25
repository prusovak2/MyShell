#ifndef SAFE_ALLOC_H
#define SAFE_ALLOC_H
#include <stdlib.h>
#include <err.h>

#define SAFE_MALLOC(pointer, numElements) \
    do \
    { \
        pointer = malloc(sizeof(*(pointer)) * numElements); \
        if(pointer ==NULL) \
        { \
            errx(1,"Out of memory"); \
        } \
    } \
    while(0)

#define SAFE_CALLOC(pointer, numElements) \
    do \
    { \
        pointer = calloc(numElements, sizeof(*(pointer))); \
        if(pointer ==NULL) \
        { \
            errx(1,"Out of memory"); \
        } \
    } \
    while(0)

#define SAFE_REALLOC(pointer, numElements) \
    do \
    { \
        pointer = realloc(pointer, sizeof(*(pointer)) * numElements); \
        if(pointer ==NULL) \
        { \
            errx(1,"Out of memory"); \
        } \
    } \
    while(0)

#endif