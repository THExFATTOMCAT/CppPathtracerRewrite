#pragma once
#include "include.h"

class Accelerator{
public:
	virtual void IntersectRay(Ray * ray, scalar * min, Hit * hit, Triangle * primitives, bool shadow_ray){std::cout << "wrong one !!\n";};
	virtual void IntersectRayLoop(Ray * ray, scalar * min, Hit * hit, Triangle * primitives, bool shadow_ray);
	
	virtual void Build(Triangle * primitives, int length){std::cout << "wrong one\n";};
};