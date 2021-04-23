#pragma once
#include "include.h"

class Scene{
public:
	scalar bg[3];
	Light ** lights;
	unsigned int light_count;
	
	unsigned int material_count;
	Material materials[128];
	Material std_material;
	
	unsigned int primitive_count;
	Triangle * primitives;
	BVH * accelerator;
	
	Camera * cameras;
	unsigned int camera_count;
	unsigned int active_camera;
	
	Scene(Light ** lights, int light_count, Camera * cameras, int camera_count, int active_camera){
		this->lights = lights;
		this->light_count = light_count;
		this->cameras = cameras;
		this->camera_count = camera_count;
		this->active_camera = active_camera;
		this->primitives = NULL;
		this->primitive_count = 0;
		
		this->std_material.color[0] = 0.8;
		this->std_material.color[1] = 0.8;
		this->std_material.color[2] = 0.8;
		
		this->std_material.emission_color[0] = 0.0;
		this->std_material.emission_color[1] = 0.0;
		this->std_material.emission_color[2] = 0.0;
		
		this->std_material.metallic = 0.5;
		this->std_material.roughness = 0.8;
		this->std_material.emission_strength = 0.0;
		bg[0] = 0.7;
		bg[1] = 0.7;
		bg[2] = 0.7;
	}
	Scene(){
		primitives = NULL;
		bg[0] = 0.7;
		bg[1] = 0.7;
		bg[2] = 0.7;
	}
	
	void GetBg(scalar * d, scalar * r){
		scalar color[3] = {0.5, 0.4, 0.2};
		scalar n[3] = {0, 0, 1};
		scalar factor = fmax(0, vDot(n, d));
		factor = 1-POW4(POW4(1-factor));
		vMix(color, bg, factor, r);
	}
	
	void FreeImport(){
		material_count = 0;
		primitive_count = 0;
		if(primitives != NULL){
		    free(primitives);
		    primitives = NULL;
		}
	}
	
};


