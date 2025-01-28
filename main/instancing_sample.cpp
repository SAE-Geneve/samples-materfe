//
// Created by Mat on 1/28/2025.
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

#include <sstream>
#include <iostream>
#include <array>
#include <numbers>

namespace gpr {
    class Instancing final : public Scene {
    public:
        void Begin() override;

        void End() override;

        void Update(float dt) override;

        void OnEvent(const SDL_Event &event, float dt) override;

        void DrawImGui() override;

    private:
        float elapsed_time_ = 0.0f;
        bool reverse_enable_ = false;
        int amount_ = 10000;
        glm::mat4 *model_matrices_ = nullptr;


        //all vertex shaders-------------
        GLuint screen_quad_vertex_shader_ = 0;
        GLuint rocks_vertex_shader_ = 0;

        //all fragment shaders-------------
        GLuint screen_quad_fragment_shader_ = 0;
        GLuint rocks_fragment_shader_ = 0;

        //all programs-------------
        GLuint program_screen_frame_buffer_ = 0;
        GLuint program_rocks_ = 0;

        //all frameBuffers-----------------
        GLuint screen_frame_buffer_ = 0;
        GLuint text_for_screen_frame_buffer = 0;

        //all renderBuffers----------------
        GLuint render_buffer_for_screen_buffer_ = 0;

        Camera *camera_ = nullptr;
        Model *rock_ = nullptr;
        Frustum frustum{};

        VAO quad_vao_{};
        GLuint buffer_ = 0;

        void SetView(const glm::mat4 &projection, GLuint &program) const;
    };

