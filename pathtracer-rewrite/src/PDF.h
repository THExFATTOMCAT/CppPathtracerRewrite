#pragma once

#include "include.h"

class PDF{
public:
	int dim;
	scalar * parameters;
	void PDF(int dim=1, int parameters){
		this->dim = dim;
		this->parameters = (scalar *) malloc(sizeof(scalar) * parameters);
	}
	scalar Pdf(scalar * input);
	void Rejection(PDF * dist, scalar * result);
	void Sample(scalar * result);
}

scalar PDF::Pdf(scalar * input){ //uniform distribution as standatd pdf
	return 1;
}

void PDF::Rejection(PDF * dist, scalar * result){ //The so called "rejection method" for correct sampling of a pdf with unknown cdf -> a brute force universal solution
	scalar compare;
	int i = 0;
	int j;
	
	while(i < 20){
		j = dim-1;
		while(j >= 0){
			result[j] = fastRand();
			compare = fastRand();
			if(compare < dist.Pdf(result)){
				break;
			}
			
			--j;
		}
		++ i;
	}
}

void PDF::Sample(scalar * result){
	result[0] = fastRand();
}



//TRY COMBINING Oren-Nayar/GXX PDF IN ORDER TO BE ABLE TO CAST JUST ONE UNIVERSAL RAY
//sampling should at least be possible using the rejection method