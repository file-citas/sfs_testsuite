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

#define W 1024
#define H 900
#define EPS 1.e-8

using namespace std;			

shader* sl;
shader* sl_normmap;
shader* sn;
map<char*,shader*> sf;
map<char*,shader*>::iterator current_filter;
FrameBuffer* fb;
GLuint texId[4];
GLuint normalmap[2];
GLuint normalmap1;
GLuint normalmap2;
float lpos[] = {15., -5., -19.};
float lpos_int[] = {0., -5., -19.};
int fw = 8;
float tsx = 1.f/float(W);
float tsy = 1.f/float(W);
float Imin = 0.f;
float Imax = 1.f;
float alpha = 0.5f;
float alpha_step = 1.e-2;
unsigned int timer = 0;

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
	sl_normmap = new shader((char*)"lambert_normalmap.vert",(char*)"lambert_normalmap.frag");
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
	fb = new FrameBuffer(4);
	if((err=glGetError())!=0) {
		printf("\n%s: %d\n", __func__, __LINE__);
		return err;
	}

	current_filter = sf.begin();

	// normal map
	glGenTextures(2, normalmap);
	for(int i=0; i<2; i++) {
		glBindTexture(GL_TEXTURE_2D, normalmap[i]);
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
	}


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
	
	// Intensity 
	for(int i=0; i<2; i++) {
		glBindTexture(GL_TEXTURE_2D, texId[i+1]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
				GL_UNSIGNED_BYTE,
				NULL);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	fb->addColorTex(texId[1]);
	fb->addColorTex(texId[2]);
	fb->addColorTex(normalmap[0]);
	fb->addColorTex(normalmap[1]);
	fb->addDepthTex(texId[0]);
	fb->unbind();

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
	sl_normmap->addUniform('f',"alpha", (void*)&alpha);
	sl_normmap->addUniform('v',"LightPos_int", (void*)lpos_int);
	sl_normmap->addUniform('t',"normalmap1",(void*)&normalmap[0]);
	sl_normmap->addUniform('t',"normalmap2",(void*)&normalmap[1]);

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

void drawQuad() 
{
	glOrtho(0,1,0,1,0,1);
	glBegin(GL_QUADS);
	glVertex2f(0.0f, 0.0f);
	glVertex2f(1.0f, 0.0f);
	glVertex2f(1.0f, 1.0f);
	glVertex2f(0.0f, 1.0f);
	glEnd();
}

void drawTexture(int x, int y, int width, int height, GLuint tex)
{
	glPushMatrix();			
	reset_cam();
	glPushAttrib(GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT);
	glViewport (x, y, width, height);
	glOrtho(0,1,0,1,0,1);

	glBindTexture(GL_TEXTURE_2D, tex);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopAttrib();			
	glPopMatrix();			
}

void display()
{
	timer++;
	if(timer%1 == 0)
	{
		alpha += alpha_step;
		if(alpha >= 1.f || alpha <= 0.f) {
			alpha_step *= -1;
			alpha += alpha_step;
		}
	}
	GLenum err;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render to texture
	GLvoid *vpixel = malloc(W*H*4*sizeof(float));
	glPushMatrix();			
		setup_cam();
		fb->bind();
		for(int i=0; i<2; i++) {
			glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT+i);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			lpos[0] *= -1.;
			sl->reload();
			sl->set();
			glutSolidTeapot(1.5);			
			sl->unset();
		}
		fb->unbind();
	glPopMatrix();			

	// orig img
	drawTexture(0,0,W*0.25,H*0.5,texId[1]);
	drawTexture(W*0.25,0,W*0.25,H*0.5,texId[2]);

	// orig normals
	glPushMatrix();			
		setup_cam();
		glPushAttrib(GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT);
		glViewport (W*0.25, H*0.5, W*0.25, H*0.5);
		sn->set();
		glutSolidTeapot(1.5);			
		sn->unset();
		glPopAttrib();
	glPopMatrix();			

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
	Imin = float(min)/255.f;
	Imax = 1.0;
	sf[(char*)"intensity"]->reload();

	// filtered img
	glPushMatrix();			
		for(int i=0; i<2; i++) {
			reset_cam();
			lpos[0] *= -1.;
			current_filter->second->reload();
			current_filter->second->set();

			fb->bind();
			glDrawBuffer(GL_COLOR_ATTACHMENT2_EXT+i);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glBindTexture(GL_TEXTURE_2D, texId[i+1]);
			drawQuad();
			glBindTexture(GL_TEXTURE_2D, 0);
			fb->unbind();
			current_filter->second->unset();

			drawTexture(W*(0.5+i*0.25), H*0.5, W*0.25, H*0.5, normalmap[i]);
		}
	glPopMatrix();			

	// relighted
	GLint active;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &active);
	glPushMatrix();			
	for(int i=0; i<1; i++) {
		reset_cam();
		//lpos[0] *= -1.;
	
		lpos_int[0] = lpos[0]*alpha - lpos[0]*(1.0-alpha);

		sl_normmap->reload();
		glPushAttrib(GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT);
		glViewport(W*(0.5+i*0.25), 0., W*0.25, H*0.5);
		sl_normmap->set();

		int id1 = glGetUniformLocationARB( sl_normmap->shaderobj, "normalmap1");
		int id2 = glGetUniformLocationARB( sl_normmap->shaderobj, "normalmap2");

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, normalmap[0]);
		glUniform1iARB(id1, 0);
		//sl_normmap->setUniform("normalmap1", &normalmap[0]);
		//
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalmap[1]);
		glUniform1iARB(id1, 1);
		//sl_normmap->setUniform("normalmap2", &normalmap[1]);
	
		drawQuad();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glPopAttrib();
		sl_normmap->unset();
	}
	glPopMatrix();			
	glActiveTexture(active);

	glFlush();				
	glutSwapBuffers();			
	if((err=glGetError())!=0) {
		printf("\n%s:%d %s\n", __func__, __LINE__, gluErrorString(err));
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
			sl->reload();
			break;
		case 's':
			lpos[2]-=0.5;
			lpos[2]-=0.5;
			current_filter->second->reload();
			sl->reload();
			break;
		case 'd':
			lpos[0]+=0.5;
			lpos[0]+=0.5;
			current_filter->second->reload();
			sl->reload();
			break;
		case 'a':
			lpos[0]-=0.5;
			lpos[0]-=0.5;
			current_filter->second->reload();
			sl->reload();
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
	glutIdleFunc(display);		
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
