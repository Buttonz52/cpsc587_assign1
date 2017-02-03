/**
 * Author:	Andrew Robert Owens
 * Email:	arowens [at] ucalgary.ca
 * Date:	January, 2017
 * Course:	CPSC 587/687 Fundamental of Computer Animation
 * Organization: University of Calgary
 *
 * Copyright (c) 2017 - Please give credit to the author.
 *
 * File:	main.cpp
 *
 * Summary:
 *
 * This is a (very) basic program to
 * 1) load shaders from external files, and make a shader program
 * 2) make Vertex Array Object and Vertex Buffer Object for the quad
 *
 * take a look at the following sites for further readings:
 * opengl-tutorial.org -> The first triangle (New OpenGL, great start)
 * antongerdelan.net -> shaders pipeline explained
 * ogldev.atspace.co.uk -> good resource
 */

#include <iostream>
#include <cmath>
#include <chrono>
#include <limits>
#include <string>
#include <vector>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "ShaderTools.h"
#include "OpenGLMatrixTools.h"
#include "Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;
//==================== GLOBAL VARIABLES ====================//
/*	Put here for simplicity. Feel free to restructure into
*	appropriate classes or abstractions.
*/

// Drawing Program
GLuint basicProgramID;

// Data needed for Quad
GLuint vaoID;
GLuint vertBufferID;
mat4 M;

// Data needed for Line 
GLuint line_vaoID;
GLuint line_vertBufferID;
mat4 line_M;

//Curve DS
vector<vec3> curve;
vector<vec3> sphere;
vector<vec2> textureCoords;

//globals
float PI =  atan(1.0)*4.0;

// Only one camera so only one veiw and perspective matrix are needed.
mat4 V;
mat4 P;

// Only one thing is rendered at a time, so only need one MVP
// When drawing different objects, update M and MVP = M * V * P
mat4 MVP;

// Camera and veiwing Stuff
Camera camera;
int g_moveUpDown = 0;
int g_moveLeftRight = 0;
int g_moveBackForward = 0;
int g_rotateLeftRight = 0;
int g_rotateUpDown = 0;
int g_rotateRoll = 0;
float g_rotationSpeed = 0.015625;
float g_panningSpeed = 0.25;
bool g_cursorLocked;
float g_cursorX, g_cursorY;

bool g_play = false;

int WIN_WIDTH = 800, WIN_HEIGHT = 600;
int FB_WIDTH = 800, FB_HEIGHT = 600;
float WIN_FOV = 60;
float WIN_NEAR = 0.01;
float WIN_FAR = 1000;

//==================== FUNCTION DECLARATIONS ====================//
void print4x4Matrix(mat4 mat);
void displayFunc();
void resizeFunc();
void init();
void generateIDs();
void deleteIDs();
void setupVAO();
void loadQuadGeometryToGPU();
float toRadians(float degree);
void getSpherePoints(float radius, vec3 center);
void loadCurve();
vec3 calcPoint(vec3 a, vec3 b, vec3 c, vec3 d, float t);
vec3 lerp(vec3 a, vec3 b, float t);
void reloadProjectionMatrix();
void loadModelViewMatrix();
void setupModelViewProjectionTransform();

void windowSetSizeFunc();
void windowKeyFunc(GLFWwindow *window, int key, int scancode, int action,
                   int mods);
void windowMouseMotionFunc(GLFWwindow *window, double x, double y);
void windowSetSizeFunc(GLFWwindow *window, int width, int height);
void windowSetFramebufferSizeFunc(GLFWwindow *window, int width, int height);
void windowMouseButtonFunc(GLFWwindow *window, int button, int action,
                           int mods);
void windowMouseMotionFunc(GLFWwindow *window, double x, double y);
void windowKeyFunc(GLFWwindow *window, int key, int scancode, int action,
                   int mods);
void animateBead(float t);
void moveCamera();
void reloadMVPUniform();
void reloadColorUniform(float r, float g, float b);
std::string GL_ERROR();
int main(int, char **);

//==================== FUNCTION DEFINITIONS ====================//

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

