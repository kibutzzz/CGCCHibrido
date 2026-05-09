#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cfloat>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

const GLuint WIDTH  = 800, HEIGHT = 800;
const int    SHADER_LOG_SIZE = 512;
const float  ROTATE_SPEED    = 1.0f;
const float  TRANSLATE_STEP  = 0.1f;
const float  SCALE_STEP      = 0.1f;
const float  SCALE_MIN       = 0.1f;

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

struct ObjModel {
    GLuint    vao;
    int       nVertices;
    glm::vec3 position;
    float     scale;
    bool      rotateX, rotateY, rotateZ;
    float     angleX, angleY, angleZ;
    string    name;
};

vector<ObjModel> models;
int selectedModel = 0;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
GLuint setupShader();
GLuint loadSimpleOBJ(const string& filePath, int& nVertices, glm::vec3 color);

GLuint compileShader(GLenum type, const GLchar* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint  success;
    GLchar log[SHADER_LOG_SIZE];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, SHADER_LOG_SIZE, NULL, log);
        cout << "Erro de compilacao do shader:\n" << log << endl;
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
    GLchar log[SHADER_LOG_SIZE];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, SHADER_LOG_SIZE, NULL, log);
        cout << "Erro de linkagem do shader:\n" << log << endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

void triangulateAndAppend(const vector<int>& faceVerts, const vector<glm::vec3>& vertices,
                          glm::vec3 color, vector<GLfloat>& vBuffer)
{
    for (int i = 1; i + 1 < (int)faceVerts.size(); i++) {
        for (int vi : { faceVerts[0], faceVerts[i], faceVerts[i + 1] }) {
            vBuffer.push_back(vertices[vi].x);
            vBuffer.push_back(vertices[vi].y);
            vBuffer.push_back(vertices[vi].z);
            vBuffer.push_back(color.r);
            vBuffer.push_back(color.g);
            vBuffer.push_back(color.b);
        }
    }
}

void normalizeVertices(vector<GLfloat>& vBuffer)
{
    const int STRIDE = 6;
    glm::vec3 minV(FLT_MAX), maxV(-FLT_MAX);
    for (int i = 0; i < (int)vBuffer.size(); i += STRIDE) {
        minV.x = min(minV.x, vBuffer[i]);
        minV.y = min(minV.y, vBuffer[i + 1]);
        minV.z = min(minV.z, vBuffer[i + 2]);
        maxV.x = max(maxV.x, vBuffer[i]);
        maxV.y = max(maxV.y, vBuffer[i + 1]);
        maxV.z = max(maxV.z, vBuffer[i + 2]);
    }
    glm::vec3 center    = (minV + maxV) * 0.5f;
    float     extent    = max({ maxV.x - minV.x, maxV.y - minV.y, maxV.z - minV.z });
    float     invExtent = (extent > 0.0f) ? (1.0f / extent) : 1.0f;
    for (int i = 0; i < (int)vBuffer.size(); i += STRIDE) {
        vBuffer[i]     = (vBuffer[i]     - center.x) * invExtent;
        vBuffer[i + 1] = (vBuffer[i + 1] - center.y) * invExtent;
        vBuffer[i + 2] = (vBuffer[i + 2] - center.z) * invExtent;
    }
}

GLuint uploadToGPU(const vector<GLfloat>& vBuffer, int& nVertices)
{
    const int STRIDE = 6;
    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vBuffer.size() * sizeof(GLfloat), vBuffer.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    nVertices = (int)(vBuffer.size() / STRIDE);
    return VAO;
}

GLuint loadSimpleOBJ(const string& filePath, int& nVertices, glm::vec3 color)
{
    vector<glm::vec3> vertices;
    vector<GLfloat>   vBuffer;

    ifstream arqEntrada(filePath);
    if (!arqEntrada.is_open()) {
        cerr << "Erro ao tentar ler o arquivo " << filePath << endl;
        return (GLuint)-1;
    }

    string line;
    while (getline(arqEntrada, line)) {
        istringstream ssline(line);
        string word;
        ssline >> word;

        if (word == "v") {
            glm::vec3 v;
            ssline >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        } else if (word == "f") {
            vector<int> faceVerts;
            while (ssline >> word) {
                istringstream ss(word);
                string index;
                int vi = 0;
                if (getline(ss, index, '/')) vi = !index.empty() ? stoi(index) - 1 : 0;
                faceVerts.push_back(vi);
            }
            triangulateAndAppend(faceVerts, vertices, color, vBuffer);
        }
    }

    normalizeVertices(vBuffer);
    return uploadToGPU(vBuffer, nVertices);
}

ObjModel loadModel(const string& name, const string& filePath, glm::vec3 position,
                   float scale, glm::vec3 color)
{
    ObjModel obj;
    obj.name     = name;
    obj.position = position;
    obj.scale    = scale;
    obj.rotateX  = obj.rotateY = obj.rotateZ = false;
    obj.angleX   = obj.angleY = obj.angleZ = 0.0f;
    obj.vao      = loadSimpleOBJ(filePath, obj.nVertices, color);
    return obj;
}

