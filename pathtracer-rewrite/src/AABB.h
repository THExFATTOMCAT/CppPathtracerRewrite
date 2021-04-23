#pragma once
#include "include.h"

unsigned long aabb_intersection_tests = 0;
unsigned long aabb_intersections_success = 0;


#pragma pack(push, 1)
class AABB{
public:	
	scalar min[3];
	scalar max[3];
	
	scalar Surface();
	bool IntersectRay(Ray * ray, scalar * t_min);
	bool IntersectRay_inline(Ray * ray, scalar * t_min);
	bool Contains(scalar * v);
};
#pragma pack(pop)

bool AABB::IntersectRay(Ray * ray, scalar * t_min){
	++ aabb_intersection_tests;
	
	if(Contains(ray->o)){
		return true;
	}
	
	scalar t_max;
	
	scalar t0[2];
	scalar t1[2];
	scalar t2[2];
	
	scalar bias = 0.0001;
	
	
	t0[0] = (this->min[0]-bias - ray->o[0]) * ray->i[0];
	t1[0] = (this->min[1]-bias - ray->o[1]) * ray->i[1];
	t2[0] = (this->min[2]-bias - ray->o[2]) * ray->i[2];
	
	t0[1] = (this->max[0]+bias - ray->o[0]) * ray->i[0];
	t1[1] = (this->max[1]+bias - ray->o[1]) * ray->i[1];
	t2[1] = (this->max[2]+bias - ray->o[2]) * ray->i[2];
	
	
	*t_min = std::max({(scalar)0.0, std::min({t0[0], t0[1]}), std::min({t1[0], t1[1]}), std::min({t2[0], t2[1]})});
	t_max = std::min({              std::max({t0[0], t0[1]}), std::max({t1[0], t1[1]}), std::max({t2[0], t2[1]})});
	
	bool result = (t_max >= 0)&(t_max >= *t_min);
	
	aabb_intersections_success += result;
	return result;
}

inline bool AABB::IntersectRay_inline(Ray * ray, scalar * t_min){
	++ aabb_intersection_tests;
	
	scalar t_max;
	
	scalar t0[2];
	scalar t1[2];
	scalar t2[2];
	
	scalar bias = 0.0001;
	
	t0[0] = (this->min[0]-bias - ray->o[0]) * ray->i[0];
	t1[0] = (this->min[1]-bias - ray->o[1]) * ray->i[1];
	t2[0] = (this->min[2]-bias - ray->o[2]) * ray->i[2];
	
	t0[1] = (this->max[0]+bias - ray->o[0]) * ray->i[0];
	t1[1] = (this->max[1]+bias - ray->o[1]) * ray->i[1];
	t2[1] = (this->max[2]+bias - ray->o[2]) * ray->i[2];
	
	
	*t_min = std::max({(scalar)0.0, std::min({t0[0], t0[1]}), std::min({t1[0], t1[1]}), std::min({t2[0], t2[1]})});
	t_max = std::min({std::max({t0[0], t0[1]}), std::max({t1[0], t1[1]}), std::max({t2[0], t2[1]})});
	
	bool result = (t_max >= 0)&(t_max >= *t_min);
	
	aabb_intersections_success += result;
	return result;
}

bool AABB::Contains(scalar * v){
	return 	v[0] >= min[0] and v[0] <= max[0] and
			v[1] >= min[1] and v[1] <= max[1] and
			v[2] >= min[2] and v[2] <= max[2]	;
}

scalar AABB::Surface(){
	return (max[0]-min[0])*(max[1]-min[1])*2+
           (max[0]-min[0])*(max[2]-min[2])*2+
           (max[1]-min[1])*(max[2]-min[2])*2;
}

void AABB_IntersectRay2(AABB * aabb0, AABB * aabb1, bool * result, Ray * ray, scalar * t_min){
	result[0] = aabb0->IntersectRay_inline(ray, &t_min[0]);
	result[1] = aabb1->IntersectRay_inline(ray, &t_min[1]);
}

