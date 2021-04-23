//This file was never compiled !

#pragma once
#include "include.h"

class LightIntegrator{
public:
	float * image_buffer;
	int * samples_buffer;
	int width;
	int height;
	
	int TraceParticle(GeneralVertex * path, int length, Scene * scene);
	void ConnectToCamera(GeneralVertex * path, int length, Scene * scene);
	void Integrate(int depth, int samples, Scene * scene);
	void UpdateRenderBuffer(float * render_buffer, int samples);
};

inline void LightIntegrator::ConnectToCamera(GeneralVertex * path, int length, Scene * scene){
	//Camera Ray :
	Ray camera_ray;
	Camera * cam = &scene->cameras[scene->active_camera];
	cam->SampleRay(0, 0, &camera_ray, NULL);
	//only interested in camera_ray.o
	int i = 0;
	scalar min;
	Hit hit;
	scalar out_color[3];
	scalar uvt[3];
	int x;
	int y;
	scalar result[3];
	while(i < length){//connect each vertex to camera:
		//Some Error in shadow rays !!
		//build camera ray :
		vCpy(camera_ray.d, path[i].pos);
		vSub(camera_ray.d, camera_ray.o);
		min = vLength(camera_ray.d);
		vDivF(camera_ray.d, min); //normalize
		min -= 0.00002;
		
		//Get vertex out color :
		path[i].GetOutColor(camera_ray.d, out_color, scene);
		
		//find camera uvs :
		vCpy(result, camera_ray.o);
		vSub(result, cam->o);
		vSolve(uvt, cam->right, cam->up, camera_ray.d, result);
		
		hit.hit = false;
		camera_ray.UpdateInverse();
		scene->accelerator->IntersectRayLoop(&camera_ray, &min, &hit, scene->primitives, true);
		
		uvt[0] /= (scalar) cam->res_x/cam->res_y;
		
		if(uvt[0] > -0.5 and uvt[0] < 0.5 and
		   uvt[1] > -0.5 and uvt[1] < 0.5 and
		   uvt[2] >  0.0){ //camera sensor is hit : calculate pixel possition and add sample
			uvt[0] *= -1; //flip image
			uvt[1] *= -1;
			
			uvt[0] += 0.5; //calculate pixel (one could interpolate over pixels)
			uvt[1] += 0.5;
			uvt[0] *= cam->res_x;
			uvt[1] *= cam->res_y;
			x = (int) fmin(uvt[0], cam->res_x-1);
			y = (int) fmin(uvt[1], cam->res_y-1);
			
			image_buffer[3*(y*cam->res_x+x)+0] = (scalar) out_color[0]; //add sample
			image_buffer[3*(y*cam->res_x+x)+1] = (scalar) out_color[1];
			image_buffer[3*(y*cam->res_x+x)+2] = (scalar) out_color[2];
			samples_buffer[y*cam->res_x+x] += 1;
			
		}
		i ++;
	}
}

