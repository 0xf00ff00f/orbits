#pragma once

#include "noncopyable.h"

#include "textureatlas.h"

#include <string>
#include <string_view>
#include <unordered_map>

namespace miniui
{

class PixmapCache : private NonCopyable
{
public:
    ~PixmapCache();

    std::optional<PackedPixmap> pixmap(std::string_view source);

private:
    std::unordered_map<std::string, std::optional<PackedPixmap>> m_pixmaps;
};

} // namespace miniui
