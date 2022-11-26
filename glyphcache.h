#pragma once

#include "textureatlas.h"
#include "util.h"

#include <glm/glm.hpp>
#include <stb_truetype.h>

#include <string>
#include <unordered_map>
#include <memory>
#include <string_view>

struct Pixmap;

namespace miniui
{

class GlyphCache
{
public:
    GlyphCache();
    ~GlyphCache();

    bool load(const std::string &ttfPath, int pixelHeight);

    struct Glyph
    {
        BoxI boundingBox;
        float advanceWidth;
        PackedPixmap pixmap;
    };
    const Glyph *glyph(int codepoint);

    int pixelHeight() const { return m_pixelHeight; }
    float ascent() const { return m_ascent; }
    float descent() const { return m_descent; }
    float lineGap() const { return m_lineGap; }

private:
    std::unique_ptr<Glyph> initializeGlyph(int codepoint);
    Pixmap codepointPixmap(int codepoint) const;

    std::vector<unsigned char> m_ttfBuffer;
    stbtt_fontinfo m_font;
    std::unordered_map<int, std::unique_ptr<Glyph>> m_glyphs;
    int m_pixelHeight;
    float m_scale = 0.0f;
    float m_ascent;
    float m_descent;
    float m_lineGap;
};

}
