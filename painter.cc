#include "painter.h"

#include "textureatlas.h"
#include "glyphcache.h"
#include "spritebatcher.h"
#include "fontcache.h"
#include "font.h"
#include "log.h"

namespace miniui
{

namespace
{
constexpr auto TextureAtlasPageSize = 512;
}

Painter::Painter()
    : m_fontCache(std::make_unique<FontCache>())
    , m_textureAtlas(std::make_unique<TextureAtlas>(TextureAtlasPageSize, TextureAtlasPageSize, PixelType::Grayscale))
    , m_spriteBatcher(std::make_unique<SpriteBatcher>())
{
}

Painter::~Painter() = default;

void Painter::setTransformMatrix(const glm::mat4 &matrix)
{
    m_spriteBatcher->setTransformMatrix(matrix);
}

glm::mat4 Painter::transformMatrix() const
{
    return m_spriteBatcher->transformMatrix();
}

void Painter::setFont(const Font &font)
{
    m_font = font.glyphCache();
}

void Painter::begin()
{
    m_spriteBatcher->begin();
    m_font = nullptr;
}

void Painter::end()
{
    m_spriteBatcher->flush();
}

void Painter::drawRect(const glm::vec2 &topLeft, const glm::vec2 &bottomRight, const glm::vec4 &color, int depth)
{
    m_spriteBatcher->setBatchProgram(ShaderManager::Flat);
    m_spriteBatcher->addSprite(topLeft, bottomRight, color, depth);
}

void Painter::drawText(std::u32string_view text, const glm::vec2 &pos, const glm::vec4 &color, int depth)
{
    if (!m_font)
    {
        log("Font not set?");
        return;
    }

    m_spriteBatcher->setBatchProgram(ShaderManager::Text);

    auto basePos = glm::vec2(pos.x, pos.y + m_font->ascent());
    for (auto ch : text)
    {
        const auto glyph = m_font->getGlyph(ch);
        if (!glyph)
            continue;
        const auto topLeft = basePos + glm::vec2(glyph->boundingBox.min);
        const auto bottomRight = topLeft + glm::vec2(glyph->boundingBox.max - glyph->boundingBox.min);
        const auto &pixmap = glyph->pixmap;
        m_spriteBatcher->addSprite(pixmap, topLeft, bottomRight, color, depth);

        basePos.x += glyph->advanceWidth;
    }
}

}
