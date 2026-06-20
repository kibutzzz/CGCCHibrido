#pragma once

#include <glad/glad.h>

#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <string>

class ShaderProgram {
 public:
  GLuint id;

  ShaderProgram(const std::string& vertPath, const std::string& fragPath) {
    id = link(compile(GL_VERTEX_SHADER, load(vertPath)),
              compile(GL_FRAGMENT_SHADER, load(fragPath)));
  }

  void use() const { glUseProgram(id); }

  void setMat4(const std::string& name, const glm::mat4& v) const {
    glUniformMatrix4fv(loc(name), 1, GL_FALSE, glm::value_ptr(v));
  }
  void setVec3(const std::string& name, const glm::vec3& v) const {
    glUniform3fv(loc(name), 1, glm::value_ptr(v));
  }
  void setFloat(const std::string& name, float v) const {
    glUniform1f(loc(name), v);
  }
  void setInt(const std::string& name, int v) const {
    glUniform1i(loc(name), v);
  }

 private:
  GLint loc(const std::string& name) const {
    return glGetUniformLocation(id, name.c_str());
  }

  static std::string load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
      std::cerr << "ShaderProgram: nao foi possivel abrir " << path << "\n";
      return "";
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
  }

  static GLuint compile(GLenum type, const std::string& src) {
    const char* cstr = src.c_str();
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &cstr, nullptr);
    glCompileShader(shader);

    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
      char log[512];
      glGetShaderInfoLog(shader, 512, nullptr, log);
      std::cerr << "ShaderProgram erro de compilacao:\n" << log << "\n";
    }
    return shader;
  }

  static GLuint link(GLuint vs, GLuint fs) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint ok;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
      char log[512];
      glGetProgramInfoLog(program, 512, nullptr, log);
      std::cerr << "ShaderProgram erro de link:\n" << log << "\n";
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
  }
};
