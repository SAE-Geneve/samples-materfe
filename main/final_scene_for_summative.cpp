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
#include <stb_image.h>

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
    static constexpr std::int32_t kTreesCount = 1000;
    static constexpr std::int32_t kLightsCount = 1;
    static constexpr std::int32_t kShadowWidth = 1024, kShadowHeight = 1024;
    static constexpr std::int32_t kScreenWidth = 1200, kScreenHeight = 800;

    static constexpr float Lerp(float a, float b, float f) {
        return a + f * (b - a);
    }

    class FinalScene final : public Scene {
    public:
        void Begin() override;

        void End() override;

        void Update(float dt) override;

        void OnEvent(const SDL_Event &event, float dt) override;

        void DrawImGui() override;

    private:
        bool reverse_enable_ = false;
        bool reverse_gamma_enable_ = false;
        bool bloom = true;
        float exposure = 1.0f;
        float elapsed_time_ = 0.0f;
        float skybox_vertices_[108]{};
        unsigned int cube_map_text_ = 0;
        unsigned int ground_text_ = 0;
        unsigned int ground_text_normal_ = 0;
        unsigned int depth_maps_ = 0;
        std::vector<glm::mat4> model_matrices_{};
        std::array<glm::vec3, kTreesCount> tree_pos_{};
        std::array<glm::vec3, kLightsCount> light_cube_pos_{};
        std::array<glm::vec3, kLightsCount> light_cube_color_{};


        //all vertex shaders-------------
        GLuint model_vertex_shader_ = 0;
        GLuint cube_map_vertex_shader_ = 0;
        GLuint screen_quad_vertex_shader_ = 0;
        GLuint gamma_vertex_shader_ = 0;
        GLuint light_cube_vertex_shader_ = 0;
        GLuint light_cube_blur_vertex_shader_ = 0;
        GLuint bloom_vertex_shader_ = 0;
        GLuint instancing_vertex_shader_ = 0;
        GLuint depth_map_making_vertex_shader_ = 0;
        GLuint shadow_vertex_shader_ = 0;
        GLuint normal_mapping_vertex_shader_ = 0;
        GLuint geometry_pass_vertex_shader_ = 0;
        GLuint lighting_pass_vertex_shader_ = 0;
        GLuint ssao_vertex_shader_ = 0;
        GLuint ssao_blur_vertex_shader_ = 0;

        //all fragment shaders-------------
        GLuint model_fragment_shader_ = 0;
        GLuint cube_map_fragment_shader_ = 0;
        GLuint screen_quad_fragment_shader_ = 0;
        GLuint gamma_fragment_shader_ = 0;
        GLuint light_cube_fragment_shader_ = 0;
        GLuint light_cube_blur_fragment_shader_ = 0;
        GLuint bloom_fragment_shader_ = 0;
        GLuint instancing_fragment_shader_ = 0;
        GLuint depth_map_making_fragment_shader_ = 0;
        GLuint shadow_fragment_shader_ = 0;
        GLuint normal_mapping_fragment_shader_ = 0;
        GLuint geometry_pass_fragment_shader_ = 0;
        GLuint lighting_pass_fragment_shader_ = 0;
        GLuint ssao_fragment_shader_ = 0;
        GLuint ssao_blur_fragment_shader_ = 0;

        //all programs (pipelines) -------------
        GLuint program_model_ = 0;
        GLuint program_cube_map_ = 0;
        GLuint program_screen_frame_buffer_ = 0;
        GLuint program_gamma_ = 0;
        GLuint program_light_cube_ = 0;
        GLuint program_light_cube_blur_ = 0;
        GLuint program_bloom_ = 0;
        GLuint program_instancing_ = 0;
        GLuint program_making_depth_map_ = 0;
        GLuint program_shadow_ = 0;
        GLuint program_normal_mapping_ = 0;
        GLuint program_geometry_pass_ = 0;
        GLuint program_lighting_pass_ = 0;
        GLuint program_ssao_ = 0;
        GLuint program_ssao_blur_ = 0;

        //all frameBuffers-----------------
        GLuint screen_frame_buffer_ = 0;
        GLuint depth_buffer = 0;
        unsigned int gBuffer_ = 0;
        unsigned int ssaoFBO_ = 0, ssaoBlurFBO_ = 0;
        unsigned int gPosition_ = 0, gNormal_ = 0, gAlbedo_ = 0;
        unsigned int noiseTexture_ = 0;
        unsigned int ssaoColorBuffer_ = 0, ssaoColorBufferBlur_ = 0;
        std::array<GLuint, 2> text_for_screen_frame_buffer{};
        unsigned int ping_pong_fbo_[2]{};
        unsigned int ping_pong_color_buffers_[2]{};
        std::vector<glm::vec3> ssaoKernel_{};

        //all renderBuffers----------------
        GLuint render_buffer_for_screen_buffer_ = 0;

        std::unique_ptr<Model> tree_model_unique_{};
        std::unique_ptr<Model> rock_model_unique_{};
        std::unique_ptr<Camera> camera_{};
        Frustum frustum{};

        VAO skybox_vao_{};
        VAO quad_vao_{};
        VAO plane_vao_{};
        VBO skybox_vbo_{};


        void SetCameraProperties(const glm::mat4 &projection, GLuint &program) const;

        void SetLightCubes();

        static void CreateLightCube();

        void RenderScene(const glm::mat4 &projection);

        void RenderSceneForDepth(GLuint &pipeline);

        static void RenderQuad();

        void RenderGroundPlane(const glm::mat4 &projection);

        void SetAllPipelines();
    };

    void FinalScene::SetLightCubes() {
//        for (auto &_: light_cube_pos_) {
//            _.x = tools::GenerateRandomNumber(-100.0f, 100.0f);
//            _.y = tools::GenerateRandomNumber(10.0f, 15.0f);
//            _.z = tools::GenerateRandomNumber(-100.0f, 100.0f);
//        }

        light_cube_pos_[0].x = 1.0f;
        light_cube_pos_[0].y = 1.0f;
        light_cube_pos_[0].z = 1.0f;
        for (auto &_: light_cube_color_) {
            _.x = tools::GenerateRandomNumber(0.1f, 10.0f);
            _.y = tools::GenerateRandomNumber(0.1f, 10.0f);
            _.z = tools::GenerateRandomNumber(0.1f, 10.0f);
        }
        for (auto &tree: tree_pos_) {
            tree.x = tools::GenerateRandomNumber(-100.0f, 100.0f);
            tree.y = -1.0f;
            tree.z = tools::GenerateRandomNumber(-100.0f, 100.0f);
        }
        std::cout << "finished setting pos\n";
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

        ground_text_ = TextureManager::LoadTexture(
                "data/texture/3D/rocky_terrain/textures/rocky_terrain_02_diff_4k.jpg", true);
        ground_text_normal_ = TextureManager::LoadTexture(
                "data/texture/3D/rocky_terrain/textures/rocky_terrain_02_nor_gl_4k.jpg");

        //load textures
        static constexpr std::array<std::string_view, 6> faces =
                {
                        "data/texture/3D/cube_map/posx.jpg",
                        "data/texture/3D/cube_map/negx.jpg",
                        "data/texture/3D/cube_map/posy.jpg",
                        "data/texture/3D/cube_map/negy.jpg",
                        "data/texture/3D/cube_map/posz.jpg",
                        "data/texture/3D/cube_map/negz.jpg"
                };
        cube_map_text_ = TextureManager::loadCubemap(faces);


        std::cout << "vertex\n";

        //set pipelines --------------------------------------------------------
        SetAllPipelines();


        //----------------------------------------------------------- set VAO / VBO

        skybox_vao_.Create();
        quad_vao_.Create();
        plane_vao_.Create();
        skybox_vbo_.Create();

        //----------------------------------------------------------- set pointers

        camera_ = std::make_unique<Camera>();
        std::string path_1 = "data/texture/3D/tree_elm/scene.gltf";
        std::string path_2 = "data/texture/3D/nordic_rocks/xisgcic_tier_2.gltf"; //TODO use with normal maps

        stbi_set_flip_vertically_on_load(true);
        tree_model_unique_ = std::make_unique<Model>(path_1);
        stbi_set_flip_vertically_on_load(false);
        rock_model_unique_ = std::make_unique<Model>(path_2);

        model_matrices_.resize(kTreesCount);

        std::cout << "matrix\n";

        for (std::int32_t i = 0; i < kTreesCount; i++) {
            auto model = glm::mat4(1.0f);
            // 1. translation: displace along circle with 'radius' in range [-offset, offset]
            model = glm::translate(model, tree_pos_[i]);

            // 2. scale: Scale between 0.05 and 0.25f
//            auto scale = static_cast<float>((rand() % 20) / 100.0 + 0.05);
//            model = glm::scale(model, glm::vec3(scale));

//            // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
            auto rotAngle = 90.0f;
            model = glm::rotate(model, glm::radians(rotAngle), glm::vec3(-1.0f, 0.0f, 0.0f));
            rotAngle = tools::GenerateRandomNumber(0.0f, 360.0f);
            model = glm::rotate(model, glm::radians(rotAngle), glm::vec3(0.0f, 0.0f, 1.0f));

            // 4. now add to list of matrices
            model_matrices_[i] = model;
        }

        std::cout << "buffer\n";

        VBO buffer_{};
        buffer_.Create();
        buffer_.Bind();
        buffer_.BindData(kTreesCount * sizeof(glm::mat4), &model_matrices_[0], GL_STATIC_DRAW);

        std::cout << "tree model\n";

        for (auto &mesh: tree_model_unique_->meshes_) {
            VAO temp_ = mesh.vao_;
            temp_.Bind();
            // set attribute pointers for matrix (4 times vec4)
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) nullptr);
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) (sizeof(glm::vec4)));
            glEnableVertexAttribArray(5);
            glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) (2 * sizeof(glm::vec4)));
            glEnableVertexAttribArray(6);
            glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) (3 * sizeof(glm::vec4)));

            glVertexAttribDivisor(3, 1);
            glVertexAttribDivisor(4, 1);
            glVertexAttribDivisor(5, 1);
            glVertexAttribDivisor(6, 1);

            glBindVertexArray(0);
        }


        //----------------------------------------------------------- frame buffer / render buffer

        static constexpr float skyboxVertices[] = {
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

        //create framebuffer ------------------------------------------------------------------------------------------------------

        glGenFramebuffers(1, &depth_buffer);
        // create depth texture
        glGenTextures(1, &depth_maps_);
        glBindTexture(GL_TEXTURE_2D, depth_maps_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, kScreenWidth, kScreenHeight, 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = {1.0, 1.0, 1.0, 1.0};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        // attach depth texture as FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, depth_buffer);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_maps_, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        static constexpr float planeVertices[] = {
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
        VBO plane_vbo_{};
        plane_vbo_.Create();
        plane_vbo_.Bind();
        plane_vbo_.BindData(sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));
        glBindVertexArray(0);

        //----------------------------------------
        glGenFramebuffers(1, &screen_frame_buffer_);
        glBindFramebuffer(GL_FRAMEBUFFER, screen_frame_buffer_);

        glGenTextures(2, text_for_screen_frame_buffer.data());
        for (unsigned int i = 0; i < 2; i++) {
            glBindTexture(GL_TEXTURE_2D, text_for_screen_frame_buffer[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, kScreenWidth, kScreenHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                            GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            // attach texture to framebuffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D,
                                   text_for_screen_frame_buffer[i], 0);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "Framebuffer not complete!" << std::endl;
        }
        // create and attach depth buffer (renderbuffer)
        unsigned int rboDepthScreen;
        glGenRenderbuffers(1, &rboDepthScreen);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepthScreen);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, kScreenWidth, kScreenHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepthScreen);
        // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
        unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2, attachments);
        // finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //check of done correctly + release memory
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << glCheckFramebufferStatus(GL_FRAMEBUFFER)
                      << std::endl;
        }

        // configure global opengl state
        // -----------------------------
        glEnable(GL_DEPTH_TEST);

        // configure (floating point) framebuffers
        // ---------------------------------------

        glGenFramebuffers(2, ping_pong_fbo_);
        glGenTextures(2, ping_pong_color_buffers_);
        for (unsigned int i = 0; i < 2; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, ping_pong_fbo_[i]);
            glBindTexture(GL_TEXTURE_2D, ping_pong_color_buffers_[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, kScreenWidth, kScreenHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                            GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ping_pong_color_buffers_[i], 0);
            // also check if framebuffers are complete (no need for depth buffer)
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "Framebuffer not complete!" << std::endl;
        }

        // shader configuration
        // --------------------
        glUseProgram(program_light_cube_blur_);
        glUniform1i(glGetUniformLocation(program_light_cube_blur_, "image"), 0);
        glUseProgram(program_bloom_);
        glUniform1i(glGetUniformLocation(program_bloom_, "scene"), 0);
        glUniform1i(glGetUniformLocation(program_bloom_, "bloomBlur"), 1);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        glUseProgram(program_normal_mapping_);
        glUniform1i(glGetUniformLocation(program_normal_mapping_, "diffuseMap"), 0);
        glUniform1i(glGetUniformLocation(program_normal_mapping_, "normalMap"), 1);

        // configure g-buffer framebuffer
        // ------------------------------------------------------------------------------------------------

        glGenFramebuffers(1, &gBuffer_);
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer_);

        // position color buffer
        glGenTextures(1, &gPosition_);
        glBindTexture(GL_TEXTURE_2D, gPosition_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, kScreenWidth, kScreenHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition_, 0);
        // normal color buffer
        glGenTextures(1, &gNormal_);
        glBindTexture(GL_TEXTURE_2D, gNormal_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, kScreenWidth, kScreenHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal_, 0);
        // color + specular color buffer
        glGenTextures(1, &gAlbedo_);
        glBindTexture(GL_TEXTURE_2D, gAlbedo_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kScreenWidth, kScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo_, 0);
        // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
        unsigned int new_attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, new_attachments);
        // create and attach depth buffer (renderbuffer)
        unsigned int rboDepth;
        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, kScreenWidth, kScreenHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
        // finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // also create framebuffer to hold SSAO processing stage
        // -----------------------------------------------------

        glGenFramebuffers(1, &ssaoFBO_);
        glGenFramebuffers(1, &ssaoBlurFBO_);
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO_);

        // SSAO color buffer
        glGenTextures(1, &ssaoColorBuffer_);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, kScreenWidth, kScreenHeight, 0, GL_RED, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer_, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "SSAO Framebuffer not complete!" << std::endl;

        // and blur stage
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO_);
        glGenTextures(1, &ssaoColorBufferBlur_);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, kScreenWidth, kScreenHeight, 0, GL_RED, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur_, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // generate sample kernel
        // ----------------------
        std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
        std::default_random_engine generator;
        for (unsigned int i = 0; i < 64; ++i) {
            glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0,
                             randomFloats(generator));
            sample = glm::normalize(sample);
            sample *= randomFloats(generator);
            float scale = float(i) / 64.0f;

            // scale samples s.t. they're more aligned to center of kernel
            scale = Lerp(0.1f, 1.0f, scale * scale);
            sample *= scale;
            ssaoKernel_.push_back(sample);
        }

        // generate noise texture
        // ----------------------
        std::vector<glm::vec3> ssaoNoise;
        for (unsigned int i = 0; i < 16; i++) {
            glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0,
                            0.0f); // rotate around z-axis (in tangent space)
            ssaoNoise.push_back(noise);
        }
        glGenTextures(1, &noiseTexture_);
        glBindTexture(GL_TEXTURE_2D, noiseTexture_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // shader configuration
        // --------------------
        glUseProgram(program_lighting_pass_);
        glUniform1i(glGetUniformLocation(program_lighting_pass_, "gPosition"), 0);
        glUniform1i(glGetUniformLocation(program_lighting_pass_, "gNormal"), 1);
        glUniform1i(glGetUniformLocation(program_lighting_pass_, "gAlbedo"), 2);
        glUniform1i(glGetUniformLocation(program_lighting_pass_, "ssao"), 3);
        glUseProgram(program_ssao_);
        glUniform1i(glGetUniformLocation(program_ssao_, "gPosition"), 0);
        glUniform1i(glGetUniformLocation(program_ssao_, "gNormal"), 1);
        glUniform1i(glGetUniformLocation(program_ssao_, "texNoise"), 2);
        glUseProgram(program_ssao_blur_);
        glUniform1i(glGetUniformLocation(program_ssao_blur_, "ssaoInput"), 0);
    }

    void FinalScene::SetAllPipelines() {
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
        vertexContent = LoadFile("data/shaders/3D_scene/shader_bloom.vert");
        ptr = vertexContent.data();
        light_cube_vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(light_cube_vertex_shader_, 1, &ptr, nullptr);
        glCompileShader(light_cube_vertex_shader_);
        glGetShaderiv(light_cube_vertex_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shader for light cube\n";
        }
        //Load vertex shader light blur 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/blur_shader.vert");
        ptr = vertexContent.data();
        light_cube_blur_vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(light_cube_blur_vertex_shader_, 1, &ptr, nullptr);
        glCompileShader(light_cube_blur_vertex_shader_);
        glGetShaderiv(light_cube_blur_vertex_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shader for blur light cube\n";
        }
        //Load vertex shader bloom 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/blur_shader.vert");
        ptr = vertexContent.data();
        bloom_vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(bloom_vertex_shader_, 1, &ptr, nullptr);
        glCompileShader(bloom_vertex_shader_);
        glGetShaderiv(bloom_vertex_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shader for bloom\n";
        }
        //Load vertex shader instancing 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/rocks_instancing_sample/rocks.vert");
        ptr = vertexContent.data();
        instancing_vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(instancing_vertex_shader_, 1, &ptr, nullptr);
        glCompileShader(instancing_vertex_shader_);
        glGetShaderiv(instancing_vertex_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shader for instancing\n";
        }
        //Load vertex shader cube 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/shadow_mapping_depth.vert");
        ptr = vertexContent.data();
        depth_map_making_vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(depth_map_making_vertex_shader_, 1, &ptr, nullptr);
        glCompileShader(depth_map_making_vertex_shader_);
        //Check success status of shader compilation
        glGetShaderiv(depth_map_making_vertex_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex depth making shader\n";
        }
        //Load vertex shader shadow 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/shadow_mapping.vert");
        ptr = vertexContent.data();
        shadow_vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(shadow_vertex_shader_, 1, &ptr, nullptr);
        glCompileShader(shadow_vertex_shader_);
        //Check success status of shader compilation
        glGetShaderiv(shadow_vertex_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shadow shader\n";
        }
        //Load vertex shader cube 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/normal_mapping.vert");
        ptr = vertexContent.data();
        normal_mapping_vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(normal_mapping_vertex_shader_, 1, &ptr, nullptr);
        glCompileShader(normal_mapping_vertex_shader_);
        //Check success status of shader compilation
        glGetShaderiv(normal_mapping_vertex_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex normal mapping\n";
        }
        //Load vertex shader geometry pass 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/all_ssao_neccessity/geometry_pass.vert");
        ptr = vertexContent.data();
        geometry_pass_vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(geometry_pass_vertex_shader_, 1, &ptr, nullptr);
        glCompileShader(geometry_pass_vertex_shader_);
        //Check success status of shader compilation
        glGetShaderiv(geometry_pass_vertex_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex geometry pass\n";
        }
        //Load vertex shader lightning pass 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/all_ssao_neccessity/lightning_pass.vert");
        ptr = vertexContent.data();
        lighting_pass_vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(lighting_pass_vertex_shader_, 1, &ptr, nullptr);
        glCompileShader(lighting_pass_vertex_shader_);
        //Check success status of shader compilation
        glGetShaderiv(lighting_pass_vertex_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex lightning pass\n";
        }
        //Load vertex shader ssao 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/all_ssao_neccessity/ssao.vert");
        ptr = vertexContent.data();
        ssao_vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(ssao_vertex_shader_, 1, &ptr, nullptr);
        glCompileShader(ssao_vertex_shader_);
        //Check success status of shader compilation
        glGetShaderiv(ssao_vertex_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex ssao\n";
        }
        //Load vertex shader ssao blur 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/all_ssao_neccessity/ssao_blur.vert");
        ptr = vertexContent.data();
        ssao_blur_vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(ssao_blur_vertex_shader_, 1, &ptr, nullptr);
        glCompileShader(ssao_blur_vertex_shader_);
        //Check success status of shader compilation
        glGetShaderiv(ssao_blur_vertex_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex ssao blur\n";
        }

        std::cout << "fragment\n";

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
        fragmentContent = LoadFile("data/shaders/3D_scene/shader_light_bloom.frag");
        ptr = fragmentContent.data();
        light_cube_fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(light_cube_fragment_shader_, 1, &ptr, nullptr);
        glCompileShader(light_cube_fragment_shader_);
        //Check success status of shader compilation
        glGetShaderiv(light_cube_fragment_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader for light cube\n";
        }
        //Load fragment shaders light blur 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/blur_shader.frag");
        ptr = fragmentContent.data();
        light_cube_blur_fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(light_cube_blur_fragment_shader_, 1, &ptr, nullptr);
        glCompileShader(light_cube_blur_fragment_shader_);
        //Check success status of shader compilation
        glGetShaderiv(light_cube_blur_fragment_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader for light cube blur\n";
        }
        //Load fragment shaders bloom 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/final_bloom.frag");
        ptr = fragmentContent.data();
        bloom_fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(bloom_fragment_shader_, 1, &ptr, nullptr);
        glCompileShader(bloom_fragment_shader_);
        //Check success status of shader compilation
        glGetShaderiv(bloom_fragment_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader for bloom\n";
        }
        //Load fragment shaders instancing 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/rocks_instancing_sample/rocks.frag");
        ptr = fragmentContent.data();
        instancing_fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(instancing_fragment_shader_, 1, &ptr, nullptr);
        glCompileShader(instancing_fragment_shader_);
        //Check success status of shader compilation
        glGetShaderiv(instancing_fragment_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader for inconstant\n";
        }
        //Load fragment shaders cube 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/shadow_mapping_depth.frag");
        ptr = fragmentContent.data();
        depth_map_making_fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(depth_map_making_fragment_shader_, 1, &ptr, nullptr);
        glCompileShader(depth_map_making_fragment_shader_);
        //Check success status of shader compilation
        glGetShaderiv(depth_map_making_fragment_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading depth making fragment shader\n";
        }
        //Load fragment shaders shadow 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/shadow_mapping.frag");
        ptr = fragmentContent.data();
        shadow_fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(shadow_fragment_shader_, 1, &ptr, nullptr);
        glCompileShader(shadow_fragment_shader_);
        //Check success status of shader compilation
        glGetShaderiv(shadow_fragment_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading shadow fragment shader\n";
        }
        //Load fragment shaders cube 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/normal_mapping.frag");
        ptr = fragmentContent.data();
        normal_mapping_fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(normal_mapping_fragment_shader_, 1, &ptr, nullptr);
        glCompileShader(normal_mapping_fragment_shader_);
        //Check success status of shader compilation
        glGetShaderiv(normal_mapping_fragment_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading normal mapping fragment shader\n";
        }
        //Load fragment shaders geometry pass 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/all_ssao_neccessity/geometry_pass.frag");
        ptr = fragmentContent.data();
        geometry_pass_fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(geometry_pass_fragment_shader_, 1, &ptr, nullptr);
        glCompileShader(geometry_pass_fragment_shader_);
        //Check success status of shader compilation
        glGetShaderiv(geometry_pass_fragment_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading geometry pass fragment shader\n";
        }
        //Load fragment lightning pass 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/all_ssao_neccessity/lightning_pass.frag");
        ptr = fragmentContent.data();
        lighting_pass_fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(lighting_pass_fragment_shader_, 1, &ptr, nullptr);
        glCompileShader(lighting_pass_fragment_shader_);
        //Check success status of shader compilation
        glGetShaderiv(lighting_pass_fragment_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading lightning pass fragment shader\n";
        }
        //Load fragment ssao 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/all_ssao_neccessity/ssao.frag");
        ptr = fragmentContent.data();
        ssao_fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(ssao_fragment_shader_, 1, &ptr, nullptr);
        glCompileShader(ssao_fragment_shader_);
        //Check success status of shader compilation
        glGetShaderiv(ssao_fragment_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading ssao fragment shader\n";
        }
        //Load fragment ssao blur 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/all_ssao_neccessity/ssao_blur.frag");
        ptr = fragmentContent.data();
        ssao_blur_fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(ssao_blur_fragment_shader_, 1, &ptr, nullptr);
        glCompileShader(ssao_blur_fragment_shader_);
        //Check success status of shader compilation
        glGetShaderiv(ssao_blur_fragment_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading ssao blur fragment shader\n";
        }

        //----------------------------------------------------------- set programs

        std::cout << "pipeline\n";

        //Load program/pipeline
        program_model_ = glCreateProgram();
        program_cube_map_ = glCreateProgram();
        program_screen_frame_buffer_ = glCreateProgram();
        program_gamma_ = glCreateProgram();
        program_light_cube_ = glCreateProgram();
        program_light_cube_blur_ = glCreateProgram();
        program_bloom_ = glCreateProgram();
        program_instancing_ = glCreateProgram();
        program_making_depth_map_ = glCreateProgram();
        program_shadow_ = glCreateProgram();
        program_normal_mapping_ = glCreateProgram();
        program_geometry_pass_ = glCreateProgram();
        program_lighting_pass_ = glCreateProgram();
        program_ssao_ = glCreateProgram();
        program_ssao_blur_ = glCreateProgram();


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

        glAttachShader(program_light_cube_blur_, light_cube_blur_vertex_shader_);
        glAttachShader(program_light_cube_blur_, light_cube_blur_fragment_shader_);

        glAttachShader(program_bloom_, bloom_vertex_shader_);
        glAttachShader(program_bloom_, bloom_fragment_shader_);

        glAttachShader(program_instancing_, instancing_vertex_shader_);
        glAttachShader(program_instancing_, instancing_fragment_shader_);

        glAttachShader(program_making_depth_map_, depth_map_making_vertex_shader_);
        glAttachShader(program_making_depth_map_, depth_map_making_fragment_shader_);

        glAttachShader(program_shadow_, shadow_vertex_shader_);
        glAttachShader(program_shadow_, shadow_fragment_shader_);

        glAttachShader(program_normal_mapping_, normal_mapping_vertex_shader_);
        glAttachShader(program_normal_mapping_, normal_mapping_fragment_shader_);

        glAttachShader(program_geometry_pass_, geometry_pass_vertex_shader_);
        glAttachShader(program_geometry_pass_, geometry_pass_fragment_shader_);

        glAttachShader(program_lighting_pass_, lighting_pass_vertex_shader_);
        glAttachShader(program_lighting_pass_, lighting_pass_fragment_shader_);

        glAttachShader(program_ssao_, ssao_vertex_shader_);
        glAttachShader(program_ssao_, ssao_fragment_shader_);

        glAttachShader(program_ssao_blur_, ssao_blur_vertex_shader_);
        glAttachShader(program_ssao_blur_, ssao_blur_fragment_shader_);

        glLinkProgram(program_model_);
        glLinkProgram(program_cube_map_);
        glLinkProgram(program_screen_frame_buffer_);
        glLinkProgram(program_gamma_);
        glLinkProgram(program_light_cube_);
        glLinkProgram(program_light_cube_blur_);
        glLinkProgram(program_bloom_);
        glLinkProgram(program_instancing_);
        glLinkProgram(program_making_depth_map_);
        glLinkProgram(program_shadow_);
        glLinkProgram(program_normal_mapping_);
        glLinkProgram(program_geometry_pass_);
        glLinkProgram(program_lighting_pass_);
        glLinkProgram(program_ssao_);
        glLinkProgram(program_ssao_blur_);
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
        glGetProgramiv(program_light_cube_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking light cube shader program\n";
        }
        glGetProgramiv(program_light_cube_blur_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking light cube blur shader program\n";
        }
        glGetProgramiv(program_bloom_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking bloom shader program\n";
        }
        glGetProgramiv(program_instancing_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking instancing shader program\n";
        }
        glGetProgramiv(program_making_depth_map_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking depth making shader program\n";
        }
        glGetProgramiv(program_shadow_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking shadow shader program\n";
        }
        glGetProgramiv(program_normal_mapping_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking normal mapping shader program\n";
        }
        glGetProgramiv(program_geometry_pass_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking geometry pass shader program\n";
        }
        glGetProgramiv(program_lighting_pass_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking lightning pass shader program\n";
        }
        glGetProgramiv(program_ssao_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking ssao shader program\n";
        }
        glGetProgramiv(program_ssao_blur_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking ssao blur shader program\n";
        }
    }

    //TODO update the end with screen buffer stuff + gamma correction + lights + depth shaders
    void FinalScene::End() {
        //Unload program/pipeline
        glDeleteProgram(program_model_);
        glDeleteProgram(program_cube_map_);

        glDeleteShader(model_vertex_shader_);
        glDeleteShader(model_fragment_shader_);

        glDeleteShader(cube_map_vertex_shader_);
        glDeleteShader(cube_map_fragment_shader_);

        skybox_vao_.Delete();
        skybox_vbo_.Delete();
        quad_vao_.Delete();
    }

    void renderQuad() {

        static constexpr float quadVertices[] = {
                // positions        // texture Coords
                -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        VAO cube_vao_{};
        VBO cube_vbo_{};
        cube_vao_.Create();
        cube_vbo_.Create();
        cube_vao_.Bind();
        cube_vbo_.Bind();
        cube_vbo_.BindData(sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));

        cube_vao_.Bind();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
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

        glBindFramebuffer(GL_FRAMEBUFFER, screen_frame_buffer_);

        //Render -> Depth map from light perspective ------------------------------------------
        //set framebuffer
        glUseProgram(program_making_depth_map_);

        glm::mat4 lightProjection(1.0f), lightView(1.0f);
        glm::mat4 lightSpaceMatrix(1.0f);
        float near_plane = 0.1f, far_plane = 50.0f;
        //lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        lightView = glm::lookAt(light_cube_pos_[0], glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        lightSpaceMatrix = lightProjection * lightView;
        // render scene from light's point of view

        int lightSpaceLocP = glGetUniformLocation(program_making_depth_map_, "lightSpaceMatrix");
        glUniformMatrix4fv(lightSpaceLocP, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

        glViewport(0, 0, kShadowWidth, kShadowHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, depth_buffer);
        glClear(GL_DEPTH_BUFFER_BIT);

        //RenderScene(projection);
        RenderSceneForDepth(program_making_depth_map_);

        // reset viewport
        glViewport(0, 0, kScreenWidth, kScreenHeight);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 2. render scene as normal using the generated depth/shadow map ->
        // depth map does not link to uniform for some magic reason
        // --------------------------------------------------------------
//        glUseProgram(program_shadow_);
//        SetCameraProperties(projection, program_shadow_);
//
//
//        glUniform3f(glGetUniformLocation(program_shadow_, "lightPos"), light_cube_pos_[0].x, light_cube_pos_[0].y,
//                    light_cube_pos_[0].z);
//
//        glUniform3f(glGetUniformLocation(program_shadow_, "viewPos"), camera_->position_.x, camera_->position_.y,
//                    camera_->position_.z);
//
//        lightSpaceLocP = glGetUniformLocation(program_shadow_, "lightSpaceMatrix");
//        glUniformMatrix4fv(lightSpaceLocP, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
//
//        //set textures
//        //set textures
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, depth_maps_);
//        glActiveTexture(GL_TEXTURE1);
//        glBindTexture(GL_TEXTURE_2D, ground_text_normal_);
//
//
//
//        //draw plane -> normal + bin long + gamma---------------------------------------------------------------
//
//        glBindFramebuffer(GL_FRAMEBUFFER, screen_frame_buffer_);
//
//        auto model_plane = glm::mat4(1.0f);
//        // rotate the quad to show normal mapping from multiple directions
//        model_plane = glm::translate(model_plane, glm::vec3(0.0f, -1.0f, 0.0f));
//        model_plane = glm::scale(model_plane, glm::vec3(100.0, 100.0, 100.0));
//        model_plane = glm::rotate(model_plane, static_cast<float>(glm::radians(90.0)), glm::vec3(1.0, 0.0, 0.0));
//
//        glUniformMatrix4fv(glGetUniformLocation(program_shadow_, "model"), 1, GL_FALSE, glm::value_ptr(model_plane));
//        //set uniform
//
//        RenderQuad();

        //RenderScene(program_);
        //glBindFramebuffer(GL_FRAMEBUFFER, screen_frame_buffer_);
        //RenderSceneForDepth(program_shadow_);

        //Render -> light and model instancing -----------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, screen_frame_buffer_);
        RenderScene(projection);

        // 1. geometry pass: render scene's geometry/color data into gbuffer
        // -----------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer_);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program_geometry_pass_);
        SetCameraProperties(projection, program_geometry_pass_);
        // room cube
        auto model = glm::mat4(1.0f);
        glUniform1i(glGetUniformLocation(program_geometry_pass_, "invertedNormals"), 0);
        // backpack model on the floor

        for (int i = 0; i < tree_model_unique_->meshes_.size(); i++) {
            glUniformMatrix4fv(glGetUniformLocation(program_geometry_pass_, "model"), 1, GL_FALSE,
                               glm::value_ptr(model_matrices_[i]));
            tree_model_unique_->meshes_[i].vao_.Bind();
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(tree_model_unique_->meshes_[i].indices_.size()),
                                    GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(kTreesCount));
            glBindVertexArray(0);
        }

        glDisable(GL_CULL_FACE);
        glFrontFace(GL_CW);
        //draw plane -> normal + bin long + gamma---------------------------------------------------------------

        model = glm::mat4(1.0f);
        // rotate the quad to show normal mapping from multiple directions
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(100.0, 100.0, 100.0));
        model = glm::rotate(model, static_cast<float>(glm::radians(90.0)), glm::vec3(1.0, 0.0, 0.0));

        glUniformMatrix4fv(glGetUniformLocation(program_geometry_pass_, "model"), 1, GL_FALSE, glm::value_ptr(model));
        //set uniform

        RenderQuad();

        //draw rock-------------------------------------------------------------------------------------
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(50.0f, -1.0f, 5.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f));
        glUniformMatrix4fv(glGetUniformLocation(program_geometry_pass_, "model"), 1, GL_FALSE, glm::value_ptr(model));

        rock_model_unique_->Draw(program_model_);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // 2. generate SSAO texture
        // ------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO_);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program_ssao_);
        // Send kernel + rotation
        for (unsigned int i = 0; i < 64; ++i) {
            std::string path = "samples[" + std::to_string(i) + "]";
            glUniform3f(glGetUniformLocation(program_ssao_, path.c_str()), ssaoKernel_[i].x, ssaoKernel_[i].y,
                        ssaoKernel_[i].z);
        }
        glUniformMatrix4fv(glGetUniformLocation(program_ssao_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition_);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal_);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, noiseTexture_);
        renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // 3. blur SSAO texture to remove noise
        // ------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO_);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program_ssao_blur_);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer_);
        renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // 4. lighting pass: traditional deferred Blinn-Phong lighting with added screen-space ambient occlusion
        // -----------------------------------------------------------------------------------------------------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program_lighting_pass_);
        // send light relevant uniforms
        glm::vec3 lightPosView = glm::vec3(camera_->view() * glm::vec4(light_cube_pos_[0], 1.0));

        glUniform3f(glGetUniformLocation(program_lighting_pass_, "light.Position"), light_cube_pos_[0].x,
                    light_cube_pos_[0].y, light_cube_pos_[0].z);
        glUniform3f(glGetUniformLocation(program_lighting_pass_, "light.Color"), light_cube_color_[0].x,
                    light_cube_color_[0].y, light_cube_color_[0].z);

        // Update attenuation parameters
        const float linear = 0.09f;
        const float quadratic = 0.032f;

        glUniform1f(glGetUniformLocation(program_lighting_pass_, "light.Linear"), linear);
        glUniform1f(glGetUniformLocation(program_lighting_pass_, "light.Quadratic"), quadratic);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition_);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal_);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedo_);
        glActiveTexture(GL_TEXTURE3); // add extra SSAO texture to lighting pass
        glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur_);
        renderQuad();


        //draw programme -> cube map --------------------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, screen_frame_buffer_);
        glDisable(GL_CULL_FACE);
        glDepthFunc(GL_LEQUAL);
        glUseProgram(program_cube_map_);
        glBindFramebuffer(GL_FRAMEBUFFER, screen_frame_buffer_);
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
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);


        //Blooming light ----------------------------------------------------------------------------------

        //3 steps :

        //make light cube ----------------------------------------
        // finally show all the light sources as bright cubes
        glBindFramebuffer(GL_FRAMEBUFFER, screen_frame_buffer_);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program_light_cube_);
        SetCameraProperties(projection, program_light_cube_);

        for (unsigned int i = 0; i < kLightsCount; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(light_cube_pos_[i]));
            model = glm::scale(model, glm::vec3(0.25f));
            int viewLocPTemp = glGetUniformLocation(program_light_cube_, "model");
            glUniformMatrix4fv(viewLocPTemp,
                               1, GL_FALSE,
                               glm::value_ptr(model)
            );
            viewLocPTemp = glGetUniformLocation(program_light_cube_, "lightColor");
            glUniform3f(viewLocPTemp, light_cube_color_[i].x, light_cube_color_[i].y, light_cube_color_[i].z);
            CreateLightCube();
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. blur bright fragments with two-pass Gaussian Blur
        // --------------------------------------------------
        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        glUseProgram(program_light_cube_blur_);
        for (unsigned int i = 0; i < amount; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, ping_pong_fbo_[horizontal]);
            glUniform1i(glGetUniformLocation(program_light_cube_blur_, "horizontal"), horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? text_for_screen_frame_buffer[1]
                                                         : ping_pong_color_buffers_[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
            renderQuad();
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, screen_frame_buffer_);

        // 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
        // --------------------------------------------------------------------------------------------------------------------------
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program_bloom_);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, text_for_screen_frame_buffer[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, ping_pong_color_buffers_[!horizontal]);
        glUniform1i(glGetUniformLocation(program_bloom_, "bloom"), bloom);
        glUniform1f(glGetUniformLocation(program_bloom_, "exposure"), exposure);
        renderQuad();

        //frame buffer screen ----------------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
