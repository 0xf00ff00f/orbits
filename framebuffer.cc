#include "framebuffer.h"

#include "texture.h"

namespace gl
{

Framebuffer::Framebuffer(int width, int height)
    : m_width(width)
    , m_height(height)
    , m_texture(std::make_unique<Texture>(width, height, PixelType::RGBA))
{
    glGenFramebuffers(1, &m_fboId);
    glGenRenderbuffers(1, &m_rboId);

    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture->id(), 0);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rboId);
    unbind();
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &m_fboId);
    glDeleteRenderbuffers(1, &m_rboId);
}

void Framebuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rboId);
}

void Framebuffer::unbind()
{
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

} // namespace gl
