//
// Created by Mat on 1/20/2025.
//

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

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
    class FinalScene final : public Scene {
    public:
        void Begin() override;

        void End() override;

        void Update(float dt) override;

        void OnEvent(const SDL_Event &event, float dt) override;

    private:
        float elapsed_time_ = 0.0f;

        GLuint vertexShader_ = 0;
        GLuint modelVertexShader_ = 0;

        GLuint fragmentShader_ = 0;
        GLuint modelFragmentShader_ = 0;

        GLuint program_ = 0;
        GLuint program_model_ = 0;

        Camera *camera_ = nullptr;
        Model* rock_model_ = nullptr;
        Frustum frustum{};


        void SetView(const glm::mat4 &projection, GLuint &program, const glm::mat4 &view = glm::mat4(1.0f)) const;
    };

    void FinalScene::OnEvent(const SDL_Event &event, const float dt) {
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

    void FinalScene::Begin() {

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
        //Load vertex shader model 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/model.vert");
        ptr = vertexContent.data();
        modelVertexShader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(modelVertexShader_, 1, &ptr, nullptr);
        glCompileShader(modelVertexShader_);
        glGetShaderiv(modelVertexShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shader for model\n";
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
        //Load fragment shaders model 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/model.frag");
        ptr = fragmentContent.data();
        modelFragmentShader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(modelFragmentShader_, 1, &ptr, nullptr);
        glCompileShader(modelFragmentShader_);
        //Check success status of shader compilation
        glGetShaderiv(modelFragmentShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader for model\n";
        }

        //----------------------------------------------------------- set programs

        //Load program/pipeline
        program_ = glCreateProgram();
        program_model_ = glCreateProgram();

        glAttachShader(program_, vertexShader_);
        glAttachShader(program_, fragmentShader_);

        glAttachShader(program_model_, modelVertexShader_);
        glAttachShader(program_model_, modelFragmentShader_);

        glLinkProgram(program_);
        glLinkProgram(program_model_);
        //Check if shader program was linked correctly

        glGetProgramiv(program_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking shader program\n";
        }
        glGetProgramiv(program_model_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking shader program\n";
        }

        //----------------------------------------------------------- set VAO / VBO

        //----------------------------------------------------------- set pointers

        camera_ = new Camera;
        std::string path_1 = "data/texture/3D/ROCK/LowPolyRockPack.obj";
        std::string path_2 = "data/texture/3D/backpack/backpack.obj";
        rock_model_ = new Model(path_2);

        //----------------------------------------------------------- frame buffer / render buffer

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glEnable(GL_DEPTH_TEST);
    }

    void FinalScene::End() {
        //Unload program/pipeline
        glDeleteProgram(program_);

        glDeleteShader(vertexShader_);
        glDeleteShader(fragmentShader_);

        free(camera_);
    }

    void FinalScene::Update(float dt) {
        elapsed_time_ += dt;
        glm::mat4 projection;

        const float aspect = 1200.0f / 800.0f; //see in the engine.Begin() -> window size
        const float zNear = 0.1f;
        const float zFar = 100.0f;
        const float fovY = std::numbers::pi_v<float> / 2;

        frustum.CreateFrustumFromCamera(*camera_, aspect, fovY, zNear, zFar);
        projection = glm::perspective(fovY, aspect, zNear, zFar);

        //Draw program -> cubes
        glUseProgram(program_);

        SetView(projection, program_);

        //draw programme -> 3D model
        glUseProgram(program_model_);

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms

        // view/projection transformations
        SetView(projection, program_model_);

        // render the loaded model
        auto model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        glUniformMatrix4fv(glGetUniformLocation(program_model_, "model"), 1, GL_FALSE, glm::value_ptr(model));
        rock_model_->Draw(program_model_);
    }

    void FinalScene::SetView(const glm::mat4 &projection, GLuint &program, const glm::mat4 &view) const {
        if (view == glm::mat4(1.0f)) {
            int viewLocP = glGetUniformLocation(program, "view");
            glUniformMatrix4fv(viewLocP, 1, GL_FALSE, glm::value_ptr(camera_->view()));
        } else {
            int viewLocP = glGetUniformLocation(program, "view");
            glUniformMatrix4fv(viewLocP, 1, GL_FALSE, glm::value_ptr(view));
        }

        int projectionLocP = glGetUniformLocation(program, "projection");
        glUniformMatrix4fv(projectionLocP, 1, GL_FALSE, glm::value_ptr(projection));
    }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
    gpr::FinalScene scene;
    gpr::Engine engine(&scene);
    engine.Run();

    return EXIT_SUCCESS;
}