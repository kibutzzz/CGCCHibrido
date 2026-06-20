#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "AnimationPath.h"
#include "Camera.h"
#include "InputHandler.h"
#include "Mesh.h"
#include "PhongShader.h"
#include "Scene.h"
#include "SceneLoader.h"
#include "SceneObject.h"
#include "Window.h"

static const int WINDOW_WIDTH = 1200;
static const int WINDOW_HEIGHT = 800;

static Camera* globalCamera = nullptr;
static Scene* globalScene = nullptr;

void keyCallback(GLFWwindow* window, int key, int scancode, int action,
                 int mods) {
  InputHandler::onKey(window, key, scancode, action, mods, *globalScene, *globalCamera);
}
void mouseCallback(GLFWwindow*, double xpos, double ypos) {
  globalCamera->processMouse(xpos, ypos);
}
void scrollCallback(GLFWwindow*, double, double yoffset) {
  globalCamera->processScroll(yoffset);
}

int main() {
  Window window(WINDOW_WIDTH, WINDOW_HEIGHT, "Grau B");

  glfwSetKeyCallback(window.window, keyCallback);
  glfwSetCursorPosCallback(window.window, mouseCallback);
  glfwSetScrollCallback(window.window, scrollCallback);
  glfwSetInputMode(window.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
  std::cout << "OpenGL:   " << glGetString(GL_VERSION) << "\n";
  std::cout << "\n=== Controles ===\n"
            << "W/A/S/D  : mover camera\n"
            << "Mouse    : olhar ao redor\n"
            << "Scroll   : zoom\n"
            << "Tab      : selecionar proximo objeto\n"
            << "P        : adicionar waypoint na posicao da camera\n"
            << "Esc      : sair\n"
            << "=================\n\n";

  glEnable(GL_DEPTH_TEST);

  GLuint whiteTexture;
  glGenTextures(1, &whiteTexture);
  glBindTexture(GL_TEXTURE_2D, whiteTexture);
  unsigned char white[3] = {255, 255, 255};
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, white);
  glBindTexture(GL_TEXTURE_2D, 0);

  PhongShader shader(SHADERS_DIR "vertex.vert", SHADERS_DIR "fragment.frag");
  shader.use();
  shader.setInt("textureBuffer", 0);

  Scene scene;
  Camera camera;
  loadScene(SCENE_FILE, ASSETS_DIR, scene, camera);
  globalScene = &scene;
  globalCamera = &camera;

  std::cout << "Scene loaded: " << scene.objects.size() << " object(s)\n";
  for (auto& obj : scene.objects)
    std::cout << "  " << obj.name << "\n";
  std::cout << "Selected: " << scene.selected().name << "\n\n";

  float previousTime = (float)glfwGetTime();

  while (!window.shouldClose()) {
    float currentTime = (float)glfwGetTime();
    float deltaTime = currentTime - previousTime;
    previousTime = currentTime;

    window.pollEvents();
    camera.processKeyboard(window.window, deltaTime);

    for (auto& obj : scene.objects)
      if (obj.hasAnimation) {
        obj.animation.update(deltaTime);
        obj.position = obj.animation.currentPosition();
      }

    shader.setMat4("view", camera.viewMatrix());
    shader.setMat4("projection",
                   camera.projectionMatrix((float)WINDOW_WIDTH / WINDOW_HEIGHT));
    shader.setCameraPos(camera.pos);

    if (!scene.lights.empty())
      shader.setLight(scene.lights[0]);

    glClearColor(0.12f, 0.12f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto& obj : scene.objects) {
      shader.setMat4("model", obj.modelMatrix());
      shader.setMaterial(obj.material);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, obj.textureID ? obj.textureID : whiteTexture);
      glBindVertexArray(obj.vertexArrayObject);
      glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount);
      glBindVertexArray(0);
    }

    window.swapBuffers();
  }

  return 0;
}
