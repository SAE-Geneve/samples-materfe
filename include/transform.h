﻿//
// Created by Mat on 12/9/2024.
//

#ifndef SAMPLES_OPENGL_TRANSFORM_H
#define SAMPLES_OPENGL_TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform
{
protected:
    //Local space information
    glm::vec3 m_pos = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_eulerRot = { 0.0f, 0.0f, 0.0f }; //In degrees
    glm::vec3 m_scale = { 1.0f, 1.0f, 1.0f };

    //Global space information concatenate in matrix
    glm::mat4 m_modelMatrix = glm::mat4(1.0f);

    //Dirty flag
    bool m_isDirty = true;

protected:
    glm::mat4 getLocalModelMatrix()
    {
        const glm::mat4 transformX = glm::rotate(glm::mat4(1.0f), glm::radians(m_eulerRot.x), glm::vec3(1.0f, 0.0f, 0.0f));
        const glm::mat4 transformY = glm::rotate(glm::mat4(1.0f), glm::radians(m_eulerRot.y), glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::mat4 transformZ = glm::rotate(glm::mat4(1.0f), glm::radians(m_eulerRot.z), glm::vec3(0.0f, 0.0f, 1.0f));

        // Y * X * Z
        const glm::mat4 rotationMatrix = transformY * transformX * transformZ;

        // translation * rotation * scale (also know as TRS matrix)
        return glm::translate(glm::mat4(1.0f), m_pos) * rotationMatrix * glm::scale(glm::mat4(1.0f), m_scale);
    }
public:

    void computeModelMatrix()
    {
        m_modelMatrix = getLocalModelMatrix();
        m_isDirty = false;
    }

    void computeModelMatrix(const glm::mat4& parentGlobalModelMatrix)
    {
        m_modelMatrix = parentGlobalModelMatrix * getLocalModelMatrix();
        m_isDirty = false;
    }

    void setLocalPosition(const glm::vec3& newPosition)
    {
        m_pos = newPosition;
        m_isDirty = true;
    }

    void setLocalRotation(const glm::vec3& newRotation)
    {
        m_eulerRot = newRotation;
        m_isDirty = true;
    }

    void setLocalScale(const glm::vec3& newScale)
    {
        m_scale = newScale;
        m_isDirty = true;
    }

    [[nodiscard]] const glm::vec3& getGlobalPosition() const
    {
        return m_modelMatrix[3];
    }

    [[nodiscard]] const glm::vec3& getLocalPosition() const
    {
        return m_pos;
    }

    [[nodiscard]] const glm::vec3& getLocalRotation() const
    {
        return m_eulerRot;
    }

    [[nodiscard]] const glm::vec3& getLocalScale() const
    {
        return m_scale;
    }

    [[nodiscard]] const glm::mat4& getModelMatrix() const
    {
        return m_modelMatrix;
    }

    [[nodiscard]] glm::vec3 getRight() const
    {
        return m_modelMatrix[0];
    }


    [[nodiscard]] glm::vec3 getUp() const
    {
        return m_modelMatrix[1];
    }

    [[nodiscard]] glm::vec3 getBackward() const
    {
        return m_modelMatrix[2];
    }

    [[nodiscard]] glm::vec3 getForward() const
    {
        return -m_modelMatrix[2];
    }

    [[nodiscard]] glm::vec3 getGlobalScale() const
    {
        return { glm::length(getRight()), glm::length(getUp()), glm::length(getBackward()) };
    }

    [[nodiscard]] bool isDirty() const
    {
        return m_isDirty;
    }
};
#endif //SAMPLES_OPENGL_TRANSFORM_H
