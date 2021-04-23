#pragma once

#include "include.h"

class Triangle{
public:
	scalar mid[3];
	
	scalar o[3];
	scalar u[3];
	scalar v[3];
	
	scalar uv[3][2];
	scalar normals[3][3];
	
	Material * material;
	
	Triangle(){
		material = NULL;
	}
	
	bool smooth_shading;
	
	void GetNormal(scalar * uvs, scalar * normal);
	void GetUV(scalar x, scalar y, scalar * uv);
	void Bounds(AABB * aabb);
	bool IntersectRay(Ray * ray, scalar min, Hit * hit);
	
};

void Triangle::GetNormal(scalar * uvs, scalar * normal){
	if(smooth_shading or true){
		normal[0] = normals[0][0]*(1-uvs[0]-uvs[1]) + normals[1][0]*uvs[0] +normals[2][0]*uvs[1];
		normal[1] = normals[0][1]*(1-uvs[0]-uvs[1]) + normals[1][1]*uvs[0] +normals[2][1]*uvs[1];
		normal[2] = normals[0][2]*(1-uvs[0]-uvs[1]) + normals[1][2]*uvs[0] +normals[2][2]*uvs[1];
		vNormalize(normal);
	}
	else{
		normal[0] = normals[0][0];
		normal[1] = normals[0][1];
		normal[2] = normals[0][2];
	}
	return;
}

void Triangle::GetUV(scalar x, scalar y, scalar * r){
	r[0] = uv[0][0]*(1-x-y) + uv[1][0]*x +uv[2][0]*y;
	r[1] = uv[0][1]*(1-x-y) + uv[1][1]*x +uv[2][1]*y;
	return;
}

void Triangle::Bounds(AABB * aabb){
	vCpy(aabb->min, o);
	vCpy(aabb->max, o);
	
	scalar a[3];
	scalar b[3];
	vCpy(a, o);
	vCpy(b, o);
	vAdd(a, u);
	vAdd(b, v);
	
	vMin(aabb->min, a);
	vMin(aabb->min, b);
	
	vMax(aabb->max, a);
	vMax(aabb->max, b);
}

bool Triangle::IntersectRay(Ray * ray, scalar min, Hit * hit){
	//if(exclude == (void *) this){
	//	return false;
	//}
	++ primitive_tests;
	scalar Q[3];
	scalar q_u[3];
	scalar d_v[3];

	vCpy(Q, ray->o);
	vSub(Q, o);

	vCross(ray->d, v, d_v);

	scalar bias = 0.00000001;
	scalar detM = vDot(d_v, u);
	if(detM == 0){
		return false;
	}
	
	detM = (scalar) 1/detM;

	scalar uv[2];
	uv[0] = vDot(d_v, Q) * detM;
	if(uv[0]-bias > 1 or uv[0]+bias < 0){
		return false;
	}

	vCross(Q, u, q_u);
	uv[1] = vDot(q_u, ray->d) * detM;
	if(uv[1]-bias > 1 || uv[1]+bias < 0 || uv[0]+uv[1]-bias*2 > 1){
		return false;
	}
	scalar t = vDot(q_u, v) * detM;
	if(t < 0 or t > min){
		return false;
	}
	
	++primitive_intersections;
	hit->t = t;
	hit-> uvs[0] = uv[0];
	hit-> uvs[1] = uv[1];
	hit->prim = (void *) this;
	return true;
}


