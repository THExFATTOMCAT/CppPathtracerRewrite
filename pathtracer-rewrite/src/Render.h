#pragma once
#include "include.h"

namespace Render{
	Material * mask = NULL;
	Sampler * sampler = new Sampler();
	
	bool profile = false;
	
	int tile_x = 64;
	int tile_y = 64;

	int threads = 8;

	std::mutex next_thread_lock;
	std::mutex worker_lock;
	int next_thread_index;

	struct RenderTile{
		int x;
		int y;
		int width;
		int height;

		int index;

		Integrator * integrator;
	};

	Integrator * integrator;

	void * Worker(struct RenderTile * tile){
		srand(tile->integrator->sampler->seed);
		int i;
		int j;
		i = 0;
		while(i < tile->width){
			j = 0;
			while(j < tile->height){
				//render each pixel of the tile to the integrators renderbuffer:
				tile->integrator->Integrate(tile->x+i, tile->y+j);
				j ++;
			}
			i ++;
		}

		worker_lock.lock();
		next_thread_index = tile->index;
		next_thread_lock.unlock();
		return NULL;
	}

	struct WorkerArgs{
		int * coords;
		int coord_count;
		bool * status;
		Integrator * integrator;
		Scene * scene;
	};
	/*
	void * Worker(void * args){
		int * coords = ((struct WorkerArgs*)args)->coords;
		int coord_count = ((struct WorkerArgs*)args)->coord_count;
		
		WorkerArgs arg = *((struct WorkerArgs*)args);
		Integrator * integrator = arg.integrator;
		Scene * scene = arg.scene;
		Camera * active_cam = scene->cameras + scene->active_camera;
		
		std::srand(coords[0]*active_cam->res_y + coords[1]); //hide random pattern
		
		
		Ray ray;
		int i = 0;
		int j;
		scalar result[3];
		int index;
		//clear buffer;
		while(i < coord_count){
			index = coords[i*2+0]    +     coords[i*2+1]*active_cam->res_x;
			active_cam->renderbuffer[(3*index)+0] = 0;
			active_cam->renderbuffer[(3*index)+1] = 0;
			active_cam->renderbuffer[(3*index)+2] = 0;
			++ i;
		}
		
		i = 0;
		Sampler sampler = Sampler();
		while(i < coord_count){
			unsigned long start_time = __rdtsc();
			
			j = 0;
			index = coords[i*2+0]    +     coords[i*2+1]*active_cam->res_x;
			while(j < integrator->samples){

				if(typeid(integrator) == typeid(BidirectionalIntegrator_exp)){
                    integrator->Integrate(coords[i*2+0], coords[i*2+1]);
				}
				else{
                    if(lattice_sampling){
                        active_cam->SampleRay(coords[i*2+0], coords[i*2+1], &ray, &sampler);
                    }
                    else{
                        active_cam->SampleRay(coords[i*2+0], coords[i*2+1], &ray);
                    }
                    integrator->Ready();
                    ray.UpdateInverse();
                    integrator->Integrate(&ray, scene, result, 0, 1, mask);
				}
				active_cam->renderbuffer[3*index+0] += result[0];
				active_cam->renderbuffer[3*index+1] += result[1];
				active_cam->renderbuffer[3*index+2] += result[2];
				++ j;
			}
			if(typeid(integrator) == typeid(BidirectionalIntegrator_exp)){

			}
			else{
				active_cam->renderbuffer[3*index+0] /= integrator->samples;
				active_cam->renderbuffer[3*index+1] /= integrator->samples;
				active_cam->renderbuffer[3*index+2] /= integrator->samples;
			}

			++i;
			
			unsigned long end_time = __rdtsc();
			unsigned long time_diff = end_time - start_time;
			if(profile){
				active_cam->renderbuffer[3*index+0] = fmin(1, (scalar)time_diff*0.000001);
				active_cam->renderbuffer[3*index+1] = 1-fmin(1, (scalar)time_diff*0.000001);
				active_cam->renderbuffer[3*index+2] = 0;
			}
			
		}
		
		sampler.Free();
		
		*((struct WorkerArgs*)args)->status = true;
		return NULL;
	}
	*/

