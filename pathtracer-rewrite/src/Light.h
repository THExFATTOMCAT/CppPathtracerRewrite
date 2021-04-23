#pragma once
#include "include.h"

class Light{
public:
	scalar color[3];
	scalar intensity;

	virtual void ConnectTo(scalar * loc, scalar * result) = 0;
	virtual void ShadowRayTo(scalar * loc, Ray * shadow_ray, scalar * min);
	virtual scalar SampleRay(Ray * ray) = 0;
	virtual scalar GetMin(scalar * loc) = 0;
	virtual scalar GetFalloff(scalar d) = 0;
};