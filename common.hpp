#ifndef common_h
#define common_h

#include <iostream>
#include <cstdio>
#include <cstring>
#include <stdint.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#ifdef ASSERT
#undef ASSERT
#endif

#define ASSERT(cond, cleanup, msg, ...)         \
    if(!(cond)) {                               \
        fprintf(stderr, msg, ##__VA_ARGS__);     \
        cleanup;                                \
        exit(EXIT_FAILURE);                     \
    }

#endif