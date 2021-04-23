#pragma once
#include "include.h"

class UnidirectionalIntegrator : public virtual Integrator{
protected:
	int BuildVertices(Vertex * vertices, int count, Ray * in_ray, Scene * scene, scalar IOR, Material * mask){
		Hit hit;
		int i = 0;
		
		Ray ray;
		Ray buffer;
		vCpy(ray.o, in_ray->o);
		vCpy(ray.d, in_ray->d);
		scalar min;
		scalar in_ior = IOR;
		scalar ior;
		
		Triangle * tri;
		
		while(i < count){
			
			min = std::numeric_limits<scalar>::max();
			ray.UpdateInverse();
			hit.hit = false;
			scene->accelerator->IntersectRayLoop(&ray, &min, &hit, scene->primitives, false);
			if(i == 0 and mask != NULL and hit.hit and mask != ((Triangle*)hit.prim)->material ){
				vCpy(vertices[i].direction, ray.d);
				vertices[i].end = true;
				i ++;
				break;
			}
			if(hit.hit){
				tri = ((Triangle*)hit.prim);
				
				//write vertex data
				ray.Trace(fmax(hit.t-0.00002, 0), vertices[i].pos);
				vCpy(vertices[i].direction, ray.d);
				tri->GetNormal(hit.uvs, vertices[i].normal);
				tri->GetUV(hit.uvs[0], hit.uvs[1], vertices[i].tex);
				
				if(vDot(vertices[i].normal, ray.d) > 0){ // > cause the ray direction is pointing TO the hit not AWAY from it
					vFlip(vertices[i].normal);
				}
				//vCpy(vertices[i].tangent, tri->u);
				vCross(vertices[i].normal, tri->u, vertices[i].tangent);
				vNormalize(vertices[i].tangent);
				
				vReflect(ray.d, vertices[i].normal, vertices[i].reflection);
				ior = tri->material->GetIor(hit.uvs);
				vRefract(ray.d, vertices[i].normal, in_ior, ior, vertices[i].refraction);
				vertices[i].in_ior = in_ior;
				vertices[i].material = tri->material;
				vertices[i].uvs[0] = hit.uvs[0];
				vertices[i].uvs[1] = hit.uvs[1];
				vertices[i].t = hit.t;
				vertices[i].end = false;
				
				//prepare for next execution:
				//not sure if buffering is needed -> I'll do it for savety
				tri->material->Scatter(&ray, vertices[i].normal, vertices[i].tangent, in_ior, vertices[i].tex, hit.t, &buffer);
				
				in_ior = ior;
				//apply buffer :
				vCpy(ray.o, buffer.o);
				vCpy(ray.d, buffer.d);
			}
			else{
				vCpy(vertices[i].direction, ray.d);
				vertices[i].end = true;
				i ++;
				break;
			}
			
			i ++;
		}
		return i;
		
	}
public:
	
	void Setup(){
		return;
	};
	void Ready(){
		return;
	};

	void UpdateResult(){
		return;
	};

	void Integrate(int x, int y);
};

void UnidirectionalIntegrator::Integrate(int x, int y){
	Vertex vertices[128];
	
	//build vertices :
	int vertex_count = BuildVertices(vertices, fmin(128, camera_depth), ray, scene, in_ior, mask);
	
	//connect vertices :
	/*
		NOTE : 
			??????????????????????????????????????????????????????????????????
			?                                                                ?
			?   Should I samplel a new random light source                   ?
			?   for each vertex or should it be fixed for the whole path..   ?
			?                                                                ?
			??????????????????????????????????????????????????????????????????
	*/
	Light * light = scene->lights + (rand()%scene->light_count); //just randomize once for now
	int i = vertex_count-1;
	
	
	vSetF(result, 0);
	scalar buff[3];
	Ray light_ray;
	Hit light_hit;
	scalar min;
	Vertex * v;
	while(i >= 0){
		v = &vertices[i];
		if(v->end){
			scene->GetBg(v->direction, result);
			i --;
			continue;
		}
		//Light source shading
		v->material->GetColor(v->tex, buff);
		//vertex shading
		vMul(result, buff);
		
		//build light ray :
		vCpy(light_ray.o, v->pos);
		if(light->sun){
			vCpy(light_ray.d, light->direction);
			vFlip(light_ray.d);
			vNormalize(light_ray.d);
			min = std::numeric_limits<scalar>::max();
		}
		else{
			vCpy(light_ray.d, light->pos);
			vSub(light_ray.d, light_ray.o);
			min = vLength(light_ray.d);
			vDivF(light_ray.d, min); //normalize
			vDivF(buff, min*min*4*M_PI);  //apply distance falloff
			min -= 0.00001;
			
		}
		light_ray.UpdateInverse();
		light_hit.hit = false;
		scene->accelerator->IntersectRayLoop(&light_ray, &min, &light_hit, scene->primitives, true);
		//continue light shading
		if(light_hit.hit == false){
			vMul(buff, light->color);
			vMulF(buff, light->intensity);
			vMulF(buff, v->material->Pdf(light_ray.d, v->direction, v->normal, v->reflection, v->refraction, v->uvs, v->in_ior));
		}
		else{
			vSetF(buff, 0);
		}
		//compositing :
		vAdd(result, buff); // add highlights
		v->material->GetEmission(v->uvs, buff); //add eission shader
		vAdd(result, buff);
		
		i --;
	}
	return;
}