inline void AABB_IntersectRay2SIMD(AABB * aabb0, AABB * aabb1, bool * result, Ray * ray, scalar * t_min){
	
	__m128 T[3]; //Load AABB data
	T[0] = _mm_set_ps( aabb0->max[0], aabb0->min[2], aabb0->min[1], aabb0->min[0] );
	T[1] = _mm_set_ps( aabb1->min[1], aabb1->min[0], aabb0->max[2], aabb0->max[1] );
	T[2] = _mm_set_ps( aabb1->max[2], aabb1->max[1], aabb1->max[0], aabb1->min[2] );
	
	
	__m128 o[3]; //Load Ray origin
	o[0] = _mm_set_ps( ray->o[0], ray->o[2], ray->o[1], ray->o[0] );
	o[1] = _mm_set_ps( ray->o[1], ray->o[0], ray->o[2], ray->o[1] );
	o[2] = _mm_set_ps( ray->o[2], ray->o[1], ray->o[0], ray->o[2] );
	
	__m128 i[3]; //Load Ray direction-inverse
	i[0] = _mm_set_ps( ray->i[0], ray->i[2], ray->i[1], ray->i[0] );
	i[1] = _mm_set_ps( ray->i[1], ray->i[0], ray->i[2], ray->i[1] );
	i[2] = _mm_set_ps( ray->i[2], ray->i[1], ray->i[0], ray->i[2] );
	
	
	T[0] = _mm_sub_ps(T[0], o[0]);
	T[1] = _mm_sub_ps(T[1], o[1]);
	T[2] = _mm_sub_ps(T[2], o[2]);
	
	
	
	
	T[0] = _mm_mul_ps(T[0], i[0]);
	T[1] = _mm_mul_ps(T[1], i[1]);
	T[2] = _mm_mul_ps(T[2], i[2]);
	
	scalar * t = (scalar*)T;
	
	scalar t_max[2];
	
	t_min[0] = std::max({(scalar)0.0, std::min({t[3], t[0]}), std::min({t[4], t[1]}), std::min({t[5], t[2]})});
	t_min[1] = std::max({(scalar)0.0, std::min({t[0+6], t[3+6]}), std::min({t[1+6], t[4+6]}), std::min({t[2+6], t[5+6]})});
	
	t_max[0] = std::min({             std::max({t[3], t[0]}), std::max({t[4], t[1]}), std::max({t[5], t[2]})});
	t_max[1] = std::min({             std::max({t[0+6], t[3+6]}), std::max({t[1+6], t[4+6]}), std::max({t[2+6], t[5+6]})});
	
	result[0] = (t_max[0] >= 0)&(t_max[0] >= t_min[0]);
	result[1] = (t_max[1] >= 0)&(t_max[1] >= t_min[1]);

	return;
	
}

