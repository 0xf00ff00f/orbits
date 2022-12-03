#include "painter.h"

#include "spritebatcher.h"
#include "font.h"
#include "log.h"

#include <GL/glew.h>

#include <glm/gtc/matrix_transform.hpp>

namespace miniui
{

Painter::Painter()
    : m_spriteBatcher(std::make_unique<gl::SpriteBatcher>())
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
    setClipRect({{0, 0}, {m_windowWidth, m_windowHeight}});
    m_spriteBatcher->begin();
}

void Painter::end()
{
    m_spriteBatcher->flush();
}

void Painter::setFont(Font *font)
{
    m_font = font;
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

void Painter::drawPixmap(const PackedPixmap &pixmap, const RectF &rect, const RectF &clipRect, const glm::vec4 &color,
                         int depth)
{
    if (m_clipRect.intersects(rect) && m_clipRect.intersects(rect))
    {
        m_spriteBatcher->setBatchProgram(ShaderManager::Decal);
        const auto texPos = [&rect, &texCoord = pixmap.texCoord](const glm::vec2 &p) {
            const float x =
                (p.x - rect.min.x) * (texCoord.max.x - texCoord.min.x) / (rect.max.x - rect.min.x) + texCoord.min.x;
            const float y =
                (p.y - rect.min.y) * (texCoord.max.y - texCoord.min.y) / (rect.max.y - rect.min.y) + texCoord.min.y;
            return glm::vec2(x, y);
        };
        const auto spriteRect = rect.intersected(clipRect);
        const auto texCoord = RectF{texPos(spriteRect.min), texPos(spriteRect.max)};
        m_spriteBatcher->addSprite(pixmap.texture, spriteRect, texCoord, color, depth);
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

void Painter::drawCircle(const glm::vec2 &center, float radius, const glm::vec4 &color, int depth)
{
    const auto topLeft = center - glm::vec2(radius, radius);
    const auto bottomRight = center + glm::vec2(radius, radius);
    const auto rect = RectF{topLeft, bottomRight};
    if (m_clipRect.intersects(rect))
    {
        m_spriteBatcher->setBatchProgram(ShaderManager::Circle);
        m_spriteBatcher->addSprite(nullptr, rect, {{0, 0}, {1, 1}}, color, depth);
    }
}

void Painter::drawCapsule(const RectF &rect, const glm::vec4 &color, int depth)
{
    if (!m_clipRect.intersects(rect))
        return;
    m_spriteBatcher->setBatchProgram(ShaderManager::Circle);
    const auto width = rect.width();
    const auto height = rect.height();

    auto addPatch = [this, &color, depth](const glm::vec2 &p0, const glm::vec2 &p1, const glm::vec2 &t0,
                                          const glm::vec2 &t1) {
        m_spriteBatcher->addSprite(nullptr, {p0, p1}, {t0, t1}, color, depth);
    };

    if (width > height)
    {
        // horizontal
        const float radius = 0.5f * height;

        const auto x0 = rect.min.x;
        const auto x1 = rect.min.x + radius;
        const auto x2 = rect.max.x - radius;
        const auto x3 = rect.max.x;
        const auto y0 = 0.5f * (rect.min.y + rect.max.y) - radius;
        const auto y1 = 0.5f * (rect.min.y + rect.max.y) + radius;

        addPatch({x0, y0}, {x1, y1}, {0.0f, 0.0f}, {0.5f, 1.0f});
        addPatch({x1, y0}, {x2, y1}, {0.5f, 0.0f}, {0.5f, 1.0f});
        addPatch({x2, y0}, {x3, y1}, {0.5f, 0.0f}, {1.0f, 1.0f});
    }
    else
    {
        // vertical
        const float radius = 0.5f * width;

        const auto y0 = rect.min.y;
        const auto y1 = rect.min.y + radius;
        const auto y2 = rect.max.y - radius;
        const auto y3 = rect.max.y;
        const auto x0 = 0.5f * (rect.min.x + rect.max.x) - radius;
        const auto x1 = 0.5f * (rect.min.x + rect.max.x) + radius;

        addPatch({x0, y0}, {x1, y1}, {0.0f, 0.0f}, {1.0f, 0.5f});
        addPatch({x0, y1}, {x1, y2}, {0.0f, 0.5f}, {1.0f, 0.5f});
        addPatch({x0, y2}, {x1, y3}, {0.0f, 0.5f}, {1.0f, 1.0f});
    }
}

void Painter::drawRoundedRect(const RectF &rect, float cornerRadius, const glm::vec4 &color, int depth)
{
    if (!m_clipRect.intersects(rect))
        return;
    m_spriteBatcher->setBatchProgram(ShaderManager::Circle);
    const auto width = rect.width();
    const auto height = rect.height();
    const auto radius = std::min(std::min(cornerRadius, 0.5f * width), 0.5f * height);

    const auto x0 = rect.min.x;
    const auto x1 = rect.min.x + radius;
    const auto x2 = rect.max.x - radius;
    const auto x3 = rect.max.x;

    const auto y0 = rect.min.y;
    const auto y1 = rect.min.y + radius;
    const auto y2 = rect.max.y - radius;
    const auto y3 = rect.max.y;

    auto addPatch = [this, &color, depth](const glm::vec2 &p0, const glm::vec2 &p1, const glm::vec2 &t0,
                                          const glm::vec2 &t1) {
        m_spriteBatcher->addSprite(nullptr, {p0, p1}, {t0, t1}, color, depth);
    };

    addPatch({x0, y0}, {x1, y1}, {0.0f, 0.0f}, {0.5f, 0.5f});
    addPatch({x1, y0}, {x2, y1}, {0.5f, 0.0f}, {0.5f, 0.5f});
    addPatch({x2, y0}, {x3, y1}, {0.5f, 0.0f}, {1.0f, 0.5f});

    addPatch({x0, y1}, {x1, y2}, {0.0f, 0.5f}, {0.5f, 0.5f});
    addPatch({x1, y1}, {x2, y2}, {0.5f, 0.5f}, {0.5f, 0.5f});
    addPatch({x2, y1}, {x3, y2}, {0.5f, 0.5f}, {1.0f, 0.5f});

    addPatch({x0, y2}, {x1, y3}, {0.0f, 0.5f}, {0.5f, 1.0f});
    addPatch({x1, y2}, {x2, y3}, {0.5f, 0.5f}, {0.5f, 1.0f});
    addPatch({x2, y2}, {x3, y3}, {0.5f, 0.5f}, {1.0f, 1.0f});
}

} // namespace miniui
