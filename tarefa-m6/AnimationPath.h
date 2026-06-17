#pragma once

#include <glm/glm.hpp>
#include <vector>

struct AnimationPath {
    std::vector<glm::vec3> controlPoints;
    int index = 0;
    float t = 0.0f;
    float speed = 2.0f;

    void update(float dt) {
        if (controlPoints.size() < 2) return;
        glm::vec3 curr = controlPoints[index];
        glm::vec3 next = controlPoints[(index + 1) % controlPoints.size()];
        float dist = glm::distance(curr, next);
        if (dist < 1e-4f) { advance(); return; }
        t += speed * dt / dist;
        if (t >= 1.0f) { t = 0.0f; advance(); }
    }

    glm::vec3 currentPosition() const {
        if (controlPoints.empty()) return glm::vec3(0.0f);
        if (controlPoints.size() == 1) return controlPoints[0];
        glm::vec3 curr = controlPoints[index];
        glm::vec3 next = controlPoints[(index + 1) % controlPoints.size()];
        return glm::mix(curr, next, t);
    }

    void addPoint(glm::vec3 p) { controlPoints.push_back(p); }

private:
    void advance() { index = (index + 1) % (int)controlPoints.size(); }
};
