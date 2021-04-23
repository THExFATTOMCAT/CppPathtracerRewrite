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
	scalar in_ior;

	Sampler * sampler;

	void Ready(){
		std::cout << "READY\n";
		std::cout << "scene: " << scene << "\n";
		Camera * cam = &scene->cameras[scene->active_camera];
		std::cout << "READY\n";
		renderbuffer = (scalar *) std::malloc(sizeof (scalar)*cam->res_x*cam->res_y*3);
		std::cout << "READY\n";
		int i = 0;
		while(i < cam->res_x*cam->res_y+3){
			renderbuffer[i] = 0;
			i ++;
		}
		std::cout << "READY\n";
		return;
	}

	void End(){
		if(renderbuffer != NULL){
			std::free(renderbuffer);
			renderbuffer = NULL;
		}
	}

	void UpdateResult(){
		int i = 0;
		Camera * cam = &scene->cameras[scene->active_camera];
		//mutex->lock();
		while(i < cam->res_x*cam->res_y*3){
			cam->renderbuffer[i] += renderbuffer[i];
			i ++;
		}
		//mutex->unlock();
	}

	//virtual void Integrate(Ray * ray, Scene * scene, scalar * result, int depth=0, scalar in_ior=1, Material * mask=NULL){std::cout<<"INT\n";};
    virtual void Integrate(int x, int y) = 0;

	virtual Integrator * clone(){std::cout << "wrong func\n";};
};
