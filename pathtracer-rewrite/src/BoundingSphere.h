#pragma once
#include "include.h"

class BoundingSphere{
public:
    scalar o[3];
    scalar radius;

    BoundingSphere(){
        o[0] = 0;
        o[1] = 0;
        o[2] = 0;
        radius = 0;
    }

    BoundingSphere(Triangle * mesh, int length, int n = 16){
        AABB aabb; //rough aabb aproximation....
        GetTrianglesAABB(mesh, length, &aabb);
        vCpy(this->o, aabb.min);
        vAdd(this->o, aabb.max);
        vMulF(this->o, 0.5);
        scalar buf[3];
        vCpy(buf, aabb.max);
        vSub(buf, this->o);
        this->radius = vLength(buf);
    }

};