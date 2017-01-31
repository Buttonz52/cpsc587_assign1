// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

uniform sampler2D tex;
// interpolated colour received from vertex stage
in vec3 Colour;
in vec2 textureCoords;
in vec3 vertPos;
in vec3 center;

// first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour;

void main(void)
{
	//check three things: vertPos, sphereCenter,lightSrc
	vec3 n = normalize(vertPos - center);
	vec3 l = normalize(lightSrc-vertPos);
 
	float diffuse = max(0.0, dot(n, l));                                 
	
    FragmentColour = texture(tex, textureCoords);

    if(doDiffuse == 1)

		FragmentColour = FragmentColour * diffuse;
	else
		FragmentColour = texture(tex, textureCoords);
}