//        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program_screen_frame_buffer_);

        //set post process
        glUniform1i(glGetUniformLocation(program_screen_frame_buffer_, "reverse"), reverse_enable_);
        glUniform1i(glGetUniformLocation(program_screen_frame_buffer_, "reverseGammaEffect"), reverse_gamma_enable_);
        quad_vao_.Bind();
        glDisable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_2D, text_for_screen_frame_buffer[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
        DrawImGui();
    }

    void FinalScene::RenderQuad() {
        // positions
        glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
        glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
        glm::vec3 pos3(1.0f, -1.0f, 0.0f);
        glm::vec3 pos4(1.0f, 1.0f, 0.0f);
        // texture coordinates
        glm::vec2 uv1(0.0f, 1.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.0f, 0.0f);
        glm::vec2 uv4(1.0f, 1.0f);
        // normal vector
        glm::vec3 nm(0.0f, 0.0f, 1.0f);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;
        // triangle 1
        // ----------
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);


        float quadVertices[] = {
                // positions                            // normal                     // texcoords          // tangent                          // bitangent
                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z,
                bitangent1.x, bitangent1.y, bitangent1.z,
                pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z,
                bitangent1.x, bitangent1.y, bitangent1.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z,
                bitangent1.x, bitangent1.y, bitangent1.z,

                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z,
                bitangent2.x, bitangent2.y, bitangent2.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z,
                bitangent2.x, bitangent2.y, bitangent2.z,
                pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z,
                bitangent2.x, bitangent2.y, bitangent2.z
        };
        // configure plane VAO
        VAO new_plane_vao{};
        VBO plane_vbo_{};
        new_plane_vao.Create();
        new_plane_vao.Bind();
        plane_vbo_.Create();
        plane_vbo_.Bind();
        plane_vbo_.BindData(sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                              (void *) nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                              (void *) (3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                              (void *) (6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                              (void *) (8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                              (void *) (11 * sizeof(float)));
        new_plane_vao.Bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    void FinalScene::RenderSceneForDepth(GLuint &pipeline) {
        //draw programme -> 3D model --------------------------------------------------------------------------
        //swap to CCW because tree's triangles are done the oposite way
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        for (int i = 0; i < tree_model_unique_->meshes_.size(); i++) {
            glUniformMatrix4fv(glGetUniformLocation(pipeline, "model"), 1, GL_FALSE,
                               glm::value_ptr(model_matrices_[i]));
            tree_model_unique_->meshes_[i].vao_.Bind();
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(tree_model_unique_->meshes_[i].indices_.size()),
                                    GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(kTreesCount));
            glBindVertexArray(0);
        }

        glDisable(GL_CULL_FACE);
        glFrontFace(GL_CW);
        //draw plane -> normal + bin long + gamma---------------------------------------------------------------

        auto model = glm::mat4(1.0f);
        // rotate the quad to show normal mapping from multiple directions
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(100.0, 100.0, 100.0));
        model = glm::rotate(model, static_cast<float>(glm::radians(90.0)), glm::vec3(1.0, 0.0, 0.0));

        glUniformMatrix4fv(glGetUniformLocation(pipeline, "model"), 1, GL_FALSE, glm::value_ptr(model));
        //set uniform

        RenderQuad();

        //draw rock-------------------------------------------------------------------------------------
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(50.0f, -1.0f, 5.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f));
        glUniformMatrix4fv(glGetUniformLocation(pipeline, "model"), 1, GL_FALSE, glm::value_ptr(model));

        rock_model_unique_->Draw(program_model_);
    }

    void FinalScene::RenderScene(
            const glm::mat4 &projection) {
        //draw programme -> 3D model --------------------------------------------------------------------------
        //swap to CCW because tree's triangles are done the oposite way
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        glUseProgram(program_instancing_);
        SetCameraProperties(projection, program_instancing_);

        // draw meteorites
        glUniform1i(glGetUniformLocation(program_instancing_, "texture_diffuse1"), 0);
        glActiveTexture(GL_TEXTURE0);

        glBindTexture(GL_TEXTURE_2D,
                      tree_model_unique_->textures_loaded_[0].id); // note: we also made the textures_loaded vector public (instead of private) from the model class.
        for (auto &mesh: tree_model_unique_->meshes_) {
            mesh.vao_.Bind();
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices_.size()),
                                    GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(kTreesCount));
            glBindVertexArray(0);
        }

        glDisable(GL_CULL_FACE);
        glFrontFace(GL_CW);
        //draw plane -> normal + bin long + gamma---------------------------------------------------------------

        RenderGroundPlane(projection);

        //draw rock-------------------------------------------------------------------------------------
        glUseProgram(program_model_);
        SetCameraProperties(projection, program_model_);

        auto model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(50.0f, -1.0f, 5.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f));
        glUniformMatrix4fv(glGetUniformLocation(program_model_, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(program_model_, "texture_diffuse1"), 0);
        glActiveTexture(GL_TEXTURE0);

        rock_model_unique_->Draw(program_model_);
    }

    void FinalScene::RenderGroundPlane(const glm::mat4 &projection) {
        glDisable(GL_CULL_FACE);
        glUseProgram(program_normal_mapping_);

        SetCameraProperties(projection, program_normal_mapping_);
        // render normal-mapped quad

        auto model = glm::mat4(1.0f);
        // rotate the quad to show normal mapping from multiple directions
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(100.0, 100.0, 100.0));
        model = glm::rotate(model, static_cast<float>(glm::radians(90.0)), glm::vec3(1.0, 0.0, 0.0));

        int modelLocP = glGetUniformLocation(program_normal_mapping_, "model");
        glUniformMatrix4fv(modelLocP, 1, GL_FALSE, glm::value_ptr(model));

        int camLocP = glGetUniformLocation(program_normal_mapping_, "viewPos");
        glUniform3f(camLocP, camera_->position_.x, camera_->position_.y, camera_->position_.z);

        int lightLocP = glGetUniformLocation(program_normal_mapping_, "lightPos");
        glUniform3f(lightLocP, light_cube_pos_[0].x, light_cube_pos_[0].y, light_cube_pos_[0].z);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ground_text_);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, ground_text_normal_);

        RenderQuad();
    }

    void FinalScene::CreateLightCube() {
        static constexpr float vertices[] = {
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

        VAO cube_vao_{};
        VBO cube_vbo_{};
        cube_vao_.Create();
        cube_vbo_.Create();
        cube_vao_.Bind();
        cube_vbo_.Bind();
        cube_vbo_.BindData(sizeof(vertices), vertices, GL_STATIC_DRAW);

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

    void FinalScene::DrawImGui() {
        // Début ImGui
        ImGui::Begin("Controls");
        ImGui::Checkbox("Enable Reverse Post-Processing", &reverse_enable_);
        ImGui::Checkbox("Enable Reverse Gamma effect", &reverse_gamma_enable_);
        ImGui::Checkbox("Enable Bloom", &bloom);
        ImGui::DragFloat("Exposure Level", &exposure);
        ImGui::DragFloat("Light Position X", &light_cube_pos_[0].x, 1.0f, 0.0f, 10.0f);
        ImGui::DragFloat("Light Position Y", &light_cube_pos_[0].y, 1.0f, 0.0f, 10.0f);
        ImGui::DragFloat("Light Position Z", &light_cube_pos_[0].z, 1.0f, 0.0f, 10.0f);
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void FinalScene::SetCameraProperties(const glm::mat4 &projection, GLuint &program) const {
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