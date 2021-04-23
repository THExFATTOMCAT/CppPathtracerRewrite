#pragma once

#include "include.h"

class PointLight : public Light{
public:
    scalar pos[3];
    PointLight(scalar intensity, scalar * color, scalar * pos){
        this->intensity = intensity / (4*M_PI);
        vCpy(this->color, color);
        vCpy(this->pos, pos);
    }

    void ConnectTo(scalar * loc, scalar * result){
        vCpy(result, loc);
        vSub(result, pos);
        vFlip(result);
        return;
    }

    scalar GetMin(scalar * loc){
        scalar result[3];
        ConnectTo(loc, result);
        return vLength(result);
    }

    void ShadowRayTo(scalar * loc, Ray * shadow_ray, scalar * min) {
        vCpy(shadow_ray->o, this->pos);
        vCpy(shadow_ray->d, loc);
        vSub(shadow_ray->d, shadow_ray->o);

        *min = vLength(shadow_ray->d);
        vDivF(shadow_ray->d, *min);

    }

    scalar GetFalloff(scalar d){
        return 1.0/(d*d);
    }

    scalar SampleRay(Ray * ray){
        vCpy(ray->o, pos);
        vRandomize(ray->d, 1);
        return 1;
    }

};