#pragma once
#include "EasyVKStart.h"

class Camera {
public:
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 invProj;
    glm::mat4 invView;
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    float fov = 45.f;
    float znear = 0.1f;
    float zfar = 100.f;
    void SetProj(int width, int height)
    {
        projection = glm::perspective(fov, float(width) / float(height), znear, zfar);
        projection[1][1] *= -1; // Invert Y axis

        invProj = glm::inverse(projection);

        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        invView = glm::transpose(view);
    }
    void updateView()
    {
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        invView = glm::transpose(view);
    }
    void setCameraPos(glm::vec3 pos)
    {
        cameraPos = pos;
    }
};