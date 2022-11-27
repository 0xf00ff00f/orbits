#pragma once

#include "noncopyable.h"

#include "util.h"

#include <glm/glm.hpp>

#include <string_view>
#include <string>
#include <memory>
#include <unordered_map>

class TextureAtlas;
struct PackedPixmap;

namespace gl
{
class SpriteBatcher;
}

namespace miniui
{
class GlyphCache;
class FontCache;
class Font;
class PixmapCache;

class Painter : private NonCopyable
{
public:
    Painter();
    ~Painter();

    void setWindowSize(int width, int height);

    void begin();
    void end();

    void setFont(const Font &font);

    void setClipRect(const RectF &rect);
    RectF clipRect() const { return m_clipRect; }

    void drawRect(const RectF &rect, const glm::vec4 &color, int depth);
    void drawPixmap(const PackedPixmap &pixmap, const RectF &rect, const glm::vec4 &color, int depth);
    void drawText(std::u32string_view text, const glm::vec2 &pos, const glm::vec4 &color, int depth);
    void drawCircle(const glm::vec2 &center, float radius, const glm::vec4 &color, int depth);

    FontCache *fontCache() const { return m_fontCache.get(); }
    PixmapCache *pixmapCache() const { return m_pixmapCache.get(); }

private:
    void render();
    void updateTransformMatrix();

    int m_windowWidth = 0;
    int m_windowHeight = 0;
    std::unique_ptr<FontCache> m_fontCache;
    std::unique_ptr<PixmapCache> m_pixmapCache;
    std::unique_ptr<TextureAtlas> m_fontTextureAtlas;
    std::unique_ptr<TextureAtlas> m_pixmapTextureAtlas;
    std::unique_ptr<gl::SpriteBatcher> m_spriteBatcher;
    GlyphCache *m_font = nullptr;
    RectF m_clipRect;
    bool m_clippingEnabled = false;

    friend class GlyphCache;
    friend class PixmapCache;
};

} // namespace miniui
