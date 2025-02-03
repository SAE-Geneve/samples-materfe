//
// Created by Mat on 1/9/2025.
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

    static constexpr unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    static constexpr unsigned int SCREEN_WIDTH = 1200, SCREEN_HEIGHT = 800;

    class PointShadowMapping final : public Scene {
    public:
        void Begin() override;

        void End() override;

        void Update(float dt) override;

        void OnEvent(const SDL_Event &event, float dt) override;

    private:
        bool are_shadows_active_ = true;
        unsigned int wood_texture_ = 0;
        unsigned int depth_cube_map_ = 0;
        unsigned int depth_map_fbo_ = 0;
        glm::vec3 light_pos_{};

        GLuint point_shadow_vertex_shader_ = 0;
        GLuint point_shadow_depth_vertex_shader_ = 0;

        GLuint point_shadow_depth_geometry_shader_ = 0;

        GLuint point_shadow_fragment_shader_ = 0;
        GLuint point_shadow_depth_fragment_shader_ = 0;

        GLuint point_shadow_program_ = 0;
        GLuint point_shadow_depth_program_ = 0;

        Camera *camera_ = nullptr;

        VAO cube_vao_{};
        VBO cube_vbo_{};

        void SetCameraDetails(const glm::mat4 &projection, GLuint &program) const;

        void RenderScene(const GLuint &shader) const;
        void RenderCube()const;
    };

    void PointShadowMapping::OnEvent(const SDL_Event &event, const float dt) {
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

    void PointShadowMapping::Begin() {

        //load texture

        //Load vertex shader cube 1 ---------------------------------------------------------
        auto vertexContent = LoadFile("data/shaders/3D_scene/point_shadows/point_shadow.vert");
        auto *ptr = vertexContent.data();
        point_shadow_vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(point_shadow_vertex_shader_, 1, &ptr, nullptr);
        glCompileShader(point_shadow_vertex_shader_);
        //Check success status of shader compilation
        GLint success;
        glGetShaderiv(point_shadow_vertex_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shader\n";
        }
        //Load vertex shader cube 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/point_shadows/point_shadow_depth.vert");
        ptr = vertexContent.data();
        point_shadow_depth_vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(point_shadow_depth_vertex_shader_, 1, &ptr, nullptr);
        glCompileShader(point_shadow_depth_vertex_shader_);
        //Check success status of shader compilation
        glGetShaderiv(point_shadow_depth_vertex_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex depth shader\n";
        }
        //Load vertex shader cube 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/point_shadows/point_shadow_depth.geom");
        ptr = vertexContent.data();
        point_shadow_depth_geometry_shader_ = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(point_shadow_depth_geometry_shader_, 1, &ptr, nullptr);
        glCompileShader(point_shadow_depth_geometry_shader_);
        //Check success status of shader compilation
        glGetShaderiv(point_shadow_depth_geometry_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading geometry depth shader\n";
        }
        std::cout << "passed geometry\n";

        //Load fragment shaders cube 1 ---------------------------------------------------------
        auto fragmentContent = LoadFile("data/shaders/3D_scene/point_shadows/point_shadow.frag");
        ptr = fragmentContent.data();
        point_shadow_fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(point_shadow_fragment_shader_, 1, &ptr, nullptr);
        glCompileShader(point_shadow_fragment_shader_);
        //Check success status of shader compilation
        glGetShaderiv(point_shadow_fragment_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader\n";
        }

        std::cout << "passed truc\n";
        //Load fragment shaders cube 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/point_shadows/point_shadow_depth.frag");
        ptr = fragmentContent.data();
        point_shadow_depth_fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(point_shadow_depth_fragment_shader_, 1, &ptr, nullptr);
        glCompileShader(point_shadow_depth_fragment_shader_);
        //Check success status of shader compilation
        glGetShaderiv(point_shadow_depth_fragment_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading depth fragment shader\n";
        }
        std::cout << "passed truc\n";

        //----------------------------------------------------------- set programs

        point_shadow_program_ = glCreateProgram();
        point_shadow_depth_program_ = glCreateProgram();

        glAttachShader(point_shadow_program_, point_shadow_vertex_shader_);
        glAttachShader(point_shadow_program_, point_shadow_fragment_shader_);

        glAttachShader(point_shadow_depth_program_, point_shadow_depth_vertex_shader_);
        glAttachShader(point_shadow_depth_program_, point_shadow_depth_fragment_shader_);
        glAttachShader(point_shadow_depth_program_, point_shadow_depth_geometry_shader_);

        glLinkProgram(point_shadow_program_);
        glLinkProgram(point_shadow_depth_program_);

        //Check if shader program was linked correctly
        glGetProgramiv(point_shadow_program_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking shader program\n";
        }
        glGetProgramiv(point_shadow_depth_program_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking depth shader program\n";
        }

        std::cout << "passed pipeline\n";
        //----------------------------------------------------------- set VAO

        cube_vao_.Create();
        cube_vao_.Create();

        //----------------------------------------------------------- set pointers

        camera_ = new Camera;

        //----------------------------------------------------------- frame buffer / render buffer
        // load textures
        // -------------
        wood_texture_ = TextureManager::LoadTexture("data/texture/2D/box.jpg");

        // configure depth map frame buffer
        // -----------------------


        glGenFramebuffers(1, &depth_map_fbo_);
        // create depth cubemap texture
        glGenTextures(1, &depth_cube_map_);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cube_map_);
        for (unsigned int i = 0; i < 6; ++i)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                         SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        // attach depth texture as FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo_);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_cube_map_, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glUseProgram(point_shadow_program_);
        glUniform1i(glGetUniformLocation(point_shadow_program_, "diffuseTexture"), 0);
        glUniform1i(glGetUniformLocation(point_shadow_program_, "depthMap"), 1);

        std::cout << "passed fbo\n";

        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //-----------------------------------------------------------
    }

    void PointShadowMapping::End() {
        //Unload program/pipeline

        glDeleteShader(point_shadow_vertex_shader_);

        free(camera_);
    }

    void PointShadowMapping::RenderCube() const {
        // initialize (if necessary)
        float vertices[] = {
                // back face
                -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
                1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
                -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // top-left
                // front face
                -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
                1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // bottom-right
                1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
                1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
                -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // top-left
                -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
                // left face
                -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
                -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
                -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
                // right face
                1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
                1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
                1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left
                // bottom face
                -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
                1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
                1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
                -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
                // top face
                -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
                1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
                1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right
                1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
                -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
                -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f  // bottom-left
        };
        cube_vbo_.Bind();
        cube_vbo_.BindData(sizeof(vertices), vertices, GL_STATIC_DRAW);
        cube_vao_.Bind();
        // link vertex attributes
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        // render Cube
        //cube_vao_.Bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    void PointShadowMapping::RenderScene(const GLuint &shader) const {
        // room cube
        auto model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(5.0f));
        glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE,
                           glm::value_ptr(model));
        glDisable(GL_CULL_FACE); // note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods.

        glUniform1i(glGetUniformLocation(shader, "reverse_normals"), 1);
        std::cout <<"passed first reverse\n";
        RenderCube();
        std::cout << "passed render\n";
        glUniform1i(glGetUniformLocation(shader, "reverse_normals"), 0); // and of course disable it
        std::cout << "passed reverse\n";
        glEnable(GL_CULL_FACE);
        // cubes
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(4.0f, -3.5f, 0.0));
        model = glm::scale(model, glm::vec3(0.5f));
        glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE,
                           glm::value_ptr(model));
        RenderCube();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 3.0f, 1.0));
        model = glm::scale(model, glm::vec3(0.75f));
        glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE,
                           glm::value_ptr(model));
        RenderCube();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-3.0f, -1.0f, 0.0));
        model = glm::scale(model, glm::vec3(0.5f));
        glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE,
                           glm::value_ptr(model));
        RenderCube();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.5f, 1.0f, 1.5));
        model = glm::scale(model, glm::vec3(0.5f));
        glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE,
                           glm::value_ptr(model));
        RenderCube();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.5f, 2.0f, -3.0));
        model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        model = glm::scale(model, glm::vec3(0.75f));
        glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE,
                           glm::value_ptr(model));
        RenderCube();
    }

    void PointShadowMapping::Update(float dt) {
        // move light position over time
        light_pos_.z = static_cast<float>(sin(dt * 0.5) * 3.0);
        std::cout << "passed light pos\n";

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 0. create depth cubemap transformation matrices
        // -----------------------------------------------
        float near_plane = 1.0f;
        float far_plane = 25.0f;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f),
                                                (float) SHADOW_WIDTH / (float) SHADOW_HEIGHT,
                                                near_plane, far_plane);
        std::vector<glm::mat4> shadowTransforms{};
        shadowTransforms.resize(6);
        shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos_, light_pos_ + glm::vec3(1.0f, 0.0f, 0.0f),
                                                            glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos_, light_pos_ + glm::vec3(-1.0f, 0.0f, 0.0f),
                                                            glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos_, light_pos_ + glm::vec3(0.0f, 1.0f, 0.0f),
                                                            glm::vec3(0.0f, 0.0f, 1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos_, light_pos_ + glm::vec3(0.0f, -1.0f, 0.0f),
                                                            glm::vec3(0.0f, 0.0f, -1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos_, light_pos_ + glm::vec3(0.0f, 0.0f, 1.0f),
                                                            glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos_, light_pos_ + glm::vec3(0.0f, 0.0f, -1.0f),
                                                            glm::vec3(0.0f, -1.0f, 0.0f)));
        std::cout << "passed vector\n";

        // 1. render scene to depth cubemap
        // --------------------------------
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo_);
        glClear(GL_DEPTH_BUFFER_BIT);
        glUseProgram(point_shadow_depth_program_);
        for (unsigned int i = 0; i < 6; ++i) {
            std::string location = "shadowMatrices[" + std::to_string(i) + "]";
            glUniformMatrix4fv(glGetUniformLocation(point_shadow_depth_program_, location.c_str()), 1, GL_FALSE,
                               glm::value_ptr(shadowTransforms[i]));
        }
        std::cout << "passed string\n";
        glUniform1f(glGetUniformLocation(point_shadow_depth_program_, "far_plane"), far_plane);
        glUniform3f(glGetUniformLocation(point_shadow_depth_program_, "lightPos"), light_pos_.x, light_pos_.y,
                    light_pos_.z);
        std::cout << "passed uni\n";
        RenderScene(point_shadow_depth_program_);
        std::cout << "passed render\n";
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. render scene as normal
        // -------------------------
        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(point_shadow_program_);
        const float aspect = 1200.0f / 800.0f; //see in the engine.Begin() -> window size
        const float zNear = 0.1f;
        const float zFar = 100.0f;
        const float fovY = std::numbers::pi_v<float> / 2;

        auto projection = glm::perspective(fovY, aspect, zNear, zFar);
        SetCameraDetails(projection, point_shadow_program_);
        std::cout << "passed camera\n";

        // set lighting uniforms
        glUniform3f(glGetUniformLocation(point_shadow_program_, "lightPos"), light_pos_.x, light_pos_.y, light_pos_.z);
        glUniform3f(glGetUniformLocation(point_shadow_program_, "viewPos"), camera_->position_.x, camera_->position_.y,
                    camera_->position_.z);
        glUniform1i(glGetUniformLocation(point_shadow_program_, "shadows"), are_shadows_active_);
        glUniform1f(glGetUniformLocation(point_shadow_program_, "far_plane"), far_plane);

        std::cout << "passed uni\n";
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wood_texture_);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cube_map_);
        RenderScene(point_shadow_program_);
    }

    void PointShadowMapping::SetCameraDetails(const glm::mat4 &projection, GLuint &program) const {
        int viewLocP = glGetUniformLocation(program, "view");
        glUniformMatrix4fv(viewLocP, 1, GL_FALSE, glm::value_ptr(camera_->view()));

        int projectionLocP = glGetUniformLocation(program, "projection");
        glUniformMatrix4fv(projectionLocP, 1, GL_FALSE, glm::value_ptr(projection));
    }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
    gpr::PointShadowMapping scene;
    gpr::Engine engine(&scene);
    engine.Run();

    return EXIT_SUCCESS;
}