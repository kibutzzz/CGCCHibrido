#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const GLuint WIDTH = 800, HEIGHT = 800;

const GLchar* vertexShaderSource = R"(
    #version 410
    layout (location = 0) in vec3 position;
    layout (location = 1) in vec3 color;
    uniform mat4 model;
    out vec4 finalColor;
    void main() {
        gl_Position = model * vec4(position, 1.0);
        finalColor  = vec4(color, 1.0);
    }
)";

const GLchar* fragmentShaderSource = R"(
    #version 410
    in  vec4 finalColor;
    out vec4 color;
    void main() {
        color = finalColor;
    }
)";

struct Cube {
    glm::vec3 position;
    float     scale;
    bool      rotateX, rotateY, rotateZ;
    float     angleX, angleY, angleZ;
};

std::vector<Cube> cubes;
int selectedCube = 0;

const float TRANSLATE_STEP = 0.1f;
const float SCALE_STEP     = 0.1f;
const float SCALE_MIN      = 0.1f;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
GLuint setupShader();
GLuint setupGeometry();

void printControls()
{
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

glm::mat4 buildModelMatrix(const Cube& cube, bool isSelected)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, cube.position);
    model = glm::rotate(model, cube.angleX, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, cube.angleY, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, cube.angleZ, glm::vec3(0.0f, 0.0f, 1.0f));

    float visualScale = isSelected ? cube.scale : cube.scale * 0.85f;
    model = glm::scale(model, glm::vec3(visualScale));

    return model;
}

GLuint compileShader(GLenum type, const GLchar* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint  success;
    GLchar log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, log);
        std::cout << "Erro de compilacao do shader:\n" << log << std::endl;
    }
    return shader;
}

