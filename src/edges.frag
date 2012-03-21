uniform sampler2D tex;
uniform int fw;

void main()
{
	const float ts = 1.0/1440.0;
	vec2 tc =  (gl_TexCoord[0].xy / gl_TexCoord[0].w)/2.0 + 0.5;
	vec3 c = texture2D(tex,tc).rbg;

	/* 
	 * avg gradient only
	 */
	vec3 cref = vec3(0,0,0);
	for(int i=-fw; i<=fw; i++) {
		if(i==0) continue;
		vec2 tcf = vec2(tc.x+float(i)*ts, tc.y);
		if(tcf.x<0.0 || tc.x>1.0) continue;
		cref.r += texture2D(tex,tcf).r/float(fw*2);
	}

	for(int i=-fw; i<=fw; i++) {
		if(i==0) continue;
		vec2 tcf = vec2(tc.x, tc.y+float(i)*ts);
		if(tcf.y<0.0 || tc.y>1.0) continue;
		cref.b += texture2D(tex,tcf).r/float(fw*2);
	}

	gl_FragColor = vec4(c.r-cref.r, c.b-cref.b, 0., 1.0);
	 /*
	 */

}
