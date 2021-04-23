#pragma once
#include "include.h"

class Texture2D{
public:
	char name[128];
	virtual void Sample(scalar x, scalar y, scalar * result) = 0;
};