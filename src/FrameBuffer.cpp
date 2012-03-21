#include "FrameBuffer.h"

FrameBuffer::FrameBuffer(unsigned int nColor) : _nColor(nColor)
{
	glGenFramebuffersEXT(1, &_fboId);

	if(_nColor == 0)
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fboId);
		glDrawBuffer(GL_NONE);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}
	else
		_colorTexId = (GLuint*) malloc(_nColor*sizeof(GLuint));

	_colorIdx 	= 0;
	_depthTexId	= 0;
	_init		= false;
}

FrameBuffer::~FrameBuffer()
{
	glDeleteTextures(_nColor, _colorTexId);
	glDeleteTextures(1, &_depthTexId);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glDeleteFramebuffersEXT(1, &_fboId);
	free((void*)_colorTexId);
}

void FrameBuffer::addColorTex(GLuint colorTexId)
{
	if(_colorIdx < _nColor)
		_colorTexId[_colorIdx] = colorTexId;
	else
		std::cerr << "Texture arry already full" << std::endl;
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fboId);
	glBindTexture(GL_TEXTURE_2D, colorTexId);
	glFramebufferTexture2DEXT(
		GL_FRAMEBUFFER_EXT, 
		GL_COLOR_ATTACHMENT0_EXT+_colorIdx, 
		GL_TEXTURE_2D, 
		colorTexId,
		0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	_colorIdx++;

	if(_colorIdx == _nColor && _depthTexId != 0) _init = true;
}

void FrameBuffer::addDepthTex(GLuint depthTexId)
{
	_depthTexId = depthTexId;
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fboId);
	glFramebufferTexture2DEXT(
		GL_FRAMEBUFFER_EXT,
		GL_DEPTH_ATTACHMENT_EXT,
		GL_TEXTURE_2D,
		_depthTexId, 0);
	if(_colorIdx == _nColor) _init = true;
}

void FrameBuffer::bind()
{
	if(_init) glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fboId);
}

void FrameBuffer::unbind()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

GLuint FrameBuffer::getColorTex(unsigned int colorTexIdx)
{
	if(colorTexIdx < _colorIdx) return _colorTexId[colorTexIdx];
}

GLuint FrameBuffer::getDepthTex()
{
	if(_init) return _depthTexId;
}

GLenum FrameBuffer::check()
{
	return glCheckFramebufferStatus(_fboId);
}
