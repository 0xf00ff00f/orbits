#include "painter.h"

#include "textureatlas.h"
#include "fontcache.h"
#include "spritebatcher.h"
#include "log.h"

namespace miniui
{

namespace
{
constexpr auto TextureAtlasPageSize = 512;

std::string fontPath(std::string_view basename)
{
    return std::string("assets/fonts/") + std::string(basename);
}
}

Painter::Painter()
    : m_textureAtlas(std::make_unique<TextureAtlas>(TextureAtlasPageSize, TextureAtlasPageSize, PixelType::Grayscale))
    , m_fontCache(std::make_unique<FontCache>())
    , m_spriteBatcher(std::make_unique<SpriteBatcher>())
{
    if (!m_fontCache->load("assets/fonts/OpenSans_Regular.ttf", 50))
        log("Failed to load font\n");
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

void Painter::begin()
{
    m_spriteBatcher->begin();
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
    m_spriteBatcher->setBatchProgram(ShaderManager::Text);

    auto basePos = glm::vec2(pos.x, pos.y + m_fontCache->ascent());
    for (auto ch : text)
    {
        const auto glyph = m_fontCache->getGlyph(ch);
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
