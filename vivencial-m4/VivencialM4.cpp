#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <algorithm>
#include <array>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace std;

const GLuint WIDTH = 800, HEIGHT = 800;
const int SHADER_LOG_SIZE = 512;
const float ROTATE_SPEED = 1.0f;
const float TRANSLATE_STEP = 0.1f;
const float SCALE_STEP = 0.1f;
const float SCALE_MIN = 0.1f;
const string OBJ_FILE = "PUSHILIN_house.obj";

const GLchar* vertexShaderSource = R"(
    #version 410
    layout (location = 0) in vec3 position;
    layout (location = 1) in vec3 color;
    layout (location = 2) in vec3 normal;
    layout (location = 3) in vec2 tex_coord;
    uniform mat4 model;
    out vec3 vNormal;
    out vec3 fragPos;
    out vec2 texCoord;
    void main() {
        vec4 worldPos = model * vec4(position, 1.0);
        gl_Position = worldPos;
        fragPos  = vec3(worldPos);
        vNormal  = mat3(transpose(inverse(model))) * normal;
        texCoord = tex_coord;
    }
)";

const GLchar* fragmentShaderSource = R"(
    #version 410
    in vec3 vNormal;
    in vec3 fragPos;
    in vec2 texCoord;
    out vec4 color;

    struct PointLight {
        vec3  position;
        vec3  lightColor;
        float intensity;
        bool  enabled;
    };
    uniform PointLight lights[3];
    uniform vec3  camPos;
    uniform vec3  ka;
    uniform vec3  kd;
    uniform vec3  ks;
    uniform float ns;
    uniform sampler2D tex_buffer;

    vec3 phongContribution(PointLight light, vec3 N, vec3 V) {
        vec3  L    = normalize(light.position - fragPos);
        float dist = length(light.position - fragPos);
        float att  = 1.0 / (1.0 + 0.09 * dist + 0.032 * dist * dist);

        vec3  ambient  = ka * light.lightColor * light.intensity;
        float diffAmt  = max(dot(N, L), 0.0);
        vec3  diffuse  = kd * diffAmt * att * light.lightColor * light.intensity;
        vec3  R        = reflect(-L, N);
        float specAmt  = pow(max(dot(R, V), 0.0), max(ns, 1.0));
        vec3  specular = ks * specAmt * light.lightColor * light.intensity;

        return ambient + diffuse + specular;
    }

    void main() {
        vec3 N = normalize(vNormal);
        vec3 V = normalize(camPos - fragPos);
        vec4 texColor = texture(tex_buffer, texCoord);

        vec3 illumination = vec3(0.0);
        for (int i = 0; i < 3; i++) {
            if (lights[i].enabled)
                illumination += phongContribution(lights[i], N, V);
        }
        color = vec4(illumination * vec3(texColor), texColor.a);
    }
)";

bool rotateX = false, rotateY = false, rotateZ = false;
float angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f;
glm::vec3 objPosition(0.0f);
float objScale = 0.5f;
bool lightsEnabled[3] = {true, true, true};

struct Material {
  glm::vec3 ka = glm::vec3(0.1f);
  glm::vec3 kd = glm::vec3(0.8f);
  glm::vec3 ks = glm::vec3(0.5f);
  float ns = 32.0f;
  string texName;
};

struct PointLight {
  glm::vec3 position;
  glm::vec3 color;
  float intensity;
};

void key_callback(GLFWwindow* window, int key, int scancode, int action,
                  int mode);
GLuint setupShader();
GLuint loadSimpleOBJ(const string& filePath, int& nVertices, string& mtlFile);
GLuint loadTexture(const string& filePath);
Material loadMTL(const string& filePath);

GLuint compileShader(GLenum type, const GLchar* source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  GLint success;
  GLchar log[SHADER_LOG_SIZE];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, SHADER_LOG_SIZE, NULL, log);
    cout << "Shader compile error:\n" << log << endl;
  }
  return shader;
}

GLuint setupShader() {
  GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
  GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

  GLuint program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);

  GLint success;
  GLchar log[SHADER_LOG_SIZE];
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, SHADER_LOG_SIZE, NULL, log);
    cout << "Shader link error:\n" << log << endl;
  }

  glDeleteShader(vs);
  glDeleteShader(fs);
  return program;
}

