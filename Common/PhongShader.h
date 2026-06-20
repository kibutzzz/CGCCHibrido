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

  void setLights(const std::vector<PointLight>& lights) const {
    setInt("numLights", (int)lights.size());
    for (int i = 0; i < (int)lights.size(); i++) {
      setVec3("lightPositions[" + std::to_string(i) + "]", lights[i].position);
      setVec3("lightColors[" + std::to_string(i) + "]", lights[i].color * lights[i].intensity);
    }
  }

  void setCameraPos(const glm::vec3& pos) const {
    setVec3("cameraPosition", pos);
  }

  void setSelectionBrightness(float b) const {
    setFloat("selectionBrightness", b);
  }
};
