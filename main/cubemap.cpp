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
        float skybox_vertices_[108] = {};
        unsigned int texture_[2] = {0, 0};
        unsigned int cubeMapText_ = 0;
        unsigned int colorBuffers_[2]{};
        unsigned int pingpongFBO[2]{};
        unsigned int pingpongColorbuffers[2]{};
        std::vector<glm::vec3> lightPositions_;
        std::vector<glm::vec3> lightColors_;
        std::array<glm::vec3, 10> all_cubes_{};

        GLuint vertexShader_ = 0;
        GLuint lightVertexShader_ = 0;
        GLuint cubeMapVertexShader_ = 0;
        GLuint quadVertexShader_ = 0;
        GLuint bloomVertexShader_ = 0;
        GLuint blurVertexShader_ = 0;

        GLuint fragmentShader_ = 0;
        GLuint fragmentLightShader_ = 0;
        GLuint fragmentCubeMapShader_ = 0;
        GLuint fragmentquadShader_ = 0;
        GLuint fragmentbloomShader_ = 0;
        GLuint fragmentbloomLightShader_ = 0;
        GLuint fragmentblurrShader_ = 0;
        GLuint fragmentfinalBlurShader_ = 0;

        GLuint program_ = 0;
        GLuint program_light_ = 0;
        GLuint program_map_ = 0;
        GLuint program_quad_ = 0;
        GLuint program_bloom_ = 0;
        GLuint program_bloom_light_ = 0;
        GLuint program_blur_ = 0;
        GLuint program_final_blur_ = 0;

        //frame buffer :
        GLuint frame_buffer_ = 0;
        GLuint render_buffer_ = 0;
        GLuint texture_of_the_color_buffer_ = 0;
        unsigned int hdrFBO_ = 0;

        VAO vao_;
        VAO cube_vao_;
        VAO skybox_vao_;
        VAO quad_vao_;

        VBO skybox_vbo_{};
        VBO cube_vbo_{};
        VBO quad_vbo_{};

        Camera *camera_ = nullptr;
        Frustum frustum{};

        void SetAndBindTextures() const;

        void SetAndDrawMultipleCubes() const;

        void SetTheCubes();

        void SetUniformsProgram(const glm::vec3 &lightPos) const;

        void SetUniformsProgramLight(const glm::vec3 &lightPos) const;

        static void DrawLightProgram();

        void SetView(const glm::mat4 &projection, GLuint &program, const glm::mat4 &view = glm::mat4(1.0f)) const;
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
        //Load vertex shader quad 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/quad.vert");
        ptr = vertexContent.data();
        quadVertexShader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(quadVertexShader_, 1, &ptr, nullptr);
        glCompileShader(quadVertexShader_);
        glGetShaderiv(quadVertexShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shader for quad\n";
        }
        //Load vertex shader bloom 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/shader_bloom.vert");
        ptr = vertexContent.data();
        bloomVertexShader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(bloomVertexShader_, 1, &ptr, nullptr);
        glCompileShader(bloomVertexShader_);
        glGetShaderiv(bloomVertexShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shader for bloom\n";
        }
        //Load vertex shader blur 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/blur_shader.vert");
        ptr = vertexContent.data();
        blurVertexShader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(blurVertexShader_, 1, &ptr, nullptr);
        glCompileShader(blurVertexShader_);
        glGetShaderiv(blurVertexShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shader for blur\n";
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
        //Load fragment shaders quad 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/quad.frag");
        ptr = fragmentContent.data();
        fragmentquadShader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentquadShader_, 1, &ptr, nullptr);
        glCompileShader(fragmentquadShader_);
        //Check success status of shader compilation
        glGetShaderiv(fragmentquadShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader for cube quad\n";
        }
        //Load fragment shaders bloom 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/shader_bloom.frag");
        ptr = fragmentContent.data();
        fragmentbloomShader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentbloomShader_, 1, &ptr, nullptr);
        glCompileShader(fragmentbloomShader_);
        //Check success status of shader compilation
        glGetShaderiv(fragmentbloomShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader for bloom\n";
        }
        //Load fragment shaders bloom light 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/shader_light_bloom.frag");
        ptr = fragmentContent.data();
        fragmentbloomLightShader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentbloomLightShader_, 1, &ptr, nullptr);
        glCompileShader(fragmentbloomLightShader_);
        //Check success status of shader compilation
        glGetShaderiv(fragmentbloomLightShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader for bloom light\n";
        }
        //Load fragment shaders blur 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/blur_shader.frag");
        ptr = fragmentContent.data();
        fragmentblurrShader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentblurrShader_, 1, &ptr, nullptr);
        glCompileShader(fragmentblurrShader_);
        //Check success status of shader compilation
        glGetShaderiv(fragmentblurrShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader for blur\n";
        }
        //Load fragment shaders bloom final 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/final_bloom.frag");
        ptr = fragmentContent.data();
        fragmentfinalBlurShader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentfinalBlurShader_, 1, &ptr, nullptr);
        glCompileShader(fragmentfinalBlurShader_);
        //Check success status of shader compilation
        glGetShaderiv(fragmentfinalBlurShader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader for bloom final\n";
        }

        //----------------------------------------------------------- set programs

        //Load program/pipeline
        program_ = glCreateProgram();
        program_light_ = glCreateProgram();
        program_map_ = glCreateProgram();
        program_quad_ = glCreateProgram();
        program_bloom_ = glCreateProgram();
        program_bloom_light_ = glCreateProgram();
        program_blur_ = glCreateProgram();
        program_final_blur_ = glCreateProgram();

        glAttachShader(program_, vertexShader_);
        glAttachShader(program_, fragmentShader_);

        glAttachShader(program_light_, lightVertexShader_);
        glAttachShader(program_light_, fragmentLightShader_);

        glAttachShader(program_map_, cubeMapVertexShader_);
        glAttachShader(program_map_, fragmentCubeMapShader_);

        glAttachShader(program_quad_, quadVertexShader_);
        glAttachShader(program_quad_, fragmentquadShader_);

        glAttachShader(program_bloom_, bloomVertexShader_);
        glAttachShader(program_bloom_, fragmentbloomShader_);

        glAttachShader(program_bloom_light_, bloomVertexShader_);
        glAttachShader(program_bloom_light_, fragmentbloomLightShader_);

        glAttachShader(program_blur_, blurVertexShader_);
        glAttachShader(program_blur_, fragmentblurrShader_);

        glAttachShader(program_final_blur_, blurVertexShader_);
        glAttachShader(program_final_blur_, fragmentfinalBlurShader_);

        glLinkProgram(program_);
        glLinkProgram(program_light_);
        glLinkProgram(program_map_);
        glLinkProgram(program_quad_);
        glLinkProgram(program_bloom_);
        glLinkProgram(program_bloom_light_);
        glLinkProgram(program_blur_);
        glLinkProgram(program_final_blur_);
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
        glGetProgramiv(program_quad_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking shader program for quad\n";
        }
        glGetProgramiv(program_bloom_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking shader program for bloom\n";
        }
        glGetProgramiv(program_bloom_light_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking shader program for bloom light\n";
        }
        glGetProgramiv(program_blur_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking shader program for blur\n";
        }
        glGetProgramiv(program_final_blur_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking shader program for finality\n";
        }

        //----------------------------------------------------------- set VAO / VBO

        //Empty vao
        vao_.Create();
        skybox_vao_.Create();
        skybox_vbo_.Create();
        quad_vao_.Create();
        quad_vbo_.Create();
        cube_vao_.Create();
        cube_vbo_.Create();

        //----------------------------------------------------------- set pointers

        camera_ = new Camera;

        //----------------------------------------------------------- set vertices

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

        glUseProgram(program_map_);
        glUniform1i(glGetUniformLocation(program_map_, "skybox"), 0);


        //----------------------------------------------------------- frame buffer / render buffer


