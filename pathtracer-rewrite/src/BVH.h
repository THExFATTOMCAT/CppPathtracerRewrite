#pragma once

#include "include.h"

unsigned long bvh_intersections = 0;

#pragma pack(push, 1)
class BVH{//:public virtual Accelerator{  //removed polymotphism for smaller class size (8bytes) and therefore about 2% performance boost
	struct leaf_struct{
		unsigned int tri_offset;
		unsigned char tri_count;
	};
private:
	
	union info_union{
		BVH * children; //8
		long long ch_int;
		struct leaf_struct leaf;
	};
	
public:
	AABB aabb; //6*4 = 24
	info_union info;
	
	void set_leaf(){
		info.ch_int |= (long long) 0b001 << 63;
	};
	void unset_leaf(){
		info.ch_int = (info.ch_int << 1) >> 1; 
	};
	bool get_leaf(){
		return info.ch_int >> 63;
	};
	
	void set_children(BVH * ptr){
		info.ch_int = ((long long)ptr << 1) >> 1;
	};
	BVH * get_children(){
		return (BVH*)((info.ch_int << 1) >> 1);
	};
	
	void IntersectRay(Ray * ray, scalar * min, Hit * hit, Triangle * primitives, bool shadow_ray);
	void IntersectRayLoop(Ray * ray, scalar * min, Hit * hit, Triangle * primitives, bool shadow_ray); //nonrecursive intersection algorithm (custom stack) (WIP)
	
	
	
	void Build(Triangle * primitives, unsigned int length);
private:
	void Divide(int axis, unsigned int depth, Triangle * primitives, unsigned int tri_count, unsigned int tri_offset, unsigned int min_tri, unsigned int * node_counter, unsigned int rejects, unsigned int bins, unsigned int max_rejects);
}__attribute__((packed));
#pragma pack(pop)

void BVH::Divide(int axis, unsigned int depth, Triangle * primitives, unsigned int tri_count, unsigned int tri_offset, unsigned int min_tri, unsigned int * node_counter, unsigned int rejects=0, unsigned int bins=10, unsigned int max_rejects=3){ //litle changes to the old algorithm (v 1.0)
	unset_leaf();
	if(tri_count == 0){
		std::cout << "build_error\n";
	}
	
	if(depth <= 0 or tri_count < min_tri){
		//is_leaf = true;
		this->info.leaf.tri_offset = tri_offset;
		this->info.leaf.tri_count = tri_count;
		set_leaf();
		
		return;
	}
	
	
	int split_pos = 0;
	
	//find split_pos based on SAH : 3 axis a tri_count bins
	int a;
	int i;
	int index;
	int add = fmax(tri_count/bins, 1);
	scalar cost;
	scalar min_cost = std::numeric_limits<scalar>::max();
	
	AABB aabb_buff;
	
	AABB aabb_left;

	
	a = 0;
	
	AABB * aabb_buff_right = (AABB*) malloc(sizeof(AABB)*tri_count);
	
	while(a < 3){
		SortTriangles(primitives+tri_offset, tri_count, a);
		
		i = tri_count-1;
		
		
		primitives[tri_offset+i].Bounds(&aabb_buff_right[i]);
		--i;
		while(i >= 0){
			vCpy(aabb_buff_right[i].min, aabb_buff_right[i+1].min);
			vCpy(aabb_buff_right[i].max, aabb_buff_right[i+1].max);
			
			primitives[tri_offset+i].Bounds(&aabb_buff);
			
			vMin(aabb_buff_right[i].min, aabb_buff.min);
			vMax(aabb_buff_right[i].max, aabb_buff.max);
			--i;
		}
		
		i = 1;
		primitives[tri_offset+i].Bounds(&aabb_left);
		
		while(i < (int) tri_count){
			
			primitives[tri_offset+i].Bounds(&aabb_buff);
			vMin(aabb_left.min, aabb_buff.min);
			vMax(aabb_left.max, aabb_buff.max);
			
			index = i;
			
			cost = aabb_left.Surface() * (index+1);
			
			cost += aabb_buff_right[i].Surface() * (tri_count - index-1);
			cost /= aabb.Surface();
			cost += 0.125*0.5;
			
			if(cost < min_cost){
				min_cost = cost;
				axis = a;
				split_pos = index;
			}
			i += 1;
		}
		++a;
	}
	
	free(aabb_buff_right);

	
	if(axis != 2){
		SortTriangles(primitives+tri_offset, tri_count, axis);
	}
	
	scalar self_cost = tri_count;
	
	
	
	if(min_cost > self_cost){ //only apply split if the cost is good enough
		++rejects;
	}
	if(rejects >= max_rejects or min_cost > 2*self_cost){
		this->info.leaf.tri_offset = tri_offset;
		this->info.leaf.tri_count = tri_count;
		set_leaf();
		//is_leaf = true;
		return;
	}
	
	
	//is_leaf = false;
	(*node_counter) += 2;
	set_children((BVH*) Allocator::Alloc(sizeof(BVH)*2));
	
	SplitTriangles(&primitives[tri_offset], tri_count, axis, primitives[split_pos].mid[axis]);
	
	GetTrianglesAABB(&primitives[tri_offset], split_pos, &get_children()[0].aabb);
	get_children()[0].set_children(NULL);
	//children[0].is_leaf = true;
	
	
	GetTrianglesAABB(&primitives[tri_offset+split_pos], tri_count-split_pos, &get_children()[1].aabb);
	get_children()[1].set_children(NULL);
	//children[1].is_leaf = true;
	if(false and depth >= 60){
		std::thread thr0(&BVH::Divide, &get_children()[0],
						   axis, depth-1, primitives, split_pos, tri_offset, min_tri, node_counter, rejects, bins, max_rejects);
		std::thread thr1(&BVH::Divide, &get_children()[1],
						   axis, depth-1, primitives, tri_count-split_pos, tri_offset+split_pos, min_tri, node_counter, rejects, bins, max_rejects);
		thr0.join();
		thr1.join();
	}
	else{
		get_children()[0].Divide(axis, depth-1, primitives, split_pos, tri_offset, min_tri, node_counter, rejects, bins, max_rejects);
		get_children()[1].Divide(axis, depth-1, primitives, tri_count-split_pos, tri_offset+split_pos, min_tri, node_counter, rejects, bins, max_rejects);
	}
	unset_leaf();
}


