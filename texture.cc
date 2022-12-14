#include "texture.h"
#include "pixmap.h"

#include <memory>

namespace gl
{

namespace
{
constexpr GLenum Target = GL_TEXTURE_2D;

GLenum toGLFormat(PixelType pixelType)
{
    return pixelType == PixelType::RGBA ? GL_RGBA : GL_LUMINANCE;
}

GLenum toGLInternalFormat(PixelType pixelType)
{
    return pixelType == PixelType::RGBA ? GL_RGBA : GL_LUMINANCE;
}
} // namespace

Texture::Texture(const Pixmap &pixmap)
    : Texture(pixmap.width, pixmap.height, pixmap.pixelType, pixmap.pixels.data())
{
}

Texture::Texture(int width, int height, PixelType pixelType, const unsigned char *data)
    : m_width(width)
    , m_height(height)
    , m_pixelType(pixelType)
{
    initialize();
    if (data)
        setData(data);
}

Texture::~Texture()
{
    glDeleteTextures(1, &m_id);
}

void Texture::initialize()
{
    glGenTextures(1, &m_id);

    bind();

    glTexParameteri(Target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(Target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(Target, 0, toGLInternalFormat(m_pixelType), m_width, m_height, 0, toGLFormat(m_pixelType),
                 GL_UNSIGNED_BYTE, nullptr);
}

void Texture::setData(const unsigned char *data) const
{
    gpuSetData(data);
}

void Texture::gpuSetData(const unsigned char *data) const
{
    bind();
    glTexSubImage2D(Target, 0, 0, 0, m_width, m_height, toGLFormat(m_pixelType), GL_UNSIGNED_BYTE, data);
}

void Texture::bind() const
{
    glBindTexture(Target, m_id);
}

} // namespace gl
