#include "painter.h"

#include "textureatlas.h"
#include "glyphcache.h"
#include "spritebatcher.h"
#include "fontcache.h"
#include "pixmapcache.h"
#include "font.h"
#include "log.h"

#include <GL/glew.h>

#include <glm/gtc/matrix_transform.hpp>

namespace miniui
{

namespace
{
constexpr auto TextureAtlasPageSize = 1024;
}

Painter::Painter()
    : m_fontCache(std::make_unique<FontCache>())
    , m_pixmapCache(std::make_unique<PixmapCache>())
    , m_fontTextureAtlas(
          std::make_unique<TextureAtlas>(TextureAtlasPageSize, TextureAtlasPageSize, PixelType::Grayscale))
    , m_pixmapTextureAtlas(std::make_unique<TextureAtlas>(TextureAtlasPageSize, TextureAtlasPageSize, PixelType::RGBA))
    , m_spriteBatcher(std::make_unique<gl::SpriteBatcher>())
{
    updateTransformMatrix();
}

Painter::~Painter() = default;

void Painter::setWindowSize(int width, int height)
{
    m_windowWidth = width;
    m_windowHeight = height;
    updateTransformMatrix();
}

void Painter::updateTransformMatrix()
{
    const auto mvp = glm::ortho(0.0f, static_cast<float>(m_windowWidth), static_cast<float>(m_windowHeight), 0.0f);
    m_spriteBatcher->setTransformMatrix(mvp);
}

void Painter::begin()
{
    m_font = nullptr;
    setClipRect(RectF{{0, 0}, {m_windowWidth, m_windowHeight}});
    m_spriteBatcher->begin();
}

void Painter::end()
{
    m_spriteBatcher->flush();
}

void Painter::setFont(const Font &font)
{
    m_font = font.glyphCache();
}

void Painter::setClipRect(const RectF &rect)
{
    m_clipRect = rect;
    m_spriteBatcher->flush();
    const auto x = static_cast<GLint>(rect.min.x);
    const auto y = static_cast<GLint>(rect.min.y);
    const auto w = static_cast<GLint>(rect.width());
    const auto h = static_cast<GLint>(rect.height());
    glScissor(x, m_windowHeight - (y + h), w, h); // XXX shouldn't be here
}

void Painter::drawRect(const RectF &rect, const glm::vec4 &color, int depth)
{
    if (m_clipRect.intersects(rect))
    {
        m_spriteBatcher->setBatchProgram(ShaderManager::Flat);
        m_spriteBatcher->addSprite(rect, color, depth);
    }
}

void Painter::drawPixmap(const PackedPixmap &pixmap, const RectF &rect, const glm::vec4 &color, int depth)
{
    if (m_clipRect.intersects(rect))
    {
        m_spriteBatcher->setBatchProgram(ShaderManager::Decal);
        m_spriteBatcher->addSprite(pixmap, rect, color, depth);
    }
}

void Painter::drawText(std::u32string_view text, const glm::vec2 &pos, const glm::vec4 &color, int depth)
{
    if (!m_font)
    {
        log("Font not set?\n");
        return;
    }

    m_spriteBatcher->setBatchProgram(ShaderManager::Text);

    auto basePos = glm::vec2(pos.x, pos.y + m_font->ascent());
    for (auto ch : text)
    {
        if (const auto *g = m_font->glyph(ch); g)
        {
            const auto topLeft = basePos + glm::vec2(g->boundingBox.min);
            const auto bottomRight = topLeft + glm::vec2(g->boundingBox.max - g->boundingBox.min);
            const auto rect = RectF{topLeft, bottomRight};
            if (m_clipRect.intersects(rect))
                m_spriteBatcher->addSprite(g->pixmap, rect, color, depth);
            basePos.x += g->advanceWidth;
        }
    }
}

}
