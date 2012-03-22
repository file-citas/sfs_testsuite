varying vec4 l;
varying vec4 v;

void main(void)
{
	v = gl_ModelViewMatrix * gl_Vertex;
	gl_TexCoord[0]  = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_Position    = gl_TexCoord[0];
}
