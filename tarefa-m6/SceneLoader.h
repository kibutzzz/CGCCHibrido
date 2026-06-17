#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Scene.h"

static GLuint uploadTexture(const std::string& filePath) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* pixels = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
    if (pixels) {
        GLenum format = (channels == 3) ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "SceneLoader: failed to load texture: " << filePath << "\n";
    }
    stbi_image_free(pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureID;
}

static Material parseMTL(const std::string& filePath) {
    Material mat;
    std::ifstream file(filePath);
    if (!file.is_open()) { std::cerr << "SceneLoader: cannot open MTL: " << filePath << "\n"; return mat; }
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream lineStream(line);
        std::string token;
        lineStream >> token;
        if      (token == "Ka") lineStream >> mat.ka.r >> mat.ka.g >> mat.ka.b;
        else if (token == "Kd") lineStream >> mat.kd.r >> mat.kd.g >> mat.kd.b;
        else if (token == "Ks") lineStream >> mat.ks.r >> mat.ks.g >> mat.ks.b;
        else if (token == "Ns") lineStream >> mat.ns;
        else if (token == "map_Kd") std::getline(lineStream >> std::ws, mat.texName);
    }
    if (glm::length(mat.ka) < 0.001f) mat.ka = glm::vec3(0.1f);
    if (glm::length(mat.kd) < 0.001f) mat.kd = glm::vec3(0.8f);
    if (glm::length(mat.ks) < 0.001f) mat.ks = glm::vec3(0.5f);
    if (mat.ns < 1.0f) mat.ns = 32.0f;
    return mat;
}

struct FaceVertex {
    int posIndex;
    int texIndex;
    int normIndex;
};

static GLuint parseOBJ(const std::string& filePath, int& nVertices, std::string& mtlFilename) {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<GLfloat>   vertexBuffer;

    std::ifstream file(filePath);
    if (!file.is_open()) { std::cerr << "SceneLoader: cannot open OBJ: " << filePath << "\n"; return 0; }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream lineStream(line);
        std::string token;
        lineStream >> token;

        if      (token == "mtllib") { std::getline(lineStream >> std::ws, mtlFilename); }
        else if (token == "v")  { glm::vec3 p; lineStream >> p.x >> p.y >> p.z; positions.push_back(p); }
        else if (token == "vt") { glm::vec2 uv; lineStream >> uv.s >> uv.t; texCoords.push_back(uv); }
        else if (token == "vn") { glm::vec3 n; lineStream >> n.x >> n.y >> n.z; normals.push_back(n); }
        else if (token == "f") {
            std::vector<FaceVertex> faceVerts;
            std::string vertToken;
            while (lineStream >> vertToken) {
                FaceVertex vertex{0, 0, 0};
                std::istringstream vertStream(vertToken);
                std::string index;
                if (std::getline(vertStream, index, '/')) vertex.posIndex  = index.empty() ? 0 : stoi(index) - 1;
                if (std::getline(vertStream, index, '/')) vertex.texIndex  = index.empty() ? 0 : stoi(index) - 1;
                if (std::getline(vertStream, index))      vertex.normIndex = index.empty() ? 0 : stoi(index) - 1;
                faceVerts.push_back(vertex);
            }
            auto pushVertex = [&](const FaceVertex& v) {
                vertexBuffer.push_back(positions[v.posIndex].x);
                vertexBuffer.push_back(positions[v.posIndex].y);
                vertexBuffer.push_back(positions[v.posIndex].z);
                vertexBuffer.push_back(1.0f); vertexBuffer.push_back(1.0f); vertexBuffer.push_back(1.0f);
                vertexBuffer.push_back(normals.empty()   ? 0.f : normals[v.normIndex].x);
                vertexBuffer.push_back(normals.empty()   ? 0.f : normals[v.normIndex].y);
                vertexBuffer.push_back(normals.empty()   ? 0.f : normals[v.normIndex].z);
                vertexBuffer.push_back(texCoords.empty() ? 0.f : texCoords[v.texIndex].s);
                vertexBuffer.push_back(texCoords.empty() ? 0.f : texCoords[v.texIndex].t);
            };
            for (int i = 1; i + 1 < (int)faceVerts.size(); i++) {
                pushVertex(faceVerts[0]);
                pushVertex(faceVerts[i]);
                pushVertex(faceVerts[i + 1]);
            }
        }
    }

    const int STRIDE = 11;
    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size() * sizeof(GLfloat), vertexBuffer.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof(GLfloat), (void*)0);                           glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));       glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));       glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, STRIDE * sizeof(GLfloat), (void*)(9 * sizeof(GLfloat)));       glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    nVertices = (int)(vertexBuffer.size() / STRIDE);
    return VAO;
}

static SceneObject makeObject(const std::string& assetsDir, const std::string& objFilename) {
    SceneObject obj;
    obj.name = objFilename;
    std::string mtlFilename;
    obj.vao = parseOBJ(assetsDir + objFilename, obj.nVertices, mtlFilename);
    if (!mtlFilename.empty()) {
        obj.material = parseMTL(assetsDir + mtlFilename);
        if (!obj.material.texName.empty())
            obj.textureID = uploadTexture(assetsDir + obj.material.texName);
    }
    return obj;
}
