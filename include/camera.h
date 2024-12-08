﻿//
// Created by Mat on 11/28/2024.
//

#ifndef SAMPLES_OPENGL_CAMERA_H
#define SAMPLES_OPENGL_CAMERA_H
#include <glm/glm.hpp>

enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    RIGHT,
    LEFT,
    UP,
    DOWN
};


class Camera
{
private:
    glm::vec3 reverse_direction_ {};
    glm::vec3 follow_ {};
    glm::vec3 right_axis_ {};
    glm::vec3 up_axis_ = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 view_ {};
    const float camera_speed_ = 5.0f;
    const float camera_sprint_speed_ = 10.0f;
    float yaw_ = 90.0f;
    float pitch_ = 0.0f;
    float sensitivity_ = 0.1f;
    void ResetDirection();

public:
    bool is_sprinting_ = false;
    glm::vec3 position_{};
    glm::vec3 camera_front_ = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 camera_up_ {};

    Camera();
    explicit Camera(glm::vec3 &target);

    //moving camera
    void Move(Camera_Movement direction, float dt);
    void Update(int x_yaw, int y_pitch);

    //SET
    void SetFollowTo(const glm::vec3& target);
    void SetPositionTo(const glm::vec3& target);

    //GET
    [[nodiscard]] glm::mat4 view() const;
};

#endif //SAMPLES_OPENGL_CAMERA_H