#if USE_QUAD_TREE
inline void AABB_IntersectRay4SIMD(AABB * aabb0, AABB * aabb1, AABB * aabb2, AABB * aabb3, bool * result, Ray * ray, scalar * t_min){
	
	__m256 T[3]; //Load AABB data //memcpy is actually not faster
	
	T[0] = SET_VEC8(
		aabb0->min[0],
		aabb0->min[1],
		aabb0->min[2],
		aabb0->max[0],
		aabb0->max[1],
		aabb0->max[2],
		//------------
		aabb1->min[0],
		aabb1->min[1]
	);
	T[1] = SET_VEC8(
		aabb1->min[2],
		aabb1->max[0],
		aabb1->max[1],
		aabb1->max[2],
		//------------
		aabb2->min[0],
		aabb2->min[1],
		aabb2->min[2],
		aabb2->max[0]
	);
	T[2] = SET_VEC8(
		aabb2->max[1],
		aabb2->max[2],
		//------------
		aabb3->min[0],
		aabb3->min[1],
		aabb3->min[2],
		aabb3->max[0],
		aabb3->max[1],
		aabb3->max[2]
	);
	
	
	__m256 o[3]; //Load Ray origin //memcpy is actually not faster
	o[0] = SET_VEC8(
		ray->o[0],
		ray->o[1],
		ray->o[2],
		ray->o[0],
		ray->o[1],
		ray->o[2],
		ray->o[0],
		ray->o[1]
	);
	o[1] = SET_VEC8(
		ray->o[2],
		ray->o[0],
		ray->o[1],
		ray->o[2],
		ray->o[0],
		ray->o[1],
		ray->o[2],
		ray->o[0]
	);
	o[2] = SET_VEC8(
		ray->o[1],
		ray->o[2],
		ray->o[0],
		ray->o[1],
		ray->o[2],
		ray->o[0],
		ray->o[1],
		ray->o[2]
	);
	
	
	__m256 i[3]; //Load Ray direction-inverse

	i[0] = SET_VEC8(
		ray->i[0],
		ray->i[1],
		ray->i[2],
		ray->i[0],
		ray->i[1],
		ray->i[2],
		ray->i[0],
		ray->i[1]
	);
	i[1] = SET_VEC8(
		ray->i[2],
		ray->i[0],
		ray->i[1],
		ray->i[2],
		ray->i[0],
		ray->i[1],
		ray->i[2],
		ray->i[0]
	);
	i[2] = SET_VEC8(
		ray->i[1],
		ray->i[2],
		ray->i[0],
		ray->i[1],
		ray->i[2],
		ray->i[0],
		ray->i[1],
		ray->i[2]
	);
	
	
	T[0] = _mm256_sub_ps(T[0], o[0]);
	T[1] = _mm256_sub_ps(T[1], o[1]);
	T[2] = _mm256_sub_ps(T[2], o[2]);
	
	
	
	
	T[0] = _mm256_mul_ps(T[0], i[0]);
	T[1] = _mm256_mul_ps(T[1], i[1]);
	T[2] = _mm256_mul_ps(T[2], i[2]);
	
	scalar * t = (scalar*)T;
	
	scalar t_max[4];
	
	__m128 buff0;
	__m128 buff1;
	__m128 buff2;
	
	buff0 = _mm_min_ps(SET_VEC4(t[3], t[6], t[12], t[18]), SET_VEC4(t[0], t[9],  t[15], t[21]));
	buff1 = _mm_min_ps(SET_VEC4(t[4], t[7], t[13], t[19]), SET_VEC4(t[1], t[10], t[16], t[22]));
	buff2 = _mm_min_ps(SET_VEC4(t[5], t[8], t[14], t[20]), SET_VEC4(t[2], t[11], t[17], t[23]));
	
	*((__m128*)t_min) = _mm_set_ps1(0);
	*((__m128*)t_min) = _mm_max_ps(*((__m128*)t_min), buff0);
	*((__m128*)t_min) = _mm_max_ps(*((__m128*)t_min), buff1);
	*((__m128*)t_min) = _mm_max_ps(*((__m128*)t_min), buff2);
	/*
	t_min[0] = std::max({(scalar)0.0, std::min({t[3],    t[0]}),    std::min({t[4],    t[1]}),    std::min({t[5],    t[2]})});
	t_min[1] = std::max({(scalar)0.0, std::min({t[0+6],  t[3+6]}),  std::min({t[1+6],  t[4+6]}),  std::min({t[2+6],  t[5+6]})});
	t_min[2] = std::max({(scalar)0.0, std::min({t[0+12], t[3+12]}), std::min({t[1+12], t[4+12]}), std::min({t[2+12], t[5+12]})});
	t_min[3] = std::max({(scalar)0.0, std::min({t[0+18], t[3+18]}), std::min({t[1+18], t[4+18]}), std::min({t[2+18], t[5+18]})});
	*/
	
	
	buff0 = _mm_max_ps(SET_VEC4(t[3], t[6], t[12], t[18]), SET_VEC4(t[0], t[9],  t[15], t[21]));
	buff1 = _mm_max_ps(SET_VEC4(t[4], t[7], t[13], t[19]), SET_VEC4(t[1], t[10], t[16], t[22]));
	buff2 = _mm_max_ps(SET_VEC4(t[5], t[8], t[14], t[20]), SET_VEC4(t[2], t[11], t[17], t[23]));
	
	*((__m128*)t_max) = _mm_min_ps(buff0, buff1);
	*((__m128*)t_max) = _mm_min_ps(*((__m128*)t_max), buff2);
	
	/*
	t_max[0] = std::min({             std::max({t[3],    t[0]}),    std::max({t[4],    t[1]}),    std::max({t[5],    t[2]})});
	t_max[1] = std::min({             std::max({t[0+6],  t[3+6]}),  std::max({t[1+6],  t[4+6]}),  std::max({t[2+6],  t[5+6]})});
	t_max[2] = std::min({             std::max({t[0+12], t[3+12]}), std::max({t[1+12], t[4+12]}), std::max({t[2+12], t[5+12]})});
	t_max[3] = std::min({             std::max({t[0+18], t[3+18]}), std::max({t[1+18], t[4+18]}), std::max({t[2+18], t[5+18]})});
	*/
	
	result[0] = (t_max[0] >= 0)&(t_max[0] >= t_min[0]);
	result[1] = (t_max[1] >= 0)&(t_max[1] >= t_min[1]);
	result[2] = (t_max[2] >= 0)&(t_max[2] >= t_min[2]);
	result[3] = (t_max[3] >= 0)&(t_max[3] >= t_min[3]);

	return;
	
}
#endif






