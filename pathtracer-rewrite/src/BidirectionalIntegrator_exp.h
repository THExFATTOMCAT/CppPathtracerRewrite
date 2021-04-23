#pragma once

#include "include.h"

class BidirectionalIntegrator_exp:public UnidirectionalIntegrator{
public:
    std::mutex * mutex;

    BidirectionalIntegrator_exp(){
        scene = NULL;
        mask = NULL;
        mutex = new std::mutex();
        mutex->unlock();
    }


    void UpdateResult();


    void Integrate(int x, int y);

    BidirectionalIntegrator_exp * clone(){
        BidirectionalIntegrator_exp * result = (BidirectionalIntegrator_exp *) std::malloc(sizeof(BidirectionalIntegrator_exp));
        memcpy(result, this, sizeof(BidirectionalIntegrator_exp));
        return result;
    }

};



void BidirectionalIntegrator_exp::Integrate(int x, int y) { //some variable seems to be incorrectly initialized, might have to do with vertex.end
    Camera * cam = &scene->cameras[scene->active_camera];
    Light * light;

    Vertex c_vertices[128];
    Vertex l_vertices[128];

    camera_depth = fmin(camera_depth, 128-1);
    light_depth = fmin(light_depth, 128-1);

    int c_length;
    int l_length;

    Ray c_ray;
    Ray l_ray;

    Ray shadow_ray;
    scalar min;
    scalar length;
    Hit hit;

    scalar c_color[3];
    scalar c_emission[3];
    scalar l_color[3];
    scalar buffer[3];
    scalar uvs[3];

    scalar result[3];

    int s;

    int c;
    int l;



    int U;
    int V;

    //start rendering loop


    s = 0;
    while(s < samples){
        /*
         * ALGORITHM :
         *
         * L(c_0, c_n) = E(0) + E(1)*C(0) + E(2)*C(0, 1) + ... + E(n)*C(0, n-1) +  K*C(0, n)
         * K          := M(c_n, l_m) * L(l_m, l_0)
         */
        //integrate a ray :

        //build camera vertices:
        cam->SampleRay(x, y, &c_ray, sampler);

        vCpy(c_vertices[0].pos, c_ray.o);
        c_vertices[0].camera = true;
        c_vertices[0].end = false;
        c_length = BuildVertices(c_vertices+1, camera_depth, &c_ray, scene, in_ior, mask) + 1;
        //build light vertices:
        light = scene->lights[rand()%scene->light_count];
        /*
        vCpy(l_ray.o, light->pos);
        vRandomize(l_ray.d, 1);*/
        scalar light_sample_prop_factor = light->SampleRay(&l_ray);

        vCpy(l_vertices[0].pos, l_ray.o);
        l_vertices[0].light = true;
        l_vertices[0].end = false;
        l_length = BuildVertices(l_vertices+1, light_depth, &l_ray, scene, in_ior, mask) + 1;

        l = 0;
        vSetF(l_color, 0);
        if(l_vertices[l_length-1].end){
            l_length --;
        }
        while(l < l_length){
            if(l_vertices[l].light){
                vCpy(buffer, light->color);
                vMulF(buffer, light->intensity);
                vAdd(l_color, buffer);
            }
            else{
                l_vertices[l].material->GetColor(l_vertices[l].tex, buffer);
                vMul(l_color, buffer);
                l_vertices[l].material->GetEmission(l_vertices[l].tex, buffer);
                vAdd(l_color, buffer);
            }
            c = 0;
            vSetF(c_emission, 0);
            vSetF(c_color, 1);
            vSetF(result, 0);
            while(c < c_length) {
                if(c_vertices[c].camera){
                    //handle direct sensor illumination
                    if(cam->ProjectOnPlane(l_vertices[l].pos, uvs, &shadow_ray, &length)){
                        min = length-0.0002;
                        hit.hit = false;
                        shadow_ray.UpdateInverse();
                        scene->accelerator->IntersectRayLoop(&shadow_ray, &min, &hit, scene->primitives, true);
                        if(hit.hit == false){
                            //record at uvs
                            U = (int)fmin(cam->res_x-1, cam->res_x*uvs[0]);
                            V = (int)fmin(cam->res_y-1, cam->res_y*uvs[1]);
                            U = (int)fmax(0, U);
                            V = (int)fmax(0, V);
                            vCpy(buffer, l_color);
                            vMulF(buffer, l_vertices[l].GetPdf(shadow_ray.d)*light_sample_prop_factor);
                            vDivF(buffer, length*length);
                            vDivF(buffer, (scalar) samples*l_length );
                            vAdd(&renderbuffer[3*(cam->res_x*V + U)], buffer);
                        }

                    }
                }
                else {
                    if (c_vertices[c].end) {
                        scene->GetBg(c_vertices[c].direction, buffer);
                        vMul(buffer, c_color);
                        vAdd(c_emission, buffer);
                        vAdd(result, c_emission);
                    } else {

                        if(l_vertices[l].light){
                            light->ShadowRayTo(c_vertices[c].pos, &shadow_ray, &length);
                        }
                        else{
                            vCpy(shadow_ray.o, l_vertices[l].pos);
                            vCpy(shadow_ray.d, c_vertices[c].pos);
                            vSub(shadow_ray.d, shadow_ray.o);

                            length = vLength(shadow_ray.d);
                            vNormalize(shadow_ray.d);
                        }


                        //length = vLength(shadow_ray.d);
                        //vDivF(shadow_ray.d, length); //normalize
                        min = length;
                        min -= 0.0002;

                        hit.hit = false;
                        shadow_ray.UpdateInverse();
                        scene->accelerator->IntersectRayLoop(&shadow_ray, &min, &hit, scene->primitives, true);

                        c_vertices[c].material->GetEmission(c_vertices[c].tex, buffer);
                        vMul(buffer, c_color);
                        vCpy(result, buffer);
                        c_vertices[c].material->GetColor(c_vertices[c].tex, buffer);
                        vMul(c_color, buffer);

                        if (hit.hit == false) {
                            vCpy(buffer, l_color); //shade light source
                            vMul(buffer, c_color);
                            vMulF(buffer, l_vertices[l].GetPdf(shadow_ray.d));
                            vFlip(shadow_ray.d);
                            vMulF(buffer, c_vertices[c].GetPdf(shadow_ray.d));
                            if(l_vertices[l].light){
                                vMulF(buffer, light->GetFalloff(length));
                            }
                            else{
                                vDivF(buffer, length*length);
                            }

                            vAdd(result, buffer);
                        }
                    }
                    //record sample the simple way:
                    vDivF(result, samples*l_length);
                    vAdd(&renderbuffer[3*(cam->res_x*y + x)], result);
                }
                if(c_vertices[c].end){
                    break;
                }
                c ++;
            }
            l ++;
        }
        s ++;
    }
    s = 0;

}