void displayFunc() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0.3, 0.3, 0.3, 1.0);

  // Use our shader
  glUseProgram(basicProgramID);

  // ===== DRAW QUAD ====== //
  MVP = P * V * M;
  reloadMVPUniform();
  reloadColorUniform(1, 0, 1);

  // Use VAO that holds buffer bindings
  // and attribute config of buffers
  glBindVertexArray(vaoID);
  // Draw Quads, start at vertex 0, draw 4 of them (for a quad)
  glDrawArrays(GL_TRIANGLE_STRIP, 0, sphere.size());

  // ==== DRAW LINE ===== //
  MVP = P * V * line_M;
  reloadMVPUniform();

  reloadColorUniform(0, 1, 1);

  // Use VAO that holds buffer bindings
  // and attribute config of buffers
  glBindVertexArray(line_vaoID);
  // Draw lines
  glDrawArrays(GL_LINE_STRIP, 0, curve.size());
  
}

void animateBead(float t) {
  //M = RotateAboutYMatrix(100 * t);

  //float s = (sin(t) + 1.f) / 2.f;
  float x = (1 - t) * (10) + t * (-10);

  //M = TranslateMatrix(x, 0, 0) * M;

  setupModelViewProjectionTransform();
  reloadMVPUniform();
}

void loadQuadGeometryToGPU() {
  // Just basic layout of floats, for a quad
  // 3 floats per vertex, 4 vertices
  //std::vector<vec3> verts;
  //verts.push_back(Vec3f(-1, -1, 0));
  //verts.push_back(Vec3f(-1, 1, 0));
  //verts.push_back(Vec3f(1, -1, 0));
  //verts.push_back(Vec3f(1, 1, 0));
  getSpherePoints(0.3, vec3(0,0,0));

  glBindBuffer(GL_ARRAY_BUFFER, vertBufferID);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(vec3) * sphere.size(), // byte size of Vec3f, 4 of them
               sphere.data(),      // pointer (Vec3f*) to contents of verts
               GL_STATIC_DRAW);   // Usage pattern of GPU buffer
}

float toRadians(float degree)
{
	return (degree * PI) / 180.0;
}

