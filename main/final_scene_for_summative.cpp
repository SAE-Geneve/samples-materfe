//
// Created by Mat on 1/20/2025.
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
#include "utility_tools.h"

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

        void DrawImGui() override;

    private:
        float elapsed_time_ = 0.0f;
        unsigned int cube_map_text_ = 0;
        float skybox_vertices_[108] = {};
        std::array<glm::vec3, 5> light_cube_pos_{};
        std::array<glm::vec3, 5> light_cube_color_{};
        bool reverse_enable_ = false;

        //all vertex shaders-------------
        GLuint model_vertex_shader_ = 0;
        GLuint cube_map_vertex_shader_ = 0;
        GLuint screen_quad_vertex_shader_ = 0;
        GLuint gamma_vertex_shader_ = 0;
        GLuint light_cube_vertex_shader_ = 0;

        //all fragment shaders-------------
        GLuint model_fragment_shader_ = 0;
        GLuint cube_map_fragment_shader_ = 0;
        GLuint screen_quad_fragment_shader_ = 0;
        GLuint gamma_fragment_shader_ = 0;
        GLuint light_cube_fragment_shader_ = 0;

        //all programs-------------
        GLuint program_model_ = 0;
        GLuint program_cube_map_ = 0;
        GLuint program_screen_frame_buffer_ = 0;
        GLuint program_gamma_ = 0;
        GLuint program_light_cube_ = 0;

        //all frameBuffers-----------------
        GLuint screen_frame_buffer_ = 0;
        GLuint text_for_screen_frame_buffer = 0;

        //all renderBuffers----------------
        GLuint render_buffer_for_screen_buffer_ = 0;

        Camera *camera_ = nullptr;
        Model *rock_model_ = nullptr;
        Frustum frustum{};

        VAO skybox_vao_{};
        VAO quad_vao_{};
        VBO skybox_vbo_{};


        void SetView(const glm::mat4 &projection, GLuint &program) const;

        void SetLightCubes();

        void CreateLightCubeAt(glm::vec3 position, glm::vec3 color) const;
    };

    void FinalScene::SetLightCubes() {
        for (auto &_: light_cube_pos_) {
            _.x = tools::GenerateRandomNumber(-100.0f, 100.0f);
            _.y = tools::GenerateRandomNumber(0.0f, 1.0f);
            _.z = tools::GenerateRandomNumber(-100.0f, 100.0f);
        }
        for (auto &_: light_cube_color_) {
            _.x = tools::GenerateRandomNumber(0.0f, 1.0f);
            _.y = tools::GenerateRandomNumber(0.0f, 1.0f);
            _.z = tools::GenerateRandomNumber(0.0f, 1.0f);
        }
    }

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

        SetLightCubes();

        //load textures
        std::vector<std::string> faces =
                {
                        "data/texture/3D/cube_map/posx.jpg",
                        "data/texture/3D/cube_map/negx.jpg",
                        "data/texture/3D/cube_map/posy.jpg",
                        "data/texture/3D/cube_map/negy.jpg",
                        "data/texture/3D/cube_map/posz.jpg",
                        "data/texture/3D/cube_map/negz.jpg"
                };
        cube_map_text_ = TextureManager::loadCubemap(faces);


        //Load vertex shader cube 1 ---------------------------------------------------------
        auto vertexContent = LoadFile("data/shaders/3D_scene/cube.vert");
        auto *ptr = vertexContent.data();
        GLint success;
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
        //Load vertex shader screen 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/quad.vert");
        ptr = vertexContent.data();
        screen_quad_vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(screen_quad_vertex_shader_, 1, &ptr, nullptr);
        glCompileShader(screen_quad_vertex_shader_);
        glGetShaderiv(screen_quad_vertex_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shader for screen\n";
        }
        //Load vertex shader gamma 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/gamma_correction/gamma_correction.vert");
        ptr = vertexContent.data();
        gamma_vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(gamma_vertex_shader_, 1, &ptr, nullptr);
        glCompileShader(gamma_vertex_shader_);
        glGetShaderiv(gamma_vertex_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shader for gamma\n";
        }
        //Load vertex shader light 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/light.vert");
        ptr = vertexContent.data();
        light_cube_vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(light_cube_vertex_shader_, 1, &ptr, nullptr);
        glCompileShader(light_cube_vertex_shader_);
        glGetShaderiv(light_cube_vertex_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shader for light cube\n";
        }

        //Load fragment shaders cube 1 ---------------------------------------------------------
        auto fragmentContent = LoadFile("data/shaders/3D_scene/cube.frag");
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
        //Load fragment shaders screen 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/quad.frag");
        ptr = fragmentContent.data();
        screen_quad_fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(screen_quad_fragment_shader_, 1, &ptr, nullptr);
        glCompileShader(screen_quad_fragment_shader_);
        //Check success status of shader compilation
        glGetShaderiv(screen_quad_fragment_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader for screen\n";
        }
        //Load fragment shaders gamma 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/gamma_correction/gamma_correction.frag");
        ptr = fragmentContent.data();
        gamma_fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(gamma_fragment_shader_, 1, &ptr, nullptr);
        glCompileShader(gamma_fragment_shader_);
        //Check success status of shader compilation
        glGetShaderiv(gamma_fragment_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader for gamma\n";
        }
        //Load fragment shaders light 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/light.frag");
        ptr = fragmentContent.data();
        light_cube_fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(light_cube_fragment_shader_, 1, &ptr, nullptr);
        glCompileShader(light_cube_fragment_shader_);
        //Check success status of shader compilation
        glGetShaderiv(light_cube_fragment_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader for light cube\n";
        }

        //----------------------------------------------------------- set programs

        //Load program/pipeline
        program_model_ = glCreateProgram();
        program_cube_map_ = glCreateProgram();
        program_screen_frame_buffer_ = glCreateProgram();
        program_gamma_ = glCreateProgram();
        program_light_cube_ = glCreateProgram();


        glAttachShader(program_model_, model_vertex_shader_);
        glAttachShader(program_model_, model_fragment_shader_);

        glAttachShader(program_cube_map_, cube_map_vertex_shader_);
        glAttachShader(program_cube_map_, cube_map_fragment_shader_);

        glAttachShader(program_screen_frame_buffer_, screen_quad_vertex_shader_);
        glAttachShader(program_screen_frame_buffer_, screen_quad_fragment_shader_);

        glAttachShader(program_gamma_, gamma_vertex_shader_);
        glAttachShader(program_gamma_, gamma_fragment_shader_);

        glAttachShader(program_light_cube_, light_cube_vertex_shader_);
        glAttachShader(program_light_cube_, light_cube_fragment_shader_);

        glLinkProgram(program_model_);
        glLinkProgram(program_cube_map_);
        glLinkProgram(program_screen_frame_buffer_);
        glLinkProgram(program_gamma_);
        glLinkProgram(program_light_cube_);
        //Check if shader program was linked correctly

        glGetProgramiv(program_model_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking model shader program\n";
        }
        glGetProgramiv(program_cube_map_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking map shader program\n";
        }
        glGetProgramiv(program_screen_frame_buffer_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking screen shader program\n";
        }
        glGetProgramiv(program_gamma_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking gamma shader program\n";
        }
        glGetProgramiv(program_gamma_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking light cube shader program\n";
        }

        //----------------------------------------------------------- set VAO / VBO

        skybox_vao_.Create();
        quad_vao_.Create();
        skybox_vbo_.Create();

        //----------------------------------------------------------- set pointers

        camera_ = new Camera;
        //std::string path_1 = "data/texture/3D/ROCK/LowPolyRockPack.obj";
        std::string path_2 = "data/texture/3D/tree_elm/scene.gltf";
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

        //create framebuffer
        glGenFramebuffers(1, &screen_frame_buffer_);
        glBindFramebuffer(GL_FRAMEBUFFER, screen_frame_buffer_);

        //Image setup
        glGenTextures(1, &text_for_screen_frame_buffer);
        glBindTexture(GL_TEXTURE_2D, text_for_screen_frame_buffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1200, 800, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // create a renderbuffer object for depth and stencil attachment
        glGenRenderbuffers(1, &render_buffer_for_screen_buffer_);
        glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_for_screen_buffer_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                              1200, 800); // use a single renderbuffer object for both a depth AND stencil buffer.
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               text_for_screen_frame_buffer, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  render_buffer_for_screen_buffer_);

        //check of done correctly + release memory
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << glCheckFramebufferStatus(GL_FRAMEBUFFER)
                      << std::endl;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    //TODO update the end with screen buffer stuff + gamma correction + lights
    void FinalScene::End() {
        //Unload program/pipeline
        glDeleteProgram(program_model_);
        glDeleteProgram(program_cube_map_);

        glDeleteShader(model_vertex_shader_);
        glDeleteShader(model_fragment_shader_);

        glDeleteShader(cube_map_vertex_shader_);
        glDeleteShader(cube_map_fragment_shader_);

        free(camera_);
        free(rock_model_);
    }

    void FinalScene::Update(float dt) {
        glBindFramebuffer(GL_FRAMEBUFFER, screen_frame_buffer_);
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

        //draw program -> light cubes -------------------------------------------------------------------------
        glUseProgram(program_light_cube_);
        for (std::size_t index = 0; index < light_cube_pos_.size(); index++) {
            SetView(projection, program_light_cube_);
            CreateLightCubeAt(glm::vec3(light_cube_pos_[index].x, light_cube_pos_[index].y, light_cube_pos_[index].z),
                              glm::vec3(light_cube_color_[index].x, light_cube_color_[index].y,light_cube_color_[index].z));
        }

        //draw programme -> 3D model --------------------------------------------------------------------------
        glUseProgram(program_model_);
        glDisable(GL_CULL_FACE);
        glDepthFunc(GL_LEQUAL);

        //glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
        //skybox cube
        skybox_vao_.Bind();

        //draw cube map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map_text_);
        //glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        //frame buffer screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program_screen_frame_buffer_);

        //set post process
        glUniform1i(glGetUniformLocation(program_screen_frame_buffer_, "reverse"), reverse_enable_);
        quad_vao_.Bind();
        glDisable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_2D, text_for_screen_frame_buffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
        DrawImGui();
    }

    void FinalScene::CreateLightCubeAt(const glm::vec3 position, const glm::vec3 color) const {
        auto model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        glUniformMatrix4fv(glGetUniformLocation(program_light_cube_, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform4f(glGetUniformLocation(program_light_cube_, "color"), color.x, color.y, color.z, 1.0f);

        //drawing
        VAO light_vao_;
        light_vao_.Create();
        light_vao_.Bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    void FinalScene::DrawImGui() {
        // Début ImGui
        ImGui::Begin("Controls");
        ImGui::Checkbox("Enable Reverse Post-Processing", &reverse_enable_);
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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