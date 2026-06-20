#pragma once

#include <nlohmann/json.hpp>

#include <fstream>
#include <iostream>

#include "Camera.h"
#include "Scene.h"
#include "SceneObject.h"

static SceneObject makeObject(const std::string& assetsDir,
                              const std::string& objFilename) {
  SceneObject obj;
  obj.name = objFilename;
  std::string mtlFilename;
  obj.vertexArrayObject =
      loadSimpleOBJ(assetsDir + objFilename, obj.vertexCount, mtlFilename);
  if (!mtlFilename.empty()) {
    obj.material = loadMTL(assetsDir + mtlFilename);
    if (!obj.material.texName.empty())
      obj.textureID = loadTexture(assetsDir + obj.material.texName);
  }
  return obj;
}

static glm::vec3 toVec3(const nlohmann::json& j) {
  return {j[0].get<float>(), j[1].get<float>(), j[2].get<float>()};
}

static void loadScene(const std::string& scenePath,
                      const std::string& assetsDir, Scene& scene,
                      Camera& camera) {
  std::ifstream file(scenePath);
  if (!file.is_open()) {
    std::cerr << "SceneLoader: cannot open " << scenePath << "\n";
    return;
  }

  nlohmann::json root = nlohmann::json::parse(file);

  const auto& jsonCamera = root["camera"];
  camera = Camera(toVec3(jsonCamera["position"]));
  camera.front = glm::normalize(toVec3(jsonCamera["front"]));
  camera.up = toVec3(jsonCamera["up"]);
  camera.fov = jsonCamera["fov"].get<float>();
  camera.near = jsonCamera["near"].get<float>();
  camera.far = jsonCamera["far"].get<float>();

  for (const auto& jsonLight : root["lights"]) {
    PointLight light;
    light.position = toVec3(jsonLight["position"]);
    light.color = toVec3(jsonLight["color"]);
    light.intensity = jsonLight["intensity"].get<float>();
    scene.lights.push_back(light);
  }

  for (const auto& jsonObject : root["objects"]) {
    SceneObject object = makeObject(assetsDir, jsonObject["obj"].get<std::string>());
    object.position = toVec3(jsonObject["position"]);
    object.rotation = toVec3(jsonObject["rotation"]);
    object.scale = jsonObject["scale"].get<float>();
    if (jsonObject.contains("animation")) {
      object.hasAnimation = true;
      for (const auto& point : jsonObject["animation"]["points"])
        object.animation.controlPoints.push_back(toVec3(point));
      object.position = object.animation.currentPosition();
    }
    scene.objects.push_back(object);
  }

  if (!scene.objects.empty())
    scene.objects[0].selected = true;
}
