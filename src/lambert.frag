uniform vec3 LightPos;
varying vec3 N;
varying vec3 L;

void main()
{ 
	/*float l = dot(normalize(L),normalize(N));*/
	float l = dot(normalize(-LightPos),normalize(N));
	float a = 0.1;
	gl_FragColor = vec4(l, l ,l ,1.);
}