void getSpherePoints(float radius, vec3 center)
{
	float x1,x2,x3,x4,y1,y2,y3,y4,z1,z2,z3,z4;
	int d = 5;	
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
				
				sphere.push_back(vec3(x1,y1,z1));
				sphere.push_back(vec3(x2,y2,z2));
				sphere.push_back(vec3(x3,y3,z3));
				
				textureCoords.push_back(vec2(u1,v1));				//1
				textureCoords.push_back(vec2(u1,v1-(2*inc)));		//2
				textureCoords.push_back(vec2(u1-inc,v1-(2*inc)));	//3
				
				//cout << x1 << " " << y1 << " " << z1 << endl;
				
				if((loop_layer != 0) && (loop_layer != layers))
				{
					x4 = (radius * cos(toRadians(i+d)) * sin(toRadians(j))) + center.x;
					y4 = (radius * cos(toRadians(j))) + center.y;
					z4 = (radius * sin(toRadians(i+d)) * sin(toRadians(j))) + center.z;
					
					sphere.push_back(vec3(x1,y1,z1));
					sphere.push_back(vec3(x3,y3,z3));
					sphere.push_back(vec3(x4,y4,z4));
						
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

void loadLineGeometryToGPU() {
  // Just basic layout of floats, for a quad
  // 3 floats per vertex, 4 vertices

  curve.push_back(vec3(0, 0, 0));
  curve.push_back(vec3(5, 5, 0));
  curve.push_back(vec3(5, 5, 5));
  curve.push_back(vec3(0, 0, 5));
  curve.push_back(vec3(-5, 5, 5));	
  curve.push_back(vec3(-5, 5, 0));	
  curve.push_back(vec3(0, 0, 0));				

  loadCurve();

  cout << curve.size() << endl;
  for(unsigned int i = 0; i < curve.size(); i++)
	//cout << curve[i] << endl;

  glBindBuffer(GL_ARRAY_BUFFER, line_vertBufferID);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(vec3) * curve.size(), // byte size of Vec3f, 4 of them
               &curve[0],      // pointer (Vec3f*) to contents of verts
               GL_STATIC_DRAW);   // Usage pattern of GPU buffer
}

void loadCurve()
{
	//check for right size of verts
	if((curve.size() - 1) % 3  != 0)
		cout << "vertex list is not 3n-1 in size.";
		
	vec3 p0, p1, p2, p3;
	int numSegments = (curve.size()-1)/3;
	int numLines = 100;
	vector<vec3> tmp;
	

	cout << numSegments << endl;

	for(int i = 0; i < numSegments*3; i=i+3)
	{
		p0 = curve[i];
		p1 = curve[i+1];
		p2 = curve[i+2];
		p3 = curve[i+3];
		

		//create B(i)
		//step t from 0-1
		float t = 0.00;
		
		for(int j = 0; j < 2*numLines/numSegments; j++)
		{
			vec3 point = calcPoint(p0,p1,p2,p3,t);			
			tmp.push_back(point);
			t += 0.01;
		}	
	}	
	curve = tmp;
}

vec3 calcPoint(vec3 a, vec3 b, vec3 c, vec3 d, float t)
{
	vec3 ab,bc,cd,abbc,bccd;
	ab = lerp(a,b,t);
	bc = lerp(b,c,t);
	cd = lerp(c,d,t);
	abbc = lerp(ab,bc,t);
	bccd = lerp(bc,cd,t);
	return lerp(abbc,bccd,t);
}

vec3 lerp(vec3 a, vec3 b, float t)
{
    return (a + (b-a)*t);
}

float calcCurveLength() 
{
    float l = 0.0;
    vec3 currPoint = curve[0];

    for(unsigned int i = 1; i <= curve.size(); i++)
    {
        l += length(currPoint - curve[i]);
        currPoint = curve[i];
    }

    return l;
}

void setupVAO() {
  glBindVertexArray(vaoID);

  glEnableVertexAttribArray(0); // match layout # in shader
  glBindBuffer(GL_ARRAY_BUFFER, vertBufferID);
  glVertexAttribPointer(0,        // attribute layout # above
                        3,        // # of components (ie XYZ )
                        GL_FLOAT, // type of components
                        GL_FALSE, // need to be normalized?
                        0,        // stride
                        (void *)0 // array buffer offset
                        );

  glBindVertexArray(line_vaoID);

  glEnableVertexAttribArray(0); // match layout # in shader
  glBindBuffer(GL_ARRAY_BUFFER, line_vertBufferID);
  glVertexAttribPointer(0,        // attribute layout # above
                        3,        // # of components (ie XYZ )
                        GL_FLOAT, // type of components
                        GL_FALSE, // need to be normalized?
                        0,        // stride
                        (void *)0 // array buffer offset
                        );

  glBindVertexArray(0); // reset to default
}

void reloadProjectionMatrix() {
  // Perspective Only

  // field of view angle 60 degrees
  // window aspect ratio
  // near Z plane > 0
  // far Z plane

  P = perspective(WIN_FOV, // FOV
                            static_cast<float>(WIN_WIDTH) /
                                WIN_HEIGHT, // Aspect
                            WIN_NEAR,       // near plane
                            WIN_FAR);       // far plane depth
}

void loadModelViewMatrix() {
  M = mat4(1);
  line_M = mat4(1);
  // view doesn't change, but if it did you would use this
  V = camera.lookatMatrix();
}

void reloadViewMatrix() { V = camera.lookatMatrix(); }

void setupModelViewProjectionTransform() {
  MVP = P * V * M; // transforms vertices from right to left (odd huh?)
}

void reloadMVPUniform() {
  GLint id = glGetUniformLocation(basicProgramID, "MVP");

  glUseProgram(basicProgramID);
  glUniformMatrix4fv(id,        // ID
                     1,         // only 1 matrix
                     GL_TRUE,   // transpose matrix, Mat4f is row major
                     &MVP[0][0] // pointer to data in Mat4f
                     );
}

void reloadColorUniform(float r, float g, float b) {
  GLint id = glGetUniformLocation(basicProgramID, "inputColor");

  glUseProgram(basicProgramID);
  glUniform3f(id, // ID in basic_vs.glsl
              r, g, b);
}

void generateIDs() {
  // shader ID from OpenGL
  string vsSource = loadShaderStringfromFile("./shaders/basic_vs.glsl");
  string fsSource = loadShaderStringfromFile("./shaders/basic_fs.glsl");
  basicProgramID = CreateShaderProgram(vsSource, fsSource);

  // VAO and buffer IDs given from OpenGL
  glGenVertexArrays(1, &vaoID);
  glGenBuffers(1, &vertBufferID);
  glGenVertexArrays(1, &line_vaoID);
  glGenBuffers(1, &line_vertBufferID);
}

void deleteIDs() {
  glDeleteProgram(basicProgramID);

  glDeleteVertexArrays(1, &vaoID);
  glDeleteBuffers(1, &vertBufferID);
  glDeleteVertexArrays(1, &line_vaoID);
  glDeleteBuffers(1, &line_vertBufferID);
}

void init() {
  glEnable(GL_DEPTH_TEST);
  glPointSize(50);

  camera = Camera(vec3(0, 0, 5), vec3(0, 0, -1), vec3(0, 1, 0));

  // SETUP SHADERS, BUFFERS, VAOs

  generateIDs();
  setupVAO();
  loadQuadGeometryToGPU();
  loadLineGeometryToGPU();

  loadModelViewMatrix();
  reloadProjectionMatrix();
  setupModelViewProjectionTransform();
  reloadMVPUniform();
}

int main(int argc, char **argv) {
  GLFWwindow *window;

  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window =
      glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "CPSC 587/687 Tut03", NULL, NULL);
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glfwSetWindowSizeCallback(window, windowSetSizeFunc);
  glfwSetFramebufferSizeCallback(window, windowSetFramebufferSizeFunc);
  glfwSetKeyCallback(window, windowKeyFunc);
  glfwSetCursorPosCallback(window, windowMouseMotionFunc);
  glfwSetMouseButtonCallback(window, windowMouseButtonFunc);

  glfwGetFramebufferSize(window, &WIN_WIDTH, &WIN_HEIGHT);

  // Initialize glad
  if (!gladLoadGL()) {
    std::cerr << "Failed to initialise GLAD" << std::endl;
    return -1;
  }

  std::cout << "GL Version: :" << glGetString(GL_VERSION) << std::endl;
  std::cout << GL_ERROR() << std::endl;

  // Initialize all the geometry, and load it once to the GPU
  init(); // our own initialize stuff func

  float t = 0;
  float dt = 0.01;

  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
         !glfwWindowShouldClose(window)) {

    if (g_play) {
      t += dt;
      animateBead(t);
    }

    displayFunc();
    moveCamera();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // clean up after loop
  deleteIDs();

  return 0;
}

