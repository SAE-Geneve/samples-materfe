//
// Created by Mat on 11/28/2024.
//

#include <iostream>
#include <sstream>
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine.h"
#include "file_utility.h"
#include "scene.h"
#include "texture_loader.h"

namespace gpr

{
    class ThreeDScene final : public Scene
    {
    public:
        void Begin() override;
        void End() override;
        void Update(float dt) override;
    private:
        GLuint vertexShader_ = 0;
        GLuint fragmentShader_ = 0;
        GLuint program_ = 0;
        GLuint vao_ = 0;
    };

    void ThreeDScene::Begin()
    {
        //Load shaders
        const auto vertexContent = LoadFile("data/shaders/3D_scene/cube.vert");
        const auto* ptr = vertexContent.data();
        vertexShader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader_, 1, &ptr, nullptr);
        glCompileShader(vertexShader_);
        //Check success status of shader compilation
        GLint success;
        glGetShaderiv(vertexShader_, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            std::cerr << "Error while loading vertex shader\n";
        }
        const auto fragmentContent = LoadFile("data/shaders/3D_scene/cube.frag");
        ptr = fragmentContent.data();
        fragmentShader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader_, 1, &ptr, nullptr);
        glCompileShader(fragmentShader_);
        //Check success status of shader compilation

        glGetShaderiv(fragmentShader_, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            std::cerr << "Error while loading fragment shader\n";
        }
        //Load program/pipeline
        program_ = glCreateProgram();
        glAttachShader(program_, vertexShader_);
        glAttachShader(program_, fragmentShader_);
        glLinkProgram(program_);
        //Check if shader program was linked correctly

        glGetProgramiv(program_, GL_LINK_STATUS, &success);
        if (!success)
        {
            std::cerr << "Error while linking shader program\n";
        }
        //Empty vao
        glCreateVertexArrays(1, &vao_);
    }

    void ThreeDScene::End()
    {
        //Unload program/pipeline
        glDeleteProgram(program_);

        glDeleteShader(vertexShader_);
        glDeleteShader(fragmentShader_);

        glDeleteVertexArrays(1, &vao_);
    }

    void ThreeDScene::Update(float dt)
    {
        //Draw program
        glUseProgram(program_);

        auto model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        glBindVertexArray(vao_);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
}
int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
    gpr::ThreeDScene scene;
    gpr::Engine engine(&scene);
    engine.Run();

    return EXIT_SUCCESS;
}