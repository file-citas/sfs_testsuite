uniform vec3 LightPos_int;
uniform float alpha;
uniform sampler2D normalmap2;
uniform sampler2D normalmap1;

void main()
{ 
	vec2 tc =  (gl_TexCoord[0].xy / gl_TexCoord[0].w)/2.0 + 0.5;
	vec3 normal2 = texture2D(normalmap2,tc).rgb;
	vec3 normal1 = texture2D(normalmap1,tc).rgb;

	if(length(normal1)<=0.2)
		normal1 = normal2;
	if(length(normal2)<=0.2) 
		normal2 = normal1;

	float theta = dot(normal1,normal2);
	vec3 normal = normalize(normal1 * sin((1.-alpha)*theta)/sin(theta) +
			normal2 * sin(alpha*theta)/sin(theta));
	float l = dot(normalize(-LightPos_int),normal);

	gl_FragColor = vec4(l, l, l, 1.);
	gl_FragColor = vec4(normal, 1.);
}
