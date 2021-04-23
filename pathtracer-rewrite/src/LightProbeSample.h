#pragma once

#include "include.h"

class LightProbeSample{
public:
	//hit data
	scalar pos[3];
	scalar normal[3];
	scalar reflection[3];
	scalar refraction[3];
	scalar direction[3];
	
	//Material Data :
	scalar roughness;
	bool clearcoat;
	scalar metallic;
	scalar ior;
	scalar in_ior;
	scalar color[3];
	scalar emission[3];
	scalar transmission;
	
	void Connect(LightProbeSample * probe, scalar * result, Scene * scene);
	void ConnectLight(Light * light, scalar * result, Scene * scene);
	
};

void LightProbeSample::Connect(LightProbeSample * probe, scalar * result, Scene * scene){
	
	//build connection ray :
	Ray ray;
	vCpy(ray.o, pos);
	vCpy(ray.d, probe->pos);
	vSub(ray.d, ray.o);
	scalar length = vLength(ray.d)-0.0001;
	vDivF(ray.d, length); //normalized
	//Intersect connection ray:
	Hit conn_hit;
	ray.UpdateInverse();
	scene->accelerator->IntersectRayLoop(&ray, &length, &conn_hit, scene->primitives, true);
	
	scalar buff;
	scalar diffuse;
	scalar glossy;
	scalar fresnel;
	
	scalar intensityA;
	scalar intensityB;
	
	
	if(not conn_hit.hit){
		//start shading : (self shading)
		vCpy(result, color);
		vDivF(result, length*length);
		
		buff = OrenNayar::Pdf(ray.d, direction, normal, roughness);
		diffuse = buff * (1-metallic);
		
		buff = GXX::Pdf(ray.d, reflection, roughness);
		glossy = buff*metallic;
		
		if(transmission > 0){
			fresnel = vFresnel(normal, ray.d, refraction, in_ior, ior);
			//buff = fresnel*(1-transmission) + transmission;
			buff = fresnel - (fresnel-1)*transmission;
			diffuse *= buff;
			glossy *= buff;
		}
		
		
		//appply probe shading :
		vFlip(ray.d);//change 'light' direction to match the reversed case :
		
		vMul(result, probe->color);
		
		buff = OrenNayar::Pdf(ray.d, probe->direction, probe->normal, probe->roughness);
		diffuse *= buff * (1-probe->metallic);
		
		buff = GXX::Pdf(ray.d, probe->reflection, probe->roughness);
		glossy *= buff*probe->metallic;
		
		if(probe->transmission > 0){
			fresnel = vFresnel(probe->normal, ray.d, probe->refraction, in_ior, ior);
			//buff = fresnel*(1-transmission) + transmission;
			buff = fresnel - (fresnel-1)*transmission;
			diffuse *= buff;
			glossy *= buff;
		}
		
		vMulF(result, glossy+diffuse);
		vAdd(result, emission);
	}
	else{
		vSetF(result, 0);
	}
}

void LightProbeSample::ConnectLight(Light * light, scalar * result, Scene * scene){
	
	//build connection ray :
	Ray ray;
	scalar length;
	vCpy(ray.o, pos);
	if(light->sun){
		vCpy(ray.d, light->direction);
		vFlip(ray.d);
		vNormalize(ray.d);
		length = std::numeric_limits<scalar>::max();
	}
	else{
		vCpy(ray.d, light->pos);
		vSub(ray.d, ray.o);
		length = vLength(ray.d);
		vDivF(ray.d, length); //normalized
	}
	
	//Intersect connection ray:
	Hit conn_hit;
	ray.UpdateInverse();
	scene->accelerator->IntersectRayLoop(&ray, &length, &conn_hit, scene->primitives, true);
	
	scalar buff;
	scalar diffuse;
	scalar glossy;
	scalar fresnel;
	
	scalar intensityA;
	scalar intensityB;
	
	
	if(not conn_hit.hit){
		//start shading : (self shading)
		vCpy(result, color);
		vMul(result, light->color);
		vMulF(result, light->intensity);
		if(not light->sun){
			vDivF(result, length*length);
		}
		
		
		buff = OrenNayar::Pdf(ray.d, direction, normal, roughness);
		diffuse = buff * (1-metallic);
		
		buff = GXX::Pdf(ray.d, reflection, roughness);
		glossy = buff*metallic;
		
		if(transmission > 0){
			fresnel = vFresnel(normal, ray.d, refraction, in_ior, ior);
			//buff = fresnel*(1-transmission) + transmission;
			buff = fresnel - (fresnel-1)*transmission;
			diffuse *= buff;
			glossy *= buff;
		}
		vMulF(result, glossy+diffuse);
		vAdd(result, emission);
	}
	else{
		vSetF(result, 0);
	}
}