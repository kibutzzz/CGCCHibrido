#include <iostream>
#include <string>

using namespace std;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ShaderProgram.h"
#include "Window.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
GLuint setupGeometry();

const GLuint WIDTH = 1000, HEIGHT = 1000;

bool rotateX = false, rotateY = false, rotateZ = false;

int main()
{
    Window win(WIDTH, HEIGHT, "Ola 3D - Leonardo Ramos");
    win.setKeyCallback(key_callback);

    cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
    cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;

    glEnable(GL_DEPTH_TEST);

    ShaderProgram shader(SHADERS_DIR "vertex.vert", SHADERS_DIR "fragment.frag");
    shader.use();

    GLuint VAO = setupGeometry();

    glm::mat4 model(1.0f);
    shader.setMat4("model", model);

    while (!win.shouldClose())
    {
        win.pollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float angle = (float)glfwGetTime();
        model = glm::mat4(1.0f);
        if (rotateX) model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
        if (rotateY) model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        if (rotateZ) model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

        shader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 18);
        glBindVertexArray(0);

        win.swapBuffers();
    }

    glDeleteVertexArrays(1, &VAO);
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_X && action == GLFW_PRESS) rotateX = !rotateX;
    if (key == GLFW_KEY_Y && action == GLFW_PRESS) rotateY = !rotateY;
    if (key == GLFW_KEY_Z && action == GLFW_PRESS) rotateZ = !rotateZ;
}

GLuint setupGeometry()
{
    GLfloat vertices[] = {
        -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,

        -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,

        -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
         0.0f,  0.5f,  0.0f,  1.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
         0.0f,  0.5f,  0.0f,  1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,

        -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
         0.0f,  0.5f,  0.0f,  1.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,

         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
         0.0f,  0.5f,  0.0f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
    };

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}
