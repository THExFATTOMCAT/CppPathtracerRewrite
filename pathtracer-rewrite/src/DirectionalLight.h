#pragma once

#include "include.h"

class DirectionalLight : public Light{
public:
    BoundingSphere * b_sphere;
    scalar direction[3]; //has to be normalized !
    DirectionalLight(scalar intensity, scalar * color, scalar * direction, BoundingSphere * b_sphere){
        this->intensity = intensity;
        vCpy(this->color, color);
        vCpy(this->direction, direction);
        this->b_sphere = b_sphere;
        vNormalize(this->direction);
    }


    void ConnectTo(scalar * loc, scalar * result){
        vCpy(result, direction);
        vFlip(result);
        return;
    }
    void ShadowRayTo(scalar * loc, Ray * shadow_ray, scalar * min){
        vCpy(shadow_ray->o, this->direction);
        vMulF(shadow_ray->o, -this->b_sphere->radius*2);
        vAdd(shadow_ray->o, loc);
        vCpy(shadow_ray->d, this->direction);
        *min = this->b_sphere->radius*2;
    }

    scalar GetMin(scalar * loc){
        return std::numeric_limits<scalar>::max();
    }

    scalar GetFalloff(scalar d){
        return 1.0;
    }

    scalar SampleRay(Ray * ray){
        scalar r = sqrt(fastRand());
        scalar phi = fastRand()*2*M_PI;
        scalar x = r * cos(phi) * b_sphere->radius;
        scalar y = r * sin(phi) * b_sphere->radius;

        scalar X[3];
        scalar Y[3];
        vRandomize(X, 1);
        vCross(X, direction, Y);
        vCross(direction, Y, X);

        vCpy(ray->o, direction);
        vMulF(ray->o, -b_sphere->radius*200); //*200 so the sun seems far away if the direct sensor illumination is calculated...
        vAdd(ray->o, b_sphere->o);
        vMulF(X, x);
        vMulF(Y, y);
        vAdd(ray->o, X);
        vAdd(ray->o, Y);

        vCpy(ray->d, direction);

        //vRandomize(ray->o, 1);
        //vMulF(ray->o, 1);
        //vRandomize(ray->d, 1);

        return M_PI*b_sphere->radius*b_sphere->radius;
    }
};