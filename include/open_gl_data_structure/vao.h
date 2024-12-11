//
// Created by Mat on 12/4/2024.
//

#ifndef SAMPLES_OPENGL_VAO_H
#define SAMPLES_OPENGL_VAO_H
#include <GL/glew.h>


class VAO
{
private:
    GLuint name_ = 0;

public:
    VAO() = default;
    explicit VAO(GLuint& name);

    //create the vao
    void Create();

    //bind the VAO_
    void Bind() const;

    //delete
    void Delete();
};


#endif //SAMPLES_OPENGL_VAO_H
