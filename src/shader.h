#ifndef SHADER
#define SHADER

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <map>
#include <iostream>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glext.h>
#include <GL/glx.h>

#define MAXLINE 80

using namespace std;

class shader
{
public:
	shader(char* vert, char* frag);
	~shader();
	void reload();
	void set();
	void unset();
	char* GetSource(char* file);
	unsigned int addUniform(char type, const char* name, void* value);
	void setUniform(GLuint id, void* value);
	void setUniform(const char* name, void* value);
	void printInfoLog(GLhandleARB obj);
	GLhandleARB shaderobj;

private:
	char* vert;
	char* frag;
	GLhandleARB vertexshader;
	GLhandleARB fragmentshader;
	unsigned int uniformIdx;
	unsigned int n_tex;
	vector<void*> value;
	vector<GLuint> uniformId;
	map<const char*, uint> stu;
	vector<char> type;
};
#endif