    void Instancing::OnEvent(const SDL_Event &event, const float dt) {
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

    void Instancing::Begin() {
        //global config
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        std::string path = "data/texture/3D/rock/rock.obj";
        rock_ = new Model(path);

        model_matrices_ = new glm::mat4[amount_];

        srand(15678); // initialize random seed
        float radius = 150.0;
        float offset = 25.0f;
        for (unsigned int i = 0; i < amount_; i++) {
            auto model = glm::mat4(1.0f);
            // 1. translation: displace along circle with 'radius' in range [-offset, offset]
            float angle = (float) i / (float) amount_ * 360.0f;
            float displacement = static_cast<float>((rand() % (int) (2 * offset * 100))) / 100.0f - offset;
            float x = sin(angle) * radius + displacement;
            displacement = static_cast<float>((rand() % (int) (2 * offset * 100))) / 100.0f - offset;
            float y = displacement * 0.4f; // keep height of asteroid field smaller compared to width of x and z
            displacement = static_cast<float>((rand() % (int) (2 * offset * 100))) / 100.0f - offset;
            float z = cos(angle) * radius + displacement;
            model = glm::translate(model, glm::vec3(x, y, z));

            // 2. scale: Scale between 0.05 and 0.25f
            auto scale = static_cast<float>((rand() % 20) / 100.0 + 0.05);
            model = glm::scale(model, glm::vec3(scale));

            // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
            auto rotAngle = static_cast<float>((rand() % 360));
            model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

            // 4. now add to list of matrices
            model_matrices_[i] = model;
        }


        glGenBuffers(1, &buffer_);
        glBindBuffer(GL_ARRAY_BUFFER, buffer_);
        glBufferData(GL_ARRAY_BUFFER, amount_ * sizeof(glm::mat4), &model_matrices_[0], GL_STATIC_DRAW);

        for (auto &mesh: rock_->meshes_) {
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

        //Load vertex shader cube 1 ---------------------------------------------------------
        auto vertexContent = LoadFile("data/shaders/3D_scene/cube.vert");
        auto *ptr = vertexContent.data();
        GLint success;
        //Load vertex shader model 1 ---------------------------------------------------------
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
        //Load vertex shader rocks 1 ---------------------------------------------------------
        vertexContent = LoadFile("data/shaders/3D_scene/rocks_instancing_sample/rocks.vert");
        ptr = vertexContent.data();
        rocks_vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(rocks_vertex_shader_, 1, &ptr, nullptr);
        glCompileShader(rocks_vertex_shader_);
        glGetShaderiv(rocks_vertex_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading vertex shader for rocks\n";
        }

        //Load fragment shaders cube 1 ---------------------------------------------------------
        auto fragmentContent = LoadFile("data/shaders/3D_scene/cube.frag");

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
        //Load fragment shaders rocks 1 ---------------------------------------------------------
        fragmentContent = LoadFile("data/shaders/3D_scene/rocks_instancing_sample/rocks.frag");
        ptr = fragmentContent.data();
        rocks_fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(rocks_fragment_shader_, 1, &ptr, nullptr);
        glCompileShader(rocks_fragment_shader_);
        //Check success status of shader compilation
        glGetShaderiv(rocks_fragment_shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cerr << "Error while loading fragment shader for rocks\n";
        }
        //----------------------------------------------------------- set programs

        //Load program/pipeline
        program_screen_frame_buffer_ = glCreateProgram();
        program_rocks_ = glCreateProgram();


        glAttachShader(program_screen_frame_buffer_, screen_quad_vertex_shader_);
        glAttachShader(program_screen_frame_buffer_, screen_quad_fragment_shader_);

        glAttachShader(program_rocks_, rocks_vertex_shader_);
        glAttachShader(program_rocks_, rocks_fragment_shader_);


        glLinkProgram(program_screen_frame_buffer_);
        glLinkProgram(program_rocks_);

        //Check if shader program was linked correctly


        glGetProgramiv(program_screen_frame_buffer_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking screen shader program\n";
        }
        glGetProgramiv(program_screen_frame_buffer_, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "Error while linking rocks shader program\n";
        }

        //----------------------------------------------------------- set VAO / VBO


        quad_vao_.Create();


        //----------------------------------------------------------- set pointers

        camera_ = new Camera;

        //----------------------------------------------------------- frame buffer / render buffer

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
    void Instancing::End() {
        //Unload program/pipeline

        free(camera_);
    }

    void Instancing::Update(float dt) {
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
        const float zFar = 1000.0f;
        const float fovY = std::numbers::pi_v<float> / 2;

        frustum.CreateFrustumFromCamera(*camera_, aspect, fovY, zNear, zFar);
        projection = glm::perspective(fovY, aspect, zNear, zFar);

        //instacing rocks ------------------------------------------------------------------------
        //glDisable(GL_CULL_FACE);
        glUseProgram(program_rocks_);
        SetView(projection, program_rocks_);

        // draw meteorites
        glUniform1i(glGetUniformLocation(program_rocks_, "texture_diffuse1"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,
                      rock_->textures_loaded_[0].id); // note: we also made the textures_loaded vector public (instead of private) from the model class.
        for (auto &mesh: rock_->meshes_) {
            mesh.vao_.Bind();
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices_.size()),
                                    GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(amount_));
            glBindVertexArray(0);
        }



        //frame buffer screen ------------------------------------------------------------------
        glDisable(GL_CULL_FACE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
        //glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
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

    void Instancing::DrawImGui() {
        // Début ImGui
        ImGui::Begin("Controls");
        ImGui::Checkbox("Enable Reverse Post-Processing", &reverse_enable_);
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void Instancing::SetView(const glm::mat4 &projection, GLuint &program) const {
        int viewLocP = glGetUniformLocation(program, "view");
        glUniformMatrix4fv(viewLocP, 1, GL_FALSE, glm::value_ptr(camera_->view()));


        int projectionLocP = glGetUniformLocation(program, "projection");
        glUniformMatrix4fv(projectionLocP, 1, GL_FALSE, glm::value_ptr(projection));
    }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
    gpr::Instancing scene;
    gpr::Engine engine(&scene);
    engine.Run();

    return EXIT_SUCCESS;
}