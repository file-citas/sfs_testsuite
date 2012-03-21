varying vec4 l;
varying vec4 v;

uniform sampler2D tex;
uniform int fw;

uniform float tsx;
uniform float tsy;

/*
 * Filter:
 *
 * Quelle: holoborodko.com/pavel/image-processing/edge-detection
 *
 * -1 -2 0 2 1
 * -2 -4 0 4 2
 * -1 -2 0 2 1
 */

void main()
{
	const float tw = 1440.0;
	const float ts = 1.0/tw;
	vec2 tc =  (gl_TexCoord[0].xy / gl_TexCoord[0].w)/2.0 + 0.5;
	vec3 L = normalize(l.xyz/l.w)-normalize(v.xyz/v.w);
	vec3 c = texture2D(tex,tc).rbg;

	vec3 cref = vec3(0., 0., 0.);
	vec3 frow = vec3(2., 4., 2.);
	for(int ix=-2; ix<=2; ix++) {
		vec3 cfrow = frow/float(-ix);
		for(int iy=-1; iy<=1; iy++) {
			if(ix==0) continue;
			vec2 tcf = vec2(tc.x+float(ix)*tsx, tc.y+float(iy)*tsy);
			if(tcf.x<0.0 || tc.x>1.0 || tcf.y<0.0 || tc.y>1.0) continue;
			cref.r += texture2D(tex,tcf).r*cfrow[iy+1]/float(32);
		}
	}

	vec3 fcol = vec3(2., 4., 2.);
	for(int iy=-2; iy<=2; iy++) {
		vec3 cfcol = fcol/float(-iy);
		for(int ix=-1; ix<=1; ix++) {
			if(iy==0) continue;
			vec2 tcf = vec2(tc.x+float(ix)*tsx, tc.y+float(iy)*tsy);
			if(tcf.x<0.0 || tc.x>1.0 || tcf.y<0.0 || tc.y>1.0) continue;
			cref.b += texture2D(tex,tcf).r*cfcol[ix+1]/float(32);
		}
	}


	float p = (cref.r);
	float q = (cref.b);
	
	vec3 gI = vec3(p, q, 0.);

	gl_FragColor = vec4(cref.r, cref.b, 0. , 1.0);
	 /*
	gl_FragColor = vec4(normalize(gI), 1.0);
	gl_FragColor = normalize(vec4(p, q, fw, 1.0));
	 */
}