glm::mat4 buildModelMatrix(const ObjModel& obj)
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, obj.position);
    model = glm::rotate(model, obj.angleX, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, obj.angleY, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, obj.angleZ, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(obj.scale));
    return model;
}

void updateRotations(float deltaTime)
{
    for (ObjModel& obj : models) {
        if (obj.rotateX) obj.angleX += ROTATE_SPEED * deltaTime;
        if (obj.rotateY) obj.angleY += ROTATE_SPEED * deltaTime;
        if (obj.rotateZ) obj.angleZ += ROTATE_SPEED * deltaTime;
    }
}

void drawModels(GLint modelLoc)
{
    for (int i = 0; i < (int)models.size(); i++) {
        glm::mat4 model = buildModelMatrix(models[i]);
        if (i == selectedModel)
            model = glm::scale(model, glm::vec3(1.05f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(models[i].vao);
        glDrawArrays(GL_TRIANGLES, 0, models[i].nVertices);
        glBindVertexArray(0);
    }
}

void printControls()
{
    cout << "\n=== Controles ===\n"
         << "Tab      : Alternar objeto selecionado\n"
         << "X / Y / Z: Ativar/desativar rotacao no eixo correspondente\n"
         << "A / D    : Transladar no eixo X\n"
         << "W / S    : Transladar no eixo Z\n"
         << "I / J    : Transladar no eixo Y\n"
         << "] / [    : Aumentar / diminuir escala\n"
         << "Esc      : Sair\n"
         << "=================\n\n";
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

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Vivencial M2 - Selecao e Transformacoes", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Falha ao inicializar GLAD" << endl;
        return -1;
    }

    cout << "Renderer: "       << glGetString(GL_RENDERER) << endl;
    cout << "OpenGL version: " << glGetString(GL_VERSION)  << endl;
    printControls();

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    glEnable(GL_DEPTH_TEST);

    GLuint shaderID = setupShader();
    GLint  modelLoc = glGetUniformLocation(shaderID, "model");
    glUseProgram(shaderID);

    const string modelsDir = ASSETS_DIR;
    models.push_back(loadModel("Plane",        modelsDir + "1405 Plane.obj",            glm::vec3(-0.6f,  0.0f, 0.0f), 0.4f, glm::vec3(0.4f, 0.7f,  1.0f)));
    models.push_back(loadModel("House",        modelsDir + "PUSHILIN_house.obj",        glm::vec3( 0.0f, -0.2f, 0.0f), 0.4f, glm::vec3(0.9f, 0.75f, 0.5f)));
    models.push_back(loadModel("Lamborghini",  modelsDir + "Lamborghini_Aventador.obj", glm::vec3( 0.6f,  0.0f, 0.0f), 0.4f, glm::vec3(1.0f, 0.4f,  0.1f)));

    cout << "Objeto selecionado: " << models[selectedModel].name << endl;

    float lastTime = (float)glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = (float)glfwGetTime();
        float deltaTime   = currentTime - lastTime;
        lastTime          = currentTime;

        glfwPollEvents();
        updateRotations(deltaTime);

        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawModels(modelLoc);

        glfwSwapBuffers(window);
    }

    for (ObjModel& obj : models)
        glDeleteVertexArrays(1, &obj.vao);

    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;

    ObjModel& obj = models[selectedModel];

    switch (key)
    {
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GL_TRUE); break;

        case GLFW_KEY_TAB:
            selectedModel = (selectedModel + 1) % (int)models.size();
            cout << "Objeto selecionado: " << models[selectedModel].name << endl;
            break;

        case GLFW_KEY_X: obj.rotateX = !obj.rotateX; break;
        case GLFW_KEY_Y: obj.rotateY = !obj.rotateY; break;
        case GLFW_KEY_Z: obj.rotateZ = !obj.rotateZ; break;

        case GLFW_KEY_D: obj.position.x += TRANSLATE_STEP; break;
        case GLFW_KEY_A: obj.position.x -= TRANSLATE_STEP; break;
        case GLFW_KEY_I: obj.position.y += TRANSLATE_STEP; break;
        case GLFW_KEY_J: obj.position.y -= TRANSLATE_STEP; break;
        case GLFW_KEY_W: obj.position.z -= TRANSLATE_STEP; break;
        case GLFW_KEY_S: obj.position.z += TRANSLATE_STEP; break;

        case GLFW_KEY_RIGHT_BRACKET: obj.scale += SCALE_STEP;                              break;
        case GLFW_KEY_LEFT_BRACKET:  obj.scale  = max(SCALE_MIN, obj.scale - SCALE_STEP); break;

        default: break;
    }
}
