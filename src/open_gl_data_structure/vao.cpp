//
// Created by Mat on 12/4/2024.
//

#include "open_gl_data_structure/vao.h"

VAO::VAO(GLuint &name) {
    name_ = name;
}

void VAO::Create() {
    glGenVertexArrays(1, &name_);
}

void VAO::Bind() const {
    glBindVertexArray(name_);
}

void VAO::Delete() {
    glDeleteVertexArrays(1, &name_);
}