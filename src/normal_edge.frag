
varying vec3 N;
varying vec3 L;

void main()
{ 
	float lambert = dot(normalize(L),normalize(N));
	gl_FragColor = vec4(lambert,lambert,lambert,1.);
}
