#pragma once

#include "include.h"

namespace Parameters{
	int argc;
	char ** argv;
}

int SplitTriangles(Triangle * primitives, int length, int a, float v){
	int i, j;
	i = 0;
	j = length-1;
	Triangle buff;
	
	while(i < j){
		
		while(i < length){
			if(primitives[i].mid[a] > v){
				break;
			}
			++i;
		}
		
		while(j >= 0){
			if(primitives[j].mid[a] <= v){
				break;
			}
			--j;
		}
		
		if(i < j){
			buff = primitives[i];
			primitives[i] = primitives[j];
			primitives[j] = buff;
		}
	}
	return i;
}

void GetTrianglesAABB(Triangle * primitives, unsigned int length, AABB * aabb){
	
	
	unsigned int i = 0;
	AABB buff;
	
	((Triangle)primitives[0]).Bounds(aabb);
	
	while(i < length){
		primitives[i].Bounds(&buff);
		vMin(aabb->min, buff.min);
		vMax(aabb->max, buff.max);
		++ i;
	}
}

void SortTriangles(Triangle * primitives, int length, int axis){
    struct {
        bool operator()(Triangle a, Triangle b) const
        {
            return a.mid[0] < b.mid[0];
        }
    } customLessX;
    struct {
        bool operator()(Triangle a, Triangle b) const
        {
            return a.mid[1] < b.mid[1];
        }
    } customLessY;
    struct {
        bool operator()(Triangle a, Triangle b) const
        {
            return a.mid[2] < b.mid[2];
        }
    } customLessZ;
    switch(axis){
        case 0:
            std::sort(primitives, primitives + length, customLessX);
            break;
        case 1:
            std::sort(primitives, primitives + length, customLessY);
            break;
        case 2:
            std::sort(primitives, primitives + length, customLessZ);
            break;
    }
}

scalar NewtonsMethod(scalar (*f)(scalar), scalar(*f_)(scalar), scalar start, scalar result, int steps){
	int i = 0;
	scalar x0 = start;
	scalar xn;
	while(i < steps){
		xn = x0-(f(x0)-result)/f_(x0);
		if(fabs(xn-x0) < 0.01){
			return xn;
		}
		x0 = xn;
		i ++;
	}
	return xn;
}
