#pragma once
#include "include.h"


namespace OpenGL{
	bool render_mesh = false;
	int textures;
	
	float * buffer;
	int width;
	int height;
	int id;
	
	Triangle * triangles_to_draw;
	int triangles_to_draw_count;
	Camera * preview_camera;
	
	
	
	void InitRenderTexture(){
		glGenTextures(1, (GLuint*) &textures);
		glBindTexture(GL_TEXTURE_2D, textures);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, buffer);
		
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
	
	void DrawTriangleBuffer(Triangle * tri, int count, Camera * cam){
		glDisable(GL_TEXTURE_2D);
		glLoadIdentity();
		scalar angle = ((scalar) (atan( (scalar) 1/(2*cam->focal_length) )*360)/M_PI);
		//std::cout << "fov : " << angle << "\n";
		gluPerspective(angle,
					   (scalar) cam->res_x/cam->res_y,
					   0.000001,
					   10000000);
		gluLookAt(cam->o[0]+1*cam->d[0], cam->o[1]+1*cam->d[1], cam->o[2]+1*cam->d[2],
				  cam->o[0]+2*cam->d[0], cam->o[1]+2*cam->d[1], cam->o[2]+2*cam->d[2],
				  0, 0, 1);
		
		int i = 0;
		glBegin(GL_POINTS);
		glColor4f(0.5, 1, 0.5, 1);
		
		while(i < count){
			glVertex3f(tri[i].o[0], tri[i].o[1], tri[i].o[2]);
			glVertex3f(tri[i].o[0] + tri[i].u[0], tri[i].o[1] + tri[i].u[1], tri[i].o[2] + tri[i].u[2]);
			glVertex3f(tri[i].o[0] + tri[i].v[0], tri[i].o[1] + tri[i].v[1], tri[i].o[2] + tri[i].v[2]);
			i ++;
		}
		glEnd();
		glEnable(GL_TEXTURE_2D);
	}
	
	void DrawImage(){
		glLoadIdentity();
		gluOrtho2D(0, 1, 0, 1);
		
		glClearColor(0, 0, 0, 1); // black
		glClear(GL_COLOR_BUFFER_BIT);
		
		int i = 0;
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, buffer);
		glBindTexture(GL_TEXTURE_2D, textures);
		glEnable(GL_TEXTURE_2D);
		
		glBegin(GL_QUADS);
		glColor3f(1, 1, 1);
		
		glTexCoord2f(0, 0);
		glVertex2f(0, 0);
		glTexCoord2f(1, 0);
		glVertex2f(1, 0);
		glTexCoord2f(1, 1);
		glVertex2f(1, 1);
		glTexCoord2f(0, 1);
		glVertex2f(0, 1);
		
		glEnd();
		if(triangles_to_draw_count > 0 and render_mesh){
			DrawTriangleBuffer(triangles_to_draw, triangles_to_draw_count, preview_camera);
		}
		
		glFinish();
		glutMainLoopEvent();
		//glDisable(GL_TEXTURE_2D);
		
	}
	
	void InitOpenGL(int argc, char ** argv, int res_x, int res_y, char * title){
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_RGBA | GLUT_MULTISAMPLE);
		glutInitWindowSize(res_x, res_y);
		glEnable(GL_TEXTURE_2D);
		
		
	}
	
	
	int NewWindow(int res_x, int res_y, char * title){
		glutInitDisplayMode(GLUT_RGBA | GLUT_MULTISAMPLE);
		glutInitWindowSize(res_x, res_y);
		glEnable(GL_TEXTURE_2D);
		
		id = glutCreateWindow(title);
		glClearColor(0, 0.3, 0.3, 1);
		gluOrtho2D(0, 1, 0, 1);		
		
		
		InitRenderTexture();
		glutDisplayFunc(DrawImage);
		
		return id;
	}

	

}