//        glGenFramebuffers(1, &frame_buffer_);
//        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_);
//        // create a color attachment texture
//
//        glGenTextures(1, &texture_of_the_color_buffer_);
//        glBindTexture(GL_TEXTURE_2D, texture_of_the_color_buffer_);
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1200, 800, 0,
//                     GL_RGB, GL_UNSIGNED_BYTE, nullptr);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
//
//
//        glGenRenderbuffers(1, &render_buffer_);
//        glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_);
//        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
//                              1200, 800); // use a single renderbuffer object for both a depth AND stencil buffer.
//
//        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
//                               texture_of_the_color_buffer_, 0);
//        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
//                                  render_buffer_); // now actually attach it
//
//        //check of done correctly + release memory
//        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
//            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << glCheckFramebufferStatus(GL_FRAMEBUFFER)
//            << std::endl;
//        }








        glGenFramebuffers(1, &hdrFBO_);
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO_);
        // create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
        glGenTextures(2, colorBuffers_);
        for (unsigned int i = 0; i < 2; i++) {
            glBindTexture(GL_TEXTURE_2D, colorBuffers_[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
                         1200, 800, 0, GL_RGBA, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                            GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            // attach texture to framebuffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                                   GL_TEXTURE_2D, colorBuffers_[i], 0);
        }
        // create and attach depth buffer (renderbuffer)
        unsigned int rboDepth;
        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1200, 800);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                  GL_RENDERBUFFER, rboDepth);
        // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
        unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2, attachments);
        // finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ping-pong-framebuffer for blurring

        glGenFramebuffers(2, pingpongFBO);
        glGenTextures(2, pingpongColorbuffers);
        for (unsigned int i = 0; i < 2; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
            glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1200, 800,
                         0, GL_RGBA, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                            GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                   pingpongColorbuffers[i], 0);
            // also check if frame buffers are complete (no need for depth buffer)
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "Framebuffer not complete!" << std::endl;
        }

        // lighting info
        // -------------
        // positions
        lightPositions_.emplace_back(0.0f, 0.5f, 1.5f);
        lightPositions_.emplace_back(-4.0f, 0.5f, -3.0f);
        lightPositions_.emplace_back(3.0f, 0.5f, 1.0f);
        lightPositions_.emplace_back(-.8f, 2.4f, -1.0f);
        // colors
        lightColors_.emplace_back(5.0f, 5.0f, 5.0f);
        lightColors_.emplace_back(10.0f, 0.0f, 0.0f);
        lightColors_.emplace_back(0.0f, 0.0f, 15.0f);
        lightColors_.emplace_back(0.0f, 5.0f, 0.0f);


        // shader configuration
        // --------------------
        glUseProgram(program_bloom_);
        glUniform1i(glGetUniformLocation(program_bloom_, "diffuseTexture"), 0);

        glUseProgram(program_blur_);
        int loc = glGetUniformLocation(program_blur_, "image");
        glUniform1d(loc, 0);
        glUseProgram(program_final_blur_);
        loc = glGetUniformLocation(program_final_blur_, "scene");
        glUniform1d(loc, 0);
        loc = glGetUniformLocation(program_final_blur_, "bloomBlur");
        glUniform1d(loc, 1);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void CubeMapScene::End() {
        //Unload program/pipeline
        glDeleteProgram(program_);
        glDeleteProgram(program_light_);
        glDeleteProgram(program_map_);
        glDeleteProgram(program_quad_);
        glDeleteProgram(program_bloom_);
        glDeleteProgram(program_bloom_light_);
        glDeleteProgram(program_blur_);
        glDeleteProgram(program_final_blur_);

        glDeleteShader(vertexShader_);
        glDeleteShader(fragmentShader_);

        glDeleteShader(lightVertexShader_);
        glDeleteShader(fragmentLightShader_);

        glDeleteShader(cubeMapVertexShader_);
        glDeleteShader(fragmentCubeMapShader_);

        glDeleteShader(quadVertexShader_);
        glDeleteShader(fragmentquadShader_);

        glDeleteShader(bloomVertexShader_);
        glDeleteShader(fragmentbloomShader_);
        glDeleteShader(fragmentbloomLightShader_);

        glDeleteShader(blurVertexShader_);
        glDeleteShader(fragmentblurrShader_);
        glDeleteShader(fragmentfinalBlurShader_);

        glDeleteFramebuffers(1, &frame_buffer_);
        glDeleteFramebuffers(1, &hdrFBO_);
        glDeleteFramebuffers(2, pingpongFBO);
        glDeleteRenderbuffers(1, &render_buffer_);

        free(camera_);
        vao_.Delete();
        skybox_vao_.Delete();
        skybox_vbo_.Delete();
        quad_vao_.Delete();
        quad_vbo_.Delete();
        cube_vao_.Delete();
        cube_vbo_.Delete();
    }

    void CubeMapScene::Update(float dt) {
        glm::mat4 projection;

        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO_);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CW);

        const float aspect = 1200.0f / 800.0f; //see in the engine.Begin() -> window size
        const float zNear = 0.1f;
        const float zFar = 100.0f;
        const float fovY = std::numbers::pi_v<float> / 2;

        frustum.CreateFrustumFromCamera(*camera_, aspect, fovY, zNear, zFar);
        projection = glm::perspective(fovY, aspect, zNear, zFar);

        //HDR FRAMEBUFFER for BLOOM
        //glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO_);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        auto model = glm::mat4(1.0f);


        glm::vec3 lightPos(1.2f, 1.0f, 2.0f);


        elapsed_time_ += dt;
        //Draw program -> cubes
        glUseProgram(program_);

        SetView(projection, program_);
        SetUniformsProgram(lightPos);

        SetAndBindTextures();

        SetAndDrawMultipleCubes();

        //CubeMap
        glDisable(GL_CULL_FACE);
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        glUseProgram(program_map_);
        auto view = glm::mat4(glm::mat3(camera_->view())); // remove translation from the view matrix
        int viewLocP = glGetUniformLocation(program_map_, "view");
        glUniformMatrix4fv(viewLocP,
                           1, GL_FALSE,
                           glm::value_ptr(view)
        );
        int projectionLocP = glGetUniformLocation(program_map_, "projection");
        glUniformMatrix4fv(projectionLocP,
                           1, GL_FALSE,
                           glm::value_ptr(projection)
        );
        // skybox cube
        skybox_vao_.Bind();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapText_);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);


        glUseProgram(program_bloom_);

        int projectionLoc = glGetUniformLocation(program_bloom_, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        int veiwLoc = glGetUniformLocation(program_bloom_, "view");
        glUniformMatrix4fv(veiwLoc, 1, GL_FALSE, glm::value_ptr(camera_->view()));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_[0]);


        // set lighting uniforms

        int tempLoc = glGetUniformLocation(program_bloom_, "lights[0].Position");
        glUniform3f(tempLoc, lightPositions_[0].x, lightPositions_[0].y, lightPositions_[0].z);
        tempLoc = glGetUniformLocation(program_bloom_, "lights[0].Color");
        glUniform3f(tempLoc, lightColors_[0].x, lightColors_[0].y, lightColors_[0].z);

        tempLoc = glGetUniformLocation(program_bloom_, "lights[1].Position");
        glUniform3f(tempLoc, lightPositions_[1].x, lightPositions_[1].y, lightPositions_[1].z);
        tempLoc = glGetUniformLocation(program_bloom_, "lights[1].Color");
        glUniform3f(tempLoc, lightColors_[1].x, lightColors_[1].y, lightColors_[1].z);

        tempLoc = glGetUniformLocation(program_bloom_, "lights[2].Position");
        glUniform3f(tempLoc, lightPositions_[2].x, lightPositions_[2].y, lightPositions_[2].z);
        tempLoc = glGetUniformLocation(program_bloom_, "lights[2].Color");
        glUniform3f(tempLoc, lightColors_[2].x, lightColors_[2].y, lightColors_[2].z);

        tempLoc = glGetUniformLocation(program_bloom_, "lights[3].Position");
        glUniform3f(tempLoc, lightPositions_[3].x, lightPositions_[3].y, lightPositions_[3].z);
        tempLoc = glGetUniformLocation(program_bloom_, "lights[3].Color");
        glUniform3f(tempLoc, lightColors_[3].x, lightColors_[3].y, lightColors_[3].z);

        int viewLoc = glGetUniformLocation(program_bloom_, "viewPos");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(camera_->position_));

        glUseProgram(program_bloom_light_);
        projectionLoc = glGetUniformLocation(program_bloom_light_, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        veiwLoc = glGetUniformLocation(program_bloom_light_, "view");
        glUniformMatrix4fv(veiwLoc, 1, GL_FALSE, glm::value_ptr(camera_->view()));

        for (unsigned int i = 0; i < lightPositions_.size(); i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(lightPositions_[i]));
            model = glm::scale(model, glm::vec3(0.25f));
            int tempModelLoc = glGetUniformLocation(program_bloom_light_, "model");
            glUniformMatrix4fv(tempModelLoc, 1, GL_FALSE, glm::value_ptr(model));
            int tempcolorLoc = glGetUniformLocation(program_bloom_light_, "lightColor");
            glUniform3f(tempcolorLoc, lightColors_[i].x, lightColors_[i].y, lightColors_[i].z);

            // initialize (if necessary)
            {
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
                // fill buffer
                cube_vbo_.Bind();
                cube_vbo_.BindData(sizeof(vertices), vertices, GL_STATIC_DRAW);
                cube_vao_.Bind();
                // link vertex attributes
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                                      (void *) nullptr);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                                      (void *) (3 * sizeof(float)));
                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                                      (void *) (6 * sizeof(float)));
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);
            }
            // render Cube
            cube_vao_.Bind();
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //blur-----------------------------
        glDisable(GL_CULL_FACE);

        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        glUseProgram(program_blur_);
        for (unsigned int i = 0; i < amount; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);

            int location = glGetUniformLocation(program_blur_, "horizontal");
            glUniform1d(location, horizontal);

            glBindTexture(GL_TEXTURE_2D, first_iteration ?
                                         colorBuffers_[1]: pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)

            float quadVertices[] = {
                    // positions        // texture Coords
                    -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                    1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                    1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            };
            // setup plane VAO
            quad_vao_.Bind();
            quad_vbo_.Bind();
            quad_vbo_.BindData(sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) nullptr);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
            quad_vao_.Bind();
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(0);


            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }



        glBindFramebuffer(GL_FRAMEBUFFER, 0);



        // 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
        // --------------------------------------------------------------------------------------------------------------------------
        bool bloom = true;
        float exposure = 1.0f;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program_final_blur_);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers_[0]); // Scene texture
        glUniform1i(glGetUniformLocation(program_final_blur_, "scene"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]); // Bloom texture
        glUniform1i(glGetUniformLocation(program_final_blur_, "bloomBlur"), 1);


        glUniform1i(glGetUniformLocation(program_final_blur_, "bloom"), bloom);
        glUniform1f(glGetUniformLocation(program_final_blur_, "exposure"), exposure);


        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        quad_vao_.Bind();
        quad_vbo_.Bind();
        quad_vbo_.BindData(sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                              (void *) nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                              (void *) (3 * sizeof(float)));

        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        glDepthFunc(GL_LESS); // set depth function back to default

        quad_vao_.Bind();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