inline int LightIntegrator::TraceParticle(GeneralVertex * path, int length, Scene * scene){
	//select random light source :
	Light * light = &scene->lights[ rand()%scene->light_count ];
	int i = 0;
	//generate first ray :  ...only support point lights again...
	Ray ray;
	Ray buffer_ray;
	scalar min;
	Hit hit;
	Triangle * tri;
	scalar ior = 1;
	scalar in_ior = 1;
	
	vCpy(ray.o, light->pos);
	vRandomize(ray.d, 1);
	scalar in_color[3];
	
	vCpy(in_color, light->color);
	vMulF(in_color, light->intensity);
	scalar buffer[3];
	
	while(i < length){
		hit.hit = false;
		min = std::numeric_limits<scalar>::max();
		ray.UpdateInverse();
		scene->accelerator->IntersectRayLoop(&ray, &min, &hit, scene->primitives, false); //no shadow ray
		if(hit.hit == false){
			vCpy(path[i].direction, ray.d);
			path[i].end = true;
			i ++;
			return i;
		}
		else{
			tri = (Triangle*) hit.prim;
			
			vCpy(path[i].in_color, in_color);
			ray.Trace(hit.t-0.02, path[i].pos);
			vCpy(path[i].direction, ray.d);
			
			//set all values of path[i]
			
			tri->GetNormal(hit.uvs, path[i].normal);
			tri->GetUV(hit.uvs[0], hit.uvs[1], path[i].tex);
			ior = tri->material->GetIor(hit.uvs);
			
			if(vDot(path[i].normal, ray.d) > 0){ // > cause the ray direction is pointing TO the hit not AWAY from it
				vFlip(path[i].normal);
			}
			vCpy(path[i].tangent, tri->u);
			vNormalize(path[i].tangent);
			
			vReflect(ray.d, path[i].normal, path[i].reflection);
			
			vRefract(ray.d, path[i].normal, in_ior, ior, path[i].refraction);
			path[i].in_ior = in_ior;
			path[i].material = tri->material;
			path[i].uvs[0] = hit.uvs[0];
			path[i].uvs[1] = hit.uvs[1];
			path[i].t = hit.t;
			path[i].end = false;
			
			path[i].weight = 1;
			
			tri->material->Scatter(&ray, path[i].normal, path[i].tangent, in_ior, path[i].tex, hit.t, &buffer_ray); //generate new ray according to bsdf
			vCpy(path[i].pos, buffer_ray.o);
			/*
			if(tri->material->transmission > 0){
				ray.Trace(hit.t+0.00002, ray.o);
			}
			else{
				ray.Trace(hit.t-0.00002, ray.o);
				vRandomize(ray.d, 0.3);
				vNormalize(ray.d);
			}
			*/
			in_ior = ior;
			
			
			vCpy(ray.o, buffer_ray.o);
			vCpy(ray.d, buffer_ray.d);
			
			//path[i].GetOutColor(ray.d, in_color, scene);
			
			tri->material->GetColor(path[i].tex, buffer);
			vMul(in_color, buffer);
			
			tri->material->GetEmission(path[i].tex, buffer);
			vAdd(in_color, buffer);
			
		}
		
		i ++;
	}
	return i;
	
}

inline void LightIntegrator::UpdateRenderBuffer(float * renderbuffer, int samples){
	
	int i = 0;
	
	while(i < width*height){
		
		vCpy(&renderbuffer[i*3], &image_buffer[i*3]);
		vDivF(&renderbuffer[i*3], (scalar) samples);
		vMulF(&renderbuffer[i*3], (scalar) width*height);
		i ++;
	}
	
}

inline void LightIntegrator::Integrate(int depth, int samples, Scene * scene){
	GeneralVertex path[128];//max path depth is 128
	depth = fmin(127, depth);
	
	Camera * cam = &scene->cameras[scene->active_camera];
	//allocate image_buffer :
	image_buffer = (float*) std::malloc(sizeof(float)*cam->res_x*cam->res_y*3);
	samples_buffer = (int*) std::malloc(sizeof(int)*cam->res_x*cam->res_y);
	width = cam->res_x;
	height = cam->res_y;
	//set buffers to 0 :
	int i = 0;
	while(i < width*height){
		vSetF(&image_buffer[i*3], 0);
		samples_buffer[i] = 0;
		i ++;
	}
	
	
	i = 0;
	int path_length;
	int n = 0;
	
	while(i < samples){ //main sampling loop :
		path_length = TraceParticle(path, depth, scene); //rendering :
		ConnectToCamera(path, path_length, scene);
		n += path_length;
		if(i % 1000 == 0){ //visulization :
			UpdateRenderBuffer(cam->renderbuffer, i);
		}
		i ++;
	}
	UpdateRenderBuffer(cam->renderbuffer, n);
	std::free(image_buffer);
	std::free(samples_buffer);
}