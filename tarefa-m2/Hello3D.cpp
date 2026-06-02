#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

#include "ShaderProgram.h"
#include "Window.h"

const GLuint WIDTH = 800, HEIGHT = 800;

struct Cube {
  glm::vec3 position;
  float scale;
  bool rotateX, rotateY, rotateZ;
  float angleX, angleY, angleZ;
};

std::vector<Cube> cubes;
int selectedCube = 0;

const float TRANSLATE_STEP = 0.1f;
const float SCALE_STEP = 0.1f;
const float SCALE_MIN = 0.1f;

void key_callback(GLFWwindow* window, int key, int scancode, int action,
                  int mode);
GLuint setupGeometry();

void printControls() {
  std::cout << "\n=== Controles ===\n"
            << "Tab      : Alternar cubo selecionado\n"
            << "X / Y / Z: Ativar/desativar rotacao no eixo correspondente\n"
            << "A / D    : Mover no eixo X (esquerda/direita)\n"
            << "W / S    : Mover no eixo Z (frente/atras)\n"
            << "I / J    : Mover no eixo Y (cima/baixo)\n"
            << "] / [    : Aumentar / diminuir escala\n"
            << "Esc      : Sair\n"
            << "=================\n\n";
}

glm::mat4 buildModelMatrix(const Cube& cube, bool isSelected) {
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, cube.position);
  model = glm::rotate(model, cube.angleX, glm::vec3(1.0f, 0.0f, 0.0f));
  model = glm::rotate(model, cube.angleY, glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::rotate(model, cube.angleZ, glm::vec3(0.0f, 0.0f, 1.0f));
  float visualScale = isSelected ? cube.scale : cube.scale * 0.85f;
  model = glm::scale(model, glm::vec3(visualScale));
  return model;
}

