//
// Created by Mat on 11/28/2024.
//

#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

Camera::Camera() {
    position_ = glm::vec3(0.0f, 0.0f, -10.0f);
    follow_ = glm::vec3(0.0f, 0.0f, 0.0f);
    ResetDirection();

    //set up right axis
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    right_axis_ = glm::normalize(glm::cross(up, reverse_direction_));
    camera_up_ = glm::cross(reverse_direction_, right_axis_);
}

void Camera::ResetDirection() {
    reverse_direction_ = glm::normalize(position_ - follow_);
}

glm::mat4 Camera::view() const {
    return view_;
}

void Camera::Move(const Camera_Movement direction, const float dt) {
    float speed = camera_speed_ * dt;
    if(is_sprinting_)
    {
        speed *= 6.0f;
    }

    switch (direction) {
        case FORWARD:
            position_ += speed * camera_front_;
            break;
        case BACKWARD:
            position_ -= speed * camera_front_;
            break;
        case LEFT:
            position_ -= glm::normalize(glm::cross(camera_front_, camera_up_)) * speed;
            break;
        case RIGHT:
            position_ += glm::normalize(glm::cross(camera_front_, camera_up_)) * speed;
            break;
        case UP:
            position_ += up_axis_ * speed;
            break;
        case DOWN:
            position_ -= up_axis_ * speed;
            break;
        default:
            break;
    }
    view_ = glm::lookAt(position_, position_ + camera_front_, camera_up_);
}

void Camera::Update(const int x_yaw, const int y_pitch)
{
    yaw_ += static_cast<float>(x_yaw) * sensitivity_;
    pitch_ -= static_cast<float>(y_pitch) * sensitivity_;
    if(pitch_ > 89.0f)
        pitch_ =  89.0f;
    if(pitch_ < -89.0f)
        pitch_ = -89.0f;
    reverse_direction_.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    reverse_direction_.y = sin(glm::radians(pitch_));
    reverse_direction_.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    camera_front_ = glm::normalize(reverse_direction_);

    view_ = glm::lookAt(position_, position_ + camera_front_, camera_up_);
}