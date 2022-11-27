#include "fontcache.h"

#include "log.h"

#include <cstddef>

namespace miniui
{

namespace
{
std::string fontPath(std::string_view basename)
{
    return std::string("assets/fonts/") + std::string(basename) + std::string(".ttf");
}
} // namespace

FontCache::FontCache(TextureAtlas *textureAtlas)
    : m_textureAtlas(textureAtlas)
{
}

FontCache::~FontCache() = default;

std::size_t FontCache::FontKeyHasher::operator()(const FontCache::FontKey &key) const
{
    std::size_t hash = 311;
    hash = hash * 31 + static_cast<std::size_t>(key.pixelHeight);
    hash = hash * 31 + std::hash<std::string>()(key.name);
    return hash;
}

Font *FontCache::font(std::string_view name, int pixelHeight)
{
    FontKey key{std::string(name), pixelHeight};
    auto it = m_fonts.find(key);
    if (it == m_fonts.end())
    {
        auto font = std::make_unique<Font>(m_textureAtlas);
        const auto path = fontPath(name);
        if (!font->load(path, pixelHeight))
        {
            log("Failed to load font %s\n", path.c_str());
            font.reset();
        }
        it = m_fonts.emplace(std::move(key), std::move(font)).first;
    }
    return it->second.get();
}

} // namespace miniui
