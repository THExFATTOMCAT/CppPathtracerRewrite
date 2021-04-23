#pragma once

#include "include.h"

class Vertex{
public:
	scalar pos[3];
	scalar direction[3];
	scalar normal[3];
	scalar tangent[3];
	scalar reflection[3];
	scalar refraction[3];
	scalar in_ior;
	Material * material;
	scalar uvs[2];
	scalar t;
	scalar tex[2];
	
	bool transmission;
	bool transmission_is_std;
	scalar alt_pos[3];
	bool end;
	bool camera;
	bool light;


	scalar GetPdf(scalar * test_direction);
};


inline scalar Vertex::GetPdf(scalar * test_direction){
	if(light){
		return (scalar) 1;
	}
	else if(camera){
		return 1;
	}
	else{
		return material->Pdf(test_direction, direction, normal, reflection, refraction, tex, in_ior);
	}
}