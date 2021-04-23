//FILE WAS NEVER COMPILED !!!

#pragma once
#include "include.h"


class MetropolisIntegrator{ //just starting it, building a rough frame....
	
private:
	int BuildPath(Vertex * vertices, int length, Scene * scene);
	bool ModifyPath(Vertex * active, Vertex * proposal, int length, Scene * scene);
	void EstimatePath(scalar * value, Vertex * vertices, int length, Scene * scene);
	void RecordSample(scalar * value, Vertex * vertices, int length, Scene * scene);
	
	void UpdateRenderBuffer(float * renderbuffer, int samples);
	
	scalar * renderbuffer;
	int * sample_buffer;
	int calculated_samples;
	int width;
	int height;

public:
	int sample_count;
	void Integrate(Scene * scene, int max_vertices);	
};

inline void MetropolisIntegrator::UpdateRenderBuffer(float * renderbuffer, int samples){
	
	int i = 0;
	scalar buff[3];
	while(i < width*height){
		vSetF(buff, (scalar) this->sample_buffer[i]);
		vDiv(buff, &this->renderbuffer[i*3]);
		vCpy(&renderbuffer[i*3], buff);
		//vDivF(&renderbuffer[i*3], (scalar) samples);
		//vMulF(&renderbuffer[i*3], (scalar) width*height);
		i ++;
	}
	
}

inline int MetropolisIntegrator::BuildPath(Vertex * vertices, int length, Scene * scene){ //dump path building :
	if(length <= 0){
		return 0;
	}
	
	//Generate random ray from Light :
	Camera * cam = &scene->cameras[scene->active_camera];
	Light * light  = &scene->lights[rand()%scene->light_count];

	BVH * bvh = scene->accelerator;
	Ray ray;
	cam->SampleRay((int) (fastRand()*cam->res_x), (int) (fastRand()*cam->res_y), &ray, NULL);
	
	//trace it for 'length' steps
	Ray new_ray;
	Hit hit;
	scalar min;
	Triangle * tri;
	scalar ior = 1;
	scalar in_ior = 1;


	int i = 0;
	while(i < length){
		
		ray.UpdateInverse();
		min = std::numeric_limits<scalar>::max();
		bvh->IntersectRayLoop(&ray, &min, &hit, scene->primitives, false);
		
		if(hit.hit == false){
			break;
		}
		else{
			tri = (Triangle * )hit.prim;
			//record in new vertex
			ray.Trace(hit.t, vertices[i].pos);
			vCpy(vertices[i].direction, ray.d);
			tri->GetNormal(hit.uvs, vertices[i].normal);
			tri->GetUV(hit.uvs[0], hit.uvs[1], vertices[i].tex);
			
			if(vDot(vertices[i].normal, ray.d) > 0){ // > cause the ray direction is pointing TO the hit not AWAY from it
				vFlip(vertices[i].normal);
			}
			vCpy(vertices[i].tangent, tri->u);
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
			vertices[i].camera = false;

			//tri->material->Scatter(&ray, vertices[i].normal, vertices[i].tangent, in_ior, hit.uvs, hit.t, &new_ray);
			vRandomize(new_ray.d, 1);
			if(vDot(new_ray.d, ray.d) > 0){ //same direction
				ray.Trace(hit.t+0.00002, new_ray.o);
			}
			else {
				ray.Trace(hit.t - 0.00002, new_ray.o);
			}
			in_ior = ior;
			//apply buffer :
			vCpy(ray.o, new_ray.o);
			vCpy(ray.d, new_ray.d);
		}
		i ++;
	}
	
	return i;
}

