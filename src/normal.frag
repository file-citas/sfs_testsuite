varying vec3 normal;

void main()
{
	gl_FragColor = vec4(normalize(normal),1.0);
}
