#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "Camera.h"
#include "ShaderProgram.h"
#include "Window.h"
#include "AnimationPath.h"
#include "SceneObject.h"
#include "Scene.h"
#include "SceneLoader.h"
#include "InputHandler.h"

static const int WINDOW_WIDTH  = 1200;
static const int WINDOW_HEIGHT = 800;

// Globals needed for GLFW plain-function callbacks
static Camera* g_camera = nullptr;
static Scene*  g_scene  = nullptr;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    InputHandler::onKey(window, key, scancode, action, mods, *g_scene, *g_camera);
}
void mouseCallback(GLFWwindow*, double xpos, double ypos) { g_camera->processMouse(xpos, ypos); }
void scrollCallback(GLFWwindow*, double, double yoffset)  { g_camera->processScroll(yoffset); }

int main() {
    Window window(WINDOW_WIDTH, WINDOW_HEIGHT, "Tarefa M6 - Trajet\xc3\xb3rias");

    glfwSetKeyCallback(window.window, keyCallback);
    glfwSetCursorPosCallback(window.window, mouseCallback);
    glfwSetScrollCallback(window.window, scrollCallback);
    glfwSetInputMode(window.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "OpenGL:   " << glGetString(GL_VERSION)  << "\n";
    std::cout << "\n=== Controles ===\n"
              << "W/A/S/D  : mover camera\n"
              << "Mouse    : olhar ao redor\n"
              << "Scroll   : zoom\n"
              << "Tab      : selecionar proximo objeto\n"
              << "P        : adicionar waypoint na posicao da camera\n"
              << "Esc      : sair\n"
              << "=================\n\n";

    glEnable(GL_DEPTH_TEST);

    ShaderProgram shader(SHADERS_DIR "vertex.vert", SHADERS_DIR "fragment.frag");
    shader.use();
    shader.setInt("tex_buffer", 0);

    // --- Build scene ---
    Scene scene;
    scene.lights.push_back({{5.0f, 8.0f, 5.0f}, {1.0f, 1.0f, 1.0f}, 1.0f});

    const std::string assetsDir = ASSETS_DIR;

    // Suzanne — circular orbit on the left
    {
        SceneObject suzanne = makeObject(assetsDir, "Suzanne.obj");
        suzanne.scale = 1.0f;
        suzanne.animation = std::make_unique<AnimationPath>();
        suzanne.animation->controlPoints = {
            {-4.0f,  0.0f,  2.0f},
            {-4.0f,  2.0f,  0.0f},
            {-4.0f,  0.0f, -2.0f},
            {-4.0f, -2.0f,  0.0f},
        };
        suzanne.position = suzanne.animation->currentPosition();
        scene.objects.push_back(std::move(suzanne));
    }

    // House — square path on the right
    {
        SceneObject house = makeObject(assetsDir, "PUSHILIN_house.obj");
        house.scale = 1.0f;
        house.animation = std::make_unique<AnimationPath>();
        house.animation->controlPoints = {
            { 4.0f, 0.0f,  3.0f},
            { 7.0f, 0.0f,  0.0f},
            { 4.0f, 0.0f, -3.0f},
            { 1.0f, 0.0f,  0.0f},
        };
        house.position = house.animation->currentPosition();
        scene.objects.push_back(std::move(house));
    }

    // Ground plane — static, no animation
    {
        SceneObject groundPlane = makeObject(assetsDir, "1405 Plane.obj");
        groundPlane.position = {0.0f, -2.5f, 0.0f};
        groundPlane.scale = 0.05f;
        scene.objects.push_back(std::move(groundPlane));
    }

    scene.objects[0].selected = true;

    Camera camera(glm::vec3(0.0f, 2.0f, 10.0f));
    g_scene  = &scene;
    g_camera = &camera;

    std::cout << "Scene loaded: " << scene.objects.size() << " object(s)\n";
    for (auto& obj : scene.objects) {
        int waypointCount = obj.animation ? (int)obj.animation->controlPoints.size() : 0;
        std::cout << "  " << obj.name << " — waypoints: " << waypointCount << "\n";
    }
    std::cout << "Selected: " << scene.selected().name << "\n\n";

    float previousTime = (float)glfwGetTime();

    while (!window.shouldClose()) {
        float currentTime = (float)glfwGetTime();
        float deltaTime   = currentTime - previousTime;
        previousTime      = currentTime;

        window.pollEvents();
        camera.processKeyboard(window.window, deltaTime);

        for (auto& obj : scene.objects)
            if (obj.animation) {
                obj.animation->update(deltaTime);
                obj.position = obj.animation->currentPosition();
            }

        shader.setMat4("view",       camera.viewMatrix());
        shader.setMat4("projection", camera.projectionMatrix((float)WINDOW_WIDTH / WINDOW_HEIGHT));
        shader.setVec3("camPos",     camera.pos);

        if (!scene.lights.empty()) {
            shader.setVec3("lightPos",   scene.lights[0].position);
            shader.setVec3("lightColor", scene.lights[0].color * scene.lights[0].intensity);
        }

        glClearColor(0.12f, 0.12f, 0.18f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto& obj : scene.objects) {
            shader.setMat4("model",  obj.modelMatrix());
            shader.setVec3("ka",     obj.material.ka);
            shader.setVec3("kd",     obj.material.kd);
            shader.setVec3("ks",     obj.material.ks);
            shader.setFloat("ns",    obj.material.ns);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, obj.textureID);
            glBindVertexArray(obj.vao);
            glDrawArrays(GL_TRIANGLES, 0, obj.nVertices);
            glBindVertexArray(0);
        }

        window.swapBuffers();
    }

    return 0;
}
