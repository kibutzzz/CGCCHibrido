#pragma once

#include <glm/glm.hpp>
#include <vector>

struct AnimationPath {
  std::vector<glm::vec3> controlPoints;
  int waypointIndex = 0;
  float segmentProgress = 0.0f;
  float speed = 2.0f;

  void update(float deltaTime) {
    if (controlPoints.size() < 2) return;
    float segmentLength =
        glm::distance(controlPoints[waypointIndex], controlPoints[nextIndex()]);
    if (segmentLength < 1e-4f) {
      advanceWaypoint();
      return;
    }
    segmentProgress += speed * deltaTime / segmentLength;
    if (segmentProgress >= 1.0f) {
      segmentProgress = 0.0f;
      advanceWaypoint();
    }
  }

  glm::vec3 currentPosition() const {
    if (controlPoints.empty()) return glm::vec3(0.0f);
    if (controlPoints.size() == 1) return controlPoints[0];
    return glm::mix(controlPoints[waypointIndex], controlPoints[nextIndex()],
                    segmentProgress);
  }

  void addWaypoint(glm::vec3 point) { controlPoints.push_back(point); }
  int waypointCount() const { return (int)controlPoints.size(); }

 private:
  int nextIndex() const {
    return (waypointIndex + 1) % (int)controlPoints.size();
  }
  void advanceWaypoint() { waypointIndex = nextIndex(); }
};
