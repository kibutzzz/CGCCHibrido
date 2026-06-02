#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <iostream>
#include <string>

class Window {
 public:
  GLFWwindow* window;
  int width;
  int height;

  Window(int width, int height, const std::string& title)
      : width(width), height(height) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      std::cerr << "Failed to initialize GLAD\n";
    }

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
  }

  ~Window() { glfwTerminate(); }

  bool shouldClose() const { return glfwWindowShouldClose(window); }
  void swapBuffers() const { glfwSwapBuffers(window); }
  void pollEvents() const { glfwPollEvents(); }

  void setKeyCallback(GLFWkeyfun fn) const { glfwSetKeyCallback(window, fn); }
};
