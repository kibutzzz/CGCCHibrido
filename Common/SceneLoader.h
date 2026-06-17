#pragma once

#include "SceneObject.h"

// SceneLoader builds SceneObjects from OBJ files using the loaders in Mesh.h.
// STB_IMAGE_IMPLEMENTATION must be defined in exactly one .cpp — it's defined
// in Mesh.h which is included transitively via SceneObject.h -> Mesh.h. Callers
// must include this header only once per translation unit.

static SceneObject makeObject(const std::string& assetsDir,
                              const std::string& objFilename) {
  SceneObject sceneObject;
  sceneObject.name = objFilename;
  std::string mtlFilename;
  sceneObject.vertexArrayObject = loadSimpleOBJ(
      assetsDir + objFilename, sceneObject.vertexCount, mtlFilename);
  if (!mtlFilename.empty()) {
    sceneObject.material = loadMTL(assetsDir + mtlFilename);
    if (!sceneObject.material.texName.empty())
      sceneObject.textureID =
          loadTexture(assetsDir + sceneObject.material.texName);
  }
  return sceneObject;
}
