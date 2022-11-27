#include "font.h"

#include "glyphcache.h"
#include "system.h"
#include "painter.h"
#include "fontcache.h"

namespace miniui
{

Font::Font(std::string_view name, int pixelHeight)
    : m_name(name)
    , m_pixelHeight(pixelHeight)
{
}

float Font::ascent() const
{
    auto *glyphs = glyphCache();
    return glyphs ? glyphs->ascent() : 0.0f;
}

float Font::descent() const
{
    auto *glyphs = glyphCache();
    return glyphs ? glyphs->descent() : 0.0f;
}

float Font::lineGap() const
{
    auto *glyphs = glyphCache();
    return glyphs ? glyphs->lineGap() : 0.0f;
}

float Font::textWidth(std::u32string_view text) const
{
    float width = 0.0f;
    if (auto *glyphs = glyphCache(); glyphs)
    {
        for (auto ch : text)
        {
            if (const auto *g = glyphs->glyph(ch); g)
                width += g->advanceWidth;
        }
    }
    return width;
}

GlyphCache *Font::glyphCache() const
{
    if (!m_glyphCache)
    {
        auto *fontCache = System::instance().uiPainter()->fontCache();
        m_glyphCache = fontCache->glyphCache(m_name, m_pixelHeight);
    }
    return m_glyphCache;
}

} // namespace miniui
