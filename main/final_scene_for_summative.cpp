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
    static constexpr std::int32_t kLightsCount = 1;
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
        bool reverse_enable_ = false;
        bool bloom = true;
        float exposure = 1.0f;
        float elapsed_time_ = 0.0f;
        float skybox_vertices_[108]{};
        unsigned int cube_map_text_ = 0;
        unsigned int ground_text_ = 0;
        unsigned int ground_text_normal_ = 0;
        std::vector<glm::mat4> model_matrices_{};
        std::array<unsigned int, kLightsCount> all_depth_maps_{};
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
        GLuint normal_mapping_vertex_shader_ = 0;

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
        GLuint normal_mapping_fragment_shader_ = 0;

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
        GLuint program_normal_mapping_ = 0;

        //all frameBuffers-----------------
        GLuint screen_frame_buffer_ = 0;
        GLuint text_for_screen_frame_buffer = 0;
        std::array<GLuint, kLightsCount> depth_map_frame_buffer{};
        unsigned int hdr_fbo_ = 0;
        unsigned int color_buffers_[2]{};
        unsigned int ping_pong_fbo_[2]{};
        unsigned int ping_pong_color_buffers_[2]{};

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

        static void CreateLightCube();

        void RenderScene(const glm::mat4 &projection);

        static void RenderQuad();
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
            _.x = tools::GenerateRandomNumber(0.1f, 1.0f);
            _.y = tools::GenerateRandomNumber(0.1f, 1.0f);
            _.z = tools::GenerateRandomNumber(0.1f, 1.0f);
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

        ground_text_ = TextureManager::LoadTexture(
                "data/texture/3D/rocky_terrain/textures/rocky_terrain_02_diff_4k.jpg", true);
        ground_text_normal_ = TextureManager::LoadTexture(
                "data/texture/3D/rocky_terrain/textures/rocky_terrain_02_nor_gl_4k.jpg");

        //load textures
        //TODO make static constexpr
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
        program_normal_mapping_ = glCreateProgram();


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

        glAttachShader(program_normal_mapping_, normal_mapping_vertex_shader_);
        glAttachShader(program_normal_mapping_, normal_mapping_fragment_shader_);

        glLinkProgram(program_model_);
        glLinkProgram(program_cube_map_);
        glLinkProgram(program_screen_frame_buffer_);
        glLinkProgram(program_gamma_);
        glLinkProgram(program_light_cube_);
        glLinkProgram(program_light_cube_blur_);
        glLinkProgram(program_bloom_);
        glLinkProgram(program_instancing_);
        glLinkProgram(program_making_depth_map_);
        glLinkProgram(program_normal_mapping_);
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
        glGetProgramiv(program_normal_mapping_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking normal mapping shader program\n";
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

        // configure global opengl state
        // -----------------------------
        glEnable(GL_DEPTH_TEST);

        // configure (floating point) framebuffers
        // ---------------------------------------

        glGenFramebuffers(1, &hdr_fbo_);
        glBindFramebuffer(GL_FRAMEBUFFER, hdr_fbo_);
        // create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)

        glGenTextures(2, color_buffers_);
        for (unsigned int i = 0; i < 2; i++) {
            glBindTexture(GL_TEXTURE_2D, color_buffers_[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, kScreenWidth, kScreenHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                            GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            // attach texture to framebuffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, color_buffers_[i], 0);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "Framebuffer not complete!" << std::endl;
        }
        // create and attach depth buffer (renderbuffer)
        unsigned int rboDepth;
        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, kScreenWidth, kScreenHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
        // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
        unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2, attachments);
        // finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ping-pong-framebuffer for blurring


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

    void renderCube() {
        static constexpr float vertices[] = {
                // back face
                -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                1.0f, 1.0f, -1.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f, 1.0f, 0.0f, // bottom-right
                1.0f, 1.0f, -1.0f, 1.0f, 1.0f, // top-right
                -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, // top-left
                // front face
                -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, // bottom-left
                1.0f, -1.0f, 1.0f, 1.0f, 0.0f, // bottom-right
                1.0f, 1.0f, 1.0f, 1.0f, 1.0f, // top-right
                1.0f, 1.0f, 1.0f, 1.0f, 1.0f, // top-right
                -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, // top-left
                -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, // bottom-left
                // left face
                -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, // top-right
                -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, // top-left
                -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, // bottom-right
                -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, // top-right
                // right face
                1.0f, 1.0f, 1.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f, -1.0f, 0.0f, 1.0f, // bottom-right
                1.0f, 1.0f, -1.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f, 0.0f, 1.0f, // bottom-right
                1.0f, 1.0f, 1.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f, 1.0f, 0.0f, 0.0f, // bottom-left
                // bottom face
                -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f, 1.0f, 1.0f, // top-left
                1.0f, -1.0f, 1.0f, 1.0f, 0.0f, // bottom-left
                1.0f, -1.0f, 1.0f, 1.0f, 0.0f, // bottom-left
                -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, // bottom-right
                -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, // top-right
                // top face
                -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, // top-left
                1.0f, 1.0f, 1.0f, 1.0f, 0.0f, // bottom-right
                1.0f, 1.0f, -1.0f, 1.0f, 1.0f, // top-right
                1.0f, 1.0f, 1.0f, 1.0f, 0.0f, // bottom-right
                -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, // top-left
                -1.0f, 1.0f, 1.0f, 0.0f, 0.0f  // bottom-left
        };

        VAO cube_vao_{};
        VBO cube_vbo_{};
        cube_vao_.Create();
        cube_vbo_.Create();
        cube_vao_.Bind();
        cube_vbo_.Bind();
        cube_vbo_.BindData(sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        // render Cube
        cube_vao_.Bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
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

//        // 1. render depth of scene to texture (from light's perspective)
//        // --------------------------------------------------------------
//        glm::mat4 lightProjection, lightView; //TODO when going on render doc -> not good angle, fix shadows
//        glm::mat4 lightSpaceMatrix;
//        float near_plane = 1.0f, far_plane = 7.5f;
//        //lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
//        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
//
//        //set uniform
//        for (std::size_t i = 0; i < kLightsCount; i++) {
//            lightView = glm::lookAt(light_cube_pos_[i], glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
//            lightSpaceMatrix = lightProjection * lightView;
//
//            glUseProgram(program_making_depth_map_);
//            int lightSpaceLocP = glGetUniformLocation(program_making_depth_map_, "lightSpaceMatrix");
//            glUniformMatrix4fv(lightSpaceLocP, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
//
//            //set framebuffer
//            glViewport(0, 0, kShadowWidth, kShadowHeight);
//            glBindFramebuffer(GL_FRAMEBUFFER, depth_map_frame_buffer[i]);
//            glClear(GL_DEPTH_BUFFER_BIT);
//
//            glActiveTexture(GL_TEXTURE0);
//            //glBindTexture(GL_TEXTURE_2D, wood_texture_);
//            RenderScene(projection);
////            auto model = glm::mat4(1.0f);
////            int loc = glGetUniformLocation(program_making_depth_map_, "model");
////            glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));
////
////            plane_vao_.Bind();
////            glDrawArrays(GL_TRIANGLES, 0, 6);
//        }
//        glBindFramebuffer(GL_FRAMEBUFFER, 0);
////        // reset viewport
//        glViewport(0, 0, kScreenWidth, kScreenHeight);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, screen_frame_buffer_);

        //Render -> light and model instancing -----------------------------------------------------------
        RenderScene(projection);


        //Blooming light ----------------------------------------------------------------------------------

        //3 steps :

        //make light cube ----------------------------------------
        // finally show all the light sources as bright cubes
//        glBindFramebuffer(GL_FRAMEBUFFER, hdr_fbo_);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        glUseProgram(program_light_cube_);
//        SetCameraProperties(projection, program_light_cube_);
//
//        for (unsigned int i = 0; i < kLightsCount; i++) {
//            auto model = glm::mat4(1.0f);
//            model = glm::translate(model, glm::vec3(light_cube_pos_[i]));
//            model = glm::scale(model, glm::vec3(0.25f));
//            int viewLocP = glGetUniformLocation(program_light_cube_, "model");
//            glUniformMatrix4fv(viewLocP,
//                               1, GL_FALSE,
//                               glm::value_ptr(model)
//            );
//            viewLocP = glGetUniformLocation(program_light_cube_, "lightColor");
//            glUniform3f(viewLocP, light_cube_color_[i].x, light_cube_color_[i].y, light_cube_color_[i].z);
//            CreateLightCube();
//        }
//        glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//        // 2. blur bright fragments with two-pass Gaussian Blur
//        // --------------------------------------------------
//        bool horizontal = true, first_iteration = true;
//        unsigned int amount = 10;
//        glUseProgram(program_light_cube_blur_);
//        for (unsigned int i = 0; i < amount; i++) {
//            glBindFramebuffer(GL_FRAMEBUFFER, ping_pong_fbo_[horizontal]);
//            glUniform1i(glGetUniformLocation(program_light_cube_blur_, "horizontal"), horizontal);
//            glBindTexture(GL_TEXTURE_2D, first_iteration ? color_buffers_[1]
//                                                         : ping_pong_color_buffers_[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
//            renderQuad();
//            horizontal = !horizontal;
//            if (first_iteration)
//                first_iteration = false;
//        }
//        glBindFramebuffer(GL_FRAMEBUFFER, screen_frame_buffer_);
//
//        // 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
//        // --------------------------------------------------------------------------------------------------------------------------
//        glUseProgram(program_bloom_);
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, color_buffers_[0]);
//        glActiveTexture(GL_TEXTURE1);
//        glBindTexture(GL_TEXTURE_2D, ping_pong_color_buffers_[!horizontal]);
//        glUniform1i(glGetUniformLocation(program_bloom_, "bloom"), bloom);
//        glUniform1f(glGetUniformLocation(program_bloom_, "exposure"), exposure);
//        //renderQuad();
//        //CreateLightCube();
//        renderCube();

        //draw programme -> cube map --------------------------------------------------------------------------
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

        //frame buffer screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
//        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);
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


    void FinalScene::RenderScene(
            const glm::mat4 &projection) {//draw program -> light cubes -------------------------------------------------------------------------
//        glUseProgram(program_light_cube_);
//        for (std::size_t index = 0; index < light_cube_pos_.size(); index++) {
//            SetCameraProperties(projection, program_light_cube_);
//            CreateLightCubeAt(
//                    glm::vec3(light_cube_pos_[index].x, light_cube_pos_[index].y, light_cube_pos_[index].z),
//                    glm::vec3(light_cube_color_[index].x, light_cube_color_[index].y, light_cube_color_[index].z));
//        }

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

        //draw plane -> normal + bin long + gamma---------------------------------------------------------------

        glDisable(GL_CULL_FACE);
        glUseProgram(program_normal_mapping_);

        SetCameraProperties(projection, program_normal_mapping_);
        // render normal-mapped quad

        auto model = glm::mat4(1.0f);
        // rotate the quad to show normal mapping from multiple directions
        model = glm::translate(model, glm::vec3(1.0f, 2.0f, 5.0f));

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

//        // render light source (simply re-renders a smaller plane at the light's position for debugging/visualization)
//        model = glm::mat4(1.0f);
//        glm::vec3 lightPose(0.5f, 1.0f, 0.3f);
//        model = glm::translate(model, lightPose);
//        model = glm::scale(model, glm::vec3(0.1f));
//        modelLocP = glGetUniformLocation(program_normal_mapping_, "model");
//        glUniformMatrix4fv(modelLocP, 1, GL_FALSE, glm::value_ptr(model));
//        RenderQuad();
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
        ImGui::Checkbox("Enable Bloom", &bloom);
        ImGui::DragFloat("Exposure Level", &exposure);
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