#pragma once

#include "include.h"

class BidirectionalIntegrator:public UnidirectionalIntegrator{
private:
	inline scalar GenerateLightDirection(scalar * light_pos, scalar * target, scalar * result);
public:
	void Setup();
	void Ready();
	void Integrate(Ray * ray, Scene * scene, scalar * result, int depth, scalar in_ior, Material * mask);
	
};

void BidirectionalIntegrator::Setup(){
	return;
}
void BidirectionalIntegrator::Ready(){
	return;
}


inline scalar BidirectionalIntegrator::GenerateLightDirection(scalar * light_pos, scalar * target, scalar * result){
	
	vCpy(result, target);
	vSub(result, light_pos);
	vNormalize(result);
	
	scalar correction_factor = vRandomizeDistribution(result, 8);
	
	/*
	scalar dir[3];
	scalar angle = 0.125; //0.5*180°
	scalar sector_area = 0;
	
	if(angle > 0.5){
		//sector_area += 2*M_PI;
		sector_area += 1;
	}
	//sector_area += 2*M_PI*(1-fabs(cos(M_PI*angle)));
	sector_area += (1-fabs(cos(M_PI*angle)));
	
	scalar focus_prob = 0.75; //distribute 50% of the rays over the oriented cone :
	if(fastRand() < focus_prob){
		vCpy(dir, target);
		vSub(dir, light_pos);
		vNormalize(dir);
		vRandomizeCone(dir, M_PI*angle);
		vCpy(result, dir);
		correction_factor = (scalar) sector_area/2.0;//(2pi(x+cos)/(4pi))
		correction_factor /= focus_prob;
	}
	else{
		vRandomize(result, 1);
		correction_factor /= (1-focus_prob);
	}
	*/
	return correction_factor;
}

