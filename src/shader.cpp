#include "shader.h"

void check()
{
	GLenum err;
	if((err=glGetError())!=0) {
		printf("\n%s:%d %s\n", __func__, __LINE__, gluErrorString(err));
		exit(1);
	}
}
shader::shader(char* vert, char* frag)
{
	//std::cerr << "creating shaders : " << vert << ", " << frag << std::endl;
	this->vert=vert;
	this->frag=frag;
	uniformIdx = 0;
	n_tex = 0;

	// create shader objects
	shaderobj = glCreateProgramObjectARB();
	vertexshader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	fragmentshader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	// get source code for fragment and vertex shader
	char * source;
	source = GetSource(vert);
	glShaderSourceARB(vertexshader, 1, (const GLcharARB**)&source, NULL);

	source = GetSource(frag);
	glShaderSourceARB(fragmentshader, 1, (const GLcharARB**)&source, NULL);

	// compile the fragment and vertex shader
	glCompileShader(vertexshader);
	glCompileShader(fragmentshader);

	// attach shaders to shader obj
	glAttachObjectARB(shaderobj, vertexshader);
	glAttachObjectARB(shaderobj, fragmentshader);

	// link the shader obj and check status
	glLinkProgramARB(shaderobj);
	printInfoLog(shaderobj);
}

shader::~shader()
{
	//std::cerr << "deleting shaders" << std::endl;
	// delete fragment and vertex shader and check status
	glDeleteObjectARB(vertexshader);
	printInfoLog(vertexshader);
	glDeleteObjectARB(fragmentshader);
	printInfoLog(fragmentshader);
}

void shader::reload()
{
	GLenum err;
	//std::cerr << "reloading " << vert << ", " << frag << std::endl;
	glDetachObjectARB(shaderobj, vertexshader);
	glDetachObjectARB(shaderobj, fragmentshader);

	vertexshader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	fragmentshader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	char * source;
	source = GetSource(vert);
	glShaderSourceARB(vertexshader, 1, (const GLcharARB**)&source, NULL);

	source = GetSource(frag);
	glShaderSourceARB(fragmentshader, 1, (const GLcharARB**)&source, NULL);

	// compile the fragment and vertex shader
	glCompileShader(vertexshader);
	glCompileShader(fragmentshader);

	// attach shaders to shader obj
	glAttachObjectARB(shaderobj, vertexshader);
	glAttachObjectARB(shaderobj, fragmentshader);

	// link the shader obj and check status
	glLinkProgramARB(shaderobj);
	printInfoLog(shaderobj);

	//t_tex = 0;
	for(size_t i=0; i<value.size(); i++)
	{
		if(type.at(i)=='t') continue;//n_tex++;
		setUniform(i,value.at(i));
	}

	if((err=glGetError())!=0) {
		printf("\n%s: %d\n", __func__, __LINE__);
		exit(1);
	}
}

void shader::set()
{
	glUseProgramObjectARB(shaderobj);
}

void shader::unset()
{
	glUseProgramObjectARB(0);
}

void shader::printInfoLog(GLhandleARB obj)
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;

	glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB,
			&infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetInfoLogARB(obj, infologLength, &charsWritten, infoLog);
		std::cerr << infoLog;
		free(infoLog);
	}
}

char* shader::GetSource(char* file)
{
	FILE* fp = fopen(file, "r");
	if(fp == NULL) {
		perror("fopen");
		//return NULL;
		exit(1);
	}
	char tmp[MAXLINE];
	char* src=NULL;
	int pos=0;

	while(fgets(tmp, MAXLINE, fp) != NULL)
	{
		int len = strlen(tmp)*sizeof(char);
		src=(char*)realloc(src, pos+len);
		if(src==NULL)
		{
			perror("realloc");
			return NULL;
		}
		strncpy(&src[pos], tmp, strlen(tmp)-1);
		pos += strlen(tmp)-1;
	}
	if(ferror(fp))
	{
		perror("fgets");
		return NULL;
	}
	src[pos]=0;
	fclose(fp);

	return src;
}

unsigned int shader::addUniform(
		char t, const char* name,
		void* value)
{
	GLuint id = glGetUniformLocationARB(shaderobj, name);
	if((int)id < 0) return id;
	uniformId.push_back(id);
	type.push_back(t);
	this->value.push_back(value);
	stu.insert(
		make_pair<const char*, unsigned int>(
			name, uniformIdx
		)
	);
	if(t=='t') 
		textures.insert(
			make_pair<GLuint, GLuint>(
				id, n_tex
			)
		);
	setUniform(uniformIdx, value);
	n_tex++;
	uniformIdx++;
	return uniformIdx-1;
}

void shader::setUniform(const char* name, void* value)
{
	setUniform(stu[name], value);
}

void shader::setUniform(unsigned int idx, void* value)
{
	glUseProgramObjectARB(shaderobj);
	switch(type.at(idx))
	{
		case 't':
			break;
			GLint active;
			glGetIntegerv(GL_ACTIVE_TEXTURE, &active);

			glActiveTexture(GL_TEXTURE0+textures[uniformId[idx]]);

			GLint activetmp;
			glGetIntegerv(GL_ACTIVE_TEXTURE, &activetmp);

			glBindTexture(GL_TEXTURE_2D, *((GLuint*)value));
			glUniform1iARB(uniformId.at(idx), textures[uniformId[idx]]);

			glActiveTexture(active);
			break;
		case 'i':
			glUniform1i(uniformId.at(idx), *((int*)value));
			break;

		case 'f':
			glUniform1f(uniformId.at(idx), *((float*)value));
			break;

		case 'v':
			glUniform3fv(uniformId.at(idx), 1, (float*)value);
			break;

		default:
			// TODO
			exit(1);
	}
	glUseProgramObjectARB(0);
}
