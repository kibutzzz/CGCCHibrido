#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Scene.h"

static GLuint _loadTexture(const std::string& filePath) {
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int w, h, ch;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filePath.c_str(), &w, &h, &ch, 0);
    if (data) {
        GLenum fmt = (ch == 3) ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "SceneLoader: failed to load texture: " << filePath << "\n";
    }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;
}

static Material _loadMTL(const std::string& filePath) {
    Material mat;
    std::ifstream f(filePath);
    if (!f.is_open()) { std::cerr << "SceneLoader: cannot open MTL: " << filePath << "\n"; return mat; }
    std::string line;
    while (std::getline(f, line)) {
        std::istringstream ss(line); std::string tok; ss >> tok;
        if      (tok == "Ka") ss >> mat.ka.r >> mat.ka.g >> mat.ka.b;
        else if (tok == "Kd") ss >> mat.kd.r >> mat.kd.g >> mat.kd.b;
        else if (tok == "Ks") ss >> mat.ks.r >> mat.ks.g >> mat.ks.b;
        else if (tok == "Ns") ss >> mat.ns;
        else if (tok == "map_Kd") std::getline(ss >> std::ws, mat.texName);
    }
    if (glm::length(mat.ka) < 0.001f) mat.ka = glm::vec3(0.1f);
    if (glm::length(mat.kd) < 0.001f) mat.kd = glm::vec3(0.8f);
    if (glm::length(mat.ks) < 0.001f) mat.ks = glm::vec3(0.5f);
    if (mat.ns < 1.0f) mat.ns = 32.0f;
    return mat;
}

struct FaceVertex { int vi, ti, ni; };

static GLuint _loadOBJ(const std::string& filePath, int& nVertices, std::string& mtlFile) {
    std::vector<glm::vec3> pos; std::vector<glm::vec2> tex; std::vector<glm::vec3> nor;
    std::vector<GLfloat> buf;
    std::ifstream f(filePath);
    if (!f.is_open()) { std::cerr << "SceneLoader: cannot open OBJ: " << filePath << "\n"; return 0; }
    std::string line;
    while (std::getline(f, line)) {
        std::istringstream ss(line); std::string tok; ss >> tok;
        if      (tok == "mtllib") { std::getline(ss >> std::ws, mtlFile); }
        else if (tok == "v")  { glm::vec3 v; ss >> v.x >> v.y >> v.z; pos.push_back(v); }
        else if (tok == "vt") { glm::vec2 vt; ss >> vt.s >> vt.t; tex.push_back(vt); }
        else if (tok == "vn") { glm::vec3 vn; ss >> vn.x >> vn.y >> vn.z; nor.push_back(vn); }
        else if (tok == "f") {
            std::vector<FaceVertex> fv;
            while (ss >> tok) {
                FaceVertex v{0,0,0}; std::istringstream fs(tok); std::string idx;
                if (std::getline(fs, idx, '/')) v.vi = idx.empty() ? 0 : stoi(idx)-1;
                if (std::getline(fs, idx, '/')) v.ti = idx.empty() ? 0 : stoi(idx)-1;
                if (std::getline(fs, idx))      v.ni = idx.empty() ? 0 : stoi(idx)-1;
                fv.push_back(v);
            }
            auto push = [&](const FaceVertex& fv) {
                buf.push_back(pos[fv.vi].x); buf.push_back(pos[fv.vi].y); buf.push_back(pos[fv.vi].z);
                buf.push_back(1); buf.push_back(1); buf.push_back(1);
                buf.push_back(nor.empty() ? 0.f : nor[fv.ni].x);
                buf.push_back(nor.empty() ? 0.f : nor[fv.ni].y);
                buf.push_back(nor.empty() ? 0.f : nor[fv.ni].z);
                buf.push_back(tex.empty() ? 0.f : tex[fv.ti].s);
                buf.push_back(tex.empty() ? 0.f : tex[fv.ti].t);
            };
            for (int i = 1; i+1 < (int)fv.size(); i++) { push(fv[0]); push(fv[i]); push(fv[i+1]); }
        }
    }
    const int STRIDE = 11;
    GLuint VBO, VAO;
    glGenBuffers(1, &VBO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, buf.size()*sizeof(GLfloat), buf.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &VAO); glBindVertexArray(VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE*sizeof(GLfloat), (void*)0);                    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE*sizeof(GLfloat), (void*)(3*sizeof(GLfloat))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, STRIDE*sizeof(GLfloat), (void*)(6*sizeof(GLfloat))); glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, STRIDE*sizeof(GLfloat), (void*)(9*sizeof(GLfloat))); glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, 0); glBindVertexArray(0);
    nVertices = (int)(buf.size() / STRIDE);
    return VAO;
}

// Build a SceneObject from an OBJ file; caller sets position/rotation/scale/animation after.
static SceneObject makeObject(const std::string& assetsDir, const std::string& objName) {
    SceneObject obj;
    obj.name = objName;
    std::string mtlFile;
    obj.vao = _loadOBJ(assetsDir + objName, obj.nVertices, mtlFile);
    if (!mtlFile.empty()) {
        obj.material = _loadMTL(assetsDir + mtlFile);
        if (!obj.material.texName.empty())
            obj.textureID = _loadTexture(assetsDir + obj.material.texName);
    }
    return obj;
}
