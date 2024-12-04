//
// Created by Mat on 11/28/2024.
//

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

#include "engine.h"
#include "scene.h"
#include "texture_loader.h"
#include "camera.h"
#include "open_gl_data_structure/vao.h"
#include "file_utility.h"

#include <sstream>
#include <iostream>
#include <array>

namespace gpr {
    class ThreeDScene final : public Scene {
    public:
        void Begin() override;

        void End() override;

        void Update(float dt) override;

        void OnEvent(const SDL_Event& event, float dt) override;

    private:
        float elapsed_time_ = 0.0f;
        unsigned int texture[2] = {0, 0};
        std::array<glm::vec3, 10> all_cubes_{};
        GLuint vertexShader_ = 0;
        GLuint lightVertexShader_ = 0;
        GLuint modelVertexShader_ = 0;
        GLuint fragmentShader_ = 0;
        GLuint fragmentLightShader_ = 0;
        GLuint fragmentModelShader_ = 0;
        GLuint program_ = 0;
        GLuint program_light_ = 0;
        GLuint program_model_ = 0;
        VAO vao_;
        Camera* camera_ = nullptr;
        Model* model_ = nullptr;

        void SetAndBindTextures() const;

        void SetAndDrawMultipleCubes() const;

        void SetTheCubes();

        void SetUniformsProgram(const glm::vec3 &lightPos) const;

        void SetUniformsProgramLight(const glm::vec3 &lightPos) const;

        static void DrawLightProgram();

        void SetAndDrawModel();
    };

    void ThreeDScene::OnEvent(const SDL_Event& event, const float dt)
    {
        // Get keyboard state
        const Uint8* state = SDL_GetKeyboardState(nullptr);

        // Camera controls
        if (state[SDL_SCANCODE_W])
        {
            camera_->Move(FORWARD, dt);
        }
        if (state[SDL_SCANCODE_S])
        {
            camera_->Move(BACKWARD, dt);
        }
        if (state[SDL_SCANCODE_A])
        {
            camera_->Move(LEFT, dt);
        }
        if (state[SDL_SCANCODE_D])
        {
            camera_->Move(RIGHT, dt);
        }
        if (state[SDL_SCANCODE_SPACE])
        {
            camera_->Move(UP, dt);
        }
        if (state[SDL_SCANCODE_LCTRL])
        {
            camera_->Move(DOWN, dt);
        }
        if (state[SDL_SCANCODE_LSHIFT])
        {
            camera_->is_sprinting_ = !camera_->is_sprinting_;
        }

        int mouseX, mouseY;
        const Uint32 mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
        if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
        {
            camera_->Update(mouseX, mouseY);
        }
    }

    void ThreeDScene::SetTheCubes() {
        for (std::size_t i = 0; i < all_cubes_.size(); i++) {
            all_cubes_[i] = glm::vec3(1 * i, 1 * i, 1 * i);
        }
    }

    void ThreeDScene::Begin() {
        SetTheCubes();

        //load texture
        texture[0] = TextureManager::Load("data/texture/2D/box.jpg");
        texture[1] = TextureManager::Load("data/texture/2D/ennemy_01.png");


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
        //Load vertex shader light 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/light.vert");
        ptr = vertexContent.data();
        lightVertexShader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(lightVertexShader_, 1, &ptr, nullptr);
        glCompileShader(lightVertexShader_);
        glGetShaderiv(lightVertexShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shader for light\n";
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
        //Load fragment shaders light 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/light.frag");
        ptr = fragmentContent.data();
        fragmentLightShader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentLightShader_, 1, &ptr, nullptr);
        glCompileShader(fragmentLightShader_);
        //Check success status of shader compilation
        glGetShaderiv(fragmentLightShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader for light\n";
        }
        //Load fragment shaders model 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/model.frag");
        ptr = fragmentContent.data();
        fragmentModelShader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentModelShader_, 1, &ptr, nullptr);
        glCompileShader(fragmentModelShader_);
        //Check success status of shader compilation
        glGetShaderiv(fragmentModelShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader for model\n";
        }


        //Load program/pipeline
        program_ = glCreateProgram();
        program_light_ = glCreateProgram();
        program_model_ = glCreateProgram();

        glAttachShader(program_, vertexShader_);
        glAttachShader(program_, fragmentShader_);

        glAttachShader(program_light_, lightVertexShader_);
        glAttachShader(program_light_, fragmentLightShader_);

