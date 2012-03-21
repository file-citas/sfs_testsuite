#include <stdio.h>
#include <map>
#include <math.h>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glx.h> 
#include <GL/gl.h>
#include <GL/glu.h>

#include "shader.h"
#include "FrameBuffer.h"

#define W 1440
#define H 900
#define EPS 1.e-8

using namespace std;			

double rotAngle = 10.;
shader* sl;
shader* sn;
map<char*,shader*> sf;
map<char*,shader*>::iterator current_filter;
FrameBuffer* fb;
GLuint texId[3];
GLuint normalmap;
float lpos[] = {0., -5., -19.};
int fw = 8;
float tsx = 1.f/float(W);
float tsy = 1.f/float(W);
float Imin = 0.f;
float Imax = 1.f;

void freeall()
{
	free(sn);
	free(sl);
	map<char*,shader*>::iterator i;
	for(i=sf.begin(); i!=sf.end(); i++)
		free(i->second);
}

GLenum init()
{
	GLenum err;

	glClearColor(0, 0, 0, 0);		
	glClearDepth(1.0);			
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
	if((err=glGetError())!=0) return err;

	glEnable(GL_DEPTH_TEST);		
	glEnable(GL_TEXTURE_2D);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	if((err=glGetError())!=0) {
		printf("\n%s: %d\n", __func__, __LINE__);
		return err;
	}

	sl = new shader((char*)"lambert.vert",(char*)"lambert.frag");
	sn = new shader((char*)"normal.vert",(char*)"normal.frag");
	sf.insert(
		make_pair<char*,shader*>(
			(char*)"intensity",
			new shader((char*)"normal_intensity.vert",(char*)"normal_intensity.frag")
		)
	);
	sf.insert(
		make_pair<char*,shader*>(
			(char*)"gradient",
			new shader((char*)"gradient.vert",(char*)"gradient.frag")
		)
	);
	sf.insert(
		make_pair<char*,shader*>(
			(char*)"edges",
			new shader((char*)"edges.vert",(char*)"edges.frag")
		)
	);
	fb = new FrameBuffer(2);
	if((err=glGetError())!=0) {
		printf("\n%s: %d\n", __func__, __LINE__);
		return err;
	}

	current_filter = sf.begin();

	// normal map
	glGenTextures(1, &normalmap);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D, normalmap);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGBA8,
			W,
			H,
			0,
			GL_RGBA,
			GL_FLOAT,
			NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(3, texId);
	// depth buffer
	glBindTexture(GL_TEXTURE_2D, texId[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_DEPTH_COMPONENT,
			W,
			H,
			0,
			GL_DEPTH_COMPONENT,
			GL_FLOAT,
			NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	// Intensity right
	glBindTexture(GL_TEXTURE_2D, texId[1]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
	glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGBA,
			W,
			H,
			0,
			GL_RGBA,
			//GL_FLOAT,
			GL_UNSIGNED_BYTE,
			NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	// Intensity left
	glBindTexture(GL_TEXTURE_2D, texId[2]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
	glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGBA,
			W,
			H,
			0,
			GL_RGBA,
			//GL_FLOAT,
			GL_UNSIGNED_BYTE,
			NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	fb->addColorTex(texId[1]);
	fb->addColorTex(texId[2]);
	fb->addDepthTex(texId[0]);
	fb->unbind();
	if((err=glGetError())!=0) {
		printf("\n%s: %d\n", __func__, __LINE__);
		return err;
	}

	map<char*,shader*>::iterator i;
	for(i=sf.begin(); i!=sf.end(); i++) {
		i->second->addUniform('t',"tex", (void*)&texId[1]);
		i->second->addUniform('v',"LightPos", (void*)lpos);
		i->second->addUniform('i',"fw",(void*)&fw);
		i->second->addUniform('f',"tsx",(void*)&tsx);
		i->second->addUniform('f',"tsy",(void*)&tsy);
		i->second->addUniform('f',"Imin",(void*)&Imin);
		i->second->addUniform('f',"Imax",(void*)&Imax);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	sl->addUniform('v',"LightPos", (void*)lpos);

	if((err=glGetError())!=0) {
		printf("\n%s: %d\n", __func__, __LINE__);
		return err;
	}
}

void setup_cam()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1, 1, 1000);	
	gluLookAt(
			0., 0., 5., //eye
			0., 0., 0., // center
			0., 1., 0. // up
			);
	glMatrixMode(GL_MODELVIEW);
}

void reset_cam() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void display()
{
	GLenum err;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if((err=glGetError())!=0) {
		printf("\n%s: %d\n", __func__, __LINE__);
		exit(1);
	}

	// render to texture
	//float* pixel = new float[W*H*4];
	GLvoid *vpixel = malloc(W*H*4*sizeof(float));
	glPushMatrix();			
		glPushAttrib(GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT);
		glRotated(rotAngle, 0, 1, 0);	
		setup_cam();
		fb->bind();

		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		lpos[0] *= -1.;
		sl->reload();
		sl->set();
		glutSolidTeapot(1.5);			
		sl->unset();

		glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		lpos[0] *= -1.;
		sl->reload();
		sl->set();
		glutSolidTeapot(1.5);			
		sl->unset();

		fb->unbind();
		glPopAttrib();
	glPopMatrix();			
	if((err=glGetError())!=0) {
		printf("\n%s:%d --  %s\n", __func__, __LINE__, gluErrorString(err));
		exit(1);
	}

	char* pixel = (char*)vpixel;
	vector<int> maxidx;
	vector<int> minidx;
	glBindTexture(GL_TEXTURE_2D, texId[1]);
	glGetTexImage(GL_TEXTURE_2D, 0,	GL_RGBA, GL_UNSIGNED_BYTE, vpixel);
	glBindTexture(GL_TEXTURE_2D, 0);
	char max = 0;
	char min = 255;
	for(int i=0; i<W*H*4; i+=4) {
		if(pixel[i] < EPS) continue;
		if(pixel[i] < min) {
			min = pixel[i];
			minidx.push_back(i);
		}
		if(pixel[i] > max) {
			max = pixel[i];
			maxidx.push_back(i);
		}
	}
	free(pixel);
	Imin = float(min)/255;
	//Imax = float(max)/255;
	//Imax = fmax(float(max)/255, 0.99);
	Imax = 1.0;
	//printf("%f, %f\n", Imin, Imax);
	sf[(char*)"intensity"]->reload();


	// orig img
	glPushMatrix();			
		reset_cam();
		glPushAttrib(GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT);
		glViewport (0, 0, W*0.25, H*0.5);
		glOrtho(0,1,0,1,0,1);

		glBindTexture(GL_TEXTURE_2D, texId[1]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);

		reset_cam();
		glViewport (W*0.25, 0, W*0.25, H*0.5);
		glOrtho(0,1,0,1,0,1);

		glBindTexture(GL_TEXTURE_2D, texId[2]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);

		glPopAttrib();
	glPopMatrix();			
	if((err=glGetError())!=0) {
		printf("\n%s: %d\n", __func__, __LINE__);
		exit(1);
	}

	// orig normals
	glPushMatrix();			
		setup_cam();
		glPushAttrib(GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT);
		glViewport (W*0.25, H*0.5, W*0.25, H*0.5);
		glRotated(rotAngle, 0, 1, 0);	
		sn->set();
		glutSolidTeapot(1.5);			
		//glutSolidSphere(1.5, 20, 20);
		sn->unset();
		glPopAttrib();
	glPopMatrix();			
	if((err=glGetError())!=0) {
		printf("\n%s: %d\n", __func__, __LINE__);
		exit(1);
	}

	// filtered img
	glPushMatrix();			
		reset_cam();
		glPushAttrib(GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT);
		glViewport (W*0.5, H*0.5, W*0.25, H*0.5);
		glOrtho(0,1,0,1,0,1);

		lpos[0] *= -1.;
		current_filter->second->reload();
		current_filter->second->set();
		glBindTexture(GL_TEXTURE_2D, texId[1]);
		glBegin(GL_QUADS);
		glVertex2f(0.0f, 0.0f);
		glVertex2f(1.0f, 0.0f);
		glVertex2f(1.0f, 1.0f);
		glVertex2f(0.0f, 1.0f);
		glEnd();
		//sf[(char*)"gradient"]->unset();
		current_filter->second->unset();
		glBindTexture(GL_TEXTURE_2D, 0);


		lpos[0] *= -1.;
		current_filter->second->reload();
		reset_cam();
		glViewport (W*0.75, H*0.5, W*0.25, H*0.5);
		glOrtho(0,1,0,1,0,1);
		glBindTexture(GL_TEXTURE_2D, texId[2]);

		current_filter->second->set();
		glBegin(GL_QUADS);
		glVertex2f(0.0f, 0.0f);
		glVertex2f(1.0f, 0.0f);
		glVertex2f(1.0f, 1.0f);
		glVertex2f(0.0f, 1.0f);
		glEnd();
		//sf[(char*)"gradient"]->unset();
		current_filter->second->unset();
		glBindTexture(GL_TEXTURE_2D, 0);

		glPopAttrib();
	glPopMatrix();			
	if((err=glGetError())!=0) {
		printf("\n%s: %d\n", __func__, __LINE__);
		exit(1);
	}

	glFlush();				
	glutSwapBuffers();			
	if((err=glGetError())!=0) {
		printf("\n%s: %d\n", __func__, __LINE__);
		exit(1);
	}
}

void mouseMove(int click, int state, int x, int y)
{
	if(x>W*0.5 || y<H*0.5) return;
	
}

void keyboard(unsigned char k, int x, int y)
{
	switch (k)
	{
		case '+':
			//fw++;			
			tsx += 1.e-4;
			tsy += 1.e-4;
			current_filter->second->reload();
			break;
		case '-':
			//fw--;			
			tsx -= 1.e-4;
			tsy -= 1.e-4;
			current_filter->second->reload();
			break;
		case 'r':
			current_filter->second->reload();
			sn->reload();
			sl->reload();
			break;
		case 'x':
			current_filter++;
			if(current_filter==sf.end())
				current_filter=sf.begin();
			break;
		// arrows
		case 'w':
			lpos[2]+=0.5;
			lpos[2]+=0.5;
			current_filter->second->reload();
			printf("%f %f %f\n", lpos[0], lpos[1], lpos[2]);
			sl->reload();
			break;
		case 's':
			lpos[2]-=0.5;
			lpos[2]-=0.5;
			current_filter->second->reload();
			printf("%f %f %f\n", lpos[0], lpos[1], lpos[2]);
			sl->reload();
			break;
		case 'd':
			lpos[0]+=0.5;
			lpos[0]+=0.5;
			current_filter->second->reload();
			printf("%f %f %f\n", lpos[0], lpos[1], lpos[2]);
			sl->reload();
			break;
		case 'a':
			lpos[0]-=0.5;
			lpos[0]-=0.5;
			current_filter->second->reload();
			sl->reload();
			printf("%f %f %f\n", lpos[0], lpos[1], lpos[2]);
			break;

		case 'q':
			freeall();
			exit(0);			
	}

	glutPostRedisplay();		
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_STENCIL);

	glutInitWindowSize(W,H);
	glutCreateWindow("GLUT Example");	

	glutDisplayFunc(display);		
	glutKeyboardFunc(keyboard);		

	glewInit();
	printf("Initializing Framework ... \n");
	GLenum err = init();
	if(err!=0) {
		printf("%s\n", gluErrorString(err));
		freeall();
		exit(1);
	}

	glutMainLoop();			
	return 0; 
}
