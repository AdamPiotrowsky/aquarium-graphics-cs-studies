#version 330

out vec4 pixelColor; //Output variable. Almost final pixel color.

//Varying variables
in vec4 ic;
in vec4 n;
in vec4 l;
in vec4 v;
in vec4 n1;
in vec4 l1;
in vec4 v1;
uniform sampler2D textureMap0; //globalnie
uniform sampler2D textureMap1; //globalnie
in vec2 iTexCoord0; //globalnie
in vec2 iTexCoord1;

void main(void) {
	//Normalized, interpolated vectors
	vec4 ml = normalize(l);
	vec4 mn = normalize(n);
	vec4 mv = normalize(v);
	//Reflected vector
	vec4 mr = reflect(-ml, mn);

	vec4 ml1 = normalize(l1);
	vec4 mn1 = normalize(n1);
	vec4 mv1 = normalize(v1);
	//Reflected vector
	vec4 mr1 = reflect(-ml1, mn1);

	//Surface parameters
	//vec4 kd = ic;
	vec4 texColor0=texture(textureMap0,iTexCoord0);//main
	vec4 texColor1=texture(textureMap1,iTexCoord1);//main
	vec4 kd = ((1*texColor0+1*texColor1 )/2);
	kd.a = ic.a;
	//vec4 ks = texture(textureMap1, iTexCoord0);
	vec4 ks= vec4(1,1,1,1);

	//Lighting model computation
	float nl = clamp(dot(mn, ml), 0, 1);
	float rv = pow(clamp(dot(mr, mv), 0, 1), 50);

	float nl1 = clamp(dot(mn1, ml1), 0, 1);
	float rv1 = pow(clamp(dot(mr1, mv1), 0, 1), 50);

	//working 2 sources of light
	pixelColor = vec4(kd.rgb * nl, kd.a) + vec4(ks.rgb * rv, 0) + vec4(kd.rgb * nl1, 0) + vec4(ks.rgb * rv1, 0);
	//pixelColor = vec4(kd.rgb * nl, 0.1) + vec4(ks.rgb * rv, 0);

}
