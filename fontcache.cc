#include "fontcache.h"

#include "glyphcache.h"
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

FontCache::~FontCache() = default;

std::size_t FontCache::FontKeyHasher::operator()(const FontCache::FontKey &key) const
{
    std::size_t hash = 311;
    hash = hash * 31 + static_cast<std::size_t>(key.pixelHeight);
    hash = hash * 31 + std::hash<std::string>()(key.name);
    return hash;
}

GlyphCache *FontCache::glyphCache(std::string_view name, int pixelHeight)
{
    FontKey key{std::string(name), pixelHeight};
    auto it = m_fonts.find(key);
    if (it == m_fonts.end())
    {
        auto glyphCache = std::make_unique<GlyphCache>();
        const auto path = fontPath(name);
        if (!glyphCache->load(path, pixelHeight))
        {
            log("Failed to load font %s\n", path.c_str());
            glyphCache.reset();
        }
        it = m_fonts.emplace(std::move(key), std::move(glyphCache)).first;
    }
    return it->second.get();
}

} // namespace miniui
