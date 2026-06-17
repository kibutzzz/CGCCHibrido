#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "SceneObject.h"

struct PointLight {
  glm::vec3 position;
  glm::vec3 color;
  float intensity;
};

struct Scene {
  std::vector<SceneObject> objects;
  std::vector<PointLight> lights;
  int selectedIndex = 0;

  void selectNext() {
    if (objects.empty()) return;
    objects[selectedIndex].selected = false;
    selectedIndex = (selectedIndex + 1) % (int)objects.size();
    objects[selectedIndex].selected = true;
  }

  void selectPrev() {
    if (objects.empty()) return;
    objects[selectedIndex].selected = false;
    selectedIndex =
        ((selectedIndex - 1) + (int)objects.size()) % (int)objects.size();
    objects[selectedIndex].selected = true;
  }

  SceneObject& selected() { return objects[selectedIndex]; }
};
