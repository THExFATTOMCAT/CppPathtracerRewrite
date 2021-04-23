#pragma once
#include "include.h"
#include <iostream>
#include <fstream>

namespace Import{
	
	int MTL(char * mtl_path, Material * material){
		
		//std::cout << mtl_path;
		std::ifstream mtl_file;
		mtl_file.open(mtl_path);
		
		unsigned int mtl_length;
		mtl_file.seekg(0, mtl_file.end);
		mtl_length = (unsigned int) mtl_file.tellg();
		mtl_file.seekg(0, mtl_file.beg);
		
		char * content = (char*) malloc(sizeof(char)*mtl_length+1);
		mtl_file.read(content, mtl_length);
		mtl_file.close();
		
		//std::cout << content;
		int count = countString(content, (char *) "newmtl ", mtl_length, 7);
		
		//std::cout << "newmtl count : " << count << "\n";
		
		
		int i = 0;
		int p = 0;
		int j = 0;
		char * buff;
		while(p < mtl_length){
			
			if(p == 0 or content[p-1]=='\n'){
				
				if(content[p+0] == 'n' and 
				   content[p+1] == 'e' and 
				   content[p+2] == 'w' and 
				   content[p+3] == 'm' and 
				   content[p+4] == 't' and 
				   content[p+5] == 'l' and 
				   content[p+6] == ' '  ){
					p += 7;
					j = 0;
					material[i] = Material();
					while(content[p] != '\n'){
						material[i].name[j] = content[p];
						++p;
						++j;
					}
					material[i].name[j] = '\0';
					//std::cout << "mat.name = " <<  material[i].name << "\n";
					++i;
				}
				else if(content[p+0] == 'N' and 
					    content[p+1] == 's' and 
					    content[p+2] == ' '  ){
					p += 3;
					scalar Ns = strtosc(&content[p], &buff);
					p = buff-content;
					material[i-1].roughness = 1-((float)sqrt(Ns/900));
				}
				else if(content[p+0] == 'N' and 
					    content[p+1] == 'i' and 
					    content[p+2] == ' '  ){
					p += 3;
					scalar Ni = strtosc(&content[p], &buff);
					p = buff-content;
					material[i-1].ior = Ni;
				}
				else if(content[p+0] == 'K' and 
					    content[p+1] == 'd' and 
					    content[p+2] == ' '  ){
					p += 3;
					scalar Kd[3];
					Kd[0] = strtosc(&content[p], &buff);
					Kd[1] = strtosc(buff, &buff);
					Kd[2] = strtosc(buff, &buff);
					
					p = buff-content;
					vCpy(material[i-1].color, Kd);
				}
				else if(content[p+0] == 'K' and 
					    content[p+1] == 's' and 
					    content[p+2] == ' '  ){
					p += 3;
					scalar Ks[3];
					Ks[0] = strtosc(&content[p], &buff);
					Ks[1] = strtosc(buff, &buff);
					Ks[2] = strtosc(buff, &buff);
					
					p = buff-content;
					material[i-1].metallic = Ks[0];
				}
				else if(content[p+0] == 'K' and 
					    content[p+1] == 'e' and 
					    content[p+2] == ' '  ){
					p += 3;
					scalar Ke[3];
					Ke[0] = strtosc(&content[p], &buff);
					Ke[1] = strtosc(buff, &buff);
					Ke[2] = strtosc(buff, &buff);
					//std::cout << "KE : \n";
					vPrint(Ke);
					p = buff-content;
					vCpy(material[i-1].emission_color, Ke);
					material[i-1].emission_strength = 1;
				}
				else if(content[p+0] == 'i' and 
						content[p+1] == 'l' and 
						content[p+2] == 'l' and 
						content[p+3] == 'u' and 
						content[p+4] == 'm' and 
						content[p+5] == ' ' and 
						content[p+6] == '2'  ){
					p += 7;
					material[i-1].metallic = 0;
				}
				else if(content[p+0] == 'd' and
						content[p+1] == ' '  ){
					p += 2;
					material[i-1].transmission = 1-strtosc(&content[p], &buff);
					//std::cout << "transmission imported !!!  " << material[i-1].transmission << "\n";
					p = buff-content;
				}
				
				else{
					++p;
				}
				
			}
			else{
				++p;
			}
			
			
			
			
		}
		
		
		
		
		free(content);
		return count;
	}
	
	
	bool OBJ(char * obj_path, Scene * scene, scalar scale = 1){
		
		std::ifstream obj_file;
		
		obj_file.open(obj_path);
		if(not obj_file.good()){
			std::cout << "file does seem not to exist.\n";
			return false;
		}
		unsigned int obj_length;
		obj_file.seekg(0, obj_file.end);
		obj_length = (unsigned int) obj_file.tellg();
		obj_file.seekg(0, obj_file.beg);
		
		
		char * content = (char*) std::malloc(sizeof(char)*obj_length+1);
		
		//OBJ PART
	    int tex_coords;
	    int vertices;
		unsigned int faces;
		unsigned int normals;
		
		unsigned int p = 0;
		while(p < obj_length){
			std::cout << "reading : " << p << "\n";
			obj_file.read(content+p, fmin(obj_length-p, 0xFFFFFFF));
			p += 0xFFFFFFF;
		}
		//obj_file.read(content, obj_length);
		obj_file.close();
			
		//count vertices/faces/normals
		tex_coords = countString(content, (char *) "\nvt ", obj_length, 4);
		vertices = countString(content, (char *) "\nv ", obj_length, 3);
		faces = countString(content, (char *) "\nf ", obj_length, 3);
		normals = countString(content, (char *) "\nvn ", obj_length, 4);
		
		std::cout << "tex coords : " << tex_coords << "\n";
		std::cout << "vertices : " << vertices << "\n";
		std::cout << "faces : " << faces << "\n";
		std::cout << "normals : " << normals << "\n";
		
		scene->primitive_count = faces;
		
		unsigned int v_counter = 0;
		unsigned int uv_counter = 0;
		unsigned int f_counter = 0;
		unsigned int n_counter = 0;
		unsigned int m_counter = 0;
		
		std::cout << "malloc...\n";
		std::cout << "tex coord buffer\n";
		scalar * uv_buffer = (scalar *) std::malloc(sizeof(scalar)*tex_coords*2);
		
		std::cout << "vertex_buffer\n";
		scalar * vertex_buffer = (scalar*) std::malloc(sizeof(scalar)*vertices*3);
		std::cout << "normal_buffer\n";
		scalar * normal_buffer = (scalar*) std::malloc(sizeof(scalar)*normals*3);
		std::cout << "scene->primitives\n";
		scene->primitives = (Triangle*) std::malloc(sizeof(Triangle)*faces);
		std::cout << "malloc done\n";
		p = 0;
		bool smooth = false;
		Material * mat = (Material *) 0x1234;
		mat = &scene->std_material;
		
		int old_m = 0;
		std::cout << "PPP : " << p << "\n";
		while(p < obj_length){
			if(p - old_m >= obj_length * 0.01){
				old_m = p;
				std::cout << "import pos : " << p << " : " << obj_length << "\n";
			}
			
			
			if(p == 0 or content[p-1]=='\n' ){	//newline		
				if(content[p+0] == 'v' and //vertex found
				   content[p+1] == ' '  ){
					p += 2;
					p += extract3sc(&vertex_buffer[v_counter], &content[p]);
					v_counter += 3;
				}
				else if(content[p+0] == 'v' and //vertex normal found
						content[p+1] == 'n' and
						content[p+2] == ' '  ){
					p += 3;
					p += extract3sc(&normal_buffer[n_counter], &content[p]);
					n_counter += 3;
				}
				else if(content[p+0] == 'v' and
						content[p+1] == 't' and
						content[p+2] == ' '  ){
					p += 3;
					p += extract2sc(&uv_buffer[uv_counter], &content[p]);
					uv_counter += 2;
				}
				else if(content[p+0] == 'f' and //face found
						content[p+1] == ' '  ){
					p += 2;
					int v[3];
					int t[3];
					int n[3];
					
					//p += extractFaceBlock(&v[0], &t[0], &n[0], &content[p]);
					//p += extractFaceBlock(&v[1], &t[1], &n[1], &content[p]);
					//p += extractFaceBlock(&v[2], &t[2], &n[2], &content[p]);
					//p -= 1;
					
					char * buff = &content[p];
					v[0] = strtol(buff, &buff, 10)-1;
					buff += 1;
					if(*buff != '/'){t[0] = strtol(buff, &buff, 10)-1;}else{t[0]=-1;};
					buff += 1;
					n[0] = strtol(buff, &buff, 10)-1;
					buff += 1;
					
					v[1] = strtol(buff, &buff, 10)-1;
					buff += 1;
					if(*buff != '/'){t[1] = strtol(buff, &buff, 10)-1;}else{t[1]=-1;};
					buff += 1;
					n[1] = strtol(buff, &buff, 10)-1;
					buff += 1;
					
					v[2] = strtol(buff, &buff, 10)-1;
					buff += 1;
					if(*buff != '/'){t[2] = strtol(buff, &buff, 10)-1;}else{t[2]=-1;};
					buff += 1;
					n[2] = strtol(buff, &buff, 10)-1;
					buff += 1;
					
					
					p = buff-content;
					
					
					Triangle * tri = scene->primitives+f_counter;
					
					tri->smooth_shading = smooth;
					if(n[0] >= 0 and n[1] >= 0 and n[2] >= 0){
						vCpy(tri->normals[0], &normal_buffer[n[0]*3]);
						vCpy(tri->normals[1], &normal_buffer[n[1]*3]);
						vCpy(tri->normals[2], &normal_buffer[n[2]*3]);
					}
					
					
					if(t[0] >= 0 and t[1] >= 0 and t[2] >= 0){
						tri->uv[0][0] = (uv_buffer[t[0]*2+0]);
						tri->uv[1][0] = (uv_buffer[t[1]*2+0]);
						tri->uv[2][0] = (uv_buffer[t[2]*2+0]);
						
						tri->uv[0][1] = (uv_buffer[t[0]*2+1]);
						tri->uv[1][1] = (uv_buffer[t[1]*2+1]);
						tri->uv[2][1] = (uv_buffer[t[2]*2+1]);
					}
					else{
						tri->uv[0][0] = 0;
						tri->uv[1][0] = 0;
						tri->uv[2][0] = 1;
						
						tri->uv[0][1] = 0;
						tri->uv[1][1] = 1;
						tri->uv[2][1] = 0;
					}
					
					vCpy(tri->o, &vertex_buffer[v[0]*3]);
					vCpy(tri->u, &vertex_buffer[v[1]*3]);
					vCpy(tri->v, &vertex_buffer[v[2]*3]);
					
					vMulF(tri->o, scale);
					vMulF(tri->u, scale);
					vMulF(tri->v, scale);
					
					vSub(tri->u, tri->o);
					vSub(tri->v, tri->o);	
					
					tri->smooth_shading = smooth;
					
					tri->material = mat;
					vCpy(tri->mid, &vertex_buffer[v[0]*3]);
					vAdd(tri->mid, &vertex_buffer[v[1]*3]);
					vAdd(tri->mid, &vertex_buffer[v[2]*3]);
					vDivF(tri->mid, 3);
					vMulF(tri->mid, scale);
					
					++f_counter;
				}
				else if(content[p+0] == 's' and
						content[p+1] == ' ' and
						content[p+2] == 'o' and
						content[p+3] == 'n'  ){ //smooth
					smooth = true;
					p += 4;
				}
				else if(content[p+0] == 's' and
						content[p+1] == ' ' and
						content[p+2] == '1'  ){ //smooth
					smooth = true;
					p += 3;
				}
				else if(content[p+0] == 's' and
						content[p+1] == ' ' and
						content[p+2] == 'o' and
						content[p+3] == 'f' and
						content[p+4] == 'f'  ){ //smooth
					smooth = false;
					p += 5;
				}
				else if(content[p+0] == 's' and
						content[p+1] == ' ' and
						content[p+2] == '0'  ){ //smooth
					smooth = false;
					p += 3;
				}
				else if(content[p+0] == 'm' and
						content[p+1] == 't' and
						content[p+2] == 'l' and
						content[p+3] == 'l' and
						content[p+4] == 'i' and
						content[p+5] == 'b' and
						content[p+6] == ' '  ){
					//std::cout << "mtllib\n";
					p += 7;
					char mtl_path[128];
					int i = 0;
					int j = -1;
					while(obj_path[i] != '\0'){
						if(obj_path[i] == '/' or obj_path[i] == '\\'){
							j = i;
						}
						mtl_path[i] = obj_path[i];
						++i;
					}
					i = 0;
					while(content[p] != '\n'){
						mtl_path[j+1+i] = content[p];
						++i;
						++p;
					}
					mtl_path[j+i+1] = '\0';
					std::cout << mtl_path << "\n";
					m_counter += MTL(mtl_path, &scene->materials[m_counter]);
					scene->material_count = m_counter;
				}
				else if(content[p+0] == 'u' and
						content[p+1] == 's' and
						content[p+2] == 'e' and
						content[p+3] == 'm' and
						content[p+4] == 't' and
						content[p+5] == 'l' and
						content[p+6] == ' '  ){
					//std::cout << "usemtl\n";
					p += 7;
					char name[128];
					int i = 0;
					while(content[p] != '\n'){
						name[i] = content[p];
						++p;
						++i;
					}
					name[i] = '\0';
					i = 0;
					mat = &scene->std_material;
					std::cout << name << "\n";
					while(i < scene->material_count){
						//std::cout << "material["<<i<<"] = " << scene->materials[i].name << "\n";
						if(compareStr(scene->materials[i].name, name)){
							mat = &scene->materials[i];
							std::cout << scene->materials[i].name << "\n";
							break;
						}
						++i;
					}
				}
				else{
					++p;
				}
			}
			else{
				++ p;
			}
		}
		
		std::cout << "free\n";
		std::cout << "content\n";
		std::free(content);
		std::cout << "vertex_buffer\n";
		std::free(vertex_buffer);
		std::cout << "normal_buffer\n";
		std::free(normal_buffer);
		std::cout << "uv buffer\n";
		std::free(uv_buffer);
		std::cout << "finished\n";
		
		return true;
	}
}