uniform vec3 LightPos;
uniform sampler2D normalmap;

varying vec3 N;
varying vec3 L;

void main()
{ 
	/*float l = dot(normalize(L),normalize(N));*/
	vec2 tc =  (gl_TexCoord[0].xy / gl_TexCoord[0].w)/2.0 + 0.5;
	vec3 normal = texture2D(normalmap,tc).rbg;
	float l = dot(normalize(-LightPos),normalize(normal));
	gl_FragColor = vec4(l, l ,l ,1.);
}
