#pragma once

#include "include.h"

namespace GXX{ //excelent resource : http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html

	scalar Pdf(scalar * test, scalar * view_refl, scalar r){
		//expect every vector to be normalized
		scalar r_4 = POW4(r);
		
		scalar cos_2 = std::max({vDot(test, view_refl), (scalar)0.000001});
		cos_2 *= cos_2;
		return fmin((r_4)/fmax(M_PI*POW2((r_4-1)*cos_2+1), (scalar) 0.00001), 1);
	}
	
	scalar PdfF(scalar x, scalar r){
		scalar r_2 = POW4(r);
		
		scalar cos_x = cos(x);
		scalar k = ((r_2-1)*POW2(cos_x) + 1);
		return (r_2)/(M_PI*k*k);
	}
	
	scalar CdfF(scalar x, scalar r){ //x is the halfvector angle
		r *= r;
		scalar r_2 = r*r;
		scalar tan_x = tan(x);
		return r_2*( ((r_2+1)*atan(tan_x/r))/(2*r_2*r) - ((r_2-1)*tan_x)/(2*r_2 * POW2(tan_x) + 2*r_2*r_2) );
	}
	
	
	scalar InvCdf(scalar result, scalar r){ //use Newtons method, should work quite well since the function in very continuous
		scalar x = r; //not sure if this start value is ideal
		scalar b;
		int i = 0;
		while(i < 8){ //8 steps have to be enough
			b = x - (CdfF(x, r)-result)/PdfF(x, r);
			if(fabs(x-b) < 0.001){ //this is not entirely accurate since the x position might not change anymore but the y value may still not be converged
				return b;
			}
			x = b;
			i ++;
		}
		return fabs(x);
	}
	
	scalar Shadowing(scalar * normal, scalar * view, scalar r, scalar factor=-1){
		r = fmax(r, 0.0001);
		scalar d = fmax(0, factor * vDot(normal, view));
		return 2*d/(d+sqrt(POW4(r) + (1-r*r)*d));
	}
	
	scalar PdfExp(scalar * normal, scalar * view, scalar * light, scalar r){
		scalar H[3];
		vCpy(H, light);
		vSub(H, view);
		vNormalize(H);
		scalar c = fmax(0, vDot(normal, H));
		c = c*c*(r*r-1)+1;
		c = c*c;
		return POW4(r)/(M_PI*c);
	}
	
	void Sample(scalar * result, scalar * view, scalar * normal, scalar r){
		
		
		return;
	}
	
	void SampleNormal(scalar * result, scalar * view, scalar * normal, scalar * tangent, scalar roughness, bool reflect=false){ //after this paper : http://jcgt.org/published/0007/04/01/paper.pdf
		roughness = fmax(roughness, 0.0001);
		scalar alpha_x = POW4(roughness); //same for now
		scalar alpha_y = POW4(roughness);
		scalar U1;
		scalar U2;
		scalar reflection[3];
		
		
		//transform view direction in local space :
		scalar X[3]; //build basis vector
		vCross(normal, tangent, X);
		scalar V[3];
		vSolve(V, X, tangent, normal, view);
		vFlip(V);
		
		scalar Vh[3] = {alpha_x*V[0], alpha_y*V[1], V[2]};
		vNormalize(Vh);
		
		scalar lensq = Vh[0]*Vh[0] + Vh[1]*Vh[1];
		scalar T1[3];
		if(lensq > 0){
			T1[0] = -Vh[1];
			T1[1] =  Vh[0];
			T1[2] =  0;
			vDivF(T1, sqrt(lensq));
		}
		else{
			T1[0] = 1;
			T1[1] = 0;
			T1[2] = 0;
		}
		scalar T2[3];
		vCross(Vh, T1, T2);
		
		scalar r;
		scalar phi;
		scalar t1;
		scalar t2;
		scalar s;
		scalar b;
		scalar Nh[3];
		scalar Ne[3];
		
		retry:
		U1 = fastRand();
		U2 = fastRand();
		r = sqrt(U1);
		phi = 2.0*M_PI*U2;
		t1 = r * cos(phi);
		t2 = r * sin(phi);
		s = 0.5 * (1.0+Vh[2]);
		t2 = (1.0-s)*sqrt(1.0-t1*t1) + s*t2;
		
		
		//apply rotation
		b = sqrt(fmax(0.0, 1.0 - t1*t1 - t2*t2));
		Nh[0] = t1*T1[0] + t2*T2[0] + b*Vh[0];
		Nh[1] = t1*T1[1] + t2*T2[1] + b*Vh[1];
		Nh[2] = t1*T1[2] + t2*T2[2] + b*Vh[2];
		
		Ne[0] = alpha_x*Nh[0];
		Ne[1] = alpha_y*Nh[1];
		Ne[2] = fmax(0.0, Nh[2]);
		
		
		if(reflect){
			vReflect(V, Ne, reflection);
			if(reflection[2] > 0){
				goto retry;
			}
		}
		
		
		vNormalize(Ne);
		
		//transform to global space
		scalar Nr[3]; //apply transformation
		
		Nr[0] = X[0]*Ne[0] + tangent[0]*Ne[1] + normal[0]*Ne[2];
		Nr[1] = X[1]*Ne[0] + tangent[1]*Ne[1] + normal[1]*Ne[2];
		Nr[2] = X[2]*Ne[0] + tangent[2]*Ne[1] + normal[2]*Ne[2];
		vNormalize(Nr);
		
		vCpy(result, Nr);
	}
	
	
	void SampleNumeric(scalar * result, scalar r){
		//use InvCdf for sampling
	}
	
	
	void SampleRefraction(scalar * result, scalar * view, scalar * n, scalar r, scalar in_ior, scalar out_ior){
		
		vRefract(view, n, in_ior, out_ior, result);
	}
}