inline bool MetropolisIntegrator::ModifyPath(Vertex * active, Vertex * proposal, int length, Scene * scene){ //basically a dummy, it's not samrt at all, it just randomly removes vertices and repalces them by new ones
	//get vertex to remove, must not be the first one in this version but may in general be
	if(length <= 0){
		return false;
	}
	int v;
	if(rand()%2 == 0){
		v = rand()%length;
	}
	else{
		v = 0;
	}
	scalar buffer[3];
	Camera * cam = &scene->cameras[scene->active_camera];
	/*
	if(v == 0) {//modify camera vertex:
		vCpy(buffer, cam->up);
		vMulF(buffer, fastRand()*0.5-0.25);
		vAdd(proposal[0].pos, buffer);
		vCpy(buffer, cam->right);
		vMulF(buffer, fastRand()*0.5-0.25);
		vAdd(proposal[0].pos, buffer);
		return true;
	}
	 */

	scalar min;
	Triangle * tri;
	scalar ior = 1;
	scalar in_ior = 1;
	Ray ray;
	Hit hit;
	if(v < length){ //not the last one :
		/*
		sample new random ray,
		and then try to connect by a shadow ray,
		if the shadow ray fails or the ray ends
		in the void the new proposal is automatically
		rejected by returning false.
		*/
		if(v == 0){
			cam->SampleRay(rand()%cam->res_x, rand()%cam->res_y, &ray, NULL);
		}
		else{
			vCpy(ray.d, active[v-0].direction);
			vCpy(ray.o, active[v-1].pos      );
			vRandomizeDistribution(ray.d, 2); //this function is symetric and > 0 and therefore valide in the given context
		}


		ray.UpdateInverse();
		min = std::numeric_limits<scalar>::max();
		hit.hit = false;
		scene->accelerator->IntersectRayLoop(&ray, &min, &hit, scene->primitives, false);
		if(hit.hit == false){
			return false;
		}
		else{

			if(v != length-1 or true){
				tri = (Triangle*) hit.prim;
				//save changes to proposal :
				ray.Trace(hit.t-0.00002, proposal[v].pos);
				//vCpy(proposal[v].pos, hit.pos);
				vCpy(proposal[v].direction, ray.d);
				tri->GetNormal(hit.uvs, proposal[v].normal);
				tri->GetUV(hit.uvs[0], hit.uvs[1], proposal[v].tex);

				if(vDot(proposal[v].normal, ray.d) > 0){ // > cause the ray direction is pointing TO the hit not AWAY from it
					vFlip(proposal[v].normal);
				}
				vCpy(proposal[v].tangent, tri->u);
				vNormalize(proposal[v].tangent);
				
				vReflect(ray.d, proposal[v].normal, proposal[v].reflection);
				ior = tri->material->GetIor(hit.uvs);
				vRefract(ray.d, proposal[v].normal, in_ior, ior, proposal[v].refraction);
				proposal[v].in_ior = in_ior;
				proposal[v].material = tri->material;
				proposal[v].uvs[0] = hit.uvs[0];
				proposal[v].uvs[1] = hit.uvs[1];
				proposal[v].t = hit.t;
				proposal[v].end = false;
			}
			if(v < length-1){
				//build and test shadow ray :
				vCpy(ray.d, ray.o);
				vSub(ray.d, active[v+1].pos);
				min = vLength(ray.d);
				vDivF(ray.d, min);
				min -= 0.00002;
				ray.UpdateInverse();
				hit.hit = false;
				scene->accelerator->IntersectRayLoop(&ray, &min, &hit, scene->primitives, true);
				if(hit.hit){
					return false;
				}
			}
		}
	}
	else{
		return false;
	}
	
	return true;
}


inline void MetropolisIntegrator::RecordSample(scalar * value, Vertex * vertices, int length, Scene * scene){
	//get camera uv pos from camera_vertex
	scalar buff[3];
	Camera * cam = &scene->cameras[scene->active_camera];
	Ray ray;
	scalar min;
	Hit hit;
	scalar UV[2];
	int x;
	int y;
	bool projection;
	projection = cam->ProjectOnPlane(vertices[0].pos, UV, &ray, &min);
	if(projection){
		hit.hit = false;
		ray.UpdateInverse();
		scene->accelerator->IntersectRayLoop(&ray, &min, &hit, scene->primitives, true);
		if(hit.hit == false){
			//actually record sample
			x = fmin((int) (UV[1]*cam->res_x), cam->res_x-1);
			y = fmin((int) (UV[0]*cam->res_y), cam->res_y-1);
			buff[0] = (scalar) 1/value[0];
			buff[1] = (scalar) 1/value[1];
			buff[2] = (scalar) 1/value[2];
			vAdd(&renderbuffer[(y*cam->res_x+x)*3], buff);
			sample_buffer[y*cam->res_x+x] ++;
		}
	}
	
	
}

