#pragma once

#include "include.h"
//define Vectors as operations on buffers insted of a class for itself
/*
#define vCpy(r, v){ \
	r[0] = v[0]; \
	r[1] = v[1]; \
	r[2] = v[2]; \
}
*/

static inline void vCpy(scalar * r, scalar * v){ 
	r[0] = v[0];
	r[1] = v[1];
	r[2] = v[2];
	//memcpy((char *)r, (char*)v, 3*8);
}

static inline void vAdd(scalar * a, scalar * b){
	a[0] += b[0];
	a[1] += b[1];
	a[2] += b[2];
}
static inline void vSub(scalar * a, scalar * b){
	a[0] -= b[0];
	a[1] -= b[1];
	a[2] -= b[2];
}
static inline void vMul(scalar * a, scalar * b){
	a[0] *= b[0];
	a[1] *= b[1];
	a[2] *= b[2];
}
static inline void vMulF(scalar * a, scalar b){
	a[0] *= b;
	a[1] *= b;
	a[2] *= b;
}
static inline void vDiv(scalar * a, scalar * b){
	a[0] /= b[0];
	a[1] /= b[1];
	a[2] /= b[2];
}
static inline void vDivF(scalar * a, scalar b){
	scalar ib = 1/b;
	a[0] *= ib;
	a[1] *= ib;
	a[2] *= ib;
}
static inline scalar vDot(scalar * a, scalar * b){
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}
static inline void vCross(scalar * a, scalar * b, scalar * r){
	r[0] = a[1]*b[2] - a[2]*b[1];
	r[1] = a[2]*b[0] - a[0]*b[2];
	r[2] = a[0]*b[1] - a[1]*b[0];
}
static inline scalar vDet(scalar * a, scalar * b, scalar * c){
	return 	(a[1]*b[2]-a[2]*b[1])*c[0] +
			(a[2]*b[0]-a[0]*b[2])*c[1] +
			(a[0]*b[1]-a[1]*b[0])*c[2] ;
}

inline void vSolve(scalar * r, scalar * x, scalar * y, scalar * z, scalar * s){
	scalar M = vDet(x, y, z);
	scalar X = vDet(s, y, z);
	scalar Y = vDet(x, s, z);
	scalar Z = vDet(x, y, s);
	r[0] = X/M;
	r[1] = Y/M;
	r[2] = Z/M;
}


static inline scalar vLength(scalar * a){
	return sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
}

static inline scalar vLengthS(scalar * a){
	return a[0]*a[0] + a[1]*a[1] + a[2]*a[2];
}

static inline void vNormalize(scalar * a){ //use rsqrt ? faster ?
	scalar i_length = (scalar) 1/sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
	a[0] *= i_length;
	a[1] *= i_length;
	a[2] *= i_length;
}

static inline void vReflect(scalar * v, scalar * n, scalar * r){
	scalar w = 2*(n[0]*v[0]+n[1]*v[1]+n[2]*v[2]);
	r[0] = v[0]-n[0]*w;
	r[1] = v[1]-n[1]*w;
	r[2] = v[2]-n[2]*w;
}


static void vRandomize(scalar * v, scalar scale) { //implement fastrand() //TO DO !
    scalar theta = fastRand()*2*M_PI;
    scalar phi =  acos(fastRand()*2 - 1);

    scalar sin_phi = sin(phi);
    v[0] = (sin_phi * cos(theta))*scale + v[0]*(1-scale);
    v[1] = (sin_phi * sin(theta))*scale + v[1]*(1-scale);
    v[2] =             (cos(phi))*scale + v[2]*(1-scale);
	
    return;
}

static void vRandomizeCone(scalar * v, scalar angle) { //expects v to be normalized for correct results
    scalar theta = fastRand()*2*M_PI;
    scalar phi =  acos(fastRand()*2 - 1)*angle/M_PI; //not sure about the *angle thing
	
    scalar sin_phi = sin(phi);
    scalar c[3];
	c[0] = (sin_phi * cos(theta));
    c[1] = (sin_phi * sin(theta));
    c[2] =             (cos(phi));
	
	//Build basis vectors for transformation :
	scalar X[3];
	scalar Y[3];
	scalar Z[3];
	
	vCpy(Z, v);
	vRandomize(X, 1); //start with random vector for basis
	vCross(Z, X, Y);
	vCross(Z, Y, X);
	
	v[0] = X[0]*c[0] + Y[0]*c[1] + Z[0]*c[2];
	v[1] = X[1]*c[0] + Y[1]*c[1] + Z[1]*c[2];
	v[2] = X[2]*c[0] + Y[2]*c[1] + Z[2]*c[2];
	
    return;
}

