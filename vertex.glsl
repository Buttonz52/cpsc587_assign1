// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

// location indices for these attributes correspond to those specified in the
// InitializeGeometry() function of the main program
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexColour;
layout(location = 2) in vec2 VertexTextureCoords;

// output to be interpolated between vertices and passed to the fragment stage
out vec3 Colour;
out vec2 textureCoords;
out vec3 vertPos;
out vec3 center;

//uniforms
// add transformation uniforms
uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

void main()
{
	center = vec3(model[3][0],model[3][1],model[3][2]);
	
    // assign vertex position without modification
    gl_Position = proj*view*model*vec4(VertexPosition, 1.0);

    // assign output colour to be interpolated
    Colour = VertexColour;
    textureCoords = VertexTextureCoords;
    vertPos = (model*vec4(VertexPosition,1.0)).xyz;
}
