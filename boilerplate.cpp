// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// specify that we want the OpenGL core profile before including GLFW headers
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>
#include <Magick++.h>
#include <math.h>

#include "ImageReader.h"

using namespace std;
using namespace glm;

// --------------------------------------------------------------------------
// OpenGL utility and support function prototypes

void QueryGLVersion();
bool CheckGLErrors();

string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

// Add uniform locations =============
GLuint projUniform;
GLuint viewUniform;
GLuint modelUniform;

float d = 5.0;
float timeScale = 0.01;

double newX = 0;
double newY = 0;
double oldX = 0;
double oldY = 0;
GLfloat height = 512;
GLfloat width = 512;
bool pressed;
bool released;

//GLOBALS
string picture = "images/stars.jpg";

float PI =  atan(1.0)*4.0;

vector<vec3> spherePoints;
vector<vec3> colours;
vector<vec2> textureCoords;

vec3 camCartCoord = vec3(0.0,1.0,2.0);
vec3 camSpheCoord = vec3(90.0,82.0,50.0);

vec3 lightSrc = vec3(0.0,0.0,0.0);
vec4 sphereCenter = vec4(0.0,0.0,0.0,0.0);

// --------------------------------------------------------------------------
// Functions to set up OpenGL shader programs for rendering

struct MyShader
{
    // OpenGL names for vertex and fragment shaders, shader program
    GLuint  vertex;
    GLuint  fragment;
    GLuint  program;

    // initialize shader and program names to zero (OpenGL reserved value)
    MyShader() : vertex(0), fragment(0), program(0)
    {}
};

// load, compile, and link shaders, returning true if successful
bool InitializeShaders(MyShader *shader)
{
    // load shader source from files
    string vertexSource = LoadSource("vertex.glsl");
    string fragmentSource = LoadSource("fragment.glsl");
    if (vertexSource.empty() || fragmentSource.empty()) return false;

    // compile shader source into shader objects
    shader->vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
    shader->fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

    // link shader program
    shader->program = LinkProgram(shader->vertex, shader->fragment);

    // check for OpenGL errors and return false if error occurred
    return !CheckGLErrors();
}

// deallocate shader-related objects
void DestroyShaders(MyShader *shader)
{
    // unbind any shader programs and destroy shader objects
    glUseProgram(0);
    glDeleteProgram(shader->program);
    glDeleteShader(shader->vertex);
    glDeleteShader(shader->fragment);
}

// --------------------------------------------------------------------------
// Add method to set transform uniforms


// Functions to set up OpenGL buffers for storing geometry data

struct MyGeometry
{
    // OpenGL names for array buffer objects, vertex array object
    GLuint  vertexBuffer;
    GLuint  colourBuffer;
    GLuint  textureCoordBuffer;
    GLuint  vertexArray;
    GLsizei elementCount;

    // initialize object names to zero (OpenGL reserved value)
    MyGeometry() : vertexBuffer(0), colourBuffer(0), vertexArray(0), elementCount(0)
    {}
};

float toRadians(float degree)
{
	return (degree * PI) / 180.0;
}

void updateCamera()
{
	camCartCoord.x = camSpheCoord.z * cos(toRadians(camSpheCoord.x)) * sin(toRadians(camSpheCoord.y));
	camCartCoord.y = camSpheCoord.z * cos(toRadians(camSpheCoord.y));
	camCartCoord.z = camSpheCoord.z * sin(toRadians(camSpheCoord.x)) * sin(toRadians(camSpheCoord.y)); 
}

