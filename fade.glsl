uniform sampler2DRect image;
uniform float opacity;
uniform int shift;

void main() {	
	int row = gl_FragCoord.y;

	int s;
	if (row % 2 == 0) {
		s = shift;
	}
	else {
		s = -shift;
	}

	vec4 color = texture2DRect(image, vec2(gl_TexCoord[0].s + s, gl_TexCoord[0].t));

	color.a *= opacity;
		
	gl_FragColor = color;
}