inline void MetropolisIntegrator::EstimatePath(scalar * value, Vertex * vertices, int length, Scene * scene){
	int i;
	vSetF(value, 0);
	Ray shadow_ray;
	Light * light;
	scalar min;
	Hit hit;
	scalar buff[3];
	i = length-1;

	while(i >= 0){
		//cast shadow ray:
		//select random light:
		light = &scene->lights[rand()%scene->light_count];
		//build shadow ray:
		vCpy(shadow_ray.o, light->pos);
		vCpy(shadow_ray.d, vertices[i].pos);
		vSub(shadow_ray.d, shadow_ray.o);
		min = vLength(shadow_ray.d);
		vDivF(shadow_ray.d, min);

		vertices[i].material->GetColor(vertices[i].tex, buff);
		vMul(value, buff);
		vDivF(buff, min*min);

		min -= 0.00002;
		
		hit.hit = false;
		shadow_ray.UpdateInverse();
		scene->accelerator->IntersectRayLoop(&shadow_ray, &min, &hit, scene->primitives, true);

		if(hit.hit){
			vSetF(buff, 0);
		}
		else{
			vMul(buff, light->color);
			vMulF(buff, light->intensity);


		}
		vAdd(value, buff); //add highlight


		if(i < length-1 and false){
			vMulF(value, vertices[i].material->Pdf(vertices[i].direction, vertices[i].direction, vertices[i].normal, vertices[i].reflection, vertices[i].refraction, vertices[i].tex, vertices[i].in_ior));
		}

		vertices[i].material->GetEmission(vertices[i].tex, buff);
		vAdd(value, buff);



		i --;
	}
}

inline void MetropolisIntegrator::Integrate(Scene * scene, int max_vertices){
	
	Camera * cam = &scene->cameras[scene->active_camera];
	renderbuffer = (scalar*) std::malloc(sizeof(scalar)*3*cam->res_x*cam->res_y);
	sample_buffer = (int*) std::malloc(sizeof(int)*cam->res_x*cam->res_y);
	int i = 0;
	width = cam->res_x;
	height = cam->res_y;
	while(i < 3*cam->res_x*cam->res_y){
		renderbuffer[i] = 0;
		if(i < cam->res_x*cam->res_y){
			sample_buffer[i] = 0;
		}
		i ++;
	}
	
	Vertex active[128];
	Vertex proposal[128];
	
	Vertex * active_ptr = active;
	Vertex * proposal_ptr = proposal;
	Vertex * buff_ptr;
	
	
	int calculated_samples = 0;
	//build an initial path
	int path_length = BuildPath(active_ptr, max_vertices, scene);
	while(path_length < 1 and calculated_samples < sample_count){
		path_length = BuildPath(active_ptr, max_vertices, scene);
		calculated_samples ++;
	}
	scalar new_est[3];
	scalar old_est[3] = {1, 1, 1};
	EstimatePath(old_est, active_ptr, path_length, scene);
	bool success_mut;
	
	while(calculated_samples < sample_count){
		//copy active to proposal :
		i = 0;
		while(i < path_length) {
			proposal_ptr[i] = active_ptr[i];
			i++;
		}

		if(rand()%100 == 0){
			path_length = BuildPath(proposal_ptr, max_vertices, scene);
			while(path_length < 1 and calculated_samples < sample_count){
				path_length = BuildPath(active_ptr, max_vertices, scene);
				calculated_samples ++;
			}
			success_mut = true;
		}
		else {
			success_mut = ModifyPath(active_ptr, proposal_ptr, path_length, scene);
		}

		if(success_mut){
			EstimatePath(new_est, proposal_ptr, path_length, scene);
		}
		//std::cout << "new_est:\n";
		//vPrint(new_est);
		//vPrint(old_est);
		if( success_mut and ((scalar)vLength(new_est)/fmax(vLength(old_est), 0.0001)) > fastRand() ){
			//accept sample
			buff_ptr = active_ptr;
			active_ptr = proposal_ptr;
			proposal_ptr = buff_ptr;

			vCpy(old_est, new_est);
		}
		RecordSample(old_est, active_ptr, path_length, scene);
		calculated_samples ++;
	}
	UpdateRenderBuffer(cam->renderbuffer, sample_count);
	
	std::free(renderbuffer);
	std::free(sample_buffer);
}