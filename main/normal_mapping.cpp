//
// Created by Mat on 1/8/2025.
//

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

#include "engine.h"
#include "scene.h"
#include "load3D/texture_loader.h"
#include "camera.h"
#include "open_gl_data_structure/vao.h"
#include "open_gl_data_structure/vbo.h"
#include "file_utility.h"

#include <sstream>
#include <iostream>
#include <array>
#include <numbers>

namespace gpr {
    class NormalMapping final : public Scene {
    public:
        void Begin() override;

        void End() override;

        void Update(float dt) override;

        void OnEvent(const SDL_Event &event, float dt) override;

    private:
        std::array<glm::vec3, 10> all_cubes_{};
        unsigned int texture_[3] = {0, 0, 0};
        unsigned int brickwall_ = -1;
        unsigned int brickwall_normal_ = -1;
        float elapsed_time_ = 0.0f;

        GLuint vertexShader_ = 0;
        GLuint vertexMappingShader_ = 0;

        GLuint fragmentShader_ = 0;
        GLuint fragmentMappingShader_ = 0;

        GLuint program_ = 0;
        GLuint program_mapping_ = 0;

        Camera *camera_ = nullptr;
        VAO small_cube_vao_{};
        VAO quad_vao_{};
        VBO quad_vbo{};

        void SetAndBindTextures() const;

        void SetAndDrawMultipleCubes() const;

        void SetTheCubes();

        void SetUniformsProgram(const glm::vec3 &lightPos) const;

        void SetView(const glm::mat4 &projection, GLuint &program) const;

        void SetUniformsProgramMapping() const;

        void SetTextureNormalMapBrickwall() const;

        void RenderQuad();
    };

