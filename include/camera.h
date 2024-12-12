//
// Created by Mat on 11/28/2024.
//

#ifndef SAMPLES_OPENGL_CAMERA_H
#define SAMPLES_OPENGL_CAMERA_H

#include <glm/glm.hpp>

#include <array>

#include "volumes.h"

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    RIGHT,
    LEFT,
    UP,
    DOWN
};

class Camera {
private:
    glm::vec3 reverse_direction_{};
    glm::vec3 follow_{};
    glm::vec3 up_axis_ = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 view_{};
    const float camera_speed_ = 5.0f;
    float yaw_ = 90.0f;
    float pitch_ = 0.0f;
    float sensitivity_ = 0.1f;

    void ResetDirection();

public:
    glm::vec3 position_{};
    glm::vec3 camera_front_ = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 camera_up_{};
    glm::vec3 right_axis_{};
    bool is_sprinting_ = false;

    Camera();

    //moving camera
    void Move(Camera_Movement direction, float dt);

    //what camera does each frame
    void Update(int x_yaw, int y_pitch);

    //GET
    [[nodiscard]] glm::mat4 view() const;
};

struct Plane {
    glm::vec3 normal = {0.f, 1.f, 0.f}; // unit vector
    float distance = 0.f;        // Distance with origin

    Plane() = default;

    Plane(const glm::vec3 &p1, const glm::vec3 &norm)
            : normal(glm::normalize(norm)),
              distance(glm::dot(normal, p1)) {}

    [[nodiscard]] float GetSignedDistanceToPlaneFromACircle(const Sphere &circle) const {
        return glm::dot(normal, circle.center()) - distance;
    }
};

struct Frustum {
private:
    Plane topFace;
    Plane bottomFace;

    Plane rightFace;
    Plane leftFace;

    Plane farFace;
    Plane nearFace;


public:

    /**
     *
     * @param cam -> camera
     * @param aspect -> ratio of the window
     * @param fovY -> vertical angle for the sight of vision, radians
     * @param zNear -> distance between cam and far plane
     * @param zFar -> distance between cam and close plane
     */
    void CreateFrustumFromCamera(const Camera &cam, float aspect, float fovY, float zNear, float zFar) {
        const float halfVSide = zFar * tanf(fovY * 0.5f);
        const float halfHSide = halfVSide * aspect;
        const glm::vec3 frontMultFar = zFar * cam.camera_front_;

        nearFace = {cam.position_ + zNear * cam.camera_front_, cam.camera_front_};
        farFace = {cam.position_ + frontMultFar, -cam.camera_front_};
        rightFace = {cam.position_, glm::cross(frontMultFar - cam.right_axis_ * halfHSide, cam.camera_up_)};
        leftFace = {cam.position_, glm::cross(cam.camera_up_, frontMultFar + cam.right_axis_ * halfHSide)};
        topFace = {cam.position_, glm::cross(cam.right_axis_, frontMultFar - cam.camera_up_ * halfVSide)};
        bottomFace = {cam.position_, glm::cross(frontMultFar + cam.camera_up_ * halfVSide, cam.right_axis_)};
    }


    [[nodiscard]] bool IsSphereInFrustum(const Sphere &sphere) const {
        //for each plan
        std::array<Plane, 6> allPlanes = {topFace, bottomFace, rightFace, leftFace, nearFace, farFace};


        for (const auto &plane: allPlanes) {
            //calculation of the distance between center of the circle and plane
            float distance = plane.GetSignedDistanceToPlaneFromACircle(sphere);

            //if the sphere is outside a plan, she is out

            if (distance < -sphere.radius()) {
                return false;
            }
        }
        return true;
    }
    [[nodiscard]] bool IsCubeInFrustum(const glm::vec3 &center, float halfSize) const {
        //these are the vertices equivalent to the halfSize of each part of the cube using its center and a half size
        glm::vec3 vertices[8] = {
                center + glm::vec3(-halfSize, -halfSize, -halfSize),
                center + glm::vec3(-halfSize, -halfSize,  halfSize),
                center + glm::vec3(-halfSize,  halfSize, -halfSize),
                center + glm::vec3(-halfSize,  halfSize,  halfSize),
                center + glm::vec3( halfSize, -halfSize, -halfSize),
                center + glm::vec3( halfSize, -halfSize,  halfSize),
                center + glm::vec3( halfSize,  halfSize, -halfSize),
                center + glm::vec3( halfSize,  halfSize,  halfSize)
        };

        // Parcourir les 6 plans du frustum
        const Plane planes[] = {topFace, bottomFace, rightFace, leftFace, nearFace, farFace};
        for (const auto &plane : planes) {
            bool allOutside = true;

            // Vérifier si tous les sommets du cube sont derrière ce plan
            for (const auto &vertex : vertices) {
                float distance = glm::dot(plane.normal, vertex) - plane.distance;
                if (distance >= 0) {
                    allOutside = false;
                    break; // Au moins un sommet est devant ce plan
                }
            }

            // Si tous les sommets sont derrière ce plan, le cube est hors du frustum
            if (allOutside) {
                return false;
            }
        }

        // Si le cube n'est derrière aucun plan, il est au moins partiellement dans le frustum
        return true;
    }

};


#endif //SAMPLES_OPENGL_CAMERA_H