#pragma once

#include "noncopyable.h"

#include <cstddef>
#include <string_view>
#include <string>
#include <unordered_map>
#include <memory>

namespace miniui
{
class GlyphCache;

class FontCache : private NonCopyable
{
public:
    ~FontCache();

    GlyphCache *glyphCache(std::string_view fontName, int pixelHeight);

private:
    struct FontKey
    {
        std::string name;
        int pixelHeight;
        bool operator==(const FontKey &other) const { return name == other.name && pixelHeight == other.pixelHeight; }
    };
    struct FontKeyHasher
    {
        std::size_t operator()(const FontKey &key) const;
    };
    std::unordered_map<FontKey, std::unique_ptr<GlyphCache>, FontKeyHasher> m_fonts;
};

}
