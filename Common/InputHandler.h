#pragma once

#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>

#include "AnimationPath.h"
#include "Scene.h"

class Camera;

class InputHandler {
public:
    static void onKey(GLFWwindow* window, int key, int scancode, int action,
                      int mods, Scene& scene, Camera& camera) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);

        if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
            scene.selectNext();
            SceneObject& selectedObject = scene.selected();
            int waypointCount = selectedObject.animation
                ? (int)selectedObject.animation->controlPoints.size() : 0;
            std::cout << "Selected: " << selectedObject.name
                      << "  waypoints: " << waypointCount << "\n";
        }

        if (key == GLFW_KEY_P && action == GLFW_PRESS) {
            SceneObject& selectedObject = scene.selected();
            if (!selectedObject.animation)
                selectedObject.animation = std::make_unique<AnimationPath>();
            selectedObject.animation->addWaypoint(camera.pos);
            std::cout << "Waypoint added to " << selectedObject.name
                      << "  total: " << selectedObject.animation->controlPoints.size() << "\n";
        }
    }

    static void onMouseMove(double xpos, double ypos, Camera& camera) {
        camera.processMouse(xpos, ypos);
    }

    static void onScroll(double yoffset, Camera& camera) {
        camera.processScroll(yoffset);
    }
};
