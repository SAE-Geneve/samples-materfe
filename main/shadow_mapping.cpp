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

    class ShadowMapping final : public Scene {
    public:
        void Begin() override;

        void End() override;

        void Update(float dt) override;

        void OnEvent(const SDL_Event &event, float dt) override;

    private:
        unsigned int wood_texture_ = 0;
        unsigned int depth_map_ = 0;

        GLuint framebuffer_depth_map = 0;

        GLuint vertexShader_ = 0;
        GLuint vertexDepthShader_ = 0;
        GLuint vertexQuadShader_ = 0;

        GLuint fragmentShader_ = 0;
        GLuint fragmentDepthShader_ = 0;
        GLuint fragmentQuadShader_ = 0;

        GLuint program_ = 0;
        GLuint program_single_depth_texture_ = 0;
        GLuint program_debug_depth_quad_ = 0;

        Camera *camera_ = nullptr;
        VAO plane_vao_{};
        VAO cube_vao_{};
        VBO plane_vbo_{};
        VBO cube_vbo_{};

        void SetView(const glm::mat4 &projection, GLuint &program) const;

        void RenderScene(const GLuint &shader) const;

        void RenderCube() const;
    };

    void ShadowMapping::OnEvent(const SDL_Event &event, const float dt) {
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

    void ShadowMapping::Begin() {

        //load texture

        //Load vertex shader cube 1 ---------------------------------------------------------
        auto vertexContent = LoadFile("data/shaders/3D_scene/shadow_mapping.vert");
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
        vertexContent = LoadFile("data/shaders/3D_scene/shadow_mapping_depth.vert");
        ptr = vertexContent.data();
        vertexDepthShader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexDepthShader_, 1, &ptr, nullptr);
        glCompileShader(vertexDepthShader_);
        //Check success status of shader compilation
        glGetShaderiv(vertexDepthShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex depth shader\n";
        }
        //Load vertex shader cube 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/debug_quad.vert");
        ptr = vertexContent.data();
        vertexQuadShader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexQuadShader_, 1, &ptr, nullptr);
        glCompileShader(vertexQuadShader_);
        //Check success status of shader compilation
        glGetShaderiv(vertexQuadShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex quad shader\n";
        }

        //Load fragment shaders cube 1 ---------------------------------------------------------
        auto fragmentContent = LoadFile("data/shaders/3D_scene/shadow_mapping.frag");
        ptr = fragmentContent.data();
        fragmentShader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader_, 1, &ptr, nullptr);
        glCompileShader(fragmentShader_);
        //Check success status of shader compilation
        glGetShaderiv(fragmentShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader\n";
        }
        //Load fragment shaders cube 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/shadow_mapping_depth.frag");
        ptr = fragmentContent.data();
        fragmentDepthShader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentDepthShader_, 1, &ptr, nullptr);
        glCompileShader(fragmentDepthShader_);
        //Check success status of shader compilation
        glGetShaderiv(fragmentDepthShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading depth fragment shader\n";
        }
        //Load fragment shaders cube 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/debug_quad.frag");
        ptr = fragmentContent.data();
        fragmentQuadShader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentQuadShader_, 1, &ptr, nullptr);
        glCompileShader(fragmentQuadShader_);
        //Check success status of shader compilation
        glGetShaderiv(fragmentQuadShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading quad fragment shader\n";
        }


        //----------------------------------------------------------- set programs

        //Load program/pipeline
        program_ = glCreateProgram();
        program_debug_depth_quad_ = glCreateProgram();
        program_single_depth_texture_ = glCreateProgram();

        glAttachShader(program_, vertexShader_);
        glAttachShader(program_, fragmentShader_);

        glAttachShader(program_debug_depth_quad_, vertexQuadShader_);
        glAttachShader(program_debug_depth_quad_, fragmentQuadShader_);

        glAttachShader(program_single_depth_texture_, vertexDepthShader_);
        glAttachShader(program_single_depth_texture_, fragmentDepthShader_);

        glLinkProgram(program_);
        glLinkProgram(program_debug_depth_quad_);
        glLinkProgram(program_single_depth_texture_);

        //Check if shader program was linked correctly
        glGetProgramiv(program_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking shader program\n";
        }
        glGetProgramiv(program_debug_depth_quad_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking quad shader program\n";
        }
        glGetProgramiv(program_single_depth_texture_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking depth shader program\n";
        }

        //----------------------------------------------------------- set VAO

        plane_vao_.Create();
        cube_vao_.Create();
        plane_vbo_.Create();
        cube_vbo_.Create();

        //----------------------------------------------------------- set pointers

        camera_ = new Camera;

        //----------------------------------------------------------- frame buffer / render buffer

        float planeVertices[] = {
                // positions            // normals         // texcoords
                25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
                -25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                -25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f,

                25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
                -25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f,
                25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 25.0f
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

        // load textures
        // -------------
        wood_texture_ = TextureManager::LoadTexture("data/texture/2D/box.jpg");

        // configure depth map frame buffer
        // -----------------------

        glGenFramebuffers(1, &framebuffer_depth_map);
        // create depth texture
        glGenTextures(1, &depth_map_);
        glBindTexture(GL_TEXTURE_2D, depth_map_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = {1.0, 1.0, 1.0, 1.0};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        // attach depth texture as FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_depth_map);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map_, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // shader configuration
        // --------------------
        glUseProgram(program_);
        glUniform1i(glGetUniformLocation(program_, "diffuseTexture"), 0);
        glUniform1i(glGetUniformLocation(program_, "shadowMap"), 1);
        glUseProgram(program_debug_depth_quad_);
        glUniform1i(glGetUniformLocation(program_debug_depth_quad_, "depthMap"), 0);

        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //-----------------------------------------------------------
    }

    void ShadowMapping::End() {
        //Unload program/pipeline
        glDeleteProgram(program_);

        glDeleteShader(vertexShader_);
        glDeleteShader(fragmentShader_);

        free(camera_);
    }


    void ShadowMapping::RenderCube() const {
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
        cube_vao_.Bind();
        cube_vbo_.Bind();
        cube_vbo_.BindData(sizeof(vertices), vertices, GL_STATIC_DRAW);
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
        cube_vao_.Bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    void ShadowMapping::RenderScene(const GLuint &shader) const {
        // floor
        auto model = glm::mat4(1.0f);
        int loc = glGetUniformLocation(shader, "model");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));

        plane_vao_.Bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // cubes
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
        model = glm::scale(model, glm::vec3(0.5f));
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));

        RenderCube();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
        model = glm::scale(model, glm::vec3(0.5f));
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));
        RenderCube();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
        model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        model = glm::scale(model, glm::vec3(0.25));
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));
        RenderCube();
    }

    void ShadowMapping::Update(float dt) {
        glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);
        glEnable(GL_DEPTH_TEST);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1. render depth of scene to texture (from light's perspective)
        // --------------------------------------------------------------
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 7.5f;
        //lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;
        // render scene from light's point of view

        //set uniform
        glUseProgram(program_single_depth_texture_);
        int lightSpaceLocP = glGetUniformLocation(program_single_depth_texture_, "lightSpaceMatrix");
        glUniformMatrix4fv(lightSpaceLocP, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

        //set framebuffer
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_depth_map);
        glClear(GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wood_texture_);
        RenderScene(program_single_depth_texture_);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // reset viewport
        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 2. render scene as normal using the generated depth/shadow map
        // --------------------------------------------------------------
        glUseProgram(program_);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float) SCREEN_WIDTH / (float) SCREEN_HEIGHT, 0.1f,
                                                100.0f);
        SetView(projection, program_);

        // set light uniforms
        glUniform3f(glGetUniformLocation(program_, "viewPos"), camera_->position_.x, camera_->position_.y,
                    camera_->position_.z);
        glUniform3f(glGetUniformLocation(program_, "lightPos"), lightPos.x, lightPos.y,
                    lightPos.z);
        lightSpaceLocP = glGetUniformLocation(program_, "lightSpaceMatrix");
        glUniformMatrix4fv(lightSpaceLocP, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

        //set textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wood_texture_);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depth_map_);
        RenderScene(program_);

        // render Depth map to quad for visual debugging
        // ---------------------------------------------
//        glUseProgram(program_debug_depth_quad_);
//        glUniform1f(glGetUniformLocation(program_debug_depth_quad_, "near_plane"), near_plane);
//        glUniform1f(glGetUniformLocation(program_debug_depth_quad_, "far_plane"), far_plane);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depth_map_);
    }

    void ShadowMapping::SetView(const glm::mat4 &projection, GLuint &program) const {
        int viewLocP = glGetUniformLocation(program, "view");
        glUniformMatrix4fv(viewLocP, 1, GL_FALSE, glm::value_ptr(camera_->view()));

        int projectionLocP = glGetUniformLocation(program, "projection");
        glUniformMatrix4fv(projectionLocP, 1, GL_FALSE, glm::value_ptr(projection));
    }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
    gpr::ShadowMapping scene;
    gpr::Engine engine(&scene);
    engine.Run();

    return EXIT_SUCCESS;
}