static inline scalar vRandomizeDistribution(scalar * v, scalar n) { //expects v to be normalized for correct results
    scalar theta = fastRand()*2*M_PI;
    scalar y =  acos(fastRand()*2 - 1);
	
	scalar factor;
	scalar C = (scalar) 1/(1-n);
	scalar s = (scalar) 1/pow(M_PI+1, n-1)*(1-n) - C;
	scalar phi = pow(((scalar)y/s) + C, (scalar) 1/(1-n)) - 1;
	
    scalar sin_phi = sin(phi);
    scalar c[3];
	c[0] = (sin_phi * cos(theta));
    c[1] = (sin_phi * sin(theta));
    c[2] =             (cos(phi));
	
	//Build basis vectors for transformation :
	scalar X[3];
	scalar Y[3];
	scalar Z[3];
	
	vCpy(Z, v);
	vRandomize(X, 1); //start with random vector for basis
	vCross(Z, X, Y);
	vCross(Z, Y, X);
	
	v[0] = X[0]*c[0] + Y[0]*c[1] + Z[0]*c[2];
	v[1] = X[1]*c[0] + Y[1]*c[1] + Z[1]*c[2];
	v[2] = X[2]*c[0] + Y[2]*c[1] + Z[2]*c[2];
	
    return s*pow(phi+1, n);
}

static inline void vFlip(scalar * a){
	a[0] *= -1;
	a[1] *= -1;
	a[2] *= -1;
}

static inline void vMix(scalar * a, scalar * b, scalar f, scalar * r){
	r[0] = (1-f)*a[0] + f*b[0];
	r[1] = (1-f)*a[1] + f*b[1];
	r[2] = (1-f)*a[2] + f*b[2];
}
//for primitive intersection tests : 
//precalculate as much of the solution as possible / leave just the ray part dynamic !

static inline void vPrint(scalar * v){
	std::cout << v[0] << "\n" << v[1] << "\n" << v[2] << "\n";
}

static void vMax(scalar * a, scalar * b){
	a[0] = std::max({a[0], b[0]});
	a[1] = std::max({a[1], b[1]});
	a[2] = std::max({a[2], b[2]});
}
static void vMin(scalar * a, scalar * b){
	a[0] = std::min({a[0], b[0]});
	a[1] = std::min({a[1], b[1]});
	a[2] = std::min({a[2], b[2]});
}


inline void vRefract(scalar * in, scalar * n, scalar in_ior, scalar out_ior, scalar * result){
	
	scalar r = in_ior/out_ior;
	scalar c = vDot(n, in);
	scalar a = r*r*(1-c*c);
	scalar b = sqrt(1-a);
	
	vCpy(result, in);
	vMulF(result, r);
	
	scalar buff[3];
	vCpy(buff, n);
	vMulF(buff, r*c+b);
	
	vSub(result, buff);
	vNormalize(result);
	return;
}

inline scalar vFresnel(scalar * n, scalar * d, scalar * refraction, scalar in_ior, scalar out_ior){
    scalar i = fabs(vDot(d, n));
    scalar t = fabs(vDot(refraction, n));
	scalar s = (scalar)(in_ior*i - out_ior*t) / (in_ior*i + out_ior*t);
	scalar p = (scalar)(out_ior*i - in_ior*t) / (out_ior*i + in_ior*t);
	
	//for 'normal' materials : (they are randomly polarized so the average is (s+p)*0.5)
	return (scalar)(p*p+s*s)*0.5;
}

inline void vSetF(scalar * v, scalar x){
	v[0] = x;
	v[1] = x;
	v[2] = x;
}


void IntersectRayQuad(scalar * o, scalar * d, scalar * po, scalar * px, scalar * py, scalar * result){
	//if(exclude == (void *) this){
	//	return false;
	//}
	scalar Q[3];
	scalar q_u[3];
	scalar d_v[3];

	vCpy(Q, o);
	vSub(Q, po);

	vCross(d, py, d_v);

	scalar detM = vDot(d_v, px);

	detM = (scalar) 1/detM;

	scalar uv[2];
	uv[0] = vDot(d_v, Q) * detM;

	vCross(Q, px, q_u);
	uv[1] = vDot(q_u, d) * detM;

	scalar t = vDot(q_u, py) * detM;

	result[0] = uv[0];
	result[1] = uv[1];
	result[2] = t;
}
