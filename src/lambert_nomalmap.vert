uniform vec3 LightPos;
varying vec3 N;
varying vec3 L;

void main()
{    
	gl_TexCoord[0]  = gl_ModelViewProjectionMatrix * gl_Vertex;
	N = normalize(gl_NormalMatrix*gl_Normal);
	/* LightPos ist relativ zu Object -> Object-Space */
	L = vec3(gl_ModelViewMatrix*(vec4(LightPos,1.)-gl_Vertex));
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