void getSpherePoints(float radius, vec3 center)
{
	float x1,x2,x3,x4,y1,y2,y3,y4,z1,z2,z3,z4,u,v;
	int layers 		= 180/d;
	int loop_layer 	= 0;
	float u1 = 1.0;
	float v1 = 1.0;
	float inc = d/360.0;
	
	//cout << inc << endl;
	
	for(int j = 0.0; j <= 180.0; j = j + d)
	{
		for(int i = 0.0; i <= 360.0; i = i + d)
		{			
				x1 = (radius * cos(toRadians(i)) * sin(toRadians(j))) + center.x;
				y1 = (radius * cos(toRadians(j))) + center.y;
				z1 = (radius * sin(toRadians(i)) * sin(toRadians(j))) + center.z;				
								
				x2 = (radius * cos(toRadians(i)) * sin(toRadians(j+d))) + center.x;
				y2 = (radius * cos(toRadians(j+d))) + center.y;
				z2 = (radius * sin(toRadians(i)) * sin(toRadians(j+d))) + center.z;				
				
				x3 = (radius * cos(toRadians(i+d)) * sin(toRadians(j+d))) + center.x;
				y3 = (radius * cos(toRadians(j+d))) + center.y;
				z3 = (radius * sin(toRadians(i+d)) * sin(toRadians(j+d))) + center.z;
				
				spherePoints.push_back(vec3(x1,y1,z1));
				spherePoints.push_back(vec3(x2,y2,z2));
				spherePoints.push_back(vec3(x3,y3,z3));
				
				textureCoords.push_back(vec2(u1,v1));				//1
				textureCoords.push_back(vec2(u1,v1-(2*inc)));		//2
				textureCoords.push_back(vec2(u1-inc,v1-(2*inc)));	//3
				
				//cout << x1 << " " << y1 << " " << z1 << endl;
				
				if((loop_layer != 0) && (loop_layer != layers))
				{
					x4 = (radius * cos(toRadians(i+d)) * sin(toRadians(j))) + center.x;
					y4 = (radius * cos(toRadians(j))) + center.y;
					z4 = (radius * sin(toRadians(i+d)) * sin(toRadians(j))) + center.z;
					
					spherePoints.push_back(vec3(x1,y1,z1));
					spherePoints.push_back(vec3(x3,y3,z3));
					spherePoints.push_back(vec3(x4,y4,z4));
						
					textureCoords.push_back(vec2(u1,v1));				//1
					textureCoords.push_back(vec2(u1-inc,v1-(2*inc)));	//3
					textureCoords.push_back(vec2(u1-inc,v1));			//4
					
				}
				u1 -= inc; 
		}
		v1 -= 2.0*inc;
		u1 = 1.0;
		loop_layer++;
	}
}

