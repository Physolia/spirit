
#include <iostream>

#ifndef __gl_h_
#include <glad/glad.h>
#endif
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "ArrowSpinRenderer.hpp"
#include "utilities.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

ArrowSpinRenderer::ArrowSpinRenderer() {
  CHECK_GL_ERROR;
  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);
  glGenBuffers(1, &_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, false, 4*3*2, nullptr);
  glEnableVertexAttribArray(0);
  glVertexAttribDivisor(0, 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, false, 4*3*2, (void *)(4*3));
  glEnableVertexAttribArray(1);
  glVertexAttribDivisor(1, 0);
  
  glGenBuffers(1, &_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
  _numIndices = 0;
  
  glGenBuffers(1, &_instancePositionVbo);
  glBindBuffer(GL_ARRAY_BUFFER, _instancePositionVbo);
  glVertexAttribPointer(2, 3, GL_FLOAT, false, 0, nullptr);
  glEnableVertexAttribArray(2);
  glVertexAttribDivisor(2, 1);
  
  glGenBuffers(1, &_instanceDirectionVbo);
  glBindBuffer(GL_ARRAY_BUFFER, _instanceDirectionVbo);
  glVertexAttribPointer(3, 3, GL_FLOAT, false, 0, nullptr);
  glEnableVertexAttribArray(3);
  glVertexAttribDivisor(3, 1);
  _numInstances = 0;
  
  _updateShaderProgram();
  _updateVertexData();
  CHECK_GL_ERROR;
}

ArrowSpinRenderer::~ArrowSpinRenderer() {
  CHECK_GL_ERROR;
  glDeleteVertexArrays(1, &_vao);
  glDeleteBuffers(1, &_vbo);
  glDeleteBuffers(1, &_ibo);
  glDeleteBuffers(1, &_instancePositionVbo);
  glDeleteBuffers(1, &_instanceDirectionVbo);
  glDeleteProgram(_program);
  CHECK_GL_ERROR;
}

void ArrowSpinRenderer::optionsHaveChanged(const std::vector<int>& changedOptions) {
  CHECK_GL_ERROR;
  bool updateShader = false;
  bool updateVertices = false;
  for (auto it = changedOptions.cbegin(); it != changedOptions.cend(); it++) {
    if (*it == ISpinRenderer::Option::COLORMAP_IMPLEMENTATION) {
      updateShader = true;
    } else if (*it == ArrowSpinRendererOptions::CONE_RADIUS) {
      updateVertices = true;
    } else if (*it == ArrowSpinRendererOptions::CONE_HEIGHT) {
      updateVertices = true;
    } else if (*it == ArrowSpinRendererOptions::CYLINDER_RADIUS) {
      updateVertices = true;
    } else if (*it == ArrowSpinRendererOptions::CYLINDER_HEIGHT) {
      updateVertices = true;
    }
  }
  if (updateShader) {
    _updateShaderProgram();
  }
  if (updateVertices) {
    _updateVertexData();
  }
  CHECK_GL_ERROR;
}

void ArrowSpinRenderer::updateSpins(const std::vector<glm::vec3>& positions,
                                    const std::vector<glm::vec3>& directions) {
  CHECK_GL_ERROR;
  glBindVertexArray(_vao);
  _numInstances = positions.size();
  glBindBuffer(GL_ARRAY_BUFFER, _instancePositionVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * positions.size(), positions.data(), GL_STREAM_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, _instanceDirectionVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * directions.size(), directions.data(), GL_STREAM_DRAW);
  CHECK_GL_ERROR;
}

void ArrowSpinRenderer::draw(float aspectRatio) const {
  CHECK_GL_ERROR;
  if (_numInstances <= 0) {
    return;
  }
  glBindVertexArray(_vao);
  glUseProgram(_program);
  
  glm::vec2 zRange = _options.get<ISpinRenderer::Option::Z_RANGE>();
  if (zRange.x <= -1) {
    zRange.x = -2;
  }
  if (zRange.y >= 1) {
    zRange.y = 2;
  }
  float verticalFieldOfView = _options.get<ISpinRenderer::Option::VERTICAL_FIELD_OF_VIEW>();
  glm::vec3 cameraPosition = _options.get<ISpinRenderer::Option::CAMERA_POSITION>();
  glm::vec3 centerPosition = _options.get<ISpinRenderer::Option::CENTER_POSITION>();
  glm::vec3 upVector = _options.get<ISpinRenderer::Option::UP_VECTOR>();

  glm::mat4 projectionMatrix;
  if (verticalFieldOfView > 0) {
    projectionMatrix = glm::perspective(verticalFieldOfView, aspectRatio, 0.1f, 10000.0f);
  } else {
    float camera_distance = glm::length(cameraPosition-centerPosition);
    float leftRight = camera_distance * aspectRatio;
    float bottomTop = camera_distance;
    projectionMatrix = glm::ortho(-leftRight, leftRight, -bottomTop, bottomTop, -10000.0f, 10000.0f);
  }
  glm::mat4 modelviewMatrix = glm::lookAt(cameraPosition, centerPosition, upVector);
  glm::vec4 lightPosition = modelviewMatrix * glm::vec4(cameraPosition, 1.0);
  
  glUniformMatrix4fv(glGetUniformLocation(_program, "uProjectionMatrix"), 1, false, glm::value_ptr(projectionMatrix));
  glUniformMatrix4fv(glGetUniformLocation(_program, "uModelviewMatrix"), 1, false, glm::value_ptr(modelviewMatrix));
  glUniform3f(glGetUniformLocation(_program, "uLightPosition"), lightPosition[0], lightPosition[1], lightPosition[2]);
  glUniform2f(glGetUniformLocation(_program, "uZRange"), zRange[0], zRange[1]);
  
  glDisable(GL_CULL_FACE);
  glDrawElementsInstanced(GL_TRIANGLES, _numIndices, GL_UNSIGNED_SHORT, nullptr, _numInstances);
  glEnable(GL_CULL_FACE);
  CHECK_GL_ERROR;
}

