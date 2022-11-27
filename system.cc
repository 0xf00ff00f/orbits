#include "system.h"

#include "fontcache.h"
#include "painter.h"
#include "pixmapcache.h"
#include "shadermanager.h"
#include "textureatlas.h"

System *System::s_instance = nullptr;

namespace
{
constexpr auto TextureAtlasPageSize = 1024;
}

bool System::initialize()
{
    return true;
}

void System::shutdown()
{
    delete s_instance;
    s_instance = nullptr;
}

System::System()
    : m_shaderManager(std::make_unique<ShaderManager>())
    , m_uiPainter(std::make_unique<miniui::Painter>())
    , m_fontTextureAtlas(
          std::make_unique<TextureAtlas>(TextureAtlasPageSize, TextureAtlasPageSize, PixelType::Grayscale))
    , m_pixmapTextureAtlas(std::make_unique<TextureAtlas>(TextureAtlasPageSize, TextureAtlasPageSize, PixelType::RGBA))
    , m_fontCache(std::make_unique<miniui::FontCache>(m_fontTextureAtlas.get()))
    , m_pixmapCache(std::make_unique<miniui::PixmapCache>(m_pixmapTextureAtlas.get()))
{
}

System::~System() = default;
