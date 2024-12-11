//
// Created by Mat on 12/2/2024.
//

#ifndef SAMPLES_OPENGL_MESH_H
#define SAMPLES_OPENGL_MESH_H

#ifndef MESH_H
#define MESH_H

//#include <glad/glad.h> // holds all OpenGL type declarations

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "GL/glew.h"

#include <string>
#include <vector>

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    // position
    glm::vec3 Position;

    glm::vec3 Normal;

    glm::vec2 TexCoords;

    glm::vec3 Tangent;

    glm::vec3 Bitangent;
    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];

    float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
public:
    // mesh Data
    std::vector<Vertex>       vertices_;
    std::vector<unsigned int> indices_;
    std::vector<Texture>      textures_;
    unsigned int VAO_ = 0;

    // constructor
    Mesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, std::vector<Texture>& textures)
    {
        this->vertices_ = vertices;
        this->indices_ = indices;
        this->textures_ = textures;

        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
    }

    // render the mesh
    void Draw(GLuint &shader)
    {
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        for(unsigned int i = 0; i < textures_.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
            // retrieve texture number (the N in diffuse_textureN)
            std::string number;
            std::string name = textures_[i].type;
            if(name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if(name == "texture_specular")
                number = std::to_string(specularNr++);
            else if(name == "texture_normal")
                number = std::to_string(normalNr++); // transfer unsigned int to string

            glUniform1i(glGetUniformLocation(shader, ("material." + name).append(number).c_str()),
                        static_cast<GLint>(i));
            glBindTexture(GL_TEXTURE_2D, textures_[i].id);
        }
        glActiveTexture(GL_TEXTURE0);

        // draw mesh
        glBindVertexArray(VAO_);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices_.size()), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

private:
    // render data
    unsigned int VBO = 0, EBO = 0;

    // initializes all the buffer objects/arrays
    void setupMesh()
    {
        // create buffers/arrays
        glGenVertexArrays(1, &VAO_);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO_);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct, and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices_.size()) * sizeof(Vertex), &vertices_[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int), &indices_[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
        // ids
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

        // weights
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
        glBindVertexArray(0);
    }
};
#endif




#endif //SAMPLES_OPENGL_MESH_H
