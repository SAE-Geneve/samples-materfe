//
// Created by Mat on 1/22/2025.
//

#include <GL/glew.h>

class FrameBuffer
{
private:
    GLuint name_ = 0;

public:
    void Create()
    {
        glGenFramebuffers(1, &name_);
    }

    void Bind() const
    {
        glBindFramebuffer(1, name_);
    }

    static void GenTexture(int amount, unsigned int tex)
    {
        glGenTextures(amount, &tex);
    }
};