//==================== CALLBACK FUNCTIONS ====================//

void windowSetSizeFunc(GLFWwindow *window, int width, int height) {
  WIN_WIDTH = width;
  WIN_HEIGHT = height;

  reloadProjectionMatrix();
  setupModelViewProjectionTransform();
  reloadMVPUniform();
}

void windowSetFramebufferSizeFunc(GLFWwindow *window, int width, int height) {
  FB_WIDTH = width;
  FB_HEIGHT = height;

  glViewport(0, 0, FB_WIDTH, FB_HEIGHT);
}

void windowMouseButtonFunc(GLFWwindow *window, int button, int action,
                           int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      g_cursorLocked = GL_TRUE;
    } else {
      g_cursorLocked = GL_FALSE;
    }
  }
}

void windowMouseMotionFunc(GLFWwindow *window, double x, double y) {
  if (g_cursorLocked) {
    float deltaX = (x - g_cursorX) * 0.01;
    float deltaY = (y - g_cursorY) * 0.01;
    camera.rotateAroundFocus(deltaX, deltaY);

    reloadViewMatrix();
    setupModelViewProjectionTransform();
    reloadMVPUniform();
  }

  g_cursorX = x;
  g_cursorY = y;
}

void windowKeyFunc(GLFWwindow *window, int key, int scancode, int action,
                   int mods) {
  bool set = action != GLFW_RELEASE && GLFW_REPEAT;
  switch (key) {
  case GLFW_KEY_ESCAPE:
    glfwSetWindowShouldClose(window, GL_TRUE);
    break;
  case GLFW_KEY_W:
    g_moveBackForward = set ? 1 : 0;
    break;
  case GLFW_KEY_S:
    g_moveBackForward = set ? -1 : 0;
    break;
  case GLFW_KEY_A:
    g_moveLeftRight = set ? 1 : 0;
    break;
  case GLFW_KEY_D:
    g_moveLeftRight = set ? -1 : 0;
    break;
  case GLFW_KEY_Q:
    g_moveUpDown = set ? -1 : 0;
    break;
  case GLFW_KEY_E:
    g_moveUpDown = set ? 1 : 0;
    break;
  case GLFW_KEY_UP:
    g_rotateUpDown = set ? -1 : 0;
    break;
  case GLFW_KEY_DOWN:
    g_rotateUpDown = set ? 1 : 0;
    break;
  case GLFW_KEY_LEFT:
    if (mods == GLFW_MOD_SHIFT)
      g_rotateRoll = set ? -1 : 0;
    else
      g_rotateLeftRight = set ? 1 : 0;
    break;
  case GLFW_KEY_RIGHT:
    if (mods == GLFW_MOD_SHIFT)
      g_rotateRoll = set ? 1 : 0;
    else
      g_rotateLeftRight = set ? -1 : 0;
    break;
  case GLFW_KEY_SPACE:
    g_play = set ? !g_play : g_play;
    break;
  case GLFW_KEY_LEFT_BRACKET:
    if (mods == GLFW_MOD_SHIFT) {
      g_rotationSpeed *= 0.5;
    } else {
      g_panningSpeed *= 0.5;
    }
    break;
  case GLFW_KEY_RIGHT_BRACKET:
    if (mods == GLFW_MOD_SHIFT) {
      g_rotationSpeed *= 1.5;
    } else {
      g_panningSpeed *= 1.5;
    }
    break;
  default:
    break;
  }
}

