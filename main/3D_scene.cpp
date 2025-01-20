//
// Created by Mat on 11/28/2024.
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
#include "file_utility.h"

#include <sstream>
#include <iostream>
#include <array>
#include <numbers>

namespace gpr {
    class ThreeDScene final : public Scene {
    public:
        void Begin() override;

        void End() override;

        void Update(float dt) override;

        void OnEvent(const SDL_Event &event, float dt) override;

    private:
        float elapsed_time_ = 0.0f;
        unsigned int texture[2] = {0, 0};
        std::array<glm::vec3, 10> all_cubes_{};

        glm::vec3 pointLightPositions = glm::vec3(0.7f, 0.2f, 2.0f);
        glm::vec3 light_position_ = glm::vec3(1.2f, 1.0f, 2.0f);
        glm::vec3 light_colour_ = glm::vec3(1.0f, 1.0f, 1.0f);

        GLuint vertexShader_ = 0;
        GLuint lightVertexShader_ = 0;
        GLuint modelVertexShader_ = 0;
        GLuint quadVertexShader_ = 0;

        GLuint fragmentShader_ = 0;
        GLuint fragmentLightShader_ = 0;
        GLuint fragmentModelShader_ = 0;
        GLuint fragmentquadShader_ = 0;

        GLuint program_ = 0;
        GLuint program_light_ = 0;
        GLuint program_model_ = 0;
        GLuint program_quad_ = 0;

        GLuint frame_buffer_ = 0;
        GLuint render_buffer_ = 0;
        GLuint image_frame_buffer_ = 0;

        VAO vao_;
        VAO quad_vao_;

        Camera *camera_ = nullptr;
        Model *model_ = nullptr;
        Frustum frustum{};

        void SetAndBindTextures() const;

        void SetAndDrawMultipleCubes() const;

        void SetTheCubes();

        void SetUniformsProgram(const glm::vec3 &lightPos) const;

        void SetUniformsProgramLight(const glm::vec3 &lightPos) const;

        static void DrawLightProgram();

        void SetAndDrawModel();

        void SetView(const glm::mat4 &projection, GLuint &program) const;
    };

