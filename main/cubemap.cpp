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
#include "open_gl_data_structure/vbo.h"
#include "file_utility.h"

#include <sstream>
#include <iostream>
#include <array>
#include <numbers>

namespace gpr {
    class CubeMapScene final : public Scene {
    public:
        void Begin() override;

        void End() override;

        void Update(float dt) override;

        void OnEvent(const SDL_Event &event, float dt) override;

    private:
        float elapsed_time_ = 0.0f;
        unsigned int texture_[2] = {0, 0};
        unsigned int cubeMapText_ = 0;
        float skybox_vertices_[108] = {};
        std::array<glm::vec3, 10> all_cubes_{};

        GLuint vertexShader_ = 0;
        GLuint lightVertexShader_ = 0;
        GLuint cubeMapVertexShader_ = 0;

        GLuint fragmentShader_ = 0;
        GLuint fragmentLightShader_ = 0;
        GLuint fragmentCubeMapShader_ = 0;

        GLuint program_ = 0;
        GLuint program_light_ = 0;
        GLuint program_map_ = 0;

        VAO vao_;
        VAO skybox_vao_;

        VBO skybox_vbo_{};

        Camera *camera_ = nullptr;
        Frustum frustum{};

        void SetAndBindTextures() const;

        void SetAndDrawMultipleCubes() const;

        void SetTheCubes();

        void SetUniformsProgram(const glm::vec3 &lightPos) const;

        void SetUniformsProgramLight(const glm::vec3 &lightPos) const;

        static void DrawLightProgram();

        void SetView(const glm::mat4 &projection, GLuint &program, const glm::mat4& view = glm::mat4(1.0f)) const;
    };