void BVH::Build(Triangle * primitives, unsigned int length){
	std::cout << "leaf_struct size = " << sizeof(leaf_struct) << "\n";
	std::cout << "info_union size = " << sizeof(info_union) << "\n";
	std::cout << "AABB size = " << sizeof(AABB) << "\n";
	unsigned int node_counter = 0;
	info.leaf.tri_count = length;
	info.leaf.tri_offset = 0;
	//is_leaf = true;
	set_children(NULL);
	std::cout << "GetAABB\n";
	std::cout << length << "\n";
	GetTrianglesAABB(primitives, length, &this->aabb);
	vPrint(this->aabb.min);
	vPrint(this->aabb.max);
	std::cout << "Divide\n";
	Divide(0, 64, primitives, length, 0, 12, &node_counter, 0, 4000, 5);
	
	std::cout << "node_counter : " << node_counter << "\n";
}

void BVH::IntersectRay(Ray * ray, scalar * min, Hit * hit, Triangle * primitives, bool shadow_ray){
	Hit buff;
	
	Hit SIMD_hit_buff[4];
	bool SIMD_bool_buff[4];
	
	bool hit_bool[2];
	scalar hit_depth[2];
	bool child_order[2];
	
#if USE_QUAD_TREE == 1
	scalar hit_depth_quad[4];
	bool hit_bool_quad[4];
	bool child_order_quad[4];
#endif	
	
	int i;
	if(get_leaf() == false){
#if USE_QUAD_TREE == 1
		if(get_children()[0].get_leaf()==false & get_children()[1].get_leaf()==false){
			AABB_IntersectRay4SIMD(
								&get_children()[0].get_children()[0].aabb,
								&get_children()[0].get_children()[1].aabb,
								&get_children()[1].get_children()[0].aabb,
								&get_children()[1].get_children()[1].aabb,
								&hit_bool_quad[0], ray, &hit_depth_quad[0]);
			
			child_order_quad[1] = hit_depth_quad[0] < hit_depth_quad[1];
			child_order_quad[0] = child_order_quad[1] ^ 0b0001;
			child_order_quad[3] = hit_depth_quad[2] < hit_depth_quad[3];
			child_order_quad[2] = child_order_quad[3] ^ 0b0001;
			
			
			if(hit_bool_quad[child_order_quad[0]]){ // & hit_depth[child_order[0]] < *min
				buff.hit = false;
				((BVH)get_children()[0].get_children()[child_order_quad[0]]).IntersectRay(ray, min, &buff, primitives, shadow_ray); //near child 0
				if(buff.hit){
					*hit = buff;
					hit->hit = true;
				}
			}
			if(hit_bool_quad[child_order_quad[1]] & hit_depth_quad[child_order_quad[1]] <= *min){
				buff.hit = false;
				((BVH)get_children()[0].get_children()[child_order_quad[1]]).IntersectRay(ray, min, &buff, primitives, shadow_ray); //far child 0
				if(buff.hit){
					*hit = buff;
					hit->hit = true;
				}
			}
			
			if(hit_bool_quad[child_order_quad[2]+2] & hit_depth_quad[child_order_quad[2]+2] < *min){
				buff.hit = false;
				((BVH)get_children()[1].get_children()[child_order_quad[2]]).IntersectRay(ray, min, &buff, primitives, shadow_ray); //near child 1
				if(buff.hit){
					*hit = buff;
					hit->hit = true;
				}
			}
			if(hit_bool_quad[child_order_quad[3]+2] & hit_depth_quad[child_order_quad[3]+2] <= *min){
				buff.hit = false;
				((BVH)get_children()[1].get_children()[child_order_quad[3]]).IntersectRay(ray, min, &buff, primitives, shadow_ray); //far child 1
				if(buff.hit){
					*hit = buff;
					hit->hit = true;
				}
			}
			
		}
		else{
#endif
			AABB_IntersectRay2SIMD(&get_children()[0].aabb, &get_children()[1].aabb, hit_bool, ray, hit_depth);
			
			//START DEPTH SORTING
			child_order[1] = hit_depth[0] < hit_depth[1];
			child_order[0] = child_order[1] ^ 0b0001;
			
			//END DEPTH SORTING
			if(hit_bool[child_order[0]]){ // & hit_depth[child_order[0]] < *min
				buff.hit = false;
				((BVH)get_children()[child_order[0]]).IntersectRay(ray, min, &buff, primitives, shadow_ray); //near child
				if(buff.hit){
					*hit = buff;
					hit->hit = true;
				}
			}
			if(hit_bool[child_order[1]] & (hit_depth[child_order[1]] <= *min)){
				buff.hit = false;
				((BVH)get_children()[child_order[1]]).IntersectRay(ray, min, &buff, primitives, shadow_ray); //far child
				if(buff.hit){
					*hit = buff;
					hit->hit = true;
				}
			}
#if USE_QUAD_TREE == 1
		}
#endif
	}
	else { //traverse mesh
		i = 0;
		while (i+4 <= info.leaf.tri_count) { //use SIMD function for 4x Intersection
				
			Triangle_IntersectRay4SIMD(&primitives[info.leaf.tri_offset + i], ray, *min, SIMD_hit_buff, SIMD_bool_buff);
			
			if(SIMD_bool_buff[0] | SIMD_bool_buff[1] | SIMD_bool_buff[2] | SIMD_bool_buff[3]){					
				if(shadow_ray){
					hit->hit = true;
					return;
				}
				
				if(SIMD_bool_buff[0]){
					*min = SIMD_hit_buff[0].t; *hit = SIMD_hit_buff[0];
				}
				if(SIMD_bool_buff[1] & (SIMD_hit_buff[1].t <= *min)){
					*min = SIMD_hit_buff[1].t; *hit = SIMD_hit_buff[1];
				}
				if(SIMD_bool_buff[2] & (SIMD_hit_buff[2].t <= *min)){
					*min = SIMD_hit_buff[2].t; *hit = SIMD_hit_buff[2];
				}
				if(SIMD_bool_buff[3] & (SIMD_hit_buff[3].t <= *min)){
					*min = SIMD_hit_buff[3].t; *hit = SIMD_hit_buff[3];
				}
				hit->hit = true;
			}
			i += 4;
		}
		while(i < info.leaf.tri_count){
			if (primitives[info.leaf.tri_offset + i].IntersectRay(ray, *min, &buff)) {
				*hit = buff;
				*min = buff.t;
				hit->prim = (void *) &primitives[info.leaf.tri_offset + i];
				//b_result = true;
				hit->hit = true;
				if (shadow_ray) {
					return;
				}
			}
			++i;
		}
	}
	return;
}


