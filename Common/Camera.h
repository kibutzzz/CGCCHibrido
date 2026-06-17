#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
 public:
  glm::vec3 pos;
  glm::vec3 front;
  glm::vec3 up;
  float yaw;
  float pitch;
  float fov;
  float near;
  float far;
  float speed;
  float sensitivity;

  Camera(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 3.0f), float yaw = -90.0f,
         float pitch = 0.0f)
      : pos(pos),
        front(glm::vec3(0.0f, 0.0f, -1.0f)),
        up(glm::vec3(0.0f, 1.0f, 0.0f)),
        yaw(yaw),
        pitch(pitch),
        fov(45.0f),
        near(0.1f),
        far(100.0f),
        speed(2.5f),
        sensitivity(0.05f),
        firstMouse(true),
        lastX(0.0f),
        lastY(0.0f) {
    updateVectors();
  }

  glm::mat4 viewMatrix() const {
    return glm::lookAt(pos, pos + front, up);
  }

  glm::mat4 projectionMatrix(float aspect) const {
    return glm::perspective(glm::radians(fov), aspect, near, far);
  }

  void processKeyboard(GLFWwindow* window, float deltaTime) {
    float v = speed * deltaTime;
    glm::vec3 right = glm::normalize(glm::cross(front, up));
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) pos += v * front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) pos -= v * front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) pos -= v * right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) pos += v * right;
  }

  void processMouse(double xpos, double ypos) {
    if (firstMouse) {
      lastX = (float)xpos;
      lastY = (float)ypos;
      firstMouse = false;
    }

    float xoffset = ((float)xpos - lastX) * sensitivity;
    float yoffset = (lastY - (float)ypos) * sensitivity;
    lastX = (float)xpos;
    lastY = (float)ypos;

    yaw += xoffset;
    pitch += yoffset;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    updateVectors();
  }

  void processScroll(double yoffset) {
    fov -= (float)yoffset;
    if (fov < 1.0f) fov = 1.0f;
    if (fov > 45.0f) fov = 45.0f;
  }

 private:
  bool firstMouse;
  float lastX;
  float lastY;

  void updateVectors() {
    glm::vec3 f;
    f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    f.y = sin(glm::radians(pitch));
    f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(f);
    glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
    up = glm::normalize(glm::cross(right, front));
  }
};