    void CubeMapScene::OnEvent(const SDL_Event &event, const float dt) {
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

    void CubeMapScene::SetTheCubes() {
        for (std::size_t i = 0; i < all_cubes_.size(); i++) {
            all_cubes_[i] = glm::vec3(1 * i, 1 * i, 1 * i);
        }
    }

    void CubeMapScene::Begin() {
        SetTheCubes();

        //----------------------------------------------------------- loads

        //load texture
        texture_[0] = TextureManager::Load("data/texture/2D/box.jpg");
        texture_[1] = TextureManager::Load("data/texture/2D/ennemy_01.png");

        std::vector<std::string> faces =
                {
                        "data/texture/3D/skybox/right.jpg",
                        "data/texture/3D/skybox/left.jpg",
                        "data/texture/3D/skybox/top.jpg",
                        "data/texture/3D/skybox/bottom.jpg",
                        "data/texture/3D/skybox/front.jpg",
                        "data/texture/3D/skybox/back.jpg"
                };
        cubeMapText_ = TextureManager::loadCubemap(faces);

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
        //Load vertex shader map 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/skybox.vert");
        ptr = vertexContent.data();
        cubeMapVertexShader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(cubeMapVertexShader_, 1, &ptr, nullptr);
        glCompileShader(cubeMapVertexShader_);
        glGetShaderiv(cubeMapVertexShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shader for map\n";
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
        //Load fragment shaders light 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/skybox.frag");
        ptr = fragmentContent.data();
        fragmentCubeMapShader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentCubeMapShader_, 1, &ptr, nullptr);
        glCompileShader(fragmentCubeMapShader_);
        //Check success status of shader compilation
        glGetShaderiv(fragmentCubeMapShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader for cube map\n";
        }

        //----------------------------------------------------------- set programs

        //Load program/pipeline
        program_ = glCreateProgram();
        program_light_ = glCreateProgram();
        program_map_ = glCreateProgram();

        glAttachShader(program_, vertexShader_);
        glAttachShader(program_, fragmentShader_);

        glAttachShader(program_light_, lightVertexShader_);
        glAttachShader(program_light_, fragmentLightShader_);

        glAttachShader(program_map_, cubeMapVertexShader_);
        glAttachShader(program_map_, fragmentCubeMapShader_);

        glLinkProgram(program_);
        glLinkProgram(program_light_);
        glLinkProgram(program_map_);
        //Check if shader program was linked correctly

        glGetProgramiv(program_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking shader program\n";
        }
        glGetProgramiv(program_light_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking shader program for light\n";
        }
        glGetProgramiv(program_map_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking shader program for map\n";
        }

        //----------------------------------------------------------- set VAO / VBO

        //Empty vao
        vao_.Create();
        skybox_vao_.Create();
        skybox_vbo_.Create();

        //----------------------------------------------------------- set pointers

        camera_ = new Camera;

        //----------------------------------------------------------- set vertices

        float skyboxVertices[] = {
                // positions
                -1.0f,  1.0f, -1.0f,
                -1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,

                -1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,

                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,

                -1.0f, -1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,

                -1.0f,  1.0f, -1.0f,
                1.0f,  1.0f, -1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f, -1.0f,

                -1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                1.0f, -1.0f,  1.0f
        };

        std::ranges::copy(skyboxVertices, skybox_vertices_);

        //skybox VAO
        skybox_vao_.Bind();
        skybox_vbo_.Bind();
        skybox_vbo_.BindData(sizeof(skybox_vertices_), &skybox_vertices_, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nullptr);

        glUseProgram(program_);
        glUniform1i(glGetUniformLocation(program_, "texture1"), 0);
        glUseProgram(program_map_);
        glUniform1i(glGetUniformLocation(program_map_, "skybox"), 0);
    }

    void CubeMapScene::End() {
        //Unload program/pipeline
        glDeleteProgram(program_);
        glDeleteProgram(program_light_);
        glDeleteProgram(program_map_);

        glDeleteShader(vertexShader_);
        glDeleteShader(fragmentShader_);

        glDeleteShader(lightVertexShader_);
        glDeleteShader(fragmentLightShader_);

        glDeleteShader(cubeMapVertexShader_);
        glDeleteShader(fragmentCubeMapShader_);

        free(camera_);
        vao_.Delete();
        skybox_vao_.Delete();
        skybox_vbo_.Delete();
    }

    void CubeMapScene::Update(float dt) {
        glm::mat4 projection;

        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        const float aspect = 1200.0f / 800.0f; //see in the engine.Begin() -> window size
        const float zNear = 0.1f;
        const float zFar = 100.0f;
        const float fovY = std::numbers::pi_v<float> / 2;

        frustum.CreateFrustumFromCamera(*camera_, aspect, fovY, zNear, zFar);
        projection = glm::perspective(fovY, aspect, zNear, zFar);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CW);

        glm::vec3 lightPos(1.2f, 1.0f, 2.0f);


        elapsed_time_ += dt;
        //Draw program -> cubes
        glUseProgram(program_);

        SetView(projection, program_);
        SetUniformsProgram(lightPos);
        SetAndBindTextures();
        SetAndDrawMultipleCubes();

        //Draw program -> light
        glUseProgram(program_light_);

        SetView(projection, program_light_);
        SetUniformsProgramLight(lightPos);
        DrawLightProgram();


        glDisable(GL_CULL_FACE);
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        glUseProgram(program_map_);
        const auto view = glm::mat4(glm::mat3(camera_->view())); // remove translation from the view matrix
        int viewLocP = glGetUniformLocation(program_map_, "view");
        glUniformMatrix4fv(viewLocP, 1, GL_FALSE, glm::value_ptr(view));
        int projectionLocP = glGetUniformLocation(program_map_, "projection");
        glUniformMatrix4fv(projectionLocP, 1, GL_FALSE, glm::value_ptr(projection));
        // skybox cube
        skybox_vao_.Bind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapText_);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default
    }

    void CubeMapScene::SetView(const glm::mat4 &projection, GLuint &program, const glm::mat4& view) const {
        if(view == glm::mat4(1.0f))
        {
            int viewLocP = glGetUniformLocation(program, "view");
            glUniformMatrix4fv(viewLocP, 1, GL_FALSE, glm::value_ptr(camera_->view()));
        }
        else
        {
            int viewLocP = glGetUniformLocation(program, "view");
            glUniformMatrix4fv(viewLocP, 1, GL_FALSE, glm::value_ptr(view));
        }

        int projectionLocP = glGetUniformLocation(program, "projection");
        glUniformMatrix4fv(projectionLocP, 1, GL_FALSE, glm::value_ptr(projection));
    }

    void CubeMapScene::DrawLightProgram() {
        VAO light_vao;
        light_vao.Create();
        light_vao.Bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    void CubeMapScene::SetUniformsProgramLight(const glm::vec3 &lightPos) const {
        glm::mat4 model{};
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f));

        int modelLoc = glGetUniformLocation(program_light_, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    }

    void CubeMapScene::SetUniformsProgram(const glm::vec3 &lightPos) const {//material setup
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

    void CubeMapScene::SetAndBindTextures() const {
        glUniform1i(glGetUniformLocation(program_, "ourTexture1"), 0);
        //glUniform1i(glGetUniformLocation(program_, "ourTexture2"), 1);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_[0]);

        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, texture[1]);
    }

    void CubeMapScene::SetAndDrawMultipleCubes() const {
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
    gpr::CubeMapScene scene;
    gpr::Engine engine(&scene);
    engine.Run();

    return EXIT_SUCCESS;
}