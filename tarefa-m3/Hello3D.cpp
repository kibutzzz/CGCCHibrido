#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace std;

const GLuint WIDTH = 800, HEIGHT = 800;
const int    SHADER_LOG_SIZE = 512;
const float  ROTATE_SPEED    = 1.0f;
const string OBJ_FILE        = "Suzanne.obj";

const GLchar* vertexShaderSource = R"(
    #version 410
    layout (location = 0) in vec3 position;
    layout (location = 1) in vec3 color;
    layout (location = 2) in vec2 tex_coord;
    uniform mat4 model;
    out vec2 texCoord;
    void main() {
        gl_Position = model * vec4(position, 1.0);
        texCoord = tex_coord;
    }
)";

const GLchar* fragmentShaderSource = R"(
    #version 410
    in vec2 texCoord;
    out vec4 color;
    uniform sampler2D tex_buffer;
    void main() {
        color = texture(tex_buffer, texCoord);
    }
)";

bool rotateX = false, rotateY = false, rotateZ = false;
float angleX = 0.0f, angleY = glm::radians(180.0f), angleZ = 0.0f;
const float TRANSLATE_STEP = 0.1f;
const float SCALE_STEP     = 0.1f;
const float SCALE_MIN      = 0.1f;
glm::vec3 objPosition(0.0f);
float objScale = 0.5f;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
GLuint setupShader();
GLuint loadSimpleOBJ(const string& filePath, int& nVertices, string& mtlFile);
GLuint loadTexture(const string& filePath);
string loadMTL(const string& filePath);

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

GLuint loadSimpleOBJ(const string& filePath, int& nVertices, string& mtlFile)
{
    vector<glm::vec3> vertices;
    vector<glm::vec2> texCoords;
    vector<glm::vec3> normals;
    vector<GLfloat>   vBuffer;
    glm::vec3 color(1.0f, 0.0f, 0.0f);

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

        if (word == "mtllib") {
            ssline >> mtlFile;
        } else if (word == "v") {
            glm::vec3 v;
            ssline >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        } else if (word == "vt") {
            glm::vec2 vt;
            ssline >> vt.s >> vt.t;
            texCoords.push_back(vt);
        } else if (word == "vn") {
            glm::vec3 vn;
            ssline >> vn.x >> vn.y >> vn.z;
            normals.push_back(vn);
        } else if (word == "f") {
            while (ssline >> word) {
                int vi = 0, ti = 0, ni = 0;
                istringstream ss(word);
                string index;

                if (getline(ss, index, '/')) vi = !index.empty() ? stoi(index) - 1 : 0;
                if (getline(ss, index, '/')) ti = !index.empty() ? stoi(index) - 1 : 0;
                if (getline(ss, index))      ni = !index.empty() ? stoi(index) - 1 : 0;

                vBuffer.push_back(vertices[vi].x);
                vBuffer.push_back(vertices[vi].y);
                vBuffer.push_back(vertices[vi].z);
                vBuffer.push_back(color.r);
                vBuffer.push_back(color.g);
                vBuffer.push_back(color.b);
                vBuffer.push_back(texCoords.empty() ? 0.0f : texCoords[ti].s);
                vBuffer.push_back(texCoords.empty() ? 0.0f : texCoords[ti].t);
            }
        }
    }
    arqEntrada.close();

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vBuffer.size() * sizeof(GLfloat), vBuffer.data(), GL_STATIC_DRAW);

    const int STRIDE         = 8;
    const int OFFSET_POS     = 0;
    const int OFFSET_COLOR   = 3;
    const int OFFSET_TEXCOORD = 6;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof(GLfloat), (GLvoid*)(OFFSET_POS * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof(GLfloat), (GLvoid*)(OFFSET_COLOR * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, STRIDE * sizeof(GLfloat), (GLvoid*)(OFFSET_TEXCOORD * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    nVertices = (int)(vBuffer.size() / STRIDE);
    return VAO;
}

string loadMTL(const string& filePath)
{
    ifstream arq(filePath);
    if (!arq.is_open()) {
        cerr << "Erro ao tentar ler o arquivo MTL " << filePath << endl;
        return "";
    }

    string line, texName;
    while (getline(arq, line)) {
        istringstream ss(line);
        string word;
        ss >> word;
        if (word == "map_Kd") {
            ss >> texName;
            break;
        }
    }
    return texName;
}

GLuint loadTexture(const string& filePath)
{
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

    if (data) {
        if (nrChannels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,  width, height, 0, GL_RGB,  GL_UNSIGNED_BYTE, data);
        else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        cerr << "Falha ao carregar textura: " << filePath << endl;
    }

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;
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

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Tarefa M3 - Texturas", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Falha ao inicializar GLAD" << endl;
        return -1;
    }

    cout << "Renderer: "       << glGetString(GL_RENDERER) << endl;
    cout << "OpenGL version: " << glGetString(GL_VERSION)  << endl;

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    glEnable(GL_DEPTH_TEST);

    GLuint shaderID = setupShader();

    const string modelsDir = ASSETS_DIR;
    const string objPath = modelsDir + OBJ_FILE;

    string mtlFilename;
    int nVertices = 0;
    GLuint VAO = loadSimpleOBJ(objPath, nVertices, mtlFilename);

    string texName = loadMTL(modelsDir + mtlFilename);
    GLuint texID   = loadTexture(modelsDir + texName);

    glUseProgram(shaderID);
    glUniform1i(glGetUniformLocation(shaderID, "tex_buffer"), 0);
    GLint modelLoc = glGetUniformLocation(shaderID, "model");

    float lastTime = (float)glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = (float)glfwGetTime();
        float deltaTime   = currentTime - lastTime;
        lastTime          = currentTime;

        glfwPollEvents();

        if (rotateX) angleX += ROTATE_SPEED * deltaTime;
        if (rotateY) angleY += ROTATE_SPEED * deltaTime;
        if (rotateZ) angleZ += ROTATE_SPEED * deltaTime;

        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model(1.0f);
        model = glm::translate(model, objPosition);
        model = glm::rotate(model, angleX, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, angleY, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, angleZ, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(objScale));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texID);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, nVertices);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteTextures(1, &texID);
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;

    switch (key)
    {
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GL_TRUE); break;

        case GLFW_KEY_X: rotateX = !rotateX; break;
        case GLFW_KEY_Y: rotateY = !rotateY; break;
        case GLFW_KEY_Z: rotateZ = !rotateZ; break;

        case GLFW_KEY_D: objPosition.x += TRANSLATE_STEP; break;
        case GLFW_KEY_A: objPosition.x -= TRANSLATE_STEP; break;
        case GLFW_KEY_I: objPosition.y += TRANSLATE_STEP; break;
        case GLFW_KEY_J: objPosition.y -= TRANSLATE_STEP; break;
        case GLFW_KEY_W: objPosition.z -= TRANSLATE_STEP; break;
        case GLFW_KEY_S: objPosition.z += TRANSLATE_STEP; break;

        case GLFW_KEY_RIGHT_BRACKET: objScale += SCALE_STEP; break;
        case GLFW_KEY_LEFT_BRACKET:  objScale = std::max(SCALE_MIN, objScale - SCALE_STEP); break;

        default: break;
    }
}
