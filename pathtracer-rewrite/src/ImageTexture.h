#pragma once
#include "include.h"


class ImageTexture : public virtual Texture2D{
private:
	int res_x;
	int res_y;
	scalar * color_buffer; //RGBA
public:	
	char name[128];
	bool interpolation;
	
	bool ImportPNG(char * path){
		FILE *fp = fopen(path, "rb");
		png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if(!png){
			return false;
		}
		
		png_infop info = png_create_info_struct(png);
		if(!info){
			return false;
		}
		
		if(setjmp(png_jmpbuf(png))){
			return false;
		}
		
		png_init_io(png, fp);
		png_read_info(png, info);
		
		res_x = png_get_image_width(png, info);
		res_y = png_get_image_height(png, info);
		png_byte color_type = png_get_color_type(png, info);
		png_byte bit_depth = png_get_bit_depth(png, info);
		
		if(bit_depth == 16){
			png_set_strip_16(png);
		}
		if(color_type == PNG_COLOR_TYPE_PALETTE){
			png_set_palette_to_rgb(png);
		}
		if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8){
			png_set_expand_gray_1_2_4_to_8(png);
		}
		if(png_get_valid(png, info, PNG_INFO_tRNS)){
			png_set_tRNS_to_alpha(png);
		}
		if( color_type == PNG_COLOR_TYPE_RGB     ||
			color_type == PNG_COLOR_TYPE_GRAY    ||
			color_type == PNG_COLOR_TYPE_PALETTE ){
			png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
		}
		if( color_type == PNG_COLOR_TYPE_GRAY ||
			color_type == PNG_COLOR_TYPE_GRAY_ALPHA){
			png_set_gray_to_rgb(png);
		}
		png_read_update_info(png, info);
		
		png_bytep * row_ptr = (png_bytep*) malloc(sizeof(png_bytep)*res_y);
		int rowbytes = (int)png_get_rowbytes(png, info);
		int i = 0;
		while(i < res_y){
			row_ptr[i] = (png_byte*) malloc(png_get_rowbytes(png, info));
			i ++;
		}
		
		png_read_image(png, row_ptr);
		fclose(fp);
		png_destroy_read_struct(&png, &info, NULL);
		
		
		color_buffer = (scalar *) std::malloc(sizeof(scalar)*4*res_x*res_y);
		std::cout << "allocated : " << 4*res_x*res_y << "\n";
		i = 0;
		int j;
		png_bytep row;
		png_bytep px;
		scalar div = (scalar) ((int) 1 << (sizeof(png_byte)*8));
		div = 1/div;
		while(i < res_y){
			j = 0;
			row = row_ptr[i];
			while(j < res_x){
				px = &(row[j*4]);
				color_buffer[(i*res_x+j)*4 + 0] = (scalar) ((int)px[0])*div;
				color_buffer[(i*res_x+j)*4 + 1] = (scalar) ((int)px[1])*div;
				color_buffer[(i*res_x+j)*4 + 2] = (scalar) ((int)px[2])*div;
				color_buffer[(i*res_x+j)*4 + 3] = (scalar) ((int)px[3])*div;
				j ++;
			}
			free(row);
			i ++;
		}
		free(row_ptr);
		
		return true;
	}
	
	ImageTexture(){
		color_buffer = NULL;
		interpolation = true;
		res_x = 0;
		res_y = 0;
	}
	
	void Sample(scalar x, scalar y, scalar * result){ //return JUST RGB 
		if(color_buffer == NULL){
			result[0] = x;
			result[1] = y;
			result[2] = 0;
		}
		else if(interpolation){
			x = (scalar) fract(x)*res_x;
			y = (scalar) fract(y)*res_y;
			
			int a = (int) fmin(x, res_x-1);
			int b = (int) fmin(y, res_y-1);
			int index0 = (b*res_x+a)*4;
			
			int c = (int) fmin(x+1, res_x-1);
			int d = (int) fmin(y+1, res_y-1);
			int index1 = (b*res_x+a)*4;
			
			x = fract(x);
			y = fract(y);
			
			scalar R0[3];
			scalar R1[3];
			R0[0] = color_buffer[index0+0];
			R0[1] = color_buffer[index0+1];
			R0[2] = color_buffer[index0+2];
			R1[0] = color_buffer[index1+0];
			R1[1] = color_buffer[index1+1];
			R1[2] = color_buffer[index1+2];
			
			result[0] = R0[0]*(1-x*y) + R1[0]*(x*y);
			result[1] = R0[1]*(1-x*y) + R1[1]*(x*y);
			result[2] = R0[2]*(1-x*y) + R1[2]*(x*y);
		}
		else{
			x = fract(x);
			y = fract(y);
			int a = (int) fmin(x*res_x, res_x-1);
			int b = (int) fmin(y*res_y, res_y-1);
			int index = (b*res_x+a)*4;
			result[0] = color_buffer[index+0];
			result[1] = color_buffer[index+1];
			result[2] = color_buffer[index+2];
		}
	}
	
};