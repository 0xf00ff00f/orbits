#include "font.h"

#include "ioutil.h"
#include "pixmap.h"
#include "log.h"
#include "system.h"

namespace miniui
{

Font::Font(TextureAtlas *textureAtlas)
    : m_textureAtlas(textureAtlas)
{
}

Font::~Font() = default;

bool Font::load(const std::string &ttfPath, int pixelHeight)
{
    log("Loading font %s:%d\n", ttfPath.c_str(), pixelHeight);

    auto buffer = Util::readFile(ttfPath);
    if (!buffer)
        return false;

    m_ttfBuffer = std::move(*buffer);

    int result = stbtt_InitFont(&m_font, m_ttfBuffer.data(), stbtt_GetFontOffsetForIndex(m_ttfBuffer.data(), 0));
    if (result == 0)
    {
        return false;
    }

    m_scale = stbtt_ScaleForPixelHeight(&m_font, pixelHeight);

    int ascent;
    int descent;
    int lineGap;
    stbtt_GetFontVMetrics(&m_font, &ascent, &descent, &lineGap);
    m_ascent = m_scale * ascent;
    m_descent = m_scale * descent;
    m_lineGap = m_scale * lineGap;

    m_pixelHeight = pixelHeight;

    return true;
}

const Font::Glyph *Font::glyph(int codepoint)
{
    auto it = m_glyphs.find(codepoint);
    if (it == m_glyphs.end())
        it = m_glyphs.insert(it, {codepoint, initializeGlyph(codepoint)});
    return it->second.get();
}

float Font::textWidth(std::u32string_view text)
{
    float width = 0.0f;
    for (auto ch : text)
    {
        if (const auto *g = glyph(ch); g)
            width += g->advanceWidth;
    }
    return width;
}

std::unique_ptr<Font::Glyph> Font::initializeGlyph(int codepoint)
{
    int ix0, iy0, ix1, iy1;
    stbtt_GetCodepointBitmapBox(&m_font, codepoint, m_scale, m_scale, &ix0, &iy0, &ix1, &iy1);

    const auto width = ix1 - ix0;
    const auto height = iy1 - iy0;

    std::vector<unsigned char> pixels;
    pixels.resize(width * height);
    stbtt_MakeCodepointBitmap(&m_font, pixels.data(), width, height, width, m_scale, m_scale, codepoint);

    constexpr auto Border = 1;

    Pixmap pixmap;
    pixmap.width = width + 2 * Border;
    pixmap.height = height + 2 * Border;
    pixmap.pixelType = PixelType::Grayscale;
    pixmap.pixels.resize(pixmap.width * pixmap.height);
    std::fill(pixmap.pixels.begin(), pixmap.pixels.end(), 0);
    for (int i = 0; i < height; ++i)
    {
        const auto *src = pixels.data() + i * width;
        auto *dest = pixmap.pixels.data() + (i + Border) * pixmap.width + Border;
        std::copy(src, src + width, dest);
    }

    auto packedPixmap = m_textureAtlas->addPixmap(pixmap);
    if (!packedPixmap)
    {
        log("Couldn't fit glyph %d in texture atlas\n", codepoint);
        return {};
    }

    int advanceWidth, leftSideBearing;
    stbtt_GetCodepointHMetrics(&m_font, codepoint, &advanceWidth, &leftSideBearing);

    ix0 -= Border;
    ix1 += Border;
    iy0 -= Border;
    iy1 += Border;
    assert(packedPixmap->width == ix1 - ix0);
    assert(packedPixmap->height == iy1 - iy0);

    auto glyph = std::make_unique<Glyph>();
    glyph->boundingBox = RectI{{ix0, iy0}, {ix1, iy1}};
    glyph->advanceWidth = m_scale * advanceWidth;
    glyph->pixmap = *packedPixmap;
    return glyph;
}

} // namespace miniui
