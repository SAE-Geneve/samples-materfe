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
    static constexpr std::int32_t kTreesCount = 1000;
    static constexpr std::int32_t kLightsCount = 3;
    static constexpr std::int32_t kShadowWidth = 1024, kShadowHeight = 1024;
    static constexpr std::int32_t kScreenWidth = 1200, kScreenHeight = 800;

    class FinalScene final : public Scene {
    public:
        void Begin() override;

        void End() override;

        void Update(float dt) override;

        void OnEvent(const SDL_Event &event, float dt) override;

        void DrawImGui() override;

    private:
        float elapsed_time_ = 0.0f;
        float skybox_vertices_[108]{};
        unsigned int cube_map_text_ = 0;
        std::vector<glm::mat4> model_matrices_{};
        std::array<unsigned int, kLightsCount> all_depth_maps_{};
        std::array<glm::vec3, kTreesCount> tree_pos_{};
        std::array<glm::vec3, kLightsCount> light_cube_pos_{};
        std::array<glm::vec3, kLightsCount> light_cube_color_{};
        bool reverse_enable_ = false;

        //all vertex shaders-------------
        GLuint model_vertex_shader_ = 0;
        GLuint cube_map_vertex_shader_ = 0;
        GLuint screen_quad_vertex_shader_ = 0;
        GLuint gamma_vertex_shader_ = 0;
        GLuint light_cube_vertex_shader_ = 0;
        GLuint instancing_vertex_shader_ = 0;
        GLuint depth_map_making_vertex_shader_ = 0;

        //all fragment shaders-------------
        GLuint model_fragment_shader_ = 0;
        GLuint cube_map_fragment_shader_ = 0;
        GLuint screen_quad_fragment_shader_ = 0;
        GLuint gamma_fragment_shader_ = 0;
        GLuint light_cube_fragment_shader_ = 0;
        GLuint instancing_fragment_shader_ = 0;
        GLuint depth_map_making_fragment_shader_ = 0;

        //all programs (pipelines) -------------
        GLuint program_model_ = 0;
        GLuint program_cube_map_ = 0;
        GLuint program_screen_frame_buffer_ = 0;
        GLuint program_gamma_ = 0;
        GLuint program_light_cube_ = 0;
        GLuint program_instancing_ = 0;
        GLuint program_making_depth_map_ = 0;

        //all frameBuffers-----------------
        GLuint screen_frame_buffer_ = 0;
        GLuint text_for_screen_frame_buffer = 0;
        std::array<GLuint, kLightsCount> depth_map_frame_buffer{};

        //all renderBuffers----------------
        GLuint render_buffer_for_screen_buffer_ = 0;

        std::unique_ptr<Model> tree_model_unique_{};
        std::unique_ptr<Camera> camera_{};
        Frustum frustum{};

        VAO skybox_vao_{};
        VAO quad_vao_{};
        VAO plane_vao_{};
        VBO skybox_vbo_{};


        void SetCameraProperties(const glm::mat4 &projection, GLuint &program) const;

        void SetLightCubes();

        void CreateLightCubeAt(glm::vec3 position, glm::vec3 color) const;

        void RenderScene(const glm::mat4 &projection);
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
        for (auto &tree: tree_pos_) {
            tree.x = tools::GenerateRandomNumber(-100.0f, 100.0f);
            tree.y = 0.0f;
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

        std::cout << "vertex\n";

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

        //----------------------------------------------------------- set programs

        std::cout << "pipeline\n";

        //Load program/pipeline
        program_model_ = glCreateProgram();
        program_cube_map_ = glCreateProgram();
        program_screen_frame_buffer_ = glCreateProgram();
        program_gamma_ = glCreateProgram();
        program_light_cube_ = glCreateProgram();
        program_instancing_ = glCreateProgram();
        program_making_depth_map_ = glCreateProgram();


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

        glAttachShader(program_instancing_, instancing_vertex_shader_);
        glAttachShader(program_instancing_, instancing_fragment_shader_);

        glAttachShader(program_making_depth_map_, depth_map_making_vertex_shader_);
        glAttachShader(program_making_depth_map_, depth_map_making_fragment_shader_);

        glLinkProgram(program_model_);
        glLinkProgram(program_cube_map_);
        glLinkProgram(program_screen_frame_buffer_);
        glLinkProgram(program_gamma_);
        glLinkProgram(program_light_cube_);
        glLinkProgram(program_instancing_);
        glLinkProgram(program_making_depth_map_);
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
        glGetProgramiv(program_instancing_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking instancing shader program\n";
        }
        glGetProgramiv(program_making_depth_map_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking depth making shader program\n";
        }


        //----------------------------------------------------------- set VAO / VBO

        skybox_vao_.Create();
        quad_vao_.Create();
        plane_vao_.Create();
        skybox_vbo_.Create();

        //----------------------------------------------------------- set pointers

        camera_ = std::make_unique<Camera>();
        std::string path_2 = "data/texture/3D/tree_elm/scene.gltf";
        tree_model_unique_ = std::make_unique<Model>(path_2);



        //tree_model_ = std::make_unique<Model>(path_2).get();
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
            model = glm::rotate(model, rotAngle, glm::vec3(0.0f, 1.0f, 0.0f));

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
        for (std::int32_t i = 0; i < kLightsCount; i++) {
            glGenFramebuffers(1, &depth_map_frame_buffer[i]);
            // create depth texture
            glGenTextures(1, &all_depth_maps_[i]);
            glBindTexture(GL_TEXTURE_2D, all_depth_maps_[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, kScreenWidth, kScreenHeight, 0,
                         GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            float borderColor[] = {1.0, 1.0, 1.0, 1.0};
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

            // attach depth texture as FBO's depth buffer
            glBindFramebuffer(GL_FRAMEBUFFER, depth_map_frame_buffer[i]);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, all_depth_maps_[i], 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

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

        // 1. render depth of scene to texture (from light's perspective)
        // --------------------------------------------------------------
        glm::mat4 lightProjection, lightView; //TODO when going on render doc -> not good angle
        glm::mat4 lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 7.5f;
        //lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

        //set uniform
        for (std::size_t i = 0; i < kLightsCount; i++) {
            lightView = glm::lookAt(light_cube_pos_[i], glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
            lightSpaceMatrix = lightProjection * lightView;

            glUseProgram(program_making_depth_map_);
            int lightSpaceLocP = glGetUniformLocation(program_making_depth_map_, "lightSpaceMatrix");
            glUniformMatrix4fv(lightSpaceLocP, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

            //set framebuffer
            glViewport(0, 0, kShadowWidth, kShadowHeight);
            glBindFramebuffer(GL_FRAMEBUFFER, depth_map_frame_buffer[i]);
            glClear(GL_DEPTH_BUFFER_BIT);

            glActiveTexture(GL_TEXTURE0);
            //glBindTexture(GL_TEXTURE_2D, wood_texture_);
            RenderScene(projection);
            auto model = glm::mat4(1.0f);
            int loc = glGetUniformLocation(program_making_depth_map_, "model");
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));

            plane_vao_.Bind();
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
//        // reset viewport
        glViewport(0, 0, kScreenWidth, kScreenHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, screen_frame_buffer_);

        //Render -> light and model instancing -----------------------------------------------------------
        RenderScene(projection);

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
        glDrawArrays(GL_TRIANGLES, 0, 36);
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

    void FinalScene::RenderScene(const glm::mat4 &projection) {//draw program -> light cubes -------------------------------------------------------------------------
        glUseProgram(program_light_cube_);
        for (size_t index = 0; index < light_cube_pos_.size(); index++) {
            SetCameraProperties(projection, program_light_cube_);
            CreateLightCubeAt(
                    glm::vec3(light_cube_pos_[index].x, light_cube_pos_[index].y, light_cube_pos_[index].z),
                    glm::vec3(light_cube_color_[index].x, light_cube_color_[index].y, light_cube_color_[index].z));
        }

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
    }

    void FinalScene::CreateLightCubeAt(const glm::vec3 position, const glm::vec3 color) const {
        auto model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        glUniformMatrix4fv(glGetUniformLocation(program_light_cube_, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform4f(glGetUniformLocation(program_light_cube_, "color"), color.x, color.y, color.z, 1.0f);

        //drawing
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