    void ThreeDScene::OnEvent(const SDL_Event &event, const float dt) {
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

    void ThreeDScene::SetTheCubes() {
        for (std::size_t i = 0; i < all_cubes_.size(); i++) {
            all_cubes_[i] = glm::vec3(1 * i, 1 * i, 1 * i);
        }
    }

    void ThreeDScene::Begin() {
        SetTheCubes();

        //----------------------------------------------------------- loads

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
        //Load quad vertex shader 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/quad.vert");
        ptr = vertexContent.data();
        quadVertexShader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(quadVertexShader_, 1, &ptr, nullptr);
        glCompileShader(quadVertexShader_);
        glGetShaderiv(quadVertexShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shader for quad\n";
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
        //Load fragment quad 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/quad.frag");
        ptr = fragmentContent.data();
        fragmentquadShader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentquadShader_, 1, &ptr, nullptr);
        glCompileShader(fragmentquadShader_);
        //Check success status of shader compilation
        glGetShaderiv(fragmentquadShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader for quad\n";
        }

        //----------------------------------------------------------- set programs

        //Load program/pipeline
        program_ = glCreateProgram();
        program_light_ = glCreateProgram();
        program_model_ = glCreateProgram();
        program_quad_ = glCreateProgram();

        glAttachShader(program_, vertexShader_);
        glAttachShader(program_, fragmentShader_);

        glAttachShader(program_light_, lightVertexShader_);
        glAttachShader(program_light_, fragmentLightShader_);

        glAttachShader(program_model_, modelVertexShader_);
        glAttachShader(program_model_, fragmentModelShader_);

        glAttachShader(program_quad_, quadVertexShader_);
        glAttachShader(program_quad_, fragmentquadShader_);

        glLinkProgram(program_);
        glLinkProgram(program_light_);
        glLinkProgram(program_model_);
        glLinkProgram(program_quad_);
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
        glGetProgramiv(program_quad_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking shader program for quad\n";
        }

        //----------------------------------------------------------- set VAO
        //Empty vao
        vao_.Create();
        quad_vao_.Create();

        //----------------------------------------------------------- set pointers

        std::string n = "data/texture/3D/backpack/backpack.obj";
        model_ = new Model(n);
        camera_ = new Camera;

        //----------------------------------------------------------- frame buffer / render buffer


        glGenFramebuffers(1, &frame_buffer_);
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_);
        // create a color attachment texture

        glGenTextures(1, &image_frame_buffer_);
        glBindTexture(GL_TEXTURE_2D, image_frame_buffer_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1200, 800, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               image_frame_buffer_, 0);
        // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)

        glGenRenderbuffers(1, &render_buffer_);
        glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                              1200, 800); // use a single renderbuffer object for both a depth AND stencil buffer.
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  render_buffer_); // now actually attach it
        //check of done correctly + release memory
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //-----------------------------------------------------------
    }

    void ThreeDScene::End() {
        //Unload program/pipeline
        glDeleteProgram(program_);
        glDeleteProgram(program_light_);
        glDeleteProgram(program_model_);
        glDeleteProgram(program_quad_);

        glDeleteShader(vertexShader_);
        glDeleteShader(fragmentShader_);

        glDeleteShader(lightVertexShader_);
        glDeleteShader(fragmentLightShader_);

        glDeleteShader(quadVertexShader_);
        glDeleteShader(fragmentquadShader_);

        glDeleteShader(modelVertexShader_);
        glDeleteShader(fragmentModelShader_);

        glDeleteFramebuffers(1, &frame_buffer_);
        glDeleteRenderbuffers(1, &render_buffer_);

        free(model_);
        free(camera_);
        vao_.Delete();
        quad_vao_.Delete();
    }

    void ThreeDScene::Update(float dt) {

        glEnable(GL_DEPTH_TEST);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const float aspect = 1200.0f / 800.0f; //see in the engine.Begin() -> window size
        const float zNear = 0.01f;
        const float zFar = 100.0f;
        const float fovY = std::numbers::pi_v<float> / 2;

        frustum.CreateFrustumFromCamera(*camera_, aspect, fovY, zNear, zFar);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CW);

        glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
        glm::mat4 projection;
        projection = glm::perspective(fovY, aspect, zNear, zFar);


        elapsed_time_ += dt;
        //Draw program -> cubes
        glUseProgram(program_);

        SetView(projection, program_);
        SetAndBindTextures();
        SetUniformsProgram(lightPos);
        SetAndDrawMultipleCubes();

        //Draw program -> light
        glUseProgram(program_light_);

        SetView(projection, program_light_);
        SetUniformsProgramLight(lightPos);
        DrawLightProgram();

        //draw programme -> 3D model
        glUseProgram(program_model_);

        SetView(projection, program_model_);

        //Directional light (sun)
        glUniform3f(glGetUniformLocation(program_model_, "dirLight.direction"), lightPos.x,lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(program_model_, "dirLight.ambient"), 0.2f, 0.2f, 0.2f);
        glUniform3f(glGetUniformLocation(program_model_, "dirLight.diffuse"), 0.5f, 0.5f, 0.5f);
        glUniform3f(glGetUniformLocation(program_model_, "dirLight.specular"), 1.0f, 1.0f, 1.0f);

        //Material
        glUniform1f(glGetUniformLocation(program_model_, "material.shininess"), 32.0f);


        vao_.Bind();

        //Draw model
        //glUniform1i(glGetUniformLocation(program_model_, "material.diffuse"), 0);
        glActiveTexture(GL_TEXTURE0);
        auto model = glm::mat4(1.0f);
        model = glm::translate(model,
                               glm::vec3(2.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));    // it's a bit too big for our scene, so scale it down
        glUniformMatrix4fv(glGetUniformLocation(program_model_, "model"),
                           1, GL_FALSE, glm::value_ptr(model));
        model_->Draw(program_);
    }

    void ThreeDScene::SetView(const glm::mat4 &projection, GLuint &program) const {
        int viewLocP = glGetUniformLocation(program, "view");
        glUniformMatrix4fv(viewLocP, 1, GL_FALSE, glm::value_ptr(camera_->view()));

        int projectionLocP = glGetUniformLocation(program, "projection");
        glUniformMatrix4fv(projectionLocP, 1, GL_FALSE, glm::value_ptr(projection));
    }

    void ThreeDScene::SetAndDrawModel() {
        // render the loaded model
        auto model = glm::mat4(1.0f);
        model = glm::translate(model,
                               glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
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
    }

    void ThreeDScene::SetAndDrawMultipleCubes() const {
        vao_.Bind();
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

            if (frustum.IsCubeInFrustum(glm::vec3(current_cube.x, current_cube.y, current_cube.z), 0.5f)) {
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }
    }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
    gpr::ThreeDScene scene;
    gpr::Engine engine(&scene);
    engine.Run();

    return EXIT_SUCCESS;
}