void BVH::IntersectRayLoop(Ray * ray, scalar * min, Hit * hit, Triangle * primitives, bool shadow_ray){
	int stack_pointer = 0;
	
#if USE_QUAD_TREE == 1
	BVH * node_stack[128];
	scalar depth_stack[128];
#else
	BVH * node_stack[64];
	scalar depth_stack[64];
#endif
	__builtin_prefetch((void*)node_stack, 0, 0);
	__builtin_prefetch((void*)depth_stack, 0, 0);
	BVH * active_node = this;
	int i;
	
	Hit buff;
	
	Hit SIMD_hit_buff[4];
	bool SIMD_bool_buff[4];
	
	bool hit_bool[2];
	scalar hit_depth[2];
	bool child_order[2];
	
	
	
#if USE_QUAD_TREE == 1
	scalar hit_depth_quad[4];
	bool hit_bool_quad[4];
	//bool child_order_quad[4];
	short child_order_quad[4];
	short hit_counter;
	short index;
	short first;
	bool bool_buffer;
#endif
	
	while(stack_pointer >= 0){
		
		if(active_node->get_leaf() == false){ //interior
#if USE_QUAD_TREE == 1
			if(active_node->get_children()[0].get_leaf() == false & active_node->get_children()[1].get_leaf() == false){
				AABB_IntersectRay4SIMD(
								&active_node->get_children()[0].get_children()[0].aabb,
								&active_node->get_children()[0].get_children()[1].aabb,
								&active_node->get_children()[1].get_children()[0].aabb,
								&active_node->get_children()[1].get_children()[1].aabb,
								&hit_bool_quad[0], ray, &hit_depth_quad[0]);
				//depth sorting
				
				
				
				child_order_quad[0] = hit_depth_quad[0] > hit_depth_quad[1];
				child_order_quad[1] = child_order_quad[0]^0b0001;
				
				child_order_quad[2] = hit_depth_quad[2] > hit_depth_quad[3];
				child_order_quad[3] = child_order_quad[2]^0b0001;
				
				bool_buffer = hit_depth_quad[child_order_quad[0]+0] > hit_depth_quad[child_order_quad[2]+2];
				
				bool_buffer ^= 0b001;
				child_order_quad[0] |= bool_buffer<<1;
				child_order_quad[1] |= bool_buffer<<1;
				bool_buffer ^= 0b001;
				child_order_quad[2] |= (bool_buffer)<<1;
				child_order_quad[3] |= (bool_buffer)<<1;
				
				
				if(hit_bool_quad[child_order_quad[3]]){
					//push to stack
					node_stack[stack_pointer] = &active_node->get_children()[child_order_quad[3]>>1].get_children()[child_order_quad[3]&0b01];
					depth_stack[stack_pointer] = hit_depth_quad[child_order_quad[3]];
					stack_pointer ++;
				}
				if(hit_bool_quad[child_order_quad[2]]){
					//push to stack
					node_stack[stack_pointer] = &active_node->get_children()[child_order_quad[2]>>1].get_children()[child_order_quad[2]&0b01];
					depth_stack[stack_pointer] = hit_depth_quad[child_order_quad[2]];
					stack_pointer ++;
				}
				if(hit_bool_quad[child_order_quad[1]]){
					//push to stack
					node_stack[stack_pointer] = &active_node->get_children()[child_order_quad[1]>>1].get_children()[child_order_quad[1]&0b01];
					depth_stack[stack_pointer] = hit_depth_quad[child_order_quad[1]];
					stack_pointer ++;
				}
				if(hit_bool_quad[child_order_quad[0]]){
					//push to stack
					node_stack[stack_pointer] = &active_node->get_children()[child_order_quad[0]>>1].get_children()[child_order_quad[0]&0b01];
					depth_stack[stack_pointer] = hit_depth_quad[child_order_quad[0]];
					stack_pointer ++;
				}
				if(hit_bool_quad[3] | hit_bool_quad[2] | hit_bool_quad[1] | hit_bool_quad[0]){ //get new active from stack
					stack_pointer --;
					active_node = node_stack[stack_pointer];
				}
				else{ //pop stack
					retry2:
					stack_pointer --;
					if(stack_pointer >= 0){
						if(depth_stack[stack_pointer] > *min){
							goto retry2;
						}
						active_node = node_stack[stack_pointer];
					}
					else{
						break;
					}
				}
			}
			else{
				
#endif
				//DEPTH SORTING
				AABB_IntersectRay2SIMD(&active_node->get_children()[0].aabb, &active_node->get_children()[1].aabb, hit_bool, ray, hit_depth);
				
				if(hit_bool[0] & hit_bool[1]){
					//START DEPTH SORTING
					child_order[1] = hit_depth[0] < hit_depth[1];
					child_order[0] = child_order[1] ^ 0b0001;
					//END DEPTH SORTING
					
					//push 2nd to stack :
					node_stack[stack_pointer] = &active_node->get_children()[child_order[1]];
					depth_stack[stack_pointer] = hit_depth[child_order[1]];
					stack_pointer ++;
					
					//set near node to active and continue
					active_node = &active_node->get_children()[child_order[0]];
				}
				else if(hit_bool[0]){
					active_node = &active_node->get_children()[0];
				}
				else if(hit_bool[1]){
					active_node = &active_node->get_children()[1];
				}
				else{//pop stack
					retry0:
					stack_pointer --;
					if(stack_pointer >= 0){
						if(depth_stack[stack_pointer] > *min){
							goto retry0;
						}
						active_node = node_stack[stack_pointer];
					}
					else{
						break;
					}
				}
#if USE_QUAD_TREE == 1
			}
#endif
		}
		else{ //leaf
			i = 0;
			while(i+4 <= active_node->info.leaf.tri_count){ //4x SIMD Triangle intersection
				Triangle_IntersectRay4SIMD(&primitives[active_node->info.leaf.tri_offset + i], ray, *min, SIMD_hit_buff, SIMD_bool_buff);
				
				if(SIMD_bool_buff[0] | SIMD_bool_buff[1] | SIMD_bool_buff[2] | SIMD_bool_buff[3]){					
					if(shadow_ray){
						hit->hit = true;
						return;
					}
					
					if(SIMD_bool_buff[0]){
						*min = SIMD_hit_buff[0].t; *hit = SIMD_hit_buff[0];
					}
					if(SIMD_bool_buff[1] & (SIMD_hit_buff[1].t <= *min)){
						*min = SIMD_hit_buff[1].t; *hit = SIMD_hit_buff[1];
					}
					if(SIMD_bool_buff[2] & (SIMD_hit_buff[2].t <= *min)){
						*min = SIMD_hit_buff[2].t; *hit = SIMD_hit_buff[2];
					}
					if(SIMD_bool_buff[3] & (SIMD_hit_buff[3].t <= *min)){
						*min = SIMD_hit_buff[3].t; *hit = SIMD_hit_buff[3];
					}
					hit->hit = true;
				}
				i += 4;
			}
			while(i < active_node->info.leaf.tri_count){ //single Tri intersection
				if (primitives[active_node->info.leaf.tri_offset + i].IntersectRay(ray, *min, &buff)) {
					*hit = buff;
					*min = buff.t;
					hit->prim = (void *) &primitives[active_node->info.leaf.tri_offset + i];
					//b_result = true;
					hit->hit = true;
					if (shadow_ray) {
						return;
					}
				}
				i ++;
			}
			
			//pop stack
				retry1:
				stack_pointer --;
				if(stack_pointer >= 0){
					if(depth_stack[stack_pointer] > *min){
						goto retry1;
					}
					
					active_node = node_stack[stack_pointer];
				}
				else{
					break;
				}
		}
		
		
	}
	
}
