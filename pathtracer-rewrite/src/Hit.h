#pragma once

#include "include.h"
class Hit{
public:
	scalar uvs[2];
	scalar tex[2];
	scalar t;
	void * prim;
	bool hit;
	
	Hit(){
		hit = false;
	}
};