        glAttachShader(program_model_, modelVertexShader_);
        glAttachShader(program_model_, fragmentModelShader_);

        glLinkProgram(program_);
        glLinkProgram(program_light_);
        glLinkProgram(program_model_);
        //Check if shader program was linked correctly

        glGetProgramiv(program_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking shader program\n";
        }
        glGetProgramiv(program_light_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking shader program for light\n";
        }
        glGetProgramiv(program_model_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking shader program for model\n";
        }
        //Empty vao
       vao_.Create();

        std::string n = "data/texture/3D/backpack/backpack.obj";
        model_ = new Model(n);
        camera_ = new Camera;
    }

    void ThreeDScene::End() {
        //Unload program/pipeline
        glDeleteProgram(program_);
        glDeleteProgram(program_light_);
        glDeleteProgram(program_model_);

        glDeleteShader(vertexShader_);
        glDeleteShader(fragmentShader_);

        glDeleteShader(lightVertexShader_);
        glDeleteShader(fragmentLightShader_);

        glDeleteShader(modelVertexShader_);
        glDeleteShader(fragmentModelShader_);

        vao_.Delete();
    }

    void ThreeDScene::Update(float dt) {
        glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.01f, 2000.0f);

        elapsed_time_ += dt;
        //Draw program -> cubes
        glUseProgram(program_);

        int viewLocP = glGetUniformLocation(program_, "view");
        glUniformMatrix4fv(viewLocP, 1, GL_FALSE, glm::value_ptr(camera_->view()));

        int projectionLocP = glGetUniformLocation(program_, "projection");
        glUniformMatrix4fv(projectionLocP, 1, GL_FALSE, glm::value_ptr(projection));

        SetUniformsProgram(lightPos);
        SetAndBindTextures();
        SetAndDrawMultipleCubes();

        //Draw program -> light
        glUseProgram(program_light_);

        int viewLoc = glGetUniformLocation(program_light_, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(camera_->view()));

        int projectionLoc = glGetUniformLocation(program_light_, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        SetUniformsProgramLight(lightPos);
        DrawLightProgram();

        //draw programme -> 3D model
        glUseProgram(program_model_);

        int viewLocM = glGetUniformLocation(program_model_, "view");
        glUniformMatrix4fv(viewLocM, 1, GL_FALSE, glm::value_ptr(camera_->view()));

        int projectionLocM = glGetUniformLocation(program_model_, "projection");
        glUniformMatrix4fv(projectionLocM, 1, GL_FALSE, glm::value_ptr(projection));

        SetAndDrawModel();
    }

    void ThreeDScene::SetAndDrawModel() {
        // render the loaded model
        glEnable(GL_TEXTURE1);
        auto model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));    // it's a bit too big for our scene, so scale it down
        int modelLoc = glGetUniformLocation(program_model_, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        model_->Draw(program_model_);
    }

    void ThreeDScene::DrawLightProgram() {
        VAO light_vao;
        light_vao.Create();
        light_vao.Bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    void ThreeDScene::SetUniformsProgramLight(const glm::vec3 &lightPos) const {
        glm::mat4 model{};
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f));

        int modelLoc = glGetUniformLocation(program_light_, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    }

    void ThreeDScene::SetUniformsProgram(const glm::vec3 &lightPos) const {//material setup
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

    void ThreeDScene::SetAndBindTextures() const {
        glUniform1i(glGetUniformLocation(program_, "ourTexture1"), 0);
        //glUniform1i(glGetUniformLocation(program_, "ourTexture2"), 1);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture[0]);

        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, texture[1]);

        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void ThreeDScene::SetAndDrawMultipleCubes() const {
        vao_.Bind();
        for (std::size_t i = 0; i < all_cubes_.size(); i++) {
            //set matrix model
            auto model = glm::mat4(1.0f);
            model = glm::translate(model, all_cubes_[i]);
            float angle = 20.0f + static_cast<float>(i);
            model = glm::rotate(model, elapsed_time_ * glm::radians(angle) * cos(elapsed_time_),
                                glm::vec3(1.0f, 0.3f, 0.5f));
            model = glm::scale(model, glm::vec3(2.0f * cos(elapsed_time_),
                                                2.0f * cos(elapsed_time_), 2.0f * cos(elapsed_time_)));

            //apply model matrix
            int modelLoc = glGetUniformLocation(program_, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
    gpr::ThreeDScene scene;
    gpr::Engine engine(&scene);
    engine.Run();

    return EXIT_SUCCESS;
}