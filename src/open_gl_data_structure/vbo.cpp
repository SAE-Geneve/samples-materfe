//
// Created by Mat on 12/4/2024.
//

#include "open_gl_data_structure/vbo.h"

VBO::VBO(GLuint &name) {
    name_ = name;
}

void VBO::Create() {
    glGenBuffers(1, &name_);
}


void VBO::Bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, name_);
}

void VBO::BindData(GLsizei size, const void *data, GLenum usage) const {
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

void VBO::Delete() {
    glDeleteBuffers(1 , &name_);
}