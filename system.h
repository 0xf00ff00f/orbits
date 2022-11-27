#pragma once

#include <memory>

class ShaderManager;

class TextureAtlas;

namespace miniui
{
class Painter;
class FontCache;
class PixmapCache;
}

class System
{
public:
    static System *instance()
    {
        if (!s_instance)
            s_instance = new System();
        return s_instance;
    }

    static bool initialize();
    static void shutdown();

    ShaderManager *shaderManager() const { return m_shaderManager.get(); }
    miniui::Painter *uiPainter() const { return m_uiPainter.get(); }
    miniui::FontCache *fontCache() const { return m_fontCache.get(); }
    miniui::PixmapCache *pixmapCache() const { return m_pixmapCache.get(); }

private:
    System();
    ~System();

    static System *s_instance;

    std::unique_ptr<ShaderManager> m_shaderManager;
    std::unique_ptr<miniui::Painter> m_uiPainter;
    std::unique_ptr<TextureAtlas> m_fontTextureAtlas;
    std::unique_ptr<TextureAtlas> m_pixmapTextureAtlas;
    std::unique_ptr<miniui::FontCache> m_fontCache;
    std::unique_ptr<miniui::PixmapCache> m_pixmapCache;
};
