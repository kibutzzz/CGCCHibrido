#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>

#include "Camera.h"
#include "Mesh.h"
#include "ShaderProgram.h"
#include "Window.h"

const int WIDTH = 1200;
const int HEIGHT = 800;
const std::string OBJ_FILE = "PUSHILIN_house.obj";

Camera camera(glm::vec3(0.0f, 1.0f, 5.0f));

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
  camera.processMouse(xpos, ypos);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
  camera.processScroll(yoffset);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action,
                 int mode) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}

int main() {
  Window win(WIDTH, HEIGHT, "Tarefa M5 - Camara em Primeira Pessoa");
  glfwSetKeyCallback(win.window, keyCallback);
  glfwSetCursorPosCallback(win.window, mouseCallback);
  glfwSetScrollCallback(win.window, scrollCallback);
  glfwSetInputMode(win.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
  std::cout << "OpenGL:   " << glGetString(GL_VERSION) << "\n";
  std::cout << "\n=== Controles ===\n"
            << "W/A/S/D  : mover camera\n"
            << "Mouse    : olhar ao redor\n"
            << "Scroll   : zoom (FOV)\n"
            << "Esc      : sair\n"
            << "=================\n\n";

  glEnable(GL_DEPTH_TEST);

  ShaderProgram shader(SHADERS_DIR "vertex.vert", SHADERS_DIR "fragment.frag");
  shader.use();

  const std::string assetsDir = ASSETS_DIR;
  std::string mtlFilename;
  int nVertices = 0;
  GLuint VAO = loadSimpleOBJ(assetsDir + OBJ_FILE, nVertices, mtlFilename);
  Material mat = loadMTL(assetsDir + mtlFilename);
  GLuint texID = loadTexture(assetsDir + mat.texName);

  shader.setInt("tex_buffer", 0);
  shader.setVec3("ka", mat.ka);
  shader.setVec3("kd", mat.kd);
  shader.setVec3("ks", mat.ks);
  shader.setFloat("ns", mat.ns);

  glm::vec3 lightPos(5.0f, 8.0f, 5.0f);
  glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
  shader.setVec3("lightPos", lightPos);
  shader.setVec3("lightColor", lightColor);

  glm::mat4 model(1.0f);
  shader.setMat4("model", model);

  shader.setMat4("projection",
                 camera.projectionMatrix((float)WIDTH / (float)HEIGHT));

  float lastTime = (float)glfwGetTime();

  while (!win.shouldClose()) {
    float currentTime = (float)glfwGetTime();
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    win.pollEvents();
    camera.processKeyboard(win.window, deltaTime);

    shader.setMat4("view", camera.viewMatrix());
    shader.setVec3("camPos", camera.pos);
    shader.setMat4("projection",
                   camera.projectionMatrix((float)WIDTH / (float)HEIGHT));

    glClearColor(0.15f, 0.15f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, nVertices);
    glBindVertexArray(0);

    win.swapBuffers();
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteTextures(1, &texID);
  return 0;
}
