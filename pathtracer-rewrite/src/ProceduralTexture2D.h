#pragma once
#include "include.h"

class ProceduralTexture2D : public virtual Texture2D{
public:
	char name[128];
	ProceduralTexture2D(){
		name[0] = '\0';
	}
	void Sample(scalar x, scalar y, scalar * result){
		int a = (int) (x*16); //basic gray checker pattern
		int b = (int) (y*16);
		
		if(a%2 == 0){
			if(b%2 == 0){
				vSetF(result, 0.4);
			}
			else{
				vSetF(result, 0.8);
			}
		}
		else{
			if(b%2 == 0){
				vSetF(result, 0.8);
			}
			else{
				vSetF(result, 0.4);
			}
		}
	}
};