// create buffers and fill with geometry data, returning true if successful
bool InitializeGeometry(MyGeometry *geometry, MyTexture *texture)
{	

	getSpherePoints(900.f,vec3(0.f,0.f,0.f));				//stores all the points for the sun
			
	for(int i=0;i<spherePoints.size();i++)
	{
		colours.push_back(vec3(1.0,0.8,0.0));	//satisfy color vector init
	}
	
    geometry->elementCount = spherePoints.size();
    

    // these vertex attribute indices correspond to those specified for the
    // input variables in the vertex shader
    const GLuint VERTEX_INDEX = 0;
    const GLuint COLOUR_INDEX = 1;
    const GLuint TEXTURE_INDEX = 2;

    // create an array buffer object for storing our vertices
    glGenBuffers(1, &geometry->vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, spherePoints.size()*sizeof(vec3), &spherePoints[0], GL_STATIC_DRAW);

    // create another one for storing our colours
    glGenBuffers(1, &geometry->colourBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
    glBufferData(GL_ARRAY_BUFFER, colours.size()*sizeof(vec3), &colours[0], GL_STATIC_DRAW);

    glGenBuffers(1, &geometry->textureCoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->textureCoordBuffer);
	glBufferData(GL_ARRAY_BUFFER, textureCoords.size()*sizeof(vec2), &textureCoords[0], GL_STATIC_DRAW);

    // create a vertex array object encapsulating all our vertex attributes
    glGenVertexArrays(1, &geometry->vertexArray);
    glBindVertexArray(geometry->vertexArray);

    // associate the position array with the vertex array object
    glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
    glVertexAttribPointer(VERTEX_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(VERTEX_INDEX);

    // assocaite the colour array with the vertex array object
    glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
    glVertexAttribPointer(COLOUR_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(COLOUR_INDEX);

    glBindBuffer(GL_ARRAY_BUFFER, geometry->textureCoordBuffer);
    glVertexAttribPointer(TEXTURE_INDEX, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(TEXTURE_INDEX);

    // unbind our buffers, resetting to default state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // check for OpenGL errors and return false if error occurred
    return !CheckGLErrors();
}

// deallocate geometry-related objects
void DestroyGeometry(MyGeometry *geometry)
{
    // unbind and destroy our vertex array object and associated buffers
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &geometry->vertexArray);
    glDeleteBuffers(1, &geometry->vertexBuffer);
    glDeleteBuffers(1, &geometry->colourBuffer);
}

void setTransformationUniform(GLuint uniform, glm::mat4 matrix)
{
	glUniformMatrix4fv(uniform,1,GL_FALSE, glm::value_ptr(matrix));
}
// --------------------------------------------------------------------------
// Rendering function that draws our scene to the frame buffer

void print4x4Matrix(mat4 mat)
{
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			cout << mat[i][j] << " ";
		}
		cout << "" << endl;
	}
	printf("\n");
}

void RenderScene(MyGeometry *geometry,MyTexture* star, MyShader *shader)
{
	// clear screen to a dark grey colour
    glClearColor(0.3, 0.3, 0.3, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    // bind our shader program and the vertex array object containing our
    // scene geometry, then tell OpenGL to draw our geometry
    glUseProgram(shader->program);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LEQUAL);
	
	glBindVertexArray(geometry->vertexArray);

	glm::mat4 I(1.f);														//(FOV, aspect ratio, z Near,z Far);
	setTransformationUniform(projUniform,glm::perspective(45.f, float(width)/float(height), .1f, 10000.f));
	setTransformationUniform(viewUniform,glm::lookAt(camCartCoord, glm::vec3(0.0,0.0,0.0), glm::vec3(0.0,1.0,0.0))); //(eye, center, up) //setTransformationUniform(viewUniform,I);
	setTransformationUniform(modelUniform,I);
	
	glBindTexture(GL_TEXTURE_2D, star->textureName);
    glDrawArrays(GL_TRIANGLES, 0, spherePoints.size());

    // reset state to default (no shader or geometry bound)
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // check for an report any OpenGL errors
    CheckGLErrors();
}

// --------------------------------------------------------------------------
// GLFW callback functions

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
    cout << "GLFW ERROR " << error << ":" << endl;
    cout << description << endl;
}

// handles keyboard input events
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
		
	}
    
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if(button == GLFW_MOUSE_BUTTON_1)
	{
		if(action == GLFW_PRESS)
		{
			////grab P0 
			glfwGetCursorPos(window, &oldX, &oldY);
			pressed = true;
			released = false;	
		}
		else if(action == GLFW_RELEASE)
		{
		//	cout << "Released" << endl;
			pressed = false;
			released = true;
		}
			
	}
}

