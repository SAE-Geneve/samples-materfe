//
// Created by Mat on 1/22/2025.
//
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"

#include "engine.h"
#include "scene.h"
#include "camera.h"
#include "load3D/texture_loader.h"
#include "file_utility.h"

#include <sstream>
#include <iostream>
#include <array>
#include <numbers>

namespace gpr {
    class GammaCorection final : public Scene {
    public:
        void Begin() override;

        void End() override;

        void Update(float dt) override;

        void OnEvent(const SDL_Event &event, float dt) override;

        void DrawImGui() override;

    private:
        float elapsed_time_ = 0.0f;
        glm::vec3 light_pos_[4]{};
        glm::vec3 light_color_[4]{};
        unsigned int floor_texture_ = 0;
        unsigned int floor_texture_gamma_corrected_ = 0;
        bool gammaEnabled = true;

        //all vertex shaders-------------
        GLuint gamma_correction_vertex_shader_ = 0;

        //all fragment shaders-------------
        GLuint gamma_correction_fragment_shader_ = 0;

        //all programs-------------
        GLuint program_gamma_correction_ = 0;

        Camera *camera_ = nullptr;
        Frustum frustum{};

        VAO plane_vao_{};
        VBO plane_vbo_{};

        void SetView(const glm::mat4 &projection, GLuint &program) const;
    };

    void GammaCorection::OnEvent(const SDL_Event &event, const float dt) {
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

    void GammaCorection::Begin() {

        floor_texture_ = TextureManager::LoadTexture("data/texture/2D/box.jpg", false);
        floor_texture_gamma_corrected_ = TextureManager::LoadTexture("data/texture/2D/box.jpg", true); //activate gamma

        //Load vertex shader cube 1 ---------------------------------------------------------
        auto vertexContent = LoadFile("data/shaders/3D_scene/cube.vert");
        auto *ptr = vertexContent.data();
        GLint success;
        //Load vertex shader model 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/gamma_correction/gamma_correction.vert");
        ptr = vertexContent.data();
        gamma_correction_vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(gamma_correction_vertex_shader_, 1, &ptr, nullptr);
        glCompileShader(gamma_correction_vertex_shader_);
        glGetShaderiv(gamma_correction_vertex_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shader for gamma\n";
        }

        //Load fragment shaders cube 1 ---------------------------------------------------------
        auto fragmentContent = LoadFile("data/shaders/3D_scene/cube.frag");
        //Load fragment shaders model 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/gamma_correction/gamma_correction.frag");
        ptr = fragmentContent.data();
        gamma_correction_fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(gamma_correction_fragment_shader_, 1, &ptr, nullptr);
        glCompileShader(gamma_correction_fragment_shader_);
        //Check success status of shader compilation
        glGetShaderiv(gamma_correction_fragment_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader for gamma\n";
        }

        std::cout << "passed frag\n";

        //----------------------------------------------------------- set programs

        //Load program/pipeline
        program_gamma_correction_ = glCreateProgram();

        glAttachShader(program_gamma_correction_, gamma_correction_vertex_shader_);
        glAttachShader(program_gamma_correction_, gamma_correction_fragment_shader_);

        glLinkProgram(program_gamma_correction_);
        //Check if shader program was linked correctly

        glGetProgramiv(program_gamma_correction_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking model shader program for gamma\n";
        }

        //----------------------------------------------------------- set VAO / VBO

        plane_vao_.Create();
        plane_vbo_.Create();

        std::cout << "passed lin + vo\n";

        //----------------------------------------------------------- set pointers

        camera_ = new Camera;

        //----------------------------------------------------------- frame buffer / render buffer
        float planeVertices[] = {
                // positions            // normals         // coords for text
                10.0f, -0.5f, 10.0f, 0.0f, 1.0f, 0.0f, 10.0f, 0.0f,
                -10.0f, -0.5f, 10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                -10.0f, -0.5f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 10.0f,

                10.0f, -0.5f, 10.0f, 0.0f, 1.0f, 0.0f, 10.0f, 0.0f,
                -10.0f, -0.5f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 10.0f,
                10.0f, -0.5f, -10.0f, 0.0f, 1.0f, 0.0f, 10.0f, 10.0f
        };
        // plane VAO
        plane_vao_.Bind();
        plane_vbo_.Bind();
        plane_vbo_.BindData(sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));
        glBindVertexArray(0);

        // shader configuration
        // --------------------
        glUseProgram(program_gamma_correction_);
        glUniform1i(glGetUniformLocation(program_gamma_correction_, "floorTexture"), 0);

        // lighting info
        // -------------
        glm::vec3 lightPositions[] = {
                glm::vec3(-3.0f, 0.0f, 0.0f),
                glm::vec3(-1.0f, 0.0f, 0.0f),
                glm::vec3(1.0f, 0.0f, 0.0f),
                glm::vec3(3.0f, 0.0f, 0.0f)
        };
        std::ranges::copy(lightPositions, light_pos_);
        glm::vec3 lightColors[] = {
                glm::vec3(0.25),
                glm::vec3(0.50),
                glm::vec3(0.75),
                glm::vec3(1.00)
        };
        std::ranges::copy(lightColors, light_color_);


        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    //TODO update
    void GammaCorection::End() {
        //Unload program/pipeline
        glDeleteProgram(program_gamma_correction_);

        glDeleteShader(gamma_correction_vertex_shader_);
        glDeleteShader(gamma_correction_fragment_shader_);

        free(camera_);
    }

    void GammaCorection::Update(float dt) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CW);
        elapsed_time_ += dt;
        glm::mat4 projection;

