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
  InputHandler::onMouseMove(xpos, ypos, *globalCamera);
}
void scrollCallback(GLFWwindow*, double, double yoffset) {
  InputHandler::onScroll(yoffset, *globalCamera);
}

static GLuint buildCrosshairVAO() {
  // Two lines: horizontal and vertical, each 2 vertices in NDC
  float crosshair[] = {
    -0.02f,  0.0f, 0.0f,
     0.02f,  0.0f, 0.0f,
     0.0f, -0.03f, 0.0f,
     0.0f,  0.03f, 0.0f,
  };
  GLuint vao, vbo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(crosshair), crosshair, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);
  return vao;
}

int main() {
  Window window(WINDOW_WIDTH, WINDOW_HEIGHT, "Grau B");

  glfwSetKeyCallback(window.window, keyCallback);
  glfwSetCursorPosCallback(window.window, mouseCallback);
  glfwSetScrollCallback(window.window, scrollCallback);
  glfwSetInputMode(window.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  std::cout << "Renderizador: " << glGetString(GL_RENDERER) << "\n";
  std::cout << "OpenGL:       " << glGetString(GL_VERSION) << "\n";
  InputHandler::printControls();

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

  GLuint crosshairVAO = buildCrosshairVAO();
  ShaderProgram crosshairShader(SHADERS_DIR "crosshair.vert",
                                SHADERS_DIR "crosshair.frag");

  Scene scene;
  Camera camera;
  loadScene(SCENE_FILE, ASSETS_DIR, scene, camera);
  globalScene = &scene;
  globalCamera = &camera;

  std::cout << "Cena carregada: " << scene.objects.size() << " objeto(s)\n";
  for (auto& obj : scene.objects)
    std::cout << "  " << obj.name << "\n";
  std::cout << "Selecionado: " << scene.selected().name << "\n\n";

  float previousTime = (float)glfwGetTime();

  while (!window.shouldClose()) {
    float currentTime = (float)glfwGetTime();
    float deltaTime = currentTime - previousTime;
    previousTime = currentTime;

    window.pollEvents();
    InputHandler::processHeldKeys(window.window, deltaTime, scene, camera);

    for (auto& obj : scene.objects)
      if (obj.hasAnimation) {
        obj.animation.update(deltaTime);
        obj.position = obj.animation.currentPosition();
      }

    shader.use();
    shader.setMat4("view", camera.viewMatrix());
    shader.setMat4("projection",
                   camera.projectionMatrix((float)WINDOW_WIDTH / WINDOW_HEIGHT));
    shader.setCameraPos(camera.pos);

    shader.setLights(scene.lights);

    glClearColor(0.12f, 0.12f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bool inTransformOrAnim = (InputHandler::mode == AppMode::Transform ||
                              InputHandler::mode == AppMode::Animation);

    for (auto& obj : scene.objects) {
      shader.setMat4("model", obj.modelMatrix());
      shader.setMaterial(obj.material);

      float brightness = (inTransformOrAnim && obj.selected) ? 1.2f : 1.0f;
      shader.setSelectionBrightness(brightness);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, obj.textureID ? obj.textureID : whiteTexture);
      glBindVertexArray(obj.vertexArrayObject);
      glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount);
      glBindVertexArray(0);
    }

    if (InputHandler::showCrosshair()) {
      glDisable(GL_DEPTH_TEST);
      crosshairShader.use();
      glBindVertexArray(crosshairVAO);
      glLineWidth(2.0f);
      glDrawArrays(GL_LINES, 0, 4);
      glBindVertexArray(0);
      glEnable(GL_DEPTH_TEST);
    }

    window.swapBuffers();
  }

  return 0;
}
