#pragma once
#include "include.h"

class Camera{
public:
	scalar o[3];
	scalar pupil[3];
	
	scalar d[3];
	scalar right[3];
	scalar up[3];
	unsigned int res_x;
	unsigned int res_y;
	float * renderbuffer;
	
	scalar focal_length;
	scalar aperture_radius;
	scalar focus_distance;
	
	Camera(unsigned int res_x, unsigned int res_y){
		right[0] = 32.421434;
		this->res_x = res_x;
		this->res_y = res_y;
		this->renderbuffer = (float *) malloc(sizeof(float)*res_x*res_y*3);
	}
	Camera(){
		this->renderbuffer = NULL;
	}
	
	void Resize(int res_x, int res_y){
		this->res_x = res_x;
		this->res_y = res_y;
		free(this->renderbuffer);
		this->renderbuffer = (float *) malloc(sizeof(float)*res_x*res_y*3);
		
		glutReshapeWindow(res_x, res_y);
		
		
	}
	
	void LookAt(scalar focal_length, scalar pupil_size, scalar focus_distance, scalar * pos, scalar * target, scalar * gravity);
	void SampleRay(unsigned int x, unsigned int y, Ray * ray, Sampler * sampler);
	void DrawPoint(scalar * p, scalar R, scalar G, scalar B){ //WIP
		
		scalar k[3];
		vCpy(k, p);
		vSub(k, o);
		
		scalar r[3];
		vCpy(r, pupil);
		vSub(r, p);
		
		scalar x = -(vDot(d, k)/vDot(d, r));
		
	}
	
	
	bool ProjectOnPlane(scalar * pos, scalar * result, Ray * result_ray, scalar * min){
	    /*
		SampleRay(0, 0, result_ray, NULL); //only care about ray.o
		scalar O[3];
		scalar P[3];
		scalar P_[3];
		scalar D[3];

		vCpy(O, this->o);
		vCpy(P, pos);;

		vCpy(P_, this->o);
		vMulF(P_, 2);
		vSub(P_, P);

		vCpy(D, P_);
		vSub(D, O);

		//intersect ray O+D*t to find position
		//R*u + U*v + CO-K = O + D*t
		//R*u + U*v - D*t = O - CO + K
		scalar K[3];
		vCpy(K, d);
		vMulF(K, -focus_distance);
		vAdd(K, o);
		IntersectRayQuad(O, D, K, right, up, result);

		result[0] *= focus_distance/focal_length;
		result[1] *= focus_distance/focal_length;

		vCpy(result_ray->d, P);
		vSub(result_ray->d, O);
		*min = vLength(result_ray->d);
		vDivF(result_ray->d, *min);
		*min = 0.1;

		result[0] = 0.5-result[0];
		result[1] = 0.5-result[1];


		if(false or result[0] < 0 or result[0] > 1 or result[1] < 0 or result[1] > 1){
			return true;
		}
        */

	    scalar P[3];
	    scalar P_[3];
	    scalar O[3];
	    scalar V[3];




	    vCpy(O, this->d);
	    vMulF(O, focal_length);
	    vAdd(O, this->o);
	    vCpy(P, pos);

	    /*
		scalar alpha = fastRand()*this->aperture_radius;
		scalar beta  = fastRand()*this->aperture_radius;
		alpha *= (scalar) res_x/res_y;
		scalar R[3];
		R[0] = right[0]*alpha + up[0]*beta + O[0];
		R[1] = right[1]*alpha + up[1]*beta + O[1];
		R[2] = right[2]*alpha + up[2]*beta + O[2];
		*/
		scalar rand[2];
		rand[0] = fastRand();
		rand[1] = fastRand();
		scalar angle = 2*M_PI*rand[0];
		scalar radius = cos(M_PI*0.5*rand[1]);

		scalar cos_angle = cos(angle);
		scalar sin_angle = sin(angle);
		scalar loc_r[3];
		scalar R[3];
		R[0] = (aperture_radius*radius*(cos_angle*right[0]*(res_x/res_y) + sin_angle*up[0])) + O[0];
		R[1] = (aperture_radius*radius*(cos_angle*right[1]*(res_x/res_y) + sin_angle*up[1])) + O[1];
		R[2] = (aperture_radius*radius*(cos_angle*right[2]*(res_x/res_y) + sin_angle*up[2])) + O[2];


		vCpy(P_, O);
		vMulF(P_, 2);
		vSub(P_, P);

	    vCpy(V, P_);
	    vSub(V, R);

	    scalar K[3];
	    vCpy(K, this->d);
	    vMulF(K, -focus_distance);
	    vAdd(K, O);

	    IntersectRayQuad(R, V, K, right, up, result);

	    result[0] /= (scalar) (focus_distance*((scalar)res_x/res_y ));
	    result[1] /= (scalar) focus_distance;
	    result[0] *= focal_length;
	    result[1] *= focal_length;

	    result[0] = -result[0]+0.5;
	    result[1] = -result[1]+0.5;
	    //result[0] = fastRand();
	    //result[1] = fastRand();


	    vCpy(result_ray->o, P);
	    vCpy(result_ray->d, R);
	    vSub(result_ray->d, P);

	    *min = vLength(result_ray->d);
	    vDivF(result_ray->d, *min);
		if(result[2] < 0 or result[0] < 0 or result[0] > 1 or result[1] < 0 or result[1] > 1){
			return false;
		}
		return true;
	}
	
};