void BidirectionalIntegrator::Integrate(Ray * ray, Scene * scene, scalar * result, int depth, scalar in_ior, Material * mask=NULL){
	Vertex l_vertices[128];
	Vertex c_vertices[128];
	
	Light * light = scene->lights + (rand()%scene->light_count);//sample random light source
	Ray l_ray;//only sample point lights for now
	vCpy(l_ray.o, light->pos);
	//vRandomize(l_ray.d, 1); //should be normalized by default if rate=1
	
	c_vertices[0].end = true;
	int l_vertex_count = 0;
	int c_vertex_count = BuildVertices(c_vertices, fmin(128, camera_depth), ray, scene, in_ior, mask);
	int rejects = 0;
	scalar light_correction;
	scalar correction = 0;
	bool hit = (c_vertex_count > 0) and (c_vertices[0].end == false);
	if(hit and light_depth > 0 ){ //something is actually hit and light paths are needed:
		light_correction = GenerateLightDirection(l_ray.o, c_vertices[0].pos, l_ray.d);
		correction += light_correction;
		l_vertex_count = BuildVertices(l_vertices, fmin(128, light_depth), &l_ray, scene, in_ior, NULL);
		while(l_vertex_count <= 0 and rejects < 0 and light_depth > 0){
			light_correction = GenerateLightDirection(l_ray.o, c_vertices[0].pos, l_ray.d);
			correction += light_correction;
			l_vertex_count = BuildVertices(l_vertices, fmin(128, light_depth), &l_ray, scene, in_ior, NULL);
			rejects += 1;
		}
	}
	
	correction /= (scalar) POW2(rejects+1);
	
	scalar buff[3];
	Vertex * w;
	scalar light_color[3];
	vCpy(light_color, light->color);
	vMulF(light_color, (scalar) light->intensity*(correction));
	scalar l_colors[128][3];
	//build light path points :
	int j = 0;
	while(j < l_vertex_count){
		w = &l_vertices[j];
		if(w->end){
			break;
		}
		//multiply by color
		w->material->GetColor(w->tex, buff);
		vMul(light_color, buff);
		//add emission:
		w->material->GetEmission(w->tex, buff);
		vAdd(light_color, buff);
		vCpy(l_colors[j], light_color);
		// connect to every camera vertex :
		j ++;
	}
	l_vertex_count = j;
	
	//do standard shading :
	int i = c_vertex_count-1;
	vSetF(result, 0);
	Ray light_ray;
	Hit light_hit;
	scalar min;
	Vertex * v;
	while(i >= 0){
		v = &c_vertices[i];
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
		v->material->GetEmission(v->tex, buff); //add eission shader
		vAdd(result, buff);
		
		scalar MIN;
		scalar light_hit_pos[3];
		
		//bidirectional shading :
		j = 0;
		
		Hit backup_hit;
		Triangle * prim;
		
		while(j < l_vertex_count){
			v->material->GetColor(v->tex, buff);
			
			vCpy(light_ray.o, l_vertices[j].pos); //ray from light to camera vertex
			vCpy(light_ray.d, c_vertices[i].pos);
			vSub(light_ray.d, light_ray.o);
			min = vLength(light_ray.d);
			vDivF(light_ray.d, min);//Normalize
			vDivF(buff, (scalar) min*min*4*M_PI*l_vertex_count);//dinstance falloff + sample cout division
			min -= 0.00001;
			MIN = min;
			
			light_ray.UpdateInverse();
			light_hit.hit = false;
			
			//if glass : do expencive visibility test :
			w = &l_vertices[j];
			if(false and (w->material->NeedTransmission(w->tex) or v->material->NeedTransmission(v->tex))){ //seems to influence the result very little
				scene->accelerator->IntersectRayLoop(&light_ray, &min, &light_hit, scene->primitives, false); //optimize thiis in the intersection function ?
				prim = (Triangle*) light_hit.prim;
				if(light_hit.hit == true and prim->material->NeedTransmission(light_hit.tex)){ //glass inbetween !
					//if not at least one vertex is near to the intersection break
					light_ray.Trace(light_hit.t, light_hit_pos);
					vSub(light_hit_pos, v->pos);
					MIN = vLength(light_hit_pos);
					if(min < 0.00002 or MIN < 0.00002){
						light_ray.Trace(light_hit.t+0.00001, light_ray.o);
						
						if(MIN > 0.00001){ //if distance is so short intersection is not precise anymore just assume its the same surface and therefore no blocking geometry
							min = fmax(0.00002, MIN);
							scene->accelerator->IntersectRayLoop(&light_ray, &min, &backup_hit, scene->primitives, true); //now a shadowray  is sufficient and defines the final result
						}
						else{
							light_hit.hit = false;
						}
						
					}
					light_hit.hit = light_hit.hit | backup_hit.hit;
					if(light_hit.hit == false){ //if actually nothing else is inbetween :
						//DO GLASS SHADING !! (simple)
						//just usr light_color, since its not used and already there...
						prim->material->GetColor(light_hit.tex, light_color);
						vMul( buff, light_color);
						vMulF(buff, prim->material->GetTransmission(light_hit.tex));
						prim->material->GetEmission(light_hit.tex, light_color);
						vAdd( buff, light_color);
					}
					
				}
			}
			else{
				scene->accelerator->IntersectRayLoop(&light_ray, &min, &light_hit, scene->primitives, true);
			}
			if(light_hit.hit == false){
				vMul(buff, l_colors[j]);
				
				scalar da = fabs(vDot(v->normal, light_ray.d));
				scalar db = fabs(vDot(w->normal, light_ray.d));
				vMulF(buff, da ); //use this approximation for now, the other one seems buggy
				vMulF(buff, db );
				/*
				vFlip(light_ray.d);
				if(vDot(v->normal, light_ray.d) > 0){vFlip(v->normal);}
				vMulF(buff, v->material->Pdf(light_ray.d, v->direction, v->normal, v->reflection, v->refraction, v->uvs, v->in_ior));
				vFlip(light_ray.d);
				if(vDot(w->normal, light_ray.d) > 0){vFlip(w->normal);}
				vMulF(buff, w->material->Pdf(light_ray.d, w->direction, w->normal, w->refraction, w->refraction, w->uvs, w->in_ior));
				*/
			}
			else{
				vSetF(buff, 0);
			}
			vAdd(result, buff);
			j ++;
		}
		
		
		i --;
	}
	
	
	
	
	return;
}