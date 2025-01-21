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
        unsigned int cube_map_text_ = 0;
        float skybox_vertices_[108] = {};

        //all vertex shaders-------------
        GLuint vertex_shader_ = 0;
        GLuint model_vertex_shader_ = 0;
        GLuint cube_map_vertex_shader_ = 0;

        //all fragment shaders-------------
        GLuint fragment_shader_ = 0;
        GLuint model_fragment_shader_ = 0;
        GLuint cube_map_fragment_shader_ = 0;

        //all programs-------------
        GLuint program_ = 0;
        GLuint program_model_ = 0;
        GLuint program_cube_map_ = 0;

        Camera *camera_ = nullptr;
        Model *rock_model_ = nullptr;
        Frustum frustum{};

        VAO skybox_vao_{};
        VBO skybox_vbo_{};


        void SetView(const glm::mat4 &projection, GLuint &program) const;
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

        //load textures
        std::vector<std::string> faces =
                {
                        "data/texture/3D/skybox/right.jpg",
                        "data/texture/3D/skybox/left.jpg",
                        "data/texture/3D/skybox/top.jpg",
                        "data/texture/3D/skybox/bottom.jpg",
                        "data/texture/3D/skybox/front.jpg",
                        "data/texture/3D/skybox/back.jpg"
                };
        cube_map_text_ = TextureManager::loadCubemap(faces);


        //Load vertex shader cube 1 ---------------------------------------------------------
        auto vertexContent = LoadFile("data/shaders/3D_scene/cube.vert");
        auto *ptr = vertexContent.data();
        vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader_, 1, &ptr, nullptr);
        glCompileShader(vertex_shader_);
        //Check success status of shader compilation
        GLint success;
        glGetShaderiv(vertex_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shader\n";
        }
        //Load vertex shader model 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/model.vert");
        ptr = vertexContent.data();
        model_vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(model_vertex_shader_, 1, &ptr, nullptr);
        glCompileShader(model_vertex_shader_);
        glGetShaderiv(model_vertex_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shader for model\n";
        }
        //Load vertex shader map 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/skybox.vert");
        ptr = vertexContent.data();
        cube_map_vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(cube_map_vertex_shader_, 1, &ptr, nullptr);
        glCompileShader(cube_map_vertex_shader_);
        glGetShaderiv(cube_map_vertex_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shader for map\n";
        }

        //Load fragment shaders cube 1 ---------------------------------------------------------
        auto fragmentContent = LoadFile("data/shaders/3D_scene/cube.frag");
        ptr = fragmentContent.data();
        fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader_, 1, &ptr, nullptr);
        glCompileShader(fragment_shader_);
        //Check success status of shader compilation
        glGetShaderiv(fragment_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader\n";
        }
        //Load fragment shaders model 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/model.frag");
        ptr = fragmentContent.data();
        model_fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(model_fragment_shader_, 1, &ptr, nullptr);
        glCompileShader(model_fragment_shader_);
        //Check success status of shader compilation
        glGetShaderiv(model_fragment_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader for model\n";
        }
        //Load fragment shaders map 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/skybox.frag");
        ptr = fragmentContent.data();
        cube_map_fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(cube_map_fragment_shader_, 1, &ptr, nullptr);
        glCompileShader(cube_map_fragment_shader_);
        //Check success status of shader compilation
        glGetShaderiv(cube_map_fragment_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader for cube map\n";
        }

        //----------------------------------------------------------- set programs

        //Load program/pipeline
        program_ = glCreateProgram();
        program_model_ = glCreateProgram();
        program_cube_map_ = glCreateProgram();

        glAttachShader(program_, vertex_shader_);
        glAttachShader(program_, fragment_shader_);

        glAttachShader(program_model_, model_vertex_shader_);
        glAttachShader(program_model_, model_fragment_shader_);

        glAttachShader(program_cube_map_, cube_map_vertex_shader_);
        glAttachShader(program_cube_map_, cube_map_fragment_shader_);

        glLinkProgram(program_);
        glLinkProgram(program_model_);
        glLinkProgram(program_cube_map_);
        //Check if shader program was linked correctly

        glGetProgramiv(program_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking shader program\n";
        }
        glGetProgramiv(program_model_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking model shader program\n";
        }
        glGetProgramiv(program_cube_map_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking map shader program\n";
        }

        //----------------------------------------------------------- set VAO / VBO

        skybox_vao_.Create();
        skybox_vbo_.Create();

        //----------------------------------------------------------- set pointers

        camera_ = new Camera;
        std::string path_1 = "data/texture/3D/ROCK/LowPolyRockPack.obj";
        std::string path_2 = "data/texture/3D/backpack/backpack.obj";
        rock_model_ = new Model(path_2);

        //----------------------------------------------------------- frame buffer / render buffer

        float skyboxVertices[] = {
                // positions
                -1.0f, 1.0f, -1.0f,
                -1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, 1.0f, -1.0f,
                -1.0f, 1.0f, -1.0f,

                -1.0f, -1.0f, 1.0f,
                -1.0f, -1.0f, -1.0f,
                -1.0f, 1.0f, -1.0f,
                -1.0f, 1.0f, -1.0f,
                -1.0f, 1.0f, 1.0f,
                -1.0f, -1.0f, 1.0f,

                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, 1.0f,
                1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,

                -1.0f, -1.0f, 1.0f,
                -1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 1.0f,
                -1.0f, -1.0f, 1.0f,

                -1.0f, 1.0f, -1.0f,
                1.0f, 1.0f, -1.0f,
                1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f,
                -1.0f, 1.0f, 1.0f,
                -1.0f, 1.0f, -1.0f,

                -1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f, 1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f, 1.0f,
                1.0f, -1.0f, 1.0f
        };

        std::ranges::copy(skyboxVertices, skybox_vertices_);

        //skybox VAO
        skybox_vao_.Bind();
        skybox_vbo_.Bind();
        skybox_vbo_.BindData(sizeof(skybox_vertices_), &skybox_vertices_, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) nullptr);

        glUseProgram(program_cube_map_);
        glUniform1i(glGetUniformLocation(program_cube_map_, "skybox"), 0);


        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    //TODO update END because added cube map
    void FinalScene::End() {
        //Unload program/pipeline
        glDeleteProgram(program_);
        glDeleteProgram(program_model_);

        glDeleteShader(vertex_shader_);
        glDeleteShader(fragment_shader_);

        glDeleteShader(model_vertex_shader_);
        glDeleteShader(model_fragment_shader_);

        free(camera_);
        free(rock_model_);
    }

    void FinalScene::Update(float dt) {
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

        //Draw program -> cubes --------------------------------------------------------------------------
        glUseProgram(program_);

        SetView(projection, program_);

        //draw programme -> 3D model --------------------------------------------------------------------------
        glUseProgram(program_model_);
        glDisable(GL_CULL_FACE);
        glDepthFunc(GL_LEQUAL);

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms

        // view/projection transformations
        SetView(projection, program_model_);

        // render the loaded model
        auto model = glm::mat4(1.0f);
        model = glm::translate(model,
                               glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));    // it's a bit too big for our scene, so scale it down
        glUniformMatrix4fv(glGetUniformLocation(program_model_, "model"), 1, GL_FALSE, glm::value_ptr(model));
        rock_model_->Draw(program_model_);

        //draw programme -> cube map --------------------------------------------------------------------------
        glDisable(GL_CULL_FACE);
        glDepthFunc(GL_LEQUAL);
        glUseProgram(program_cube_map_);
        auto view = glm::mat4(glm::mat3(camera_->view())); // remove translation from the view matrix
        int viewLocP = glGetUniformLocation(program_cube_map_, "view");
        glUniformMatrix4fv(viewLocP,
                           1, GL_FALSE,
                           glm::value_ptr(view)
        );
        int projectionLocP = glGetUniformLocation(program_cube_map_, "projection");
        glUniformMatrix4fv(projectionLocP,
                           1, GL_FALSE,
                           glm::value_ptr(projection)
        );
        // skybox cube
        skybox_vao_.Bind();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map_text_);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    void FinalScene::SetView(const glm::mat4 &projection, GLuint &program) const {
        int viewLocP = glGetUniformLocation(program, "view");
        glUniformMatrix4fv(viewLocP, 1, GL_FALSE, glm::value_ptr(camera_->view()));


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