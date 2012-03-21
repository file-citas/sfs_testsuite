varying vec3 v;
varying vec3 normal;

void main()
{
	vec3 N = normalize(normal);
	vec3 L = normalize(gl_LightSource[0].position.xyz - v);
	vec3 E = normalize(-v);
	vec4 d = vec4 (1.0, 1.0, 1.0, 1.0);

	//calculate Ambient Term:  
	vec4 Iamb = vec4(0.1, 0.1, 0.1, 1.0);    

	//calculate Diffuse Term:  
	vec4 Idiff = vec4(0.5, 0.5, 0.5, 1.0) * max(dot(N,L), 0.0);
	Idiff = clamp(Idiff, 0.0, 1.0);     

	// calculate Specular Term:
	vec4 Ispec = vec4(1.0, 1.0, 1.0, 1.0) 
		* pow(max(dot(R,E),0.0),0.3*10.0);
	Ispec = clamp(Ispec, 0.0, 1.0); 

	gl_FragColor = gl_FrontLightModelProduct.sceneColor + Iamb + Idiff + Ispec; 
}