//call back for moving the mouse
void mouseMoveCallback(GLFWwindow* window, double x, double y)
{
	if(pressed)
	{
		//grab P1 
		newX = x;
		newY = y;

		//calc P1 - P0
		GLfloat nX = (newX - oldX); 
		GLfloat nY = (newY - oldY);

		camSpheCoord.x += nX/1.5;
		
		if(camSpheCoord.y > 5.0 && camSpheCoord.y < 175.0)
			camSpheCoord.y -= nY/1.5;
		if(camSpheCoord.y < 5.0)
			camSpheCoord.y = 5.5;
		if(camSpheCoord.y > 175.0)
			camSpheCoord.y = 174.5;
		
		oldX = newX;
		oldY = newY;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camSpheCoord.z  -= yoffset*10 + 0.1;
}

// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{
    Magick::InitializeMagick(NULL);

    // initialize the GLFW windowing system
    if (!glfwInit()) {
        cout << "ERROR: GLFW failed to initilize, TERMINATING" << endl;
        return -1;
    }
    glfwSetErrorCallback(ErrorCallback);

    // attempt to create a window with an OpenGL 4.1 core profile context
    GLFWwindow *window = 0;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    int width = 1024, height = 1024;
    window = glfwCreateWindow(width, height, "CPSC 453 Assignment 4 Brendan Petras", 0, 0);
    if (!window) {
        cout << "Program failed to create GLFW window, TERMINATING" << endl;
        glfwTerminate();
        return -1;
    }

    // set keyboard callback function and make our context current (active)
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, mouseMoveCallback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwMakeContextCurrent(window);

    // query and print out information about our OpenGL environment
    QueryGLVersion();

    // call function to load and compile shader programs
    MyShader shader;
    if (!InitializeShaders(&shader)) {
        cout << "Program could not initialize shaders, TERMINATING" << endl;
        return -1;
    }
    
    MyTexture Tex;
    if(!InitializeTexture(&Tex, picture)) 
        cout << "Failed to load texture!" << endl;

    // Setup transformation uniforms ==================
	glUseProgram(shader.program);
	projUniform = glGetUniformLocation(shader.program, "proj");
	viewUniform = glGetUniformLocation(shader.program, "view");
	modelUniform = glGetUniformLocation(shader.program, "model"); 
	
	
    MyGeometry geometry;
    if (!InitializeGeometry(&geometry, &Tex))
        cout << "Program failed to intialize geometry!" << endl; 

    // run an event-triggered main loop
    while (!glfwWindowShouldClose(window))
    {
	glUseProgram(shader.program);
				
		

        // call function to draw our scene
       
        RenderScene(&geometry, &Tex, &shader);
        updateCamera();
        // scene is rendered to the back buffer, so swap to front for display
        glfwSwapBuffers(window);
        // sleep until next event before drawing again
        glfwPollEvents();
    }

    // clean up allocated resources before exit
    DestroyGeometry(&geometry);
    DestroyShaders(&shader);
    glfwDestroyWindow(window);
    glfwTerminate();

    cout << "Goodbye!" << endl;
    return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
    // query opengl version and renderer information
    string version  = reinterpret_cast<const char *>(glGetString(GL_VERSION));
    string glslver  = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

    cout << "OpenGL [ " << version << " ] "
         << "with GLSL [ " << glslver << " ] "
         << "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors()
{
    bool error = false;
    for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
    {
        cout << "OpenGL ERROR:  ";
        switch (flag) {
        case GL_INVALID_ENUM:
            cout << "GL_INVALID_ENUM" << endl; break;
        case GL_INVALID_VALUE:
            cout << "GL_INVALID_VALUE" << endl; break;
        case GL_INVALID_OPERATION:
            cout << "GL_INVALID_OPERATION" << endl; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
        case GL_OUT_OF_MEMORY:
            cout << "GL_OUT_OF_MEMORY" << endl; break;
        default:
            cout << "[unknown error code]" << endl;
        }
        error = true;
    }
    return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
    string source;

    ifstream input(filename);
    if (input) {
        copy(istreambuf_iterator<char>(input),
             istreambuf_iterator<char>(),
             back_inserter(source));
        input.close();
    }
    else {
        cout << "ERROR: Could not load shader source from file "
             << filename << endl;
    }

    return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
    // allocate shader object name
    GLuint shaderObject = glCreateShader(shaderType);

    // try compiling the source as a shader of the given type
    const GLchar *source_ptr = source.c_str();
    glShaderSource(shaderObject, 1, &source_ptr, 0);
    glCompileShader(shaderObject);

    // retrieve compile status
    GLint status;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
        cout << "ERROR compiling shader:" << endl << endl;
        cout << source << endl;
        cout << info << endl;
    }

    return shaderObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    // allocate program object name
    GLuint programObject = glCreateProgram();

    // attach provided shader objects to this program
    if (vertexShader)   glAttachShader(programObject, vertexShader);
    if (fragmentShader) glAttachShader(programObject, fragmentShader);

    // try linking the program with given attachments
    glLinkProgram(programObject);

    // retrieve link status
    GLint status;
    glGetProgramiv(programObject, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
        cout << "ERROR linking shader program:" << endl;
        cout << info << endl;
    }

    return programObject;
}


// ==========================================================================
