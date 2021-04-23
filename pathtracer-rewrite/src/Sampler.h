#pragma once

#include "include.h"

class Sampler{
public:
    int seed;
    void Sample1D(scalar * result){result[0] = fastRand();};
    void Sample2D(scalar * result){result[0] = fastRand(); result[1] = fastRand();};
    void Sample3D(scalar * result){result[0] = fastRand(); result[1] = fastRand(); result[2] = fastRand();};

    Sampler * clone(){
        Sampler * result = (Sampler*) std::malloc(sizeof(Sampler));
        memcpy(result, this, sizeof(Sampler));
        result->seed = rand();
        return result;
    };
};