	/*
	void Render(int threads, int block_size, Integrator * integrator, Scene * scene){
		
		Camera * active_cam = &scene->cameras[scene->active_camera];
		int * coords = (int *) malloc(sizeof(int)*active_cam->res_x*active_cam->res_y*2);
		int pixels = (active_cam->res_x)*(active_cam->res_y);
		
		int i = 0;
		int j;
		int k = 0;
		while(i < active_cam->res_y){ //fill coords buffer
			j = 0;
			while(j < active_cam->res_x){
				coords[k+0] = j;
				coords[k+1] = i;
				k += 2;
				++j;
			}
			++i;
		}
		if(tiled){
			int x, y, X, Y, vX, vY;
			i = 0;
			x = 0;
			while(x < active_cam->res_x){
				y = 0;
				while(y < active_cam->res_y){
					X = 0;
					while(X < tile_x and x+X < active_cam->res_x){
						Y = 0;
						while(Y < tile_y and y+Y < active_cam->res_y){
							coords[i*2+0] = x+X;
							coords[i*2+1] = y+Y;
							i ++;
							Y++;
						}
						X ++;
					}
					y += tile_y;
				}
				x += tile_x;
			}
		}
		else{
			//shuffle
			int buff[2];
			i = 0;
			while(i < pixels){
				j = (rand() % active_cam->res_y) * active_cam->res_x + (rand() % active_cam->res_x);
				
				buff[0] = coords[i * 2 + 0];
				buff[1] = coords[i * 2 + 1];
				coords[i * 2 + 0] = coords[j * 2 + 0];
				coords[i * 2 + 1] = coords[j * 2 + 1];
				coords[j * 2 + 0] = buff[0];
				coords[j * 2 + 1] = buff[1];
				++i;
			}
		}
		
		
		//prepare threads
		
		std::thread worker[threads];
		BidirectionalIntegrator_exp * bidir_buff;
		bool status[threads];
		WorkerArgs args[threads];
		i = 0;
		std::cout << "integrator_ptr: " << integrator << "\n";
		while(i < threads){
			status[i] = true;
			args[i].status = status+i;
			if(typeid(*integrator) == typeid(BidirectionalIntegrator_exp)){
				bidir_buff = new BidirectionalIntegrator_exp();
				bidir_buff->scene = integrator->scene;
				bidir_buff->Ready();
				args[i].integrator = bidir_buff;

			} else{
				args[i].integrator = new UnidirectionalIntegrator();
				*args[i].integrator = *integrator;
			}
			++i;
		}
		
		block_size = tile_x*tile_y;
		
		//render
		std::cout << "Start actual rendering-process\n";
		i = 0;
		int wait = 0;
		while(i < pixels){
			j = 0;
			while(j < threads and i < pixels){
				if(status[j]){
					status[j] = false;
					args[j].coords = &coords[i*2];
					args[j].coord_count = std::min({pixels-i, block_size});
					args[j].scene = scene;

					i += block_size;
					worker[j] = std::thread(Worker, (void*) &args[j]);
					worker[j].detach();
					//pthread_create(&worker[j], NULL, Worker, &args[j]);
				}
				++j;
			}
			
			//do other stuff (Draw the image)
			if(wait%100){
				if(typeid(*integrator) == typeid(BidirectionalIntegrator_exp)){
					args[j].integrator->UpdateResult();
				}
				OpenGL::DrawImage();
				glutMainLoopEvent();
				wait = 0;
			}
			++ wait;
			
			usleep((int)10000);
		}
		//wait for all threads to finish
		i = 0;
		while(i < threads){
			if(status[i]){
				++ i;
			}
			//(Draw the Image)
			OpenGL::DrawImage();
			glutMainLoopEvent();
			
			usleep((int)10000);
		}

		while(i < threads){
			if(typeid(*integrator) == typeid(BidirectionalIntegrator_exp)){
				free(args[j].integrator);
			}
			++i;
		}

		return;
	}

	*/


	void RenderImage(){

		Camera * cam = &integrator->scene->cameras[integrator->scene->active_camera];

		if(cam->renderbuffer == NULL){
			cam->renderbuffer = (scalar*) std::malloc(sizeof(scalar)*3*cam->res_x*cam->res_y);
		}

		int tiles_x = (int)(cam->res_x / tile_x) + (cam->res_x%tile_x != 0);
		int tiles_y = (int)(cam->res_y / tile_y) + (cam->res_y%tile_y != 0);

		std::cout << "res_x/tile_x : " << (cam->res_x/tile_x) << "\n";
		std::cout << "res_y/tile_y : " << (cam->res_y/tile_y) << "\n";
		std::cout << "tiles_x: " << tiles_x << "\n";
		std::cout << "tiles_y: " << tiles_y << "\n";

		RenderTile tiles[tiles_x][tiles_y];
		//generate tiles:
		int i, j, k;
		i = 0;
		while(i < tiles_x){
			j = 0;
			while(j < tiles_y){

				tiles[i][j].x = i * tile_x;
				tiles[i][j].y = j * tile_y;
				tiles[i][j].width  = (int) fmin(tile_x, cam->res_x - tiles[i][j].x);
				tiles[i][j].height = (int) fmin(tile_y, cam->res_y - tiles[i][j].y);

				j += 1;
			}
			i += 1;
		}
		std::thread thread_list[threads];
		Integrator * thread_integrators[threads];

		i = 0;
		k = 0;
		next_thread_lock.unlock();
		worker_lock.unlock();
		int l;
		while(i < tiles_x){
			j = 0;
			while(j < tiles_y){
				if(k < threads){
					std::cout << "clone integrator " << k << "\n";
					thread_integrators[k] = integrator->clone();
					thread_integrators[k]->Ready();
					thread_integrators[k]->sampler = sampler->clone();
					tiles[i][j].index = k;
					tiles[i][j].integrator = thread_integrators[k];
					thread_list[k] = std::thread(Worker, &tiles[i][j]);
					//thread_list[k].detach();
				}
				else{
					next_thread_lock.lock();

					thread_list[next_thread_index].join();

					tiles[i][j].index = next_thread_index;
					tiles[i][j].integrator = thread_integrators[next_thread_index];
					thread_list[next_thread_index] = std::thread(Worker, &tiles[i][j]);
					//thread_list[next_thread_index].detach();

					worker_lock.unlock();
				}
				l = 0;
				while(l < cam->res_x*cam->res_y*3){
					cam->renderbuffer[l] = 0;
					l ++;
				}
				l = 0;
				while(l < threads and l < k){
					thread_integrators[l]->UpdateResult();
					l ++;
				}
				OpenGL::DrawImage();
				k ++;
				j ++;
			}
			i ++;
		}


		i = 0;
		while(i < threads){
			next_thread_lock.lock();
			thread_list[next_thread_index].join();
			worker_lock.unlock();
			i ++;
		}

		l = 0;
		while(l < cam->res_x*cam->res_y*3){
			cam->renderbuffer[l] = 0;
			l ++;
		}
		l = 0;
		while(l < threads and l < k){
			thread_integrators[l]->UpdateResult();
			l ++;
		}

		i = 0;
		while(i < threads){
			thread_integrators[i]->End();
			//std::free(thread_integrators[i]);
			thread_integrators[i] = NULL;
			i ++;
		}


		OpenGL::DrawImage();

	}


}
