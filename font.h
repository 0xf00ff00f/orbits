#pragma once

#include <string>
#include <string_view>

namespace miniui
{
class GlyphCache;

class Font
{
public:
    Font(std::string_view name, int pixelHeight);

    std::string name() const { return m_name; }
    int pixelHeight() const { return m_pixelHeight; }
    float ascent() const;
    float descent() const;
    float lineGap() const;
    float textWidth(std::u32string_view text) const;

    bool operator==(const Font &other) const { return m_name == other.m_name && m_pixelHeight == other.m_pixelHeight; }

private:
    GlyphCache *glyphCache() const;

    std::string m_name;
    int m_pixelHeight;
    mutable GlyphCache *m_glyphCache = nullptr;

    friend class Painter;
};

} // namespace miniui
