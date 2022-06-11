#version 330

//Uniform variables
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

//Attributes
in vec4 vertex; //Vertex coordinates in model space
in vec4 color; //vertex color
in vec4 normal; //Vertex normal in model space
in vec2 texCoord0;
out vec2 iTexCoord0; //globalnie
out vec2 iTexCoord1; //globalnie

//Varying variables
out vec4 ic;
out vec4 l;
out vec4 n;
out vec4 v;
out vec4 l1;
out vec4 n1;
out vec4 v1;

void main(void) {
    vec4 lp = vec4(0, 0, -25, 1); //light position, world space
    l = normalize(V * lp - V * M * vertex); //vector towards the light in eye space
    v = normalize(vec4(0, 0, 0, 1) - V * M * vertex); //vector towards the viewer in eye space
    n = normalize(V * M * normal); //normal vector in eye space

    vec4 lp1 = vec4(0, 25, 0, 1); //light position, world space
    l1 = normalize(V * lp1 - V * M * vertex); //vector towards the light in eye space
    v1 = normalize(vec4(0, 0, 0, 1) - V * M * vertex); //vector towards the viewer in eye space
    n1 = normalize(V * M * normal); //normal vector in eye space
	iTexCoord0=texCoord0; //w main
    ic = color;

	vec4 eyeN=normalize(V*M*normal);
	iTexCoord1=(eyeN.xy+1)/2;
    
	//working 2 sources of light
    gl_Position = P * V * M * vertex;
}
