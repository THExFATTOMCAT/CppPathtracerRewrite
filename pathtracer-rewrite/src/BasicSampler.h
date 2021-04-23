#pragma once
#include "include.h"

class BasicSampler : public Sampler{

public:
    BasicSampler * clone(){
        BasicSampler * result = (BasicSampler *) std::malloc(sizeof(BasicSampler));
        memcpy(result, this, sizeof(BasicSampler));
        return result;
    }

};