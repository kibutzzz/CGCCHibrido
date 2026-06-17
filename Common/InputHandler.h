#pragma once

#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>

#include "AnimationPath.h"
#include "Scene.h"

class Camera;

class InputHandler {
 public:
  static void onKey(GLFWwindow* window, int key, int scancode, int action,
                    int mods, Scene& scene, Camera& camera) {
    if (action != GLFW_PRESS) return;

    if (key == GLFW_KEY_ESCAPE) onEscape(window);
    if (key == GLFW_KEY_TAB) onSelectNext(scene);
    if (key == GLFW_KEY_P) onAddWaypoint(scene, camera);
  }

  static void onMouseMove(double xpos, double ypos, Camera& camera) {
    camera.processMouse(xpos, ypos);
  }

  static void onScroll(double yoffset, Camera& camera) {
    camera.processScroll(yoffset);
  }

 private:
  static void onEscape(GLFWwindow* window) {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }

  static void onSelectNext(Scene& scene) {
    scene.selectNext();
    SceneObject& obj = scene.selected();
    int count = obj.animation ? obj.animation->waypointCount() : 0;
    std::cout << "Selected: " << obj.name << "  waypoints: " << count << "\n";
  }

  static void onAddWaypoint(Scene& scene, Camera& camera) {
    SceneObject& obj = scene.selected();
    if (!obj.animation) obj.animation = std::make_unique<AnimationPath>();
    obj.animation->addWaypoint(camera.pos);
    std::cout << "Waypoint added to " << obj.name
              << "  total: " << obj.animation->waypointCount() << "\n";
  }
};