GLuint uploadMeshToGPU(const vector<GLfloat>& vBuffer, int& nVertices) {
  const int STRIDE = 11;
  const int OFFSET_POS = 0;
  const int OFFSET_COLOR = 3;
  const int OFFSET_NORMAL = 6;
  const int OFFSET_TEXCOORD = 9;

  GLuint VBO, VAO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vBuffer.size() * sizeof(GLfloat),
               vBuffer.data(), GL_STATIC_DRAW);

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof(GLfloat),
                        (GLvoid*)(OFFSET_POS * sizeof(GLfloat)));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof(GLfloat),
                        (GLvoid*)(OFFSET_COLOR * sizeof(GLfloat)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof(GLfloat),
                        (GLvoid*)(OFFSET_NORMAL * sizeof(GLfloat)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, STRIDE * sizeof(GLfloat),
                        (GLvoid*)(OFFSET_TEXCOORD * sizeof(GLfloat)));
  glEnableVertexAttribArray(3);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  nVertices = (int)(vBuffer.size() / STRIDE);
  return VAO;
}

GLuint loadSimpleOBJ(const string& filePath, int& nVertices, string& mtlFile) {
  vector<glm::vec3> positions;
  vector<glm::vec2> texCoords;
  vector<glm::vec3> normals;
  vector<GLfloat> vBuffer;
  glm::vec3 defaultColor(1.0f, 0.0f, 0.0f);

  ifstream file(filePath);
  if (!file.is_open()) {
    cerr << "Erro ao ler o arquivo " << filePath << endl;
    return (GLuint)-1;
  }

  string line;
  while (getline(file, line)) {
    istringstream ssline(line);
    string token;
    ssline >> token;

    if (token == "mtllib") {
      ssline >> mtlFile;
    } else if (token == "v") {
      glm::vec3 v;
      ssline >> v.x >> v.y >> v.z;
      positions.push_back(v);
    } else if (token == "vt") {
      glm::vec2 vt;
      ssline >> vt.s >> vt.t;
      texCoords.push_back(vt);
    } else if (token == "vn") {
      glm::vec3 vn;
      ssline >> vn.x >> vn.y >> vn.z;
      normals.push_back(vn);
    } else if (token == "f") {
      struct FaceVertex {
        int vi, ti, ni;
      };
      vector<FaceVertex> faceVerts;
      while (ssline >> token) {
        FaceVertex fv = {0, 0, 0};
        istringstream ss(token);
        string idx;
        if (getline(ss, idx, '/')) fv.vi = !idx.empty() ? stoi(idx) - 1 : 0;
        if (getline(ss, idx, '/')) fv.ti = !idx.empty() ? stoi(idx) - 1 : 0;
        if (getline(ss, idx)) fv.ni = !idx.empty() ? stoi(idx) - 1 : 0;
        faceVerts.push_back(fv);
      }
      auto pushVertex = [&](const FaceVertex& fv) {
        vBuffer.push_back(positions[fv.vi].x);
        vBuffer.push_back(positions[fv.vi].y);
        vBuffer.push_back(positions[fv.vi].z);
        vBuffer.push_back(defaultColor.r);
        vBuffer.push_back(defaultColor.g);
        vBuffer.push_back(defaultColor.b);
        vBuffer.push_back(normals.empty() ? 0.0f : normals[fv.ni].x);
        vBuffer.push_back(normals.empty() ? 0.0f : normals[fv.ni].y);
        vBuffer.push_back(normals.empty() ? 0.0f : normals[fv.ni].z);
        vBuffer.push_back(texCoords.empty() ? 0.0f : texCoords[fv.ti].s);
        vBuffer.push_back(texCoords.empty() ? 0.0f : texCoords[fv.ti].t);
      };
      for (int i = 1; i + 1 < (int)faceVerts.size(); i++) {
        pushVertex(faceVerts[0]);
        pushVertex(faceVerts[i]);
        pushVertex(faceVerts[i + 1]);
      }
    }
  }

  return uploadMeshToGPU(vBuffer, nVertices);
}

