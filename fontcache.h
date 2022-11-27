#pragma once

#include "noncopyable.h"

#include "font.h"

#include <cstddef>
#include <string_view>
#include <string>
#include <unordered_map>
#include <memory>

class TextureAtlas;

namespace miniui
{
class Font;

class FontCache : private NonCopyable
{
public:
    explicit FontCache(TextureAtlas *textureAtlas);
    ~FontCache();

    Font *font(std::string_view fontName, int pixelHeight);

private:
    TextureAtlas *m_textureAtlas;
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
    std::unordered_map<FontKey, std::unique_ptr<Font>, FontKeyHasher> m_fonts;
};

} // namespace miniui
