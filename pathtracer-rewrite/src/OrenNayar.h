#pragma once

#include "include.h"
namespace OrenNayar{ //https://mimosa-pudica.net/improved-oren-nayar.html
	scalar Pdf(scalar * light, scalar * view, scalar * normal, scalar o){
		
		//Full O-N
		o *= o;
		scalar p = 0.8;
		scalar A = (1-(0.5*o)/(o+0.33) + 0.17*p*o/(o+0.13))/M_PI;
		scalar B = (0.45*o/(o+0.09))/M_PI;
		
		//Approximation
		//scalar A = 1/(M_PI+(M_PI*0.5 - (scalar)2/3)*o);
		//scalar B = A*o;
		
		scalar s = vDot(light, view) - (vDot(normal, light)*vDot(normal, view));
		scalar t;
		if(s <= 0){
			t = 1;
		}
		else{
			t = fmax(vDot(normal, light), vDot(normal, view));
		}
		scalar r = 1;
		scalar L = r*(vDot(normal, light))*(A+B*(s/t));
		return fmax(L, 0);
	}
	
	void Sample(scalar * result, scalar * light, scalar * view, scalar o){  //find the normal
		/*
		//doing rejection sampling
		int i = 16; //max 16 rejections
		scalar compare;
		scalar vec[3];
		
		scalar A = 1/(M_PI+(M_PI*0.5 - 0.6666666666666666)*o);
		scalar B = A*o;
		vec[2] = vDot(light, view);
		scalar r = 1;
		
		scalar s;
		scalar t;
		
		scalar L;
		
		vec[0] = fastRand()*2-1; //not entirely sure if this shuld be insidde the loop
		while(i >= 0){
			vec[1] = fastRand()*2-1;
			// light_view - normal_light * normal_view
			s = vec[2] - (vec[0]*vec[1]);
			
			if(s <= 0){
				t = 1;
			}
			else{
				t = fmax(vec[0], vec[1]);
			}
			L = r*vec[1]*(A+B*(s/t));
			compare = fastRand(); //better sampling function than uniform ? try GXX, could have quite a bit of a performance impact
			if(compare >= L){
				break;
			}
			--i;
		}
		//calculate normal vector from angles.....somehow.....
		//solve equation set :
		//(calculation might contain errors)
		scalar v[3];
		scalar * l = light;
		scalar fac = view[0]/l[0];
		vec[0] -= vec[1]*fac;
		v[0] = view[0]-l[0]*fac;
		v[1] = view[1]-l[1]*fac;
		v[2] = view[2]-l[2]*fac;
		
		result[2] = fastRand()*100-50; // a random number between -inf and +inf
		result[1] = (vec[0]-result[2]*v[2])/v[1];
		result[0] = (vec[1]-result[2]*l[2]-result[1]*l[1]) / l[0];
		vNormalize(result);
		return;
		*/
		

	}
}