void Triangle_IntersectRay4SIMD(Triangle * tri, Ray * ray, scalar min, Hit * hit, bool * RESULT){
	

	__m128 D[3];
	__m128 Q[3];
	__m128 U[3];
	__m128 V[3];
	
	__m128 CROSS_d_v[3];
	__m128 CROSS_q_u[3];

	D[0] = SET_VEC4(
		ray->d[0],
		ray->d[1],
		ray->d[2],
		ray->d[0]
	);
	D[1] = SET_VEC4(
		ray->d[1],
		ray->d[2],
		ray->d[0],
		ray->d[1]
	);
	D[2] = SET_VEC4(
		ray->d[2],
		ray->d[0],
		ray->d[1],
		ray->d[2]
	);

	Q[0] = SET_VEC4(
		ray->o[0]-tri[0].o[0],
		ray->o[1]-tri[0].o[1],
		ray->o[2]-tri[0].o[2],
		ray->o[0]-tri[1].o[0]
	);
	Q[1] = SET_VEC4(
		ray->o[1]-tri[1].o[1],
		ray->o[2]-tri[1].o[2],
		ray->o[0]-tri[2].o[0],
		ray->o[1]-tri[2].o[1]
	);
	Q[2] = SET_VEC4(
		ray->o[2]-tri[2].o[2],
		ray->o[0]-tri[3].o[0],
		ray->o[1]-tri[3].o[1],
		ray->o[2]-tri[3].o[2]
	);
	
	U[0] = SET_VEC4(
		tri[0].u[0],
		tri[0].u[1],
		tri[0].u[2],
		tri[1].u[0]
	);
	U[1] = SET_VEC4(
		tri[1].u[1],
		tri[1].u[2],
		tri[2].u[0],
		tri[2].u[1]
	);
	U[2] = SET_VEC4(
		tri[2].u[2],
		tri[3].u[0],
		tri[3].u[1],
		tri[3].u[2]
	);
	
	V[0] = SET_VEC4(
		tri[0].v[0],
		tri[0].v[1],
		tri[0].v[2],
		tri[1].v[0]
	);
	V[1] = SET_VEC4(
		tri[1].v[1],
		tri[1].v[2],
		tri[2].v[0],
		tri[2].v[1]
	);
	V[2] = SET_VEC4(
		tri[2].v[2],
		tri[3].v[0],
		tri[3].v[1],
		tri[3].v[2]
	);
	
	scalar inv_M[4];
	scalar detU[4];
	scalar detT[4];
	scalar detV[4];
	
	CROSS_4(D, V, CROSS_d_v);
	DOT_4((scalar*)CROSS_d_v, (scalar*)U, (scalar*) inv_M);

	__m128 vec_0 = _mm_set_ps1(0);
	__m128 vec_1 = _mm_set_ps1(1.0000001);
	
	int result[4];
	
	*((__m128*)result) = _mm_cmpneq_ps(*((__m128*) inv_M), vec_0); //determinant non-zero

	if(not (result[0] | result[1] | result[2] | result[3])){
		RESULT[0] = result[0];
		RESULT[1] = result[1];
		RESULT[2] = result[2];
		RESULT[3] = result[3];
		return;
	}

	(* (__m128*)inv_M) = _mm_div_ps(_mm_set_ps1(1.0), *((__m128*) inv_M));

	if(not (result[0] | result[1] | result[2] | result[3])){
		RESULT[0] = result[0];
		RESULT[1] = result[1];
		RESULT[2] = result[2];
		RESULT[3] = result[3];
		return;
	}
	DOT_4((scalar*)CROSS_d_v, (scalar*)Q, detU); //U
	*((__m128*)detU) = _mm_mul_ps(*((__m128*)detU), *((__m128*) inv_M));
	*((__m128*)result) = _mm_and_ps(*((__m128*)result), // &= u <= 1 and u >= 0
									_mm_and_ps(_mm_cmple_ps(*((__m128*)detU), vec_1),
											   _mm_cmpge_ps(*((__m128*)detU), vec_0)
									)
	);

	CROSS_4(Q, U, CROSS_q_u);
	DOT_4((scalar*)CROSS_q_u, (scalar*)D, detV); //V
	*((__m128*)detV) = _mm_mul_ps(*((__m128*)detV), *((__m128*) inv_M));

	*((__m128*)result) = _mm_and_ps(*((__m128*)result), // &= v <= 1 and v >= 0
		_mm_and_ps(_mm_cmple_ps(*((__m128*)detV), vec_1),
				   _mm_cmpge_ps(*((__m128*)detV), vec_0)
				   )
	);
	*((__m128*)result) = _mm_and_ps(*((__m128*)result), // &= u+v <= 1
									_mm_cmple_ps(_mm_add_ps(*((__m128*)detU), *((__m128*)detV)), vec_1)
	);
	if(not (result[0] | result[1] | result[2] | result[3])){
		RESULT[0] = result[0];
		RESULT[1] = result[1];
		RESULT[2] = result[2];
		RESULT[3] = result[3];
		return;
	}

	DOT_4((scalar*)CROSS_q_u, (scalar*)V, detT); //T
	*((__m128*)detT) = _mm_mul_ps(*((__m128*)detT), *((__m128*) inv_M));

	*((__m128*)result) = _mm_and_ps(*((__m128*)result), // &= t <= min and t >= 0
		_mm_and_ps(_mm_cmple_ps(*((__m128*)detT), _mm_set_ps1(min)),
				   _mm_cmpge_ps(*((__m128*)detT), vec_0)
				   )
	);
	//write hit results :
	hit[0].uvs[0] = detU[0];
	hit[1].uvs[0] = detU[1];
	hit[2].uvs[0] = detU[2];
	hit[3].uvs[0] = detU[3];
	
	hit[0].uvs[1] = detV[0];
	hit[1].uvs[1] = detV[1];
	hit[2].uvs[1] = detV[2];
	hit[3].uvs[1] = detV[3];
	
	hit[0].t = detT[0];
	hit[1].t = detT[1];
	hit[2].t = detT[2];
	hit[3].t = detT[3];
	
	hit[0].prim = (void *) &tri[0];
	hit[1].prim = (void *) &tri[1];
	hit[2].prim = (void *) &tri[2];
	hit[3].prim = (void *) &tri[3];
	
		RESULT[0] = result[0];
		RESULT[1] = result[1];
		RESULT[2] = result[2];
		RESULT[3] = result[3];
	return;
}