/**
 * Author:	Andrew Robert Owens
 * Email:	arowens [at] ucalgary.ca
 * Date:	January, 2017
 * Course:	CPSC 587/687 Fundamental of Computer Animation
 * Organization: University of Calgary
 *
 * Copyright (c) 2017 - Please give credit to the author.
 *
 * File:	Camera.cpp
 */

#include "Camera.h"
Camera::Camera(vec3 const &pos, vec3 const &forward, vec3 const &up)
    : m_focusDist(length(forward)), m_pos(pos), m_up(up), m_forward(forward) {}

void Camera::rotateAroundFocus(float deltaX, float deltaY) {
  vec3 focus = m_pos + m_forward * m_focusDist;
  vec3 diff = m_pos - focus;

  rotateAround(diff, m_up, -deltaX);
  m_forward = -(normalize(diff));

  vec3 bi = cross(m_up, m_forward);
  normalize(bi);

  rotateAround(diff, bi, deltaY);
  rotateAround(m_up, bi, deltaY);
  m_forward = -(normalize(diff));

  m_pos = focus + diff;
}

void Camera::rotateUpDown(float t) {
  vec3 bi = cross(m_up, m_forward);
  rotateAround(m_forward, bi, t);

  normalize(m_forward);
  m_up = cross(m_forward, bi);
  normalize(m_up);
}

void Camera::rotateLeftRight(float t) {
  rotateAround(m_forward, m_up, t);

  normalize(m_forward);
}

void Camera::rotateRoll(float t) {
  rotateAround(m_up, m_forward, t);

  normalize(m_up);
}

mat4 Camera::lookatMatrix() const {

  return lookAt(m_pos, vec3(0.0,0.0,0.0), m_up);
}

void Camera::move(vec3 const &offset) {
  vec3 bi = cross(m_up, m_forward);
  normalize(bi);

  m_pos += bi * offset.x + m_up * offset.y + m_forward * offset.z;
}

vec3 Camera::rotateAround(vec3 const &vec, vec3 const &axis, float radians) {
  radians *= 0.5;
  const float sinAngle = std::sin(radians);
  const float cosAngle = std::cos(radians);

  vec3 n = vec3(axis);
  normalize(n);

  vec4 qAxis = vec4(cosAngle, n * sinAngle);

  vec3 rotated = vec3(qAxis * vec4(vec,0));
  return rotated;
}

float Camera::focusDistance() const { return m_focusDist; }
vec3 const &Camera::position() const { return m_pos; }
vec3 const &Camera::forward() const { return m_forward; }
vec3 const &Camera::up() const { return m_up; }
vec3 Camera::right() const { return normalize(cross(m_up, m_forward)); }
