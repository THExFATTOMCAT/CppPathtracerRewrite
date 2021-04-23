#pragma once

#include <stdlib.h>
#include <algorithm>
#include <thread>
#include <mutex>
#include <math.h>
#include <float.h>
#include <iostream>
#include <unistd.h>
#include <time.h>
#include <chrono>
#include <x86intrin.h>
#include <immintrin.h>
#include <bits/stdc++.h>
#include <typeinfo>

#include <png.h>

#include <gtk/gtk.h>

#ifdef __WIN32
/*
#include "myGL/gl.h"
#include "myGL/glext.h"
#include "myGL/wglext.h"
#include "myGL/freeglut.h"
*/

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL/freeglut.h>
#else
	
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL/freeglut.h>
#endif


#include "define.h"
#include "base_utility.h"

#include "Vec3f.h"
#include "SIMD_util.h"

#include "GXX.h"
#include "OrenNayar.h"


#include "Allocator.h"

#include "Sampler.h"
#include "LatticeSampler.h"
#include "BasicSampler.h"
#include "Ray.h"

#include "Texture.h"
#include "ProceduralTexture2D.h"
#include "ImageTexture.h"

#include "Material.h"

#include "AABB.h"
#include "Hit.h"
#include "Primitive.h"
#include "Triangle.h"

#include "utility.h"

#include "BoundingSphere.h"

#include "Accelerator.h"
#include "BVH.h"
#include "QBVH.h"



#include "Light.h"
#include "PointLight.h"
#include "DirectionalLight.h"

#include "Camera.h"


#include "OpenGL_utility.h"

#include "Scene.h"
//#include "LightProbeSample.h"
//#include "LightProbe.h"
#include "Vertex.h"
//#include "GeneralVertex.h"


#include "Integrator.h"
//#include "BasicIntegrator.h"
#include "UnidirectionalIntegrator.h"
//#include "BidirectionalIntegrator.h"
#include "BidirectionalIntegrator_exp.h"

//#include "LightIntegrator.h"
//#include "MetropolisIntegrator.h"

#include "Import.h"


#include "Render.h"

