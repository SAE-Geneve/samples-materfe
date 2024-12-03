//
// Created by Mat on 11/28/2024.
//

#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera() {
    position_ = glm::vec3(0.0f, 0.0f, 0.0f);
    follow_ = glm::vec3(0.0f, 0.0f, 0.0f);
    ResetDirection();

    //set up right axis
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    right_axis_ = glm::normalize(glm::cross(up, reverse_direction_));
    camera_up_ = glm::cross(reverse_direction_, right_axis_);
}
Camera::Camera(glm::vec3 &target) {
    position_ = glm::vec3(0.0f, 0.0f, 0.0f);
    follow_ = target;
    ResetDirection();

    //set up right axis
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    right_axis_ = glm::normalize(glm::cross(up, reverse_direction_));
    camera_up_ = glm::cross(reverse_direction_, right_axis_);
}

void Camera::ResetDirection() {
    reverse_direction_ = glm::normalize(position_ - follow_);
}

void Camera::SetFollowTo (const glm::vec3 &target) {
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    view_ = glm::lookAt(position_,
                        target,
                       up);
}

void Camera::SetPositionTo(const glm::vec3 &target) {
    position_ = target;
}

glm::mat4 Camera::view() const {
    return view_;
}