Material loadMTL(const string& filePath) {
  Material mat;
  ifstream file(filePath);
  if (!file.is_open()) {
    cerr << "Erro ao ler MTL " << filePath << endl;
    return mat;
  }

  bool hasKa = false, hasKd = false, hasKs = false;
  string line;
  while (getline(file, line)) {
    istringstream ss(line);
    string token;
    ss >> token;

    if (token == "Ka") {
      ss >> mat.ka.r >> mat.ka.g >> mat.ka.b;
      hasKa = true;
    } else if (token == "Kd") {
      ss >> mat.kd.r >> mat.kd.g >> mat.kd.b;
      hasKd = true;
    } else if (token == "Ks") {
      ss >> mat.ks.r >> mat.ks.g >> mat.ks.b;
      hasKs = true;
    } else if (token == "Ns") {
      ss >> mat.ns;
    } else if (token == "map_Kd") {
      ss >> mat.texName;
    }
  }

  if (!hasKa || glm::length(mat.ka) < 0.001f) mat.ka = glm::vec3(0.1f);
  if (!hasKd || glm::length(mat.kd) < 0.001f) mat.kd = glm::vec3(0.8f);
  if (!hasKs || glm::length(mat.ks) < 0.001f) mat.ks = glm::vec3(0.5f);
  if (mat.ns < 1.0f) mat.ns = 32.0f;

  return mat;
}

GLuint loadTexture(const string& filePath) {
  GLuint texID;
  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_2D, texID);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int width, height, nrChannels;
  stbi_set_flip_vertically_on_load(true);
  unsigned char* data =
      stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);
  if (data) {
    GLenum fmt = (nrChannels == 3) ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, width, height, 0, fmt, GL_UNSIGNED_BYTE,
                 data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    cerr << "Falha ao carregar textura: " << filePath << endl;
  }
  stbi_image_free(data);
  glBindTexture(GL_TEXTURE_2D, 0);
  return texID;
}

array<PointLight, 3> computeThreePointLights(glm::vec3 objectCenter,
                                             float objectScale) {
  float r = 4.0f * objectScale;
  return {{
      {objectCenter + glm::vec3(r * 0.8f, r, -r * 0.5f),
       glm::vec3(1.0f, 0.95f, 0.85f), 1.6f},
      {objectCenter + glm::vec3(-r, r * 0.5f, -r * 0.5f),
       glm::vec3(0.8f, 0.85f, 1.0f), 0.7f},
      {objectCenter + glm::vec3(0.0f, r * 0.5f, r), glm::vec3(1.0f, 1.0f, 1.0f),
       0.9f},
  }};
}

void uploadLightUniforms(GLuint shaderID, const array<PointLight, 3>& lights,
                         const bool enabled[3]) {
  for (int i = 0; i < 3; i++) {
    string base = "lights[" + to_string(i) + "].";
    glUniform3fv(glGetUniformLocation(shaderID, (base + "position").c_str()), 1,
                 glm::value_ptr(lights[i].position));
    glUniform3fv(glGetUniformLocation(shaderID, (base + "lightColor").c_str()),
                 1, glm::value_ptr(lights[i].color));
    glUniform1f(glGetUniformLocation(shaderID, (base + "intensity").c_str()),
                lights[i].intensity);
    glUniform1i(glGetUniformLocation(shaderID, (base + "enabled").c_str()),
                enabled[i] ? 1 : 0);
  }
}

