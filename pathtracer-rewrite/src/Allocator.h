#pragma once

#include "include.h"

namespace Allocator{
	unsigned int base;
	void * base_ptr;	
	
	void * pointer;
	unsigned int length = 0;
	unsigned int pos = 0;
	
	void * allocation_list[16384]; // approx 16GB capacity
	unsigned int allocation_list_pos = 0;

	unsigned int buffer_size = 1024*1024;//1 MB
	
	std::mutex mtx;
	
	void Extend(){
		pos = 0;
		
		allocation_list_pos ++;
		allocation_list[allocation_list_pos] = malloc(buffer_size);
		if(allocation_list_pos == 1){
			base_ptr = allocation_list[allocation_list_pos];
			memcpy(&base, &base_ptr, 4);
		}
		
		pointer = allocation_list[allocation_list_pos];
		std::cout << "allocation blocks : " << allocation_list_pos << "\n";
		std::cout << "ptr : " << (unsigned long long) allocation_list[allocation_list_pos] << "\n";
	}
	
	void * Alloc(unsigned int size){
		mtx.lock();
		if(pos+size > buffer_size or allocation_list_pos <= 0){
			std::cout << "pos : " << pos << "    size : " << size << "\n";
			Extend();
		}
		pos += size;
		mtx.unlock();
		return pointer + pos - size;
	}
	
	int AllocInt(unsigned int size){
		union R{
			void * ptr;
			long long lng;
			int half[2];
		};
		R r;
		r.ptr = Alloc(size);
		r.lng -= (long long)base_ptr;
		std::cout << "int : " << r.half[0] << "\n";
		return r.half[0];
	}
	
	
	void FreeAll(){
		while(allocation_list_pos > 0){
			free(allocation_list[allocation_list_pos]);
			allocation_list_pos --;
			
			pos = 0;
		}
	}
	
}