//        elapsed_time_ += dt;
//        //Draw program -> cubes
//        glUseProgram(program_);
//
//        SetView(projection, program_);
//        SetUniformsProgram(lightPos);
//
//        SetAndBindTextures();
//
//        SetAndDrawMultipleCubes();
//
//        //Draw program -> light
//        glUseProgram(program_light_);
//
//        SetView(projection, program_light_);
//        SetUniformsProgramLight(lightPos);
//
//        DrawLightProgram();
//
//        //CubeMap
//        glDisable(GL_CULL_FACE);
//        glDepthFunc(
//                GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
//        glUseProgram(program_map_);
//        const auto view = glm::mat4(glm::mat3(camera_->view())); // remove translation from the view matrix
//        int viewLocP = glGetUniformLocation(program_map_, "view");
//        glUniformMatrix4fv(viewLocP,
//                           1, GL_FALSE,
//                           glm::value_ptr(view)
//        );
//        int projectionLocP = glGetUniformLocation(program_map_, "projection");
//        glUniformMatrix4fv(projectionLocP,
//                           1, GL_FALSE,
//                           glm::value_ptr(projection)
//        );
//        // skybox cube
//        skybox_vao_.Bind();
//
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapText_);
//        glDrawArrays(GL_TRIANGLES, 0, 36);
//        glBindVertexArray(0);
//        glDepthFunc(GL_LESS); // set depth function back to default


        // second pass
//        glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
//        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);

//        glUseProgram(program_quad_);
//        quad_vao_.Bind();
//
//        glDisable(GL_DEPTH_TEST);
//        glBindTexture(GL_TEXTURE_2D, texture_of_the_color_buffer_);
//        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void CubeMapScene::SetView(const glm::mat4 &projection, GLuint &program, const glm::mat4 &view) const {
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