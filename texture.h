#pragma once

#include "abstracttexture.h"
#include "pixeltype.h"

#include <GL/glew.h>

#include <memory>
#include <string>

struct Pixmap;

namespace gl
{

class Texture : public AbstractTexture
{
public:
    explicit Texture(const Pixmap &pixmap);
    Texture(int width, int height, PixelType pixelType, const unsigned char *data = nullptr);
    ~Texture() override;

    void setData(const unsigned char *data) const;

    int width() const { return m_width; }
    int height() const { return m_height; }

    void bind() const override;

    GLuint id() const { return m_id; }

private:
    void initialize();
    void gpuSetData(const unsigned char *data) const;

    int m_width;
    int m_height;
    PixelType m_pixelType;
    GLuint m_id;
};

} // namespace gl
