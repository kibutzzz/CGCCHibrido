#pragma once

#include <glad/glad.h>

#include <fstream>
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct Material {
  glm::vec3 ka = glm::vec3(0.1f);
  glm::vec3 kd = glm::vec3(0.8f);
  glm::vec3 ks = glm::vec3(0.5f);
  float ns = 32.0f;
  std::string texName;
};

GLuint loadTexture(const std::string& filePath) {
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
    std::cerr << "Mesh: failed to load texture: " << filePath << "\n";
  }
  stbi_image_free(data);
  glBindTexture(GL_TEXTURE_2D, 0);
  return texID;
}

Material loadMTL(const std::string& filePath) {
  Material mat;
  std::ifstream file(filePath);
  if (!file.is_open()) {
    std::cerr << "Mesh: failed to open MTL: " << filePath << "\n";
    return mat;
  }

  bool hasKa = false, hasKd = false, hasKs = false;
  std::string line;
  while (std::getline(file, line)) {
    std::istringstream ss(line);
    std::string token;
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
      std::getline(ss >> std::ws, mat.texName);
    }
  }

  if (!hasKa || glm::length(mat.ka) < 0.001f) mat.ka = glm::vec3(0.1f);
  if (!hasKd || glm::length(mat.kd) < 0.001f) mat.kd = glm::vec3(0.8f);
  if (!hasKs || glm::length(mat.ks) < 0.001f) mat.ks = glm::vec3(0.5f);
  if (mat.ns < 1.0f) mat.ns = 32.0f;

  return mat;
}

struct FaceVertex {
  int vi, ti, ni;
};

void pushVertex(std::vector<GLfloat>& vBuffer,
                const std::vector<glm::vec3>& positions,
                const std::vector<glm::vec3>& normals,
                const std::vector<glm::vec2>& texCoords,
                const glm::vec3& defaultColor, const FaceVertex& fv) {
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
}

GLuint loadSimpleOBJ(const std::string& filePath, int& nVertices,
                     std::string& mtlFile) {
  std::vector<glm::vec3> positions;
  std::vector<glm::vec2> texCoords;
  std::vector<glm::vec3> normals;
  std::vector<GLfloat> vBuffer;
  glm::vec3 defaultColor(1.0f, 0.0f, 0.0f);
  bool hasOBJNormals = false;

  std::ifstream file(filePath);
  if (!file.is_open()) {
    std::cerr << "Mesh: failed to open OBJ: " << filePath << "\n";
    return (GLuint)-1;
  }

  std::string line;
  while (std::getline(file, line)) {
    std::istringstream ssline(line);
    std::string token;
    ssline >> token;

    if (token == "mtllib") {
      std::getline(ssline >> std::ws, mtlFile);
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
      hasOBJNormals = true;
    } else if (token == "f") {      std::vector<FaceVertex> faceVerts;
      while (ssline >> token) {
        FaceVertex fv = {0, 0, 0};
        std::istringstream ss(token);
        std::string idx;
        if (std::getline(ss, idx, '/'))
          fv.vi = !idx.empty() ? stoi(idx) - 1 : 0;
        if (std::getline(ss, idx, '/'))
          fv.ti = !idx.empty() ? stoi(idx) - 1 : 0;
        if (std::getline(ss, idx)) fv.ni = !idx.empty() ? stoi(idx) - 1 : 0;
        faceVerts.push_back(fv);
      }
      for (int i = 1; i + 1 < (int)faceVerts.size(); i++) {
        if (!hasOBJNormals) {
          glm::vec3 v0 = positions[faceVerts[0].vi];
          glm::vec3 v1 = positions[faceVerts[i].vi];
          glm::vec3 v2 = positions[faceVerts[i + 1].vi];
          glm::vec3 faceNormal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
          normals.push_back(faceNormal);
          int ni = (int)normals.size() - 1;
          faceVerts[0].ni = faceVerts[i].ni = faceVerts[i + 1].ni = ni;
        }
        pushVertex(vBuffer, positions, normals, texCoords, defaultColor,
                   faceVerts[0]);
        pushVertex(vBuffer, positions, normals, texCoords, defaultColor,
                   faceVerts[i]);
        pushVertex(vBuffer, positions, normals, texCoords, defaultColor,
                   faceVerts[i + 1]);
      }
    }
  }

  const int STRIDE = 11;
  GLuint VBO, VAO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vBuffer.size() * sizeof(GLfloat),
               vBuffer.data(), GL_STATIC_DRAW);
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof(GLfloat),
                        (GLvoid*)(0 * sizeof(GLfloat)));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof(GLfloat),
                        (GLvoid*)(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof(GLfloat),
                        (GLvoid*)(6 * sizeof(GLfloat)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, STRIDE * sizeof(GLfloat),
                        (GLvoid*)(9 * sizeof(GLfloat)));
  glEnableVertexAttribArray(3);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  nVertices = (int)(vBuffer.size() / STRIDE);
  return VAO;
}
