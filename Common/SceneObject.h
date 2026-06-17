#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <string>

#include "AnimationPath.h"
#include "Mesh.h"

struct SceneObject {
  std::string name;
  GLuint vertexArrayObject = 0;
  int vertexCount = 0;
  Material material;
  GLuint textureID = 0;
  glm::vec3 position = glm::vec3(0.0f);
  glm::vec3 rotation = glm::vec3(0.0f);
  float scale = 1.0f;
  bool selected = false;
  std::unique_ptr<AnimationPath> animation;

  glm::mat4 modelMatrix() const {
    glm::mat4 m(1.0f);
    m = glm::translate(m, position);
    m = glm::rotate(m, glm::radians(rotation.x), glm::vec3(1, 0, 0));
    m = glm::rotate(m, glm::radians(rotation.y), glm::vec3(0, 1, 0));
    m = glm::rotate(m, glm::radians(rotation.z), glm::vec3(0, 0, 1));
    m = glm::scale(m, glm::vec3(scale));
    return m;
  }
};