//assuming 36mm sensor
void Camera::LookAt(scalar focal_length, scalar pupil_size, scalar focus_distance, scalar * pos, scalar * target, scalar * up){
	vCpy(this->o, pos);
	
	
	vCpy(this->d, target);
	vSub(this->d, pos);
	vNormalize(this->d);
	
	//vCpy(this->up, up);
	vCross(this->d, up, this->right);
	vNormalize(this->right);
	vCross(this->right, this->d, this->up);
	vNormalize(this->up);
	
	//vMulF(this->d, focal_length);
	this->focal_length = (scalar) focal_length/36; //36mm sensor
	this->focus_distance = focus_distance;
	this->aperture_radius = pupil_size;
	
	
	vCpy(this->pupil, this->o);
	vAdd(this->pupil, this->d);
	
}

void Camera::SampleRay(unsigned int x, unsigned int y, Ray * result, Sampler * sampler=NULL){
	
	scalar angle;
	scalar radius;
	if(sampler == NULL){
		angle = 2*M_PI*fastRand();
		radius = cos(M_PI*0.5*fastRand());
	}
	else{
		scalar rand[2];
		
		sampler->Sample2D(rand);
		
		angle = 2*M_PI*rand[0];
		radius = cos(M_PI*0.5*rand[1]);
		
	}
	scalar cos_angle = cos(angle);
	scalar sin_angle = sin(angle);
	scalar loc_r[3];
	loc_r[0] = (aperture_radius*radius*(cos_angle*right[0] + sin_angle*up[0]));
	loc_r[1] = (aperture_radius*radius*(cos_angle*right[1] + sin_angle*up[1]));
	loc_r[2] = (aperture_radius*radius*(cos_angle*right[2] + sin_angle*up[2]));
	
	scalar loc[3];
	loc[0] = o[0] + d[0]*focal_length;
	loc[1] = o[1] + d[1]*focal_length;
	loc[2] = o[2] + d[2]*focal_length;
	
	loc_r[0] += loc[0];
	loc_r[1] += loc[1];
	loc_r[2] += loc[2];
	
	
	scalar alpha = -(((scalar)x/res_x)-0.5);
	scalar beta  = -(((scalar)y/res_y)-0.5);
	alpha *= (scalar) res_x/res_y;
	scalar pos[3];
	pos[0] = right[0]*alpha + up[0]*beta + o[0];
	pos[1] = right[1]*alpha + up[1]*beta + o[1];
	pos[2] = right[2]*alpha + up[2]*beta + o[2];
	
	
	scalar target_d[3];
	target_d[0] = loc[0]-pos[0];
	target_d[1] = loc[1]-pos[1];
	target_d[2] = loc[2]-pos[2];
	
	
	scalar target[3];
	target[0] = target_d[0]*(1);
	target[1] = target_d[1]*(1);
	target[2] = target_d[2]*(1);
	vNormalize(target_d);
	target[0] += pos[0] + target_d[0]*(focus_distance);
	target[1] += pos[1] + target_d[1]*(focus_distance);
	target[2] += pos[2] + target_d[2]*(focus_distance);
	
	result->d[0] = target[0]-loc_r[0];
	result->d[1] = target[1]-loc_r[1];
	result->d[2] = target[2]-loc_r[2];
	vNormalize(result->d);
	
	vCpy(result->o, loc_r);
	
	
	return;
}