#pragma once
#include "include.h"

class BasicIntegrator:public virtual Integrator{
private:
	void SampleTransparentShadow(scalar * pos, Light * light, scalar * alternative, Scene * scene, scalar * result, int depth, scalar in_ior, Material * mask);
public:
	unsigned int max_recursions;
	
	void Setup();
	void Ready();
	void UpdateResult(){
		return;
	};
	void Integrate(Ray * ray, Scene * scene, scalar * result, int depth, scalar in_ior);
};


void BasicIntegrator::Setup(){
	return;
}

void BasicIntegrator::Ready(){
	return;
}

inline void BasicIntegrator::SampleTransparentShadow(scalar * pos, Light * light, scalar * alternative, Scene * scene, scalar * result, int depth, scalar in_ior=1, Material * mask=NULL){ //result has to be set to 1 !
	
	Ray ray; //check for direct light connection
	//build primary ray:
	scalar min;
	scalar distance;
	
	vCpy(ray.o, pos);
	if(light->sun){
		vCpy(ray.d, light->direction);
		vFlip(ray.d);
		min = std::numeric_limits<scalar>::max();
		distance = 1;
	}
	else{
		vCpy(ray.d, light->pos);
		vSub(ray.d, ray.o);
		min = vLength(ray.d);
		distance = min;
	}
	
	vNormalize(ray.d);
	Hit hit;
	
	scalar normal[3];
	
	ray.UpdateInverse();
	scene->accelerator->IntersectRayLoop(&ray, &min, &hit, scene->primitives, true);
	Triangle * tri;
	
	scalar roughness;
	bool clearcoat;
	scalar metallic;
	scalar ior;
	scalar color[3];
	scalar emission[3];
	scalar transmission;
	
	scalar refraction[3];
	
	scalar fresnel;
	if(hit.hit){ //sample alternative ray
		if(depth > 0){
			//build alternative ray :
			vCpy(ray.d, alternative);
			ray.UpdateInverse();
			min = std::numeric_limits<scalar>::max();
			scene->accelerator->IntersectRayLoop(&ray, &min, &hit, scene->primitives, false);
			
			if( hit.hit ){
				tri = (Triangle*) hit.prim;
				transmission = -1;
				//std::cout << "tri : " << tri << "  " << hit.prim << "  " << hit.hit << "  " << depth << "\n";
				tri->material->Get(hit.uvs, &roughness, &clearcoat, &metallic, &ior, color, emission, &transmission);
				if(transmission > 0){
					tri->GetNormal(hit.uvs, normal);
					//build new alternative direction/refract
					GXX::SampleRefraction(refraction, ray.d, normal, roughness, in_ior, ior);
					vNormalize(refraction);
					
					fresnel = vFresnel(normal, ray.d, refraction, in_ior, ior);
					//std::cout << "fresnel : " << fresnel << " " << in_ior << " " << ior << "\n";
					ray.Trace(hit.t, ray.o);
					
					vMulF(result, fmax(0, 1-fresnel));
					//vMulF(result, 0.75);
					
					//do recursion:
					std::cout << "depth : " << depth << "\n";
					SampleTransparentShadow(ray.o, light, refraction, scene, result, depth-1, ior);
				}
				else{
					result[0] = 0;
					result[1] = 0;
					result[2] = 0;
					return;
				}
			}
			else{
				result[0] = 0;
				result[1] = 0;
				result[2] = 0;
				return;
			}
		}
		else{
			result[0] = 0;
			result[1] = 0;
			result[2] = 0;
			return;
		}
		
	}
	else{
		//result[0] = 1;
		//result[1] = 1;
		//result[2] = 1;
		if(not light->sun){
			;//vDivF(result, distance*distance);
		}
		return;
	}
	
}

