#pragma once
#include "include.h"


class LightProbe{
	LightProbeSample * samples;
	int sample_count;
	int sample_depth;
	
	LightProbe(){
		samples = NULL;
	}
	
	
};