        const float aspect = 1200.0f / 800.0f; //see in the engine.Begin() -> window size
        const float zNear = 0.1f;
        const float zFar = 100.0f;
        const float fovY = std::numbers::pi_v<float> / 2;

        frustum.CreateFrustumFromCamera(*camera_, aspect, fovY, zNear, zFar);
        projection = glm::perspective(fovY, aspect, zNear, zFar);

        // draw gamma correction -------------------------------------
        glUseProgram(program_gamma_correction_);
        SetView(projection, program_gamma_correction_);
        // set light uniforms
        glUniform3fv(glGetUniformLocation(program_gamma_correction_, "lightPositions"), 4, &light_pos_[0][0]);
        glUniform3fv(glGetUniformLocation(program_gamma_correction_, "lightColors"), 4, &light_color_[0][0]);
        glUniform3f(glGetUniformLocation(program_gamma_correction_, "viewPos"),
                    camera_->position_.x, camera_->position_.y, camera_->position_.z);
        glUniform1i(glGetUniformLocation(program_gamma_correction_, "gamma"), gammaEnabled);

        // floor
        plane_vao_.Bind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gammaEnabled ? floor_texture_gamma_corrected_ : floor_texture_);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        std::cout << (gammaEnabled ? "Gamma enabled" : "Gamma disabled") << std::endl;
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
        DrawImGui();
    }

    void GammaCorection::DrawImGui() {
        // Début ImGui
        ImGui::Begin("Gamma Control");
        ImGui::Checkbox("Enable Gamma Correction", &gammaEnabled);
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }


    void GammaCorection::SetView(const glm::mat4 &projection, GLuint &program) const {
        int viewLocP = glGetUniformLocation(program, "view");
        glUniformMatrix4fv(viewLocP, 1, GL_FALSE, glm::value_ptr(camera_->view()));


        int projectionLocP = glGetUniformLocation(program, "projection");
        glUniformMatrix4fv(projectionLocP, 1, GL_FALSE, glm::value_ptr(projection));
    }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
    gpr::GammaCorection scene;
    gpr::Engine engine(&scene);
    engine.Run();

    return EXIT_SUCCESS;
}