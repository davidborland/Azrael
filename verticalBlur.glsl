uniform sampler2DRect image;
uniform int kernelRadius;

void main() {	
	int count = 0;
	
	vec4 outColor;
	for (int i = -kernelRadius; i <= kernelRadius; i++) {
		outColor += texture2DRect(image, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t + i));
		count++;
	}
	outColor /= count;

	gl_FragColor = outColor;
}