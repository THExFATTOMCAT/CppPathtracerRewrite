#pragma once

#include "include.h"

class Material{
public:
	scalar emission_color[3];
	scalar emission_strength;
	
	scalar color[3];
	scalar reflectivity; //unused ?
	scalar metallic;
	scalar roughness;
	scalar transmission;
	
	Texture2D * t_color;
	Texture2D * t_metallic;
	Texture2D * t_roughness;
	Texture2D * t_transmission;
	
	scalar specular; //not supported at the time
	
	bool clearcoat;
	scalar ior;
	char name[128];
	
	Material(){
		t_color = NULL;
		t_metallic = NULL;
		t_roughness = NULL;
		t_transmission = NULL;
	}
	
	
	void Get(scalar * uvs, scalar * roughness, bool * clearcoat, scalar * metallic, scalar * ior, scalar * color, scalar * emission, scalar * transmission){ //this could use uvs for texture mapping or procedural shaders
		*roughness = this->roughness;
		*clearcoat = this->clearcoat;
		*metallic = this->metallic;
		*ior = this->ior;
		*transmission = this->transmission;
		
		vCpy(color, this->color);
		vCpy(emission, this->emission_color);
		vMulF(emission, this->emission_strength);
	}
	
	scalar GetTransmission(scalar * uvs){
		return this->transmission;
	}
	
	scalar GetIor(scalar * uvs){
		return this->ior;
	}
	scalar GetRoughness(scalar * uvs){
		return this->roughness;
	}
	void GetColor(scalar * uvs, scalar * result){
		vCpy(result, color);
		if(t_color != 0){
			scalar t[3];
			t_color->Sample(uvs[0], uvs[1], t);
			vMul(result, t);
		}
	}
	scalar GetMetallic(scalar * uvs){
		scalar result = metallic;
		if(t_metallic != 0){
			scalar t[3];
			t_color->Sample(uvs[0], uvs[1], t);
			result *= vLength(t);
		}
		return result;
	}
	
	
	
	void GetEmission(scalar * uvs, scalar * result){
		vCpy(result, emission_color);
		vMulF(result, emission_strength);
	}
	
	void Scatter(Ray * in, scalar * normal, scalar * tangent, scalar in_ior, scalar * uvs, scalar t, Ray * out){
		scalar r = roughness;
		scalar out_ior = ior;
		if(out_ior == in_ior){
			out_ior = 1;
		}
		
		scalar fresnel;
		scalar rand_normal[3];
		scalar refraction_direction[3];
		scalar buff[3];
		if(fastRand() < metallic){ //metallic reflection
			GXX::SampleNormal(rand_normal, in->d, normal, tangent, r, true);
			vReflect(in->d, rand_normal, out->d);
			if(vDot(out->d, normal) < 0){
				vReflect(rand_normal, normal, rand_normal);
				vReflect(out->d, rand_normal, out->d);
			}
			
			in->Trace(fmax(t-0.0002, 0), out->o);
		}
		else{
			if(fastRand() < transmission){ //transmissive shading
				vRefract(in->d, normal, in_ior, out_ior, refraction_direction);
				fresnel = 1-vFresnel(normal, in->d, refraction_direction, in_ior, out_ior);
				
				if(fastRand() < fresnel){ //refraction
					
					GXX::SampleNormal(rand_normal, in->d, normal, tangent, r);
					vRefract(in->d, rand_normal, in_ior, out_ior, out->d);
					if(vDot(out->d, normal) > 0){
						vCpy(buff, out->d);
						vReflect(buff, normal, out->d);
					}
					
					in->Trace(t+0.0002, out->o);
				}
				else{ //glossy reflection
					GXX::SampleNormal(rand_normal, in->d, normal, tangent, r);
					vReflect(in->d, rand_normal, out->d);
					if(vDot(out->d, normal) < 0){
						vCpy(buff, out->d);
						vReflect(buff, normal, out->d);
					}
					
					in->Trace(fmax(t-0.0002, 0), out->o);
				}
			}
			else{ //diffuse reflection
				vCpy(rand_normal, normal);
				vRandomize(rand_normal, 1); //this is wrong in every manner
				vNormalize(rand_normal);
				
				vReflect(in->d, rand_normal, out->d);
				
				if(vDot(out->d, normal) < 0){
					vFlip(out->d);
				}
				
				in->Trace(fmax(t-0.0002, 0), out->o);
			}
			
		}
		
		vNormalize(out->d);
		//in->Trace(t, out->o);
		//out->Trace(0.0001, out->o);
		out->UpdateInverse();
		
		return;
	}
	
	
	scalar Pdf(scalar * test_direction, scalar * direction, scalar * normal, scalar * reflection, scalar * refraction, scalar * uvs, scalar in_ior){
		scalar r = roughness;
		scalar out_ior = ior;
		
		scalar gxx_d = GXX::PdfExp(normal, direction, test_direction, r);
		scalar gxx_g = GXX::Shadowing(normal, direction, r)*GXX::Shadowing(normal, test_direction, r, 1);
		
		scalar metallic_result = gxx_d*gxx_g;
		scalar fresnel = fmax(1-vFresnel(normal, test_direction, refraction, in_ior, out_ior), 0);
		
		scalar n_buffer[3];
		vCpy(n_buffer, normal);
		vFlip(n_buffer);
		
		gxx_d = GXX::PdfExp(n_buffer, direction, test_direction, r);
		gxx_g = GXX::Shadowing(n_buffer, direction, r)*GXX::Shadowing(n_buffer, test_direction, r, 1);
		scalar transmission_result = gxx_d*gxx_g*fresnel + (1-fresnel)*metallic_result;
		
		transmission_result *= transmission;
		scalar diffuse_result = OrenNayar::Pdf(test_direction, direction, normal, r)*(1-transmission);
		
		return metallic_result*metallic + (1-metallic) * (diffuse_result + transmission_result);
	}
	
	
	bool NeedTransmission(scalar * uvs){
		if(GetTransmission(uvs) > 0 and GetMetallic(uvs) < 1){
			return true;
		}

		return false;
	}
	
};