#ifndef FRAMEBUFFER
#define FRAMEBUFFER

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glx.h>    /* this includes the necessary X headers */
#include <GL/gl.h>

#include <malloc.h>
#include <iostream>

class FrameBuffer
{
public:
	FrameBuffer(unsigned int nColor);
	~FrameBuffer();

	void addColorTex(GLuint colorTexId);
	void addDepthTex(GLuint depthTexId);
	GLuint getColorTex(unsigned int colorTexIdx);
	GLuint getDepthTex();

	void bind();
	void unbind();

	GLenum check();

private:
	GLuint 		_fboId;
	GLuint	 	_depthTexId;
	GLuint* 	_colorTexId;

	unsigned int 	_nColor;
	unsigned int 	_colorIdx;

	bool		_init;
};

#endif