    void NormalMapping::OnEvent(const SDL_Event &event, const float dt) {
        // Get keyboard state
        const Uint8 *state = SDL_GetKeyboardState(nullptr);

        // Camera controls
        if (state[SDL_SCANCODE_W]) {
            camera_->Move(FORWARD, dt);
        }
        if (state[SDL_SCANCODE_S]) {
            camera_->Move(BACKWARD, dt);
        }
        if (state[SDL_SCANCODE_A]) {
            camera_->Move(LEFT, dt);
        }
        if (state[SDL_SCANCODE_D]) {
            camera_->Move(RIGHT, dt);
        }
        if (state[SDL_SCANCODE_SPACE]) {
            camera_->Move(UP, dt);
        }
        if (state[SDL_SCANCODE_LCTRL]) {
            camera_->Move(DOWN, dt);
        }
        if (state[SDL_SCANCODE_LSHIFT]) {
            camera_->is_sprinting_ = !camera_->is_sprinting_;
        }

        int mouseX, mouseY;
        const Uint32 mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
        if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            camera_->Update(mouseX, mouseY);
        }
    }

    void NormalMapping::SetTheCubes() {
        for (std::size_t i = 0; i < all_cubes_.size(); i++) {
            all_cubes_[i] = glm::vec3(1 * i, 1 * i, 1 * i);
        }
    }

    void NormalMapping::Begin() {
        SetTheCubes();

        //load texture
        //texture_[0] = TextureManager::Load("data/texture/2D/box.jpg");
        brickwall_ = TextureManager::Load("data/texture/2D/brickwall.jpg");
        brickwall_normal_ = TextureManager::Load("data/texture/2D/brickwall_normal.jpg");

        //Load vertex shader cube 1 ---------------------------------------------------------
        auto vertexContent = LoadFile("data/shaders/3D_scene/cube.vert");
        auto *ptr = vertexContent.data();
        vertexShader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader_, 1, &ptr, nullptr);
        glCompileShader(vertexShader_);
        //Check success status of shader compilation
        GLint success;
        glGetShaderiv(vertexShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shader\n";
        }
        //Load vertex shader cube 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/normal_mapping.vert");
        ptr = vertexContent.data();
        vertexMappingShader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexMappingShader_, 1, &ptr, nullptr);
        glCompileShader(vertexMappingShader_);
        //Check success status of shader compilation
        glGetShaderiv(vertexMappingShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shader for mapping\n";
        }

        //Load fragment shaders cube 1 ---------------------------------------------------------
        auto fragmentContent = LoadFile("data/shaders/3D_scene/cube.frag");
        ptr = fragmentContent.data();
        fragmentShader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader_, 1, &ptr, nullptr);
        glCompileShader(fragmentShader_);
        //Check success status of shader compilation
        glGetShaderiv(fragmentShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader\n";
        }
        //Load fragment shaders cube mapping 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/normal_mapping.frag");
        ptr = fragmentContent.data();
        fragmentMappingShader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentMappingShader_, 1, &ptr, nullptr);
        glCompileShader(fragmentMappingShader_);
        //Check success status of shader compilation
        glGetShaderiv(fragmentMappingShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader for mapping\n";
        }


        //----------------------------------------------------------- set programs

        //Load program/pipeline
        program_ = glCreateProgram();
        program_mapping_ = glCreateProgram();

        glAttachShader(program_, vertexShader_);
        glAttachShader(program_, fragmentShader_);
        glAttachShader(program_mapping_, vertexMappingShader_);
        glAttachShader(program_mapping_, fragmentMappingShader_);

        glLinkProgram(program_);
        glLinkProgram(program_mapping_);

        //Check if shader program was linked correctly
        glGetProgramiv(program_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking shader program\n";
        }
        glGetProgramiv(program_mapping_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking shader program for mapping\n";
        }

        //----------------------------------------------------------- set VAO

        small_cube_vao_.Create();
        quad_vao_.Create();
        quad_vbo.Create();

        //----------------------------------------------------------- set pointers

        camera_ = new Camera;

        //----------------------------------------------------------- frame buffer / render buffer

        glUseProgram(program_mapping_);
        glUniform1i(glGetUniformLocation(program_mapping_, "diffuseMap"), 0);
        glUniform1i(glGetUniformLocation(program_mapping_, "normalMap"), 1);

        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //-----------------------------------------------------------
    }

    void NormalMapping::End() {
        //Unload program/pipeline
        glDeleteProgram(program_);
        glDeleteProgram(program_mapping_);

        glDeleteShader(vertexShader_);
        glDeleteShader(fragmentShader_);

        glDeleteShader(fragmentMappingShader_);
        glDeleteShader(fragmentMappingShader_);

        free(camera_);
        small_cube_vao_.Delete();
        quad_vao_.Delete();
        quad_vbo.Delete();
    }

    void NormalMapping::Update(float dt) {

        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

        const float aspect = 1200.0f / 800.0f; //see in the engine.Begin() -> window size
        const float zNear = 0.01f;
        const float zFar = 100.0f;
        const float fovY = std::numbers::pi_v<float> / 2;

        //frustum.CreateFrustumFromCamera(*camera_, aspect, fovY, zNear, zFar);

//        glEnable(GL_CULL_FACE);
//        glCullFace(GL_BACK);
//        glFrontFace(GL_CW);
//
//        glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
        glm::mat4 projection;
        projection = glm::perspective(fovY, aspect, zNear, zFar);
//
//
//        elapsed_time_ += dt;
//        //Draw program -> cubes
//        glUseProgram(program_);
//
//        SetView(projection, program_);
//        SetAndBindTextures();
//        SetUniformsProgram(lightPos);
//        SetAndDrawMultipleCubes();

        //normal mapping
        glDisable(GL_CULL_FACE);
        glUseProgram(program_mapping_);

        SetView(projection, program_mapping_);
        SetUniformsProgramMapping();
        SetTextureNormalMapBrickwall();

        RenderQuad();

        // render light source (simply re-renders a smaller plane at the light's position for debugging/visualization)
        auto model = glm::mat4(1.0f);
        glm::vec3 lightPosi(0.5f, 1.0f, 0.3f);
        model = glm::translate(model, lightPosi);
        model = glm::scale(model, glm::vec3(0.1f));
        int modelLocP = glGetUniformLocation(program_mapping_, "model");
        glUniformMatrix4fv(modelLocP, 1, GL_FALSE, glm::value_ptr(model));
        RenderQuad();
    }

    void NormalMapping::SetView(const glm::mat4 &projection, GLuint &program) const {
        int viewLocP = glGetUniformLocation(program, "view");
        glUniformMatrix4fv(viewLocP, 1, GL_FALSE, glm::value_ptr(camera_->view()));

        int projectionLocP = glGetUniformLocation(program, "projection");
        glUniformMatrix4fv(projectionLocP, 1, GL_FALSE, glm::value_ptr(projection));
    }


    void NormalMapping::RenderQuad() {
        // positions
        glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
        glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
        glm::vec3 pos3(1.0f, -1.0f, 0.0f);
        glm::vec3 pos4(1.0f, 1.0f, 0.0f);
        // texture coordinates
        glm::vec2 uv1(0.0f, 1.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.0f, 0.0f);
        glm::vec2 uv4(1.0f, 1.0f);
        // normal vector
        glm::vec3 nm(0.0f, 0.0f, 1.0f);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;
        // triangle 1
        // ----------
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);


        float quadVertices[] = {
                // positions                            // normal                     // texcoords          // tangent                          // bitangent
                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z,
                bitangent1.x, bitangent1.y, bitangent1.z,
                pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z,
                bitangent1.x, bitangent1.y, bitangent1.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z,
                bitangent1.x, bitangent1.y, bitangent1.z,

                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z,
                bitangent2.x, bitangent2.y, bitangent2.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z,
                bitangent2.x, bitangent2.y, bitangent2.z,
                pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z,
                bitangent2.x, bitangent2.y, bitangent2.z
        };
        // configure plane VAO
        quad_vao_.Bind();
        quad_vbo.Bind();
        quad_vbo.BindData(sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                              (void *) nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                              (void *) (3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                              (void *) (6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                              (void *) (8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                              (void *) (11 * sizeof(float)));
        quad_vao_.Bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    void NormalMapping::SetTextureNormalMapBrickwall() const {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, brickwall_);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, brickwall_normal_);
    }

    void NormalMapping::SetUniformsProgramMapping() const {
        // render normal-mapped quad
        glm::vec3 lightPos(0.5f, 1.0f, 0.3f);

        auto model = glm::mat4(1.0f);
        // rotate the quad to show normal mapping from multiple directions
        model = glm::rotate(model, glm::radians( elapsed_time_ * -10.0f),
                            glm::normalize(glm::vec3(1.0, 0.0, 1.0)));

        int modelLocP = glGetUniformLocation(program_mapping_, "model");
        glUniformMatrix4fv(modelLocP, 1, GL_FALSE, glm::value_ptr(model));

        int camLocP = glGetUniformLocation(program_mapping_, "viewPos");
        glUniform3f(camLocP, camera_->position_.x, camera_->position_.y, camera_->position_.z);

        int lightLocP = glGetUniformLocation(program_mapping_, "lightPos");
        glUniform3f(lightLocP, lightPos.x, lightPos.y, lightPos.z);
    }

    void NormalMapping::SetUniformsProgram(const glm::vec3 &lightPos) const {
        glUniform3f(glGetUniformLocation(program_, "material.ambient"), 1.0f, 0.5f, 0.31f);
        glUniform3f(glGetUniformLocation(program_, "material.diffuse"), 1.0f, 0.5f, 0.31f);
        glUniform3f(glGetUniformLocation(program_, "material.specular"), 0.5f, 0.5f, 0.5f);
        glUniform1f(glGetUniformLocation(program_, "material.shininess"), 32.0f);

        //stat light material
        glUniform3f(glGetUniformLocation(program_, "light.ambient"), 0.2f, 0.2f, 0.2f);
        glUniform3f(glGetUniformLocation(program_, "light.diffuse"), 0.5f, 0.5f, 0.5f);
        glUniform3f(glGetUniformLocation(program_, "light.specular"), 1.0f, 1.0f, 1.0f);


        glUniform3f(glGetUniformLocation(program_, "lightColor"), 1.0f, 1.0f, 1.0f);
        glUniform3f(glGetUniformLocation(program_, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(program_, "viewPos"), camera_->position_.x, camera_->position_.y,
                    camera_->position_.z);
    }

    void NormalMapping::SetAndBindTextures() const {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_[0]);
    }

    void NormalMapping::SetAndDrawMultipleCubes() const {
        small_cube_vao_.Bind();
        for (auto current_cube: all_cubes_) {
            //set matrix model
            auto model = glm::mat4(1.0f);
            model = glm::translate(model, current_cube);
//            float angle = 20.0f + static_cast<float>(i);
//            model = glm::rotate(model, elapsed_time_ * glm::radians(angle) * cos(elapsed_time_),
//                                glm::vec3(1.0f, 0.3f, 0.5f));
//            model = glm::scale(model, glm::vec3(2.0f * cos(elapsed_time_),
//                                                2.0f * cos(elapsed_time_), 2.0f * cos(elapsed_time_)));

            //apply model matrix
            int modelLoc = glGetUniformLocation(program_, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);

//            if (frustum.IsCubeInFrustum(glm::vec3(current_cube.x, current_cube.y, current_cube.z), 0.5f)) {
//                glDrawArrays(GL_TRIANGLES, 0, 36);
//            }
        }
    }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
    gpr::NormalMapping scene;
    gpr::Engine engine(&scene);
    engine.Run();

    return EXIT_SUCCESS;
}