#pragma once

#include "Mesh.h"
#include "Scene.h"
#include "ShaderProgram.h"

class PhongShader : public ShaderProgram {
 public:
  PhongShader(const std::string& vertPath, const std::string& fragPath)
      : ShaderProgram(vertPath, fragPath) {}

  void setMaterial(const Material& m) const {
    setVec3("ka", m.ka);
    setVec3("kd", m.kd);
    setVec3("ks", m.ks);
    setFloat("ns", m.ns);
  }

  void setLight(const PointLight& light) const {
    setVec3("lightPosition", light.position);
    setVec3("lightColor", light.color * light.intensity);
  }

  void setCameraPos(const glm::vec3& pos) const {
    setVec3("cameraPosition", pos);
  }
};
