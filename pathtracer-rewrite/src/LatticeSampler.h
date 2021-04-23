#pragma once
#include "include.h"


class LatticeSampler:public virtual  Sampler{
	int grid_res;
	int *cell_positions;
	
	int cell_counter;

public:
	~LatticeSampler(){
		std::free(cell_positions);
		cell_positions = NULL;
	};
	LatticeSampler(int grid_res=4){
		this->grid_res = grid_res;
		cell_positions = (int *) malloc(sizeof(int)*grid_res*grid_res*2);
		cell_counter = 0;
		int x = 0;
		int y = 0;
		int i = 0;
		while(x < grid_res){
			while(y < grid_res){
				cell_positions[i*2+0] = x;
				cell_positions[i*2+1] = y;
				i ++;
				y ++;
			}
			y = 0;
			x ++;
		}
	};

	LatticeSampler * clone(){
		LatticeSampler * result = (LatticeSampler *) std::malloc(sizeof(LatticeSampler));
		memcpy(result, this, sizeof(LatticeSampler));
		return result;
	}


	
};