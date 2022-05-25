#ifndef RANDOM_GENERATORS_H
#define RANDOM_GENERATORS_H

#include <stdlib.h>
#include <time.h>

static float randomFloatTo(float limit) {
    return static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/limit));
}

static float randomFloatRange(float low, float high) {
    return rand() / static_cast<float>(RAND_MAX) * (high - low) + low;
}

static float randomCoinFlip (float a, float b) {
    srand(time(NULL));

    int r = rand()%2;

    if(r==0)
        return a;
    else
        return b;        
}
#endif // RANDOM_GENERATORS.H