#pragma once

#include "include.h"

#define SET_VEC4(a, b, c, d) _mm_set_ps(d, c, b, a)
#define SET_VEC8(a, b, c, d, e, f, g, h) _mm256_set_ps(h, g, f, e, d, c, b, a)

inline void CROSS_4(__m128 * A, __m128 * B, __m128 * R){
	
	__m128 bA[3];
	__m128 bB[3];
	
	scalar * a = (scalar *) A;
	scalar * b = (scalar *) B;
	
	bA[0] = SET_VEC4(
		a[1 + 0*3],
		a[2 + 0*3],
		a[0 + 0*3],
		a[1 + 1*3]
	);
	bA[1] = SET_VEC4(
		a[2 + 1*3],
		a[0 + 1*3],
		a[1 + 2*3],
		a[2 + 2*3]
	);
	bA[2] = SET_VEC4(
		a[0 + 2*3],
		a[1 + 3*3],
		a[2 + 3*3],
		a[0 + 3*3]
	);
	
	bB[0] = SET_VEC4(
		b[2 + 0*3],
		b[0 + 0*3],
		b[1 + 0*3],
		b[2 + 1*3]
	);
	bB[1] = SET_VEC4(
		b[0 + 1*3],
		b[1 + 1*3],
		b[2 + 2*3],
		b[0 + 2*3]
	);
	bB[2] = SET_VEC4(
		b[1 + 2*3],
		b[2 + 3*3],
		b[0 + 3*3],
		b[1 + 3*3]
	);
	
	__m128 bC[3];
	
	bC[0] = _mm_mul_ps(bA[0], bB[0]);
	bC[1] = _mm_mul_ps(bA[1], bB[1]);
	bC[2] = _mm_mul_ps(bA[2], bB[2]);
	
	
	bA[0] = SET_VEC4(
		a[2 + 0*3],
		a[0 + 0*3],
		a[1 + 0*3],
		a[2 + 1*3]
	);
	bA[1] = SET_VEC4(
		a[0 + 1*3],
		a[1 + 1*3],
		a[2 + 2*3],
		a[0 + 2*3]
	);
	bA[2] = SET_VEC4(
		a[1 + 2*3],
		a[2 + 3*3],
		a[0 + 3*3],
		a[1 + 3*3]
	);
	
	bB[0] = SET_VEC4(
		b[1 + 0*3],
		b[2 + 0*3],
		b[0 + 0*3],
		b[1 + 1*3]
	);
	bB[1] = SET_VEC4(
		b[2 + 1*3],
		b[0 + 1*3],
		b[1 + 2*3],
		b[2 + 2*3]
	);
	bB[2] = SET_VEC4(
		b[0 + 2*3],
		b[1 + 3*3],
		b[2 + 3*3],
		b[0 + 3*3]
	);
	
	R[0] = _mm_sub_ps(bC[0], _mm_mul_ps(bA[0], bB[0]));
	R[1] = _mm_sub_ps(bC[1], _mm_mul_ps(bA[1], bB[1]));
	R[2] = _mm_sub_ps(bC[2], _mm_mul_ps(bA[2], bB[2]));
	
}

inline void DOT_4(scalar * A, scalar * B, scalar * R){
	R[0] = vDot(&A[0], &B[0]);
	R[1] = vDot(&A[3], &B[3]);
	R[2] = vDot(&A[6], &B[6]);
	R[3] = vDot(&A[9], &B[9]);
}