varying vec3 v;
varying vec3 normal;


void main(void)
{
	normal		= normalize(gl_NormalMatrix * gl_Normal);
	v		= vec3(gl_ModelViewMatrix * gl_Vertex);
	gl_Position    = gl_ModelViewProjectionMatrix * gl_Vertex;
}
