#pragma once

#include "noncopyable.h"

#include <GL/glew.h>

#include <memory>

namespace gl
{

class Texture;

class Framebuffer : private NonCopyable
{
public:
    Framebuffer(int width, int height);
    ~Framebuffer();

    void bind() const;
    static void unbind();

    int width() const { return m_width; }
    int height() const { return m_height; }

private:
    int m_width;
    int m_height;
    GLuint m_fboId, m_rboId;
    std::unique_ptr<Texture> m_texture;
};

} // namespace gl
