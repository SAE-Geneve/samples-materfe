//
// Created by Mat on 12/4/2024.
//

#ifndef SAMPLES_OPENGL_EBO_H
#define SAMPLES_OPENGL_EBO_H

#include <GL/glew.h>

class EBO
{
private:
    GLuint name_;

public:
    EBO() = default;
    explicit EBO(GLuint& name);

    //create the VBO
    void Create();

    //bind the VBO
    void Bind() const;

    //bind the data of the VBO
    void BindData(GLsizei size, const void* data, GLenum usage) const;

    //delete
    void Delete();
};


#endif //SAMPLES_OPENGL_EBO_H