//==================== OPENGL HELPER FUNCTIONS ====================//

void moveCamera() {
  vec3 dir;

  if (g_moveBackForward) {
    dir += vec3(0, 0, g_moveBackForward * g_panningSpeed);
  }
  if (g_moveLeftRight) {
    dir += vec3(g_moveLeftRight * g_panningSpeed, 0, 0);
  }
  if (g_moveUpDown) {
    dir += vec3(0, g_moveUpDown * g_panningSpeed, 0);
  }

  if (g_rotateUpDown) {
    camera.rotateUpDown(g_rotateUpDown * g_rotationSpeed);
  }
  if (g_rotateLeftRight) {
    camera.rotateLeftRight(g_rotateLeftRight * g_rotationSpeed);
  }
  if (g_rotateRoll) {
    camera.rotateRoll(g_rotateRoll * g_rotationSpeed);
  }

  if (g_moveUpDown || g_moveLeftRight || g_moveBackForward ||
      g_rotateLeftRight || g_rotateUpDown || g_rotateRoll) {
    camera.move(dir);
    reloadViewMatrix();
    setupModelViewProjectionTransform();
    reloadMVPUniform();
  }
}

std::string GL_ERROR() {
  GLenum code = glGetError();

  switch (code) {
  case GL_NO_ERROR:
    return "GL_NO_ERROR";
  case GL_INVALID_ENUM:
    return "GL_INVALID_ENUM";
  case GL_INVALID_VALUE:
    return "GL_INVALID_VALUE";
  case GL_INVALID_OPERATION:
    return "GL_INVALID_OPERATION";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "GL_INVALID_FRAMEBUFFER_OPERATION";
  case GL_OUT_OF_MEMORY:
    return "GL_OUT_OF_MEMORY";
  default:
    return "Non Valid Error Code";
  }
}
