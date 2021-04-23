#pragma once
#include "include.h"



class Integrator{
public:
	int samples;
	int camera_depth;
	int light_depth;
	Scene * scene;
	scalar * renderbuffer;
	Material * mask;

	virtual void Setup(){};
	virtual void Ready(){};
	virtual void UpdateResult(){std::cout << "WRONG\n";};
	//virtual void Integrate(Ray * ray, Scene * scene, scalar * result, int depth=0, scalar in_ior=1, Material * mask=NULL){std::cout<<"INT\n";};
    virtual void Integrate(int x, int y){std::cout<<"INT\n";};
};
