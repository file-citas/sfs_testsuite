void main(void)
{
	gl_TexCoord[0]  = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_Position    = gl_ModelViewProjectionMatrix * gl_Vertex;
}