glm::mat4 buildModelMatrix(glm::vec3 position, float scale, float ax, float ay,
                           float az) {
  glm::mat4 model(1.0f);
  model = glm::translate(model, position);
  model = glm::rotate(model, ax, glm::vec3(1.0f, 0.0f, 0.0f));
  model = glm::rotate(model, ay, glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::rotate(model, az, glm::vec3(0.0f, 0.0f, 1.0f));
  model = glm::scale(model, glm::vec3(scale));
  return model;
}

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow* window = glfwCreateWindow(
      WIDTH, HEIGHT, "Phong Viewer M4 - 3-Point Lighting", nullptr, nullptr);
  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, key_callback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    cout << "Falha ao inicializar GLAD" << endl;
    return -1;
  }

  cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
  cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
  cout << "\n=== Controles ===\n"
       << "X/Y/Z  : toggle rotacao\n"
       << "A/D    : mover X   W/S: mover Z   I/J: mover Y\n"
       << "[/]    : escala\n"
       << "1/2/3  : toggle luz principal/preenchimento/fundo\n"
       << "Esc    : sair\n"
       << "=================\n\n";

  int fbWidth, fbHeight;
  glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
  glViewport(0, 0, fbWidth, fbHeight);
  glEnable(GL_DEPTH_TEST);

  GLuint shaderID = setupShader();

  const string modelsDir = ASSETS_DIR;
  string mtlFilename;
  int nVertices = 0;
  GLuint VAO = loadSimpleOBJ(modelsDir + OBJ_FILE, nVertices, mtlFilename);

  Material mat = loadMTL(modelsDir + mtlFilename);
  GLuint texID = loadTexture(modelsDir + mat.texName);

  glUseProgram(shaderID);
  glUniform1i(glGetUniformLocation(shaderID, "tex_buffer"), 0);
  glUniform3fv(glGetUniformLocation(shaderID, "ka"), 1, glm::value_ptr(mat.ka));
  glUniform3fv(glGetUniformLocation(shaderID, "kd"), 1, glm::value_ptr(mat.kd));
  glUniform3fv(glGetUniformLocation(shaderID, "ks"), 1, glm::value_ptr(mat.ks));
  glUniform1f(glGetUniformLocation(shaderID, "ns"), mat.ns);

  glm::vec3 camPos(0.0f);
  glUniform3fv(glGetUniformLocation(shaderID, "camPos"), 1,
               glm::value_ptr(camPos));

  GLint modelLoc = glGetUniformLocation(shaderID, "model");
  float lastTime = (float)glfwGetTime();

  while (!glfwWindowShouldClose(window)) {
    float currentTime = (float)glfwGetTime();
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    glfwPollEvents();

    if (rotateX) angleX += ROTATE_SPEED * deltaTime;
    if (rotateY) angleY += ROTATE_SPEED * deltaTime;
    if (rotateZ) angleZ += ROTATE_SPEED * deltaTime;

    auto lights = computeThreePointLights(objPosition, objScale);
    uploadLightUniforms(shaderID, lights, lightsEnabled);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 model =
        buildModelMatrix(objPosition, objScale, angleX, angleY, angleZ);
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

void key_callback(GLFWwindow* window, int key, int scancode, int action,
                  int mode) {
  if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

  switch (key) {
    case GLFW_KEY_ESCAPE:
      glfwSetWindowShouldClose(window, GL_TRUE);
      break;

    case GLFW_KEY_X:
      rotateX = !rotateX;
      break;
    case GLFW_KEY_Y:
      rotateY = !rotateY;
      break;
    case GLFW_KEY_Z:
      rotateZ = !rotateZ;
      break;

    case GLFW_KEY_D:
      objPosition.x += TRANSLATE_STEP;
      break;
    case GLFW_KEY_A:
      objPosition.x -= TRANSLATE_STEP;
      break;
    case GLFW_KEY_I:
      objPosition.y += TRANSLATE_STEP;
      break;
    case GLFW_KEY_J:
      objPosition.y -= TRANSLATE_STEP;
      break;
    case GLFW_KEY_W:
      objPosition.z -= TRANSLATE_STEP;
      break;
    case GLFW_KEY_S:
      objPosition.z += TRANSLATE_STEP;
      break;

    case GLFW_KEY_RIGHT_BRACKET:
      objScale += SCALE_STEP;
      break;
    case GLFW_KEY_LEFT_BRACKET:
      objScale = std::max(SCALE_MIN, objScale - SCALE_STEP);
      break;

    case GLFW_KEY_1:
      lightsEnabled[0] = !lightsEnabled[0];
      cout << "Key light: " << (lightsEnabled[0] ? "ON" : "OFF") << "\n";
      break;
    case GLFW_KEY_2:
      lightsEnabled[1] = !lightsEnabled[1];
      cout << "Fill light: " << (lightsEnabled[1] ? "ON" : "OFF") << "\n";
      break;
    case GLFW_KEY_3:
      lightsEnabled[2] = !lightsEnabled[2];
      cout << "Back light: " << (lightsEnabled[2] ? "ON" : "OFF") << "\n";
      break;

    default:
      break;
  }
}
