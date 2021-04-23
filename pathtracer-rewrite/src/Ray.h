#pragma once

#include "include.h"

class Ray{
public:	
	scalar o[3];
	scalar d[3];
	scalar i[3];
	
	void UpdateInverse();
	void Trace(scalar t, scalar * r);
};

void Ray::UpdateInverse(){
	i[0] = (scalar)1/d[0];
	i[1] = (scalar)1/d[1];
	i[2] = (scalar)1/d[2];
}

void Ray::Trace(scalar t, scalar * r){
	r[0] = o[0]+t*d[0];
	r[1] = o[1]+t*d[1];
	r[2] = o[2]+t*d[2];
}
