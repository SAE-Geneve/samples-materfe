//
// Created by Mat on 12/9/2024.
//

#ifndef SAMPLES_OPENGL_VOLUMES_H
#define SAMPLES_OPENGL_VOLUMES_H

struct Sphere {
private:
    glm::vec3 center_{0.f, 0.f, 0.f};
    float radius_{10.f};

public:
    Sphere(const glm::vec3 &inCenter, float inRadius) : center_{inCenter}, radius_{inRadius} {}

    [[nodiscard]] glm::vec3 center() const{return center_;}
    [[nodiscard]] float radius() const{return radius_;}
};





//
//
//struct SquareAABB : public BoundingVolume {
//    glm::vec3 center{0.f, 0.f, 0.f};
//    float extent{0.f};
//
//    SquareAABB(const glm::vec3 &inCenter, float inExtent)
//            : BoundingVolume{}, center{inCenter}, extent{inExtent} {}
//
//    [[nodiscard]] bool isOnOrForwardPlane(const Plane &plane) const final {
//        // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
//        const float r = extent * (std::abs(plane.normal.x) + std::abs(plane.normal.y) + std::abs(plane.normal.z));
//        return -r <= plane.GetSignedDistanceToPlaneFromACircle(center);
//    }
//
//    [[nodiscard]] bool isOnFrustum(const Frustum &camFrustum, const Transform &transform) const final {
//        //Get global scale thanks to our transform
//        const glm::vec3 globalCenter{transform.getModelMatrix() * glm::vec4(center, 1.f)};
//
//        // Scaled orientation
//        const glm::vec3 right = transform.getRight() * extent;
//        const glm::vec3 up = transform.getUp() * extent;
//        const glm::vec3 forward = transform.getForward() * extent;
//
//        const float newIi = std::abs(glm::dot(glm::vec3{1.f, 0.f, 0.f}, right)) +
//                            std::abs(glm::dot(glm::vec3{1.f, 0.f, 0.f}, up)) +
//                            std::abs(glm::dot(glm::vec3{1.f, 0.f, 0.f}, forward));
//
//        const float newIj = std::abs(glm::dot(glm::vec3{0.f, 1.f, 0.f}, right)) +
//                            std::abs(glm::dot(glm::vec3{0.f, 1.f, 0.f}, up)) +
//                            std::abs(glm::dot(glm::vec3{0.f, 1.f, 0.f}, forward));
//
//        const float newIk = std::abs(glm::dot(glm::vec3{0.f, 0.f, 1.f}, right)) +
//                            std::abs(glm::dot(glm::vec3{0.f, 0.f, 1.f}, up)) +
//                            std::abs(glm::dot(glm::vec3{0.f, 0.f, 1.f}, forward));
//
//        const SquareAABB globalAABB(globalCenter, std::fmax(std::fmax(newIi, newIj), newIk));
//
//        return (globalAABB.isOnOrForwardPlane(camFrustum.leftFace) &&
//                globalAABB.isOnOrForwardPlane(camFrustum.rightFace) &&
//                globalAABB.isOnOrForwardPlane(camFrustum.topFace) &&
//                globalAABB.isOnOrForwardPlane(camFrustum.bottomFace) &&
//                globalAABB.isOnOrForwardPlane(camFrustum.nearFace) &&
//                globalAABB.isOnOrForwardPlane(camFrustum.farFace));
//    };
//};
//
//struct AABB : public BoundingVolume {
//    glm::vec3 center{0.f, 0.f, 0.f};
//    glm::vec3 extents{0.f, 0.f, 0.f};
//
//    AABB(const glm::vec3 &min, const glm::vec3 &max)
//            : BoundingVolume{}, center{(max + min) * 0.5f},
//              extents{max.x - center.x, max.y - center.y, max.z - center.z} {}
//
//    AABB(const glm::vec3 &inCenter, float iI, float iJ, float iK)
//            : BoundingVolume{}, center{inCenter}, extents{iI, iJ, iK} {}
//
//    [[maybe_unused]] [[nodiscard]] std::array<glm::vec3, 8> getVertices() const {
//        std::array<glm::vec3, 8> vertices{};
//        vertices[0] = {center.x - extents.x, center.y - extents.y, center.z - extents.z};
//        vertices[1] = {center.x + extents.x, center.y - extents.y, center.z - extents.z};
//        vertices[2] = {center.x - extents.x, center.y + extents.y, center.z - extents.z};
//        vertices[3] = {center.x + extents.x, center.y + extents.y, center.z - extents.z};
//        vertices[4] = {center.x - extents.x, center.y - extents.y, center.z + extents.z};
//        vertices[5] = {center.x + extents.x, center.y - extents.y, center.z + extents.z};
//        vertices[6] = {center.x - extents.x, center.y + extents.y, center.z + extents.z};
//        vertices[7] = {center.x + extents.x, center.y + extents.y, center.z + extents.z};
//        return vertices;
//    }
//
//
//    [[nodiscard]] bool isOnOrForwardPlane(const Plane &plane) const final {
//        // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
//        const float r = extents.x * std::abs(plane.normal.x) + extents.y * std::abs(plane.normal.y) +
//                        extents.z * std::abs(plane.normal.z);
//
//        return -r <= plane.GetSignedDistanceToPlaneFromACircle(center);
//    }
//
//    [[nodiscard]] bool isOnFrustum(const Frustum &camFrustum, const Transform &transform) const final {
//        //Get global scale thanks to our transform
//        const glm::vec3 globalCenter{transform.getModelMatrix() * glm::vec4(center, 1.f)};
//
//        // Scaled orientation
//        const glm::vec3 right = transform.getRight() * extents.x;
//        const glm::vec3 up = transform.getUp() * extents.y;
//        const glm::vec3 forward = transform.getForward() * extents.z;
//
//        const float newIi = std::abs(glm::dot(glm::vec3{1.f, 0.f, 0.f}, right)) +
//                            std::abs(glm::dot(glm::vec3{1.f, 0.f, 0.f}, up)) +
//                            std::abs(glm::dot(glm::vec3{1.f, 0.f, 0.f}, forward));
//
//        const float newIj = std::abs(glm::dot(glm::vec3{0.f, 1.f, 0.f}, right)) +
//                            std::abs(glm::dot(glm::vec3{0.f, 1.f, 0.f}, up)) +
//                            std::abs(glm::dot(glm::vec3{0.f, 1.f, 0.f}, forward));
//
//        const float newIk = std::abs(glm::dot(glm::vec3{0.f, 0.f, 1.f}, right)) +
//                            std::abs(glm::dot(glm::vec3{0.f, 0.f, 1.f}, up)) +
//                            std::abs(glm::dot(glm::vec3{0.f, 0.f, 1.f}, forward));
//
//        const AABB globalAABB(globalCenter, newIi, newIj, newIk);
//
//        return (globalAABB.isOnOrForwardPlane(camFrustum.leftFace) &&
//                globalAABB.isOnOrForwardPlane(camFrustum.rightFace) &&
//                globalAABB.isOnOrForwardPlane(camFrustum.topFace) &&
//                globalAABB.isOnOrForwardPlane(camFrustum.bottomFace) &&
//                globalAABB.isOnOrForwardPlane(camFrustum.nearFace) &&
//                globalAABB.isOnOrForwardPlane(camFrustum.farFace));
//    };
//};


#endif //SAMPLES_OPENGL_VOLUMES_H
