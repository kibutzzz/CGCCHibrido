#pragma once

#include <GLFW/glfw3.h>
#include <iostream>

#include "Scene.h"

// Camera is passed in from Common/Camera.h — forward-declared here
class Camera;

class InputHandler {
public:
    static void onKey(GLFWwindow* window, int key, int scancode, int action,
                      int mods, Scene& scene, Camera& camera) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);

        if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
            scene.selectNext();
            auto& obj = scene.selected();
            int wps = obj.animation ? (int)obj.animation->controlPoints.size() : 0;
            std::cout << "Selected: " << obj.name << "  waypoints: " << wps << "\n";
        }

        if (key == GLFW_KEY_P && action == GLFW_PRESS) {
            auto& obj = scene.selected();
            if (!obj.animation)
                obj.animation = std::make_unique<AnimationPath>();
            obj.animation->addPoint(camera.pos);
            std::cout << "Waypoint added to " << obj.name
                      << "  total: " << obj.animation->controlPoints.size() << "\n";
        }
    }

    static void onMouseMove(double xpos, double ypos, Camera& camera) {
        camera.processMouse(xpos, ypos);
    }

    static void onScroll(double yoffset, Camera& camera) {
        camera.processScroll(yoffset);
    }
};
