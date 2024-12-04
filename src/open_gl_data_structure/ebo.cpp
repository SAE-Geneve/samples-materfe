//
// Created by Mat on 12/4/2024.
//

#include "open_gl_data_structure/ebo.h"

EBO::EBO(GLuint &name) {
    name_ = name;
}

void EBO::Create() {
    glGenBuffers(1, &name_);
}


void EBO::Bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, name_);
}

void EBO::BindData(GLsizei size, const void *data, GLenum usage) const {
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
}

void EBO::Delete() {
    glDeleteBuffers(1 , &name_);
}