#pragma once

#include "include.h"

unsigned long primitive_tests = 0;
unsigned long primitive_intersections = 0;

class Primitive{
public:
	Material * material;
	scalar mid[3];
	
	virtual void GetNormal(scalar * uvs, scalar * normal){};
	virtual void Bounds(AABB * aabb){std::cout << "Primitive::Bounds()\n";};
	virtual bool IntersectRay(Ray * ray, scalar min, Primitive * exclude, Hit * hit) = 0;
};