GLuint setupShader()
{
    GLuint vs = compileShader(GL_VERTEX_SHADER,   vertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint  success;
    GLchar log[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, log);
        std::cout << "Erro de linkagem do shader:\n" << log << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

GLuint setupGeometry()
{
    const float RED[]     = { 1.0f, 0.2f, 0.2f };
    const float GREEN[]   = { 0.2f, 1.0f, 0.2f };
    const float BLUE[]    = { 0.2f, 0.4f, 1.0f };
    const float YELLOW[]  = { 1.0f, 1.0f, 0.1f };
    const float CYAN[]    = { 0.1f, 1.0f, 1.0f };
    const float MAGENTA[] = { 1.0f, 0.2f, 1.0f };

    #define V(x, y, z, col) x, y, z, col[0], col[1], col[2]

    GLfloat vertices[] = {
        V(-0.5f, -0.5f,  0.5f, RED),
        V( 0.5f, -0.5f,  0.5f, RED),
        V( 0.5f,  0.5f,  0.5f, RED),
        V( 0.5f,  0.5f,  0.5f, RED),
        V(-0.5f,  0.5f,  0.5f, RED),
        V(-0.5f, -0.5f,  0.5f, RED),

        V( 0.5f, -0.5f, -0.5f, GREEN),
        V(-0.5f, -0.5f, -0.5f, GREEN),
        V(-0.5f,  0.5f, -0.5f, GREEN),
        V(-0.5f,  0.5f, -0.5f, GREEN),
        V( 0.5f,  0.5f, -0.5f, GREEN),
        V( 0.5f, -0.5f, -0.5f, GREEN),

        V(-0.5f, -0.5f, -0.5f, BLUE),
        V(-0.5f, -0.5f,  0.5f, BLUE),
        V(-0.5f,  0.5f,  0.5f, BLUE),
        V(-0.5f,  0.5f,  0.5f, BLUE),
        V(-0.5f,  0.5f, -0.5f, BLUE),
        V(-0.5f, -0.5f, -0.5f, BLUE),

        V( 0.5f, -0.5f,  0.5f, YELLOW),
        V( 0.5f, -0.5f, -0.5f, YELLOW),
        V( 0.5f,  0.5f, -0.5f, YELLOW),
        V( 0.5f,  0.5f, -0.5f, YELLOW),
        V( 0.5f,  0.5f,  0.5f, YELLOW),
        V( 0.5f, -0.5f,  0.5f, YELLOW),

        V(-0.5f,  0.5f,  0.5f, CYAN),
        V( 0.5f,  0.5f,  0.5f, CYAN),
        V( 0.5f,  0.5f, -0.5f, CYAN),
        V( 0.5f,  0.5f, -0.5f, CYAN),
        V(-0.5f,  0.5f, -0.5f, CYAN),
        V(-0.5f,  0.5f,  0.5f, CYAN),

        V(-0.5f, -0.5f, -0.5f, MAGENTA),
        V( 0.5f, -0.5f, -0.5f, MAGENTA),
        V( 0.5f, -0.5f,  0.5f, MAGENTA),
        V( 0.5f, -0.5f,  0.5f, MAGENTA),
        V(-0.5f, -0.5f,  0.5f, MAGENTA),
        V(-0.5f, -0.5f, -0.5f, MAGENTA),
    };

    #undef V

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Tarefa M2 - Cubos", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Falha ao inicializar GLAD" << std::endl;
        return -1;
    }

    std::cout << "Renderer: "       << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL version: " << glGetString(GL_VERSION)  << std::endl;
    printControls();

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    glEnable(GL_DEPTH_TEST);

    GLuint shaderID = setupShader();
    GLuint VAO      = setupGeometry();
    GLint  modelLoc = glGetUniformLocation(shaderID, "model");

    cubes.push_back({ glm::vec3(-0.5f,  0.0f, 0.0f), 1.0f, false, false, false, 0.0f, 0.0f, 0.0f });
    cubes.push_back({ glm::vec3( 0.5f,  0.0f, 0.0f), 1.0f, false, false, false, 0.0f, 0.0f, 0.0f });
    cubes.push_back({ glm::vec3( 0.0f, -0.5f, 0.0f), 1.0f, false, false, false, 0.0f, 0.0f, 0.0f });

    glUseProgram(shaderID);

    const float ROTATION_SPEED = 1.0f;
    float lastTime = (float)glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = (float)glfwGetTime();
        float deltaTime   = currentTime - lastTime;
        lastTime          = currentTime;

        glfwPollEvents();

        for (Cube& cube : cubes)
        {
            if (cube.rotateX) cube.angleX += ROTATION_SPEED * deltaTime;
            if (cube.rotateY) cube.angleY += ROTATION_SPEED * deltaTime;
            if (cube.rotateZ) cube.angleZ += ROTATION_SPEED * deltaTime;
        }

        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(VAO);
        for (int i = 0; i < (int)cubes.size(); i++)
        {
            glm::mat4 model = buildModelMatrix(cubes[i], i == selectedCube);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;

    Cube& cube = cubes[selectedCube];

    switch (key)
    {
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GL_TRUE); break;

        case GLFW_KEY_TAB: selectedCube = (selectedCube + 1) % (int)cubes.size(); break;

        case GLFW_KEY_X: cube.rotateX = !cube.rotateX; break;
        case GLFW_KEY_Y: cube.rotateY = !cube.rotateY; break;
        case GLFW_KEY_Z: cube.rotateZ = !cube.rotateZ; break;

        case GLFW_KEY_D: cube.position.x += TRANSLATE_STEP; break;
        case GLFW_KEY_A: cube.position.x -= TRANSLATE_STEP; break;
        case GLFW_KEY_I: cube.position.y += TRANSLATE_STEP; break;
        case GLFW_KEY_J: cube.position.y -= TRANSLATE_STEP; break;
        case GLFW_KEY_W: cube.position.z -= TRANSLATE_STEP; break;
        case GLFW_KEY_S: cube.position.z += TRANSLATE_STEP; break;

        case GLFW_KEY_RIGHT_BRACKET: cube.scale += SCALE_STEP;                                  break;
        case GLFW_KEY_LEFT_BRACKET:  cube.scale  = std::max(SCALE_MIN, cube.scale - SCALE_STEP); break;

        default: break;
    }
}