void BasicIntegrator::Integrate(Ray * ray, Scene * scene, scalar * result, int depth = 0, scalar in_ior=1){
	
	result[0] = 0;
	result[1] = 0;
	result[2] = 0;
	
	scalar light_buffer[3];
	scalar normal_buffer[3];
	scalar refraction_buffer[3];
	
	scalar light_color[3];
	
	scalar roughness;
	bool clearcoat;
	scalar metallic;
	scalar ior;
	scalar color[3];
	scalar emission[3];
	scalar transmission = 1;
	
	if(depth < camera_depth){
		Hit primary_hit;
		scalar primary_min = std::numeric_limits<scalar>::max();
		primary_hit.hit = false;
		scene->accelerator->IntersectRayLoop(ray, &primary_min, &primary_hit, scene->primitives, false);
		if(primary_hit.hit){
			//Get Material information
			
			((Triangle *)primary_hit.prim)->material->Get(primary_hit.uvs, &roughness, &clearcoat, &metallic, &ior, color, emission, &transmission);
			
			//Get shading data
			scalar normal[3];
			((Triangle*)primary_hit.prim)->GetNormal(primary_hit.uvs, normal);
			//Calculate shading variables
			scalar reflection[3];
			vReflect(ray->d, normal, reflection);
			
			/********LAMPS********/
			//Sample Lightsources uniformly (can be improved by selectively sample more "important" lights more often->would need a correction for the brightness value)
			unsigned int light_index = (unsigned int) rand()%scene->light_count;
			scalar light_min;
			scalar light_distance;
			Hit light_hit;
			
			//declare general shading variables
			scalar highlight_color[3] = {0, 0, 0};
			scalar glossy_color[3] = {0, 0, 0};
			scalar value;
			
			//declare direct shading variables
			scalar diffuse_highlight[3];
			scalar glossy_highlight[3];
			
			//Build light ray
			Ray light_ray;
			if(scene->light_count > 0){
				ray->Trace(primary_hit.t-0.0001, light_ray.d);
				vCpy(light_ray.o, light_ray.d);
				
				if(scene->lights[light_index].sun == false){
					vSub(light_ray.d, scene->lights[light_index].pos);
					vFlip(light_ray.d);
					light_distance = vLength(light_ray.d);
					light_min = light_distance;
				}
				else{
					light_min = std::numeric_limits<scalar>::max();
					light_distance = 1;
					vCpy(light_ray.d, scene->lights[light_index].direction);
					vFlip(light_ray.d);
				}
				vNormalize(light_ray.d);
				
				light_ray.UpdateInverse();
				light_hit.hit = false;
				scene->accelerator->IntersectRayLoop(&light_ray, &light_min, &light_hit, scene->primitives, true);
				
				if(light_hit.hit ^ 0b0001){
					
					vCpy(light_color, scene->lights[light_index].color);
					vMulF(light_color, scene->lights[light_index].intensity/(POW2(light_distance)));
					
					//diffuse highlight :
					value = OrenNayar::Pdf(light_ray.d, ray->d, normal, roughness);
					vCpy(diffuse_highlight, light_color);
					vMulF(diffuse_highlight, value*(1-metallic));
					
					//glossy highlight :
					value = GXX::Pdf(light_ray.d, reflection, roughness);
					vCpy(glossy_highlight, light_color);
					vMulF(glossy_highlight, value*metallic);
					
					vAdd(highlight_color, diffuse_highlight);
					vAdd(highlight_color, glossy_highlight);
				}
				//vMulF(highlight_color, 1-metallic);
			}
			/********INDIRECT*********/
			//declare indirect shading variables
			scalar indirect_color[3] = {0, 0, 0};
			scalar indirect_glossy[3] = {0, 0, 0};
			scalar indirect_diffuse[3] = {0, 0, 0};
			scalar transmission_color[3] = {0, 0, 0};
			Ray gi_ray;
			
			scalar fresnel;
			
			ray->Trace(primary_hit.t-0.0001, gi_ray.o);
			
			//decide ray type:
			if((transmission) > 0 and fastRand() < (transmission)){ //do transmission sample
				vRefract(ray->d, normal, in_ior, ior, gi_ray.d);
				fresnel = vFresnel(normal, ray->d, gi_ray.d, in_ior, ior);
				
				if(fastRand() < (1-fresnel)*transmission){
					
					ray->Trace(primary_hit.t+0.0001, gi_ray.o);
					GXX::SampleRefraction(gi_ray.d, ray->d, normal, roughness, in_ior, ior);
					gi_ray.UpdateInverse();
					
					this->Integrate(&gi_ray, scene, transmission_color, depth+1, ior);
				}
				else{
					GXX::Sample(gi_ray.d, ray->d, normal, roughness);
					
					//vReflect(ray->d, normal, gi_ray.d);
					
					gi_ray.UpdateInverse();
					this->Integrate(&gi_ray, scene, indirect_glossy, depth+1, ior);
				}
			}
			else{ //do some kind of reflective sample
				if(fastRand() < metallic){ //glossy sample
					GXX::Sample(gi_ray.d, ray->d, normal, roughness);
					
					gi_ray.UpdateInverse();
					this->Integrate(&gi_ray, scene, indirect_glossy, depth+1, ior);
					//vDivF(indirect_glossy, metallic);
				}
				else{ //diffuse sample
					//OrenNayae::Sample is broken
					
					scalar rand_normal[3];
					vCpy(rand_normal, normal);
					vRandomize(rand_normal, 1);
					if(vDot(rand_normal, normal) < 0){
						vFlip(rand_normal);
					}
					vReflect(ray->d, rand_normal, gi_ray.d);
					vNormalize(gi_ray.d);
					
					gi_ray.UpdateInverse();
					this->Integrate(&gi_ray, scene, indirect_diffuse, depth+1, ior);
					//vDivF(indirect_diffuse, 1-metallic);
				}
			}
			
			
			//vMix(indirect_diffuse, indirect_glossy, metallic, indirect_color);
			vAdd(indirect_color, indirect_diffuse);
			vAdd(indirect_color, indirect_glossy);
			vAdd(indirect_color, transmission_color);
			
			vCpy(result, highlight_color);
			vAdd(result, indirect_color);
			vMul(result, color);
			vAdd(result, emission);
		}
		else{
			vCpy(result, scene->bg);
		}
	}
	return;
}