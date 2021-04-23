#pragma once

#include "include.h"



//int frand_seed = 17438;
inline scalar fastRand() {
    //frand_seed = (214013*frand_seed+2531011);
    //return (scalar)((frand_seed>>16)&0x7FFF)/RAND_MAX;
    return (scalar)rand()/RAND_MAX;
}

#define POW2(X) ((X)*(X))
#define POW3(X) ((X)*(X)*(X))
#define POW4(X) ((X)*(X)*(X)*(X))

unsigned int countString(char * text, char * sub, unsigned int length, unsigned int sub_len){
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int k = 0;
	unsigned int result = 0;
	unsigned int n = length;
	unsigned int m = sub_len;
	while(i < n){
		j = 0;
		k = i;
		while(sub[j] == text[k] and j < m and k < n){
			++j;
			++k;
		}
		if(j == m){
			i = k;
			++ result;
		}
		else{
			++i;
		}
	}
	return result;
}

int extract3sc(scalar * out, char * in){
	char * end_ptr = in;
	out[0] = std::strtof(end_ptr, &end_ptr);
	out[1] = std::strtof(end_ptr, &end_ptr);
	out[2] = std::strtof(end_ptr, &end_ptr);
	return -(in-end_ptr);
}
int extract2sc(scalar * out, char * in){
	char * end_ptr = in;
	out[0] = std::strtof(end_ptr, &end_ptr);
	out[1] = std::strtof(end_ptr, &end_ptr);
	return -(in-end_ptr);
}
int extractFaceBlock(int * v, int * t, int * n, char * in){
	
	//*v = 0;
	*t = 0;
	//*n = 0;
	
	char * end_ptr = in;
	*v = strtol(end_ptr, &end_ptr, 10);
	end_ptr += 2;
	*n = strtol(end_ptr, &end_ptr, 10);
	end_ptr ++;
	std::cout << "diff : " << -(in-end_ptr) << "\n";
	return -(in-end_ptr);
}


bool compareStr(char * a, char * b){
	int i = 0;
	while(a[i]==b[i]){
		if(a[i]=='\0'){
			return true;
		}
		++i;
	}
	return false;
}

void copyStr128(char * d, char * str){
	int i = 0;
	while(i < 128){
		d[i] = str[i];
		if(str[i] == '\0'){
			break;
		}
		i ++;
	}
}

inline scalar fract(scalar x){
	return x - floor(x);
}

inline scalar min4(scalar a, scalar b, scalar c, scalar d){
	return fmin(fmin(a, b), fmin(c, d));
}
inline scalar max4(scalar a, scalar b, scalar c, scalar d){
	return fmax(fmax(a, b), fmax(c, d));
}