void ArrowSpinRenderer::_updateShaderProgram() {
  CHECK_GL_ERROR;
  if (_program) {
    glDeleteProgram(_program);
  }
  std::string vertexShaderSource =
#include "arrows.vert.txt"
  ;
  vertexShaderSource += _options.get<ISpinRenderer::Option::COLORMAP_IMPLEMENTATION>();
  std::string fragmentShaderSource =
#include "arrows.frag.txt"
  ;
  GLuint program = createProgram(vertexShaderSource, fragmentShaderSource, {"ivPosition", "ivNormal", "ivInstanceOffset", "ivInstanceDirection"});
  if (program) {
    _program = program;
  }
  CHECK_GL_ERROR;
}

void ArrowSpinRenderer::_updateVertexData() {
  CHECK_GL_ERROR;
  unsigned int levelOfDetail = _options.get<ArrowSpinRendererOptions::LEVEL_OF_DETAIL>();
  float coneHeight = _options.get<ArrowSpinRendererOptions::CONE_HEIGHT>();
  float coneRadius = _options.get<ArrowSpinRendererOptions::CONE_RADIUS>();
  float cylinderHeight = _options.get<ArrowSpinRendererOptions::CYLINDER_HEIGHT>();
  float cylinderRadius = _options.get<ArrowSpinRendererOptions::CYLINDER_RADIUS>();
  
  // Enforce valid range
  if (levelOfDetail < 3) {
    levelOfDetail = 3;
  }
  if (coneHeight < 0) {
    coneHeight = 0;
  }
  if (coneRadius < 0) {
    coneRadius = 0;
  }
  if (cylinderHeight < 0) {
    cylinderHeight = 0;
  }
  if (cylinderRadius < 0) {
    cylinderRadius = 0;
  }
  unsigned int i;
  glm::vec3 baseNormal = {0, 0, -1};
  float zOffset = (cylinderHeight-coneHeight)/2;
  float l = sqrt(coneRadius*coneRadius+coneHeight*coneHeight);
  float f1 = coneRadius/l;
  float f2 = coneHeight/l;
  std::vector<glm::vec3> vertexData;
  vertexData.reserve(levelOfDetail*5*2);
  // The tip has no normal to prevent a discontinuity.
  vertexData.push_back({0, 0, zOffset+coneHeight});
  vertexData.push_back({0, 0, 0});
  for (i = 0; i < levelOfDetail; i++) {
    float alpha = 2*(float)M_PI*i/levelOfDetail;
    vertexData.push_back({coneRadius*cos(alpha), coneRadius*sin(alpha), zOffset});
    vertexData.push_back({f2*cos(alpha), f2*sin(alpha), f1});
  }
  for (i = 0; i < levelOfDetail; i++) {
    float alpha = 2* (float)M_PI*i/levelOfDetail;
    vertexData.push_back({coneRadius*cos(alpha), coneRadius*sin(alpha), zOffset});
    vertexData.push_back(baseNormal);
  }
  for (i = 0; i < levelOfDetail; i++) {
    float alpha = 2* (float)M_PI*i/levelOfDetail;
    vertexData.push_back({cylinderRadius*cos(alpha), cylinderRadius*sin(alpha), zOffset-cylinderHeight});
    vertexData.push_back(baseNormal);
  }
  for (i = 0; i < levelOfDetail; i++) {
    float alpha = 2* (float)M_PI*i/levelOfDetail;
    vertexData.push_back({cylinderRadius*cos(alpha), cylinderRadius*sin(alpha), zOffset-cylinderHeight});
    vertexData.push_back({cos(alpha), sin(alpha), 0});
  }
  for (i = 0; i < levelOfDetail; i++) {
    float alpha = 2* (float)M_PI*i/levelOfDetail;
    vertexData.push_back({cylinderRadius*cos(alpha), cylinderRadius*sin(alpha), zOffset});
    vertexData.push_back({cos(alpha), sin(alpha), 0});
  }
  std::vector<GLushort> indices;
  indices.reserve(levelOfDetail*15);
  for (i = 0; i < levelOfDetail; i++) {
    indices.push_back(1+i);
    indices.push_back(1+(i+1)%levelOfDetail);
    indices.push_back(0);
  }
  for (i = 0; i < levelOfDetail; i++) {
    indices.push_back(levelOfDetail+1);
    indices.push_back(levelOfDetail+1+(i+1)%levelOfDetail);
    indices.push_back(levelOfDetail+1+i);
  }
  for (i = 0; i < levelOfDetail; i++) {
    indices.push_back(levelOfDetail*2+1);
    indices.push_back(levelOfDetail*2+1+(i+1)%levelOfDetail);
    indices.push_back(levelOfDetail*2+1+i);
  }
  for (i = 0; i < levelOfDetail; i++) {
    indices.push_back(levelOfDetail*3+1+i);
    indices.push_back(levelOfDetail*3+1+(i+1)%levelOfDetail);
    indices.push_back(levelOfDetail*4+1+i);
    indices.push_back(levelOfDetail*4+1+i);
    indices.push_back(levelOfDetail*3+1+(i+1)%levelOfDetail);
    indices.push_back(levelOfDetail*4+1+(i+1)%levelOfDetail);
  }
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertexData.size(), vertexData.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices.size(), indices.data(), GL_STATIC_DRAW);
  _numIndices = indices.size();
  CHECK_GL_ERROR;
}