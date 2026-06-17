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
    setSelected((selectedIndex + 1) % (int)objects.size());
  }

  void selectPrev() {
    if (objects.empty()) return;
    setSelected(((selectedIndex - 1) + (int)objects.size()) %
                (int)objects.size());
  }

  SceneObject& selected() { return objects[selectedIndex]; }

 private:
  void setSelected(int index) {
    objects[selectedIndex].selected = false;
    selectedIndex = index;
    objects[selectedIndex].selected = true;
  }
};
