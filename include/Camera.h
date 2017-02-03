/**
 * Author:	Andrew Robert Owens
 * Email:	arowens [at] ucalgary.ca
 * Date:	January, 2017
 * Course:	CPSC 587/687 Fundamental of Computer Animation
 * Organization: University of Calgary
 *
 * Copyright (c) 2017 - Please give credit to the author.
 *
 * File:	Camera.h
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "OpenGLMatrixTools.h"

using namespace glm;

class Camera {
public:
  explicit Camera(vec3 const &pos = vec3(), vec3 const &forward = vec3(),
                  vec3 const &up = vec3());

  void rotateAroundFocus(float deltaX, float deltaY);
  void rotateLeftRight(float t);
  void rotateUpDown(float t);
  void rotateRoll(float t);

  mat4 lookatMatrix() const;

  void move(vec3 const &offset);

  float focusDistance() const;
  vec3 const &position() const;
  vec3 const &forward() const;
  vec3 const &up() const;
  vec3 right() const;

private:
  float m_focusDist;
  vec3 m_pos;
  vec3 m_up;
  vec3 m_forward;
  vec3 rotateAround(vec3 const &vec, vec3 const &axis, float radians);
};
#endif /* defined(____Camera__) */
