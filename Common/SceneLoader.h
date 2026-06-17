#pragma once

#include "SceneObject.h"

// SceneLoader builds SceneObjects from OBJ files using the loaders in Mesh.h.
// STB_IMAGE_IMPLEMENTATION must be defined in exactly one .cpp — it's defined in Mesh.h
// which is included transitively via SceneObject.h -> Mesh.h. Callers must include
// this header only once per translation unit.

static SceneObject makeObject(const std::string& assetsDir, const std::string& objFilename) {
    SceneObject obj;
    obj.name = objFilename;
    std::string mtlFilename;
    obj.vao = loadSimpleOBJ(assetsDir + objFilename, obj.nVertices, mtlFilename);
    if (!mtlFilename.empty()) {
        obj.material = loadMTL(assetsDir + mtlFilename);
        if (!obj.material.texName.empty())
            obj.textureID = loadTexture(assetsDir + obj.material.texName);
    }
    return obj;
}
