#pragma once

#include "noncopyable.h"

#include "textureatlas.h"

#include <string>
#include <string_view>
#include <unordered_map>

class TextureAtlas;

namespace miniui
{

class PixmapCache : private NonCopyable
{
public:
    explicit PixmapCache(TextureAtlas *textureAtlas);
    ~PixmapCache();

    std::optional<PackedPixmap> pixmap(std::string_view source);

private:
    TextureAtlas *m_textureAtlas;
    std::unordered_map<std::string, std::optional<PackedPixmap>> m_pixmaps;
};

} // namespace miniui