GLuint setupGeometry() {
  const float RED[] = {1.0f, 0.2f, 0.2f};
  const float GREEN[] = {0.2f, 1.0f, 0.2f};
  const float BLUE[] = {0.2f, 0.4f, 1.0f};
  const float YELLOW[] = {1.0f, 1.0f, 0.1f};
  const float CYAN[] = {0.1f, 1.0f, 1.0f};
  const float MAGENTA[] = {1.0f, 0.2f, 1.0f};

#define V(x, y, z, col) x, y, z, col[0], col[1], col[2]

  GLfloat vertices[] = {
      V(-0.5f, -0.5f, 0.5f, RED),      V(0.5f, -0.5f, 0.5f, RED),
      V(0.5f, 0.5f, 0.5f, RED),        V(0.5f, 0.5f, 0.5f, RED),
      V(-0.5f, 0.5f, 0.5f, RED),       V(-0.5f, -0.5f, 0.5f, RED),

      V(0.5f, -0.5f, -0.5f, GREEN),    V(-0.5f, -0.5f, -0.5f, GREEN),
      V(-0.5f, 0.5f, -0.5f, GREEN),    V(-0.5f, 0.5f, -0.5f, GREEN),
      V(0.5f, 0.5f, -0.5f, GREEN),     V(0.5f, -0.5f, -0.5f, GREEN),

      V(-0.5f, -0.5f, -0.5f, BLUE),    V(-0.5f, -0.5f, 0.5f, BLUE),
      V(-0.5f, 0.5f, 0.5f, BLUE),      V(-0.5f, 0.5f, 0.5f, BLUE),
      V(-0.5f, 0.5f, -0.5f, BLUE),     V(-0.5f, -0.5f, -0.5f, BLUE),

      V(0.5f, -0.5f, 0.5f, YELLOW),    V(0.5f, -0.5f, -0.5f, YELLOW),
      V(0.5f, 0.5f, -0.5f, YELLOW),    V(0.5f, 0.5f, -0.5f, YELLOW),
      V(0.5f, 0.5f, 0.5f, YELLOW),     V(0.5f, -0.5f, 0.5f, YELLOW),

      V(-0.5f, 0.5f, 0.5f, CYAN),      V(0.5f, 0.5f, 0.5f, CYAN),
      V(0.5f, 0.5f, -0.5f, CYAN),      V(0.5f, 0.5f, -0.5f, CYAN),
      V(-0.5f, 0.5f, -0.5f, CYAN),     V(-0.5f, 0.5f, 0.5f, CYAN),

      V(-0.5f, -0.5f, -0.5f, MAGENTA), V(0.5f, -0.5f, -0.5f, MAGENTA),
      V(0.5f, -0.5f, 0.5f, MAGENTA),   V(0.5f, -0.5f, 0.5f, MAGENTA),
      V(-0.5f, -0.5f, 0.5f, MAGENTA),  V(-0.5f, -0.5f, -0.5f, MAGENTA),
  };

#undef V

  GLuint VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
                        (GLvoid*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
                        (GLvoid*)(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  return VAO;
}

int main() {
  Window win(WIDTH, HEIGHT, "Tarefa M2 - Cubos");
  win.setKeyCallback(key_callback);

  std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
  std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
  printControls();

  glEnable(GL_DEPTH_TEST);

  ShaderProgram shader(SHADERS_DIR "vertex.vert", SHADERS_DIR "fragment.frag");
  shader.use();

  GLuint VAO = setupGeometry();

  cubes.push_back({glm::vec3(-0.5f, 0.0f, 0.0f), 1.0f, false, false, false,
                   0.0f, 0.0f, 0.0f});
  cubes.push_back({glm::vec3(0.5f, 0.0f, 0.0f), 1.0f, false, false, false, 0.0f,
                   0.0f, 0.0f});
  cubes.push_back({glm::vec3(0.0f, -0.5f, 0.0f), 1.0f, false, false, false,
                   0.0f, 0.0f, 0.0f});

  const float ROTATION_SPEED = 1.0f;
  float lastTime = (float)glfwGetTime();

  while (!win.shouldClose()) {
    float currentTime = (float)glfwGetTime();
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    win.pollEvents();

    for (Cube& cube : cubes) {
      if (cube.rotateX) cube.angleX += ROTATION_SPEED * deltaTime;
      if (cube.rotateY) cube.angleY += ROTATION_SPEED * deltaTime;
      if (cube.rotateZ) cube.angleZ += ROTATION_SPEED * deltaTime;
    }

    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(VAO);
    for (int i = 0; i < (int)cubes.size(); i++) {
      shader.setMat4("model", buildModelMatrix(cubes[i], i == selectedCube));
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);

    win.swapBuffers();
  }

  glDeleteVertexArrays(1, &VAO);
  return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action,
                  int mode) {
  if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

  Cube& cube = cubes[selectedCube];

  switch (key) {
    case GLFW_KEY_ESCAPE:
      glfwSetWindowShouldClose(window, GL_TRUE);
      break;
    case GLFW_KEY_TAB:
      selectedCube = (selectedCube + 1) % (int)cubes.size();
      break;
    case GLFW_KEY_X:
      cube.rotateX = !cube.rotateX;
      break;
    case GLFW_KEY_Y:
      cube.rotateY = !cube.rotateY;
      break;
    case GLFW_KEY_Z:
      cube.rotateZ = !cube.rotateZ;
      break;
    case GLFW_KEY_D:
      cube.position.x += TRANSLATE_STEP;
      break;
    case GLFW_KEY_A:
      cube.position.x -= TRANSLATE_STEP;
      break;
    case GLFW_KEY_I:
      cube.position.y += TRANSLATE_STEP;
      break;
    case GLFW_KEY_J:
      cube.position.y -= TRANSLATE_STEP;
      break;
    case GLFW_KEY_W:
      cube.position.z -= TRANSLATE_STEP;
      break;
    case GLFW_KEY_S:
      cube.position.z += TRANSLATE_STEP;
      break;
    case GLFW_KEY_RIGHT_BRACKET:
      cube.scale += SCALE_STEP;
      break;
    case GLFW_KEY_LEFT_BRACKET:
      cube.scale = std::max(SCALE_MIN, cube.scale - SCALE_STEP);
      break;
    default:
      break;
  }
}
