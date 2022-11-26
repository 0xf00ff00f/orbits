#include "pixmapcache.h"

#include "system.h"
#include "painter.h"

namespace miniui
{

namespace
{
std::string pixmapPath(std::string_view source)
{
    return std::string("assets/images/") + std::string(source);
}
}

PixmapCache::~PixmapCache() = default;

std::optional<PackedPixmap> PixmapCache::pixmap(std::string_view source)
{
    auto key = std::string(source);
    auto it = m_pixmaps.find(key);
    if (it == m_pixmaps.end())
    {
        auto pixmap = [&source]() -> std::optional<PackedPixmap> {
            Pixmap pm = loadPixmap(pixmapPath(source));
            if (!pm)
                return std::nullopt;
            auto *painter = System::instance().uiPainter();
            return painter->m_pixmapTextureAtlas->addPixmap(pm);
        }();
        it = m_pixmaps.emplace(std::move(key), pixmap).first;
    }
    return it->second;
}

}
