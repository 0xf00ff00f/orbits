#include "pixmapcache.h"

#include "log.h"

namespace miniui
{

namespace
{
std::string pixmapPath(std::string_view source)
{
    return std::string("assets/images/") + std::string(source);
}
} // namespace

PixmapCache::PixmapCache(TextureAtlas *textureAtlas)
    : m_textureAtlas(textureAtlas)
{
}

PixmapCache::~PixmapCache() = default;

std::optional<PackedPixmap> PixmapCache::pixmap(std::string_view source)
{
    auto key = std::string(source);
    auto it = m_pixmaps.find(key);
    if (it == m_pixmaps.end())
    {
        auto pixmap = [this, &source]() -> std::optional<PackedPixmap> {
            const auto path = pixmapPath(source);
            Pixmap pm = loadPixmap(path);
            if (!pm)
            {
                log("Failed to load image %s\n", path.c_str());
                return std::nullopt;
            }
            return m_textureAtlas->addPixmap(pm);
        }();
        it = m_pixmaps.emplace(std::move(key), pixmap).first;
    }
    return it->second;
}

} // namespace miniui
