#pragma once

#include <GLFW/glfw3.h>

#include <algorithm>
#include <iostream>

#include "AnimationPath.h"
#include "Scene.h"

class Camera;

enum class AppMode { Navigate = 0, Transform = 1, Animation = 2, Light = 3 };

class InputHandler {
 public:
  static AppMode mode;
  static int selectedLight;

  static void onKey(GLFWwindow* window, int key, int scancode, int action,
                    int mods, Scene& scene, Camera& camera) {
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
      return;
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      setMode(AppMode::Navigate, window);
      return;
    }
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
      setMode(AppMode::Transform, window);
      return;
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
      setMode(AppMode::Animation, window);
      return;
    }

    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
      setMode(AppMode::Light, window);
      return;
    }

    if (mode == AppMode::Transform) onTransformKey(key, action, scene);
    if (mode == AppMode::Animation) onAnimationKey(key, action, scene, camera);
    if (mode == AppMode::Light) onLightKey(key, action, scene);
  }

  static void processHeldKeys(GLFWwindow* window, float dt, Scene& scene,
                               Camera& camera) {
    if (mode == AppMode::Navigate) {
      camera.processKeyboard(window, dt);
    } else if (mode == AppMode::Transform && !scene.objects.empty()) {
      processTransformHeld(window, dt, scene.selected());
    } else if (mode == AppMode::Light && !scene.lights.empty()) {
      processLightHeld(window, dt, scene.lights[selectedLight]);
    }
  }

  static void onMouseMove(double xpos, double ypos, Camera& camera) {
    camera.processMouse(xpos, ypos);
  }

  static void onScroll(double yoffset, Camera& camera) {
    camera.processScroll(yoffset);
  }

  static bool showCrosshair() {
    return mode == AppMode::Animation || mode == AppMode::Light;
  }

 private:
  static void setMode(AppMode m, GLFWwindow* window) {
    mode = m;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (m == AppMode::Navigate) {
      std::cout << "[mode] navigate\n";
    } else if (m == AppMode::Transform) {
      std::cout << "[mode] transform  (WASD=XZ  I/J=Y  U/O=yaw  K/;=pitch  N/M=roll  +/-=scale)\n";
    } else if (m == AppMode::Animation) {
      std::cout << "[mode] animation  (Space=add waypoint 10u ahead)\n";
    } else if (m == AppMode::Light) {
      std::cout << "[mode] light  (Tab=select  +/-=intensity  WASD=XZ  I/J=Y)\n";
    }
  }

  static void onTransformKey(int key, int action, Scene& scene) {
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_TAB) {
      scene.selectNext();
      std::cout << "Selected: " << scene.selected().name << "\n";
    }
    const float step = 0.1f;
    if (key == GLFW_KEY_EQUAL) {
      scene.selected().scale *= (1.0f + step);
      std::cout << scene.selected().name << " scale=" << scene.selected().scale << "\n";
    }
    if (key == GLFW_KEY_MINUS) {
      scene.selected().scale *= (1.0f - step);
      std::cout << scene.selected().name << " scale=" << scene.selected().scale << "\n";
    }
  }

  static void processTransformHeld(GLFWwindow* window, float dt,
                                    SceneObject& obj) {
    const float moveSpeed = 3.0f;
    const float rotSpeed = 60.0f;
    const float v = moveSpeed * dt;
    const float r = rotSpeed * dt;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) obj.position.z -= v;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) obj.position.z += v;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) obj.position.x -= v;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) obj.position.x += v;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) obj.position.y += v;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) obj.position.y -= v;

    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) obj.rotation.y += r;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) obj.rotation.y -= r;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) obj.rotation.x += r;
    if (glfwGetKey(window, GLFW_KEY_SEMICOLON) == GLFW_PRESS) obj.rotation.x -= r;
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) obj.rotation.z += r;
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) obj.rotation.z -= r;
  }

  static void onAnimationKey(int key, int action, Scene& scene,
                              Camera& camera) {
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_SPACE) {
      glm::vec3 waypoint = camera.pos + glm::normalize(camera.front) * 10.0f;
      scene.selected().hasAnimation = true;
      scene.selected().animation.addWaypoint(waypoint);
      std::cout << "Waypoint added at ("
                << waypoint.x << ", " << waypoint.y << ", " << waypoint.z
                << ")  total: " << scene.selected().animation.waypointCount() << "\n";
    }
    if (key == GLFW_KEY_TAB) {
      scene.selectNext();
      std::cout << "Selected: " << scene.selected().name << "\n";
    }
  }
  static void onLightKey(int key, int action, Scene& scene) {
    if (action != GLFW_PRESS) return;
    if (scene.lights.empty()) return;
    if (key == GLFW_KEY_TAB) {
      selectedLight = (selectedLight + 1) % (int)scene.lights.size();
      std::cout << "Light " << selectedLight << " selected  intensity="
                << scene.lights[selectedLight].intensity << "\n";
    }
    const float step = 0.1f;
    if (key == GLFW_KEY_EQUAL) {
      scene.lights[selectedLight].intensity += step;
      std::cout << "Light " << selectedLight
                << " intensity=" << scene.lights[selectedLight].intensity << "\n";
    }
    if (key == GLFW_KEY_MINUS) {
      scene.lights[selectedLight].intensity =
          std::max(0.0f, scene.lights[selectedLight].intensity - step);
      std::cout << "Light " << selectedLight
                << " intensity=" << scene.lights[selectedLight].intensity << "\n";
    }
  }

  static void processLightHeld(GLFWwindow* window, float dt, PointLight& light) {
    const float speed = 3.0f;
    const float v = speed * dt;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) light.position.z -= v;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) light.position.z += v;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) light.position.x -= v;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) light.position.x += v;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) light.position.y += v;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) light.position.y -= v;
  }
};

inline AppMode InputHandler::mode = AppMode::Navigate;
inline int InputHandler::selectedLight = 0;
