#pragma once

#include "noncopyable.h"

#include <glm/glm.hpp>

#include <string_view>
#include <string>
#include <memory>
#include <unordered_map>

class TextureAtlas;
class SpriteBatcher;
struct PackedPixmap;

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

    void setTransformMatrix(const glm::mat4 &matrix);
    glm::mat4 transformMatrix() const;

    void setFont(const Font &font);

    void begin();
    void end();

    void drawRect(const glm::vec2 &topLeft, const glm::vec2 &bottomRight, const glm::vec4 &color, int depth);
    void drawPixmap(const PackedPixmap &pixmap, const glm::vec2 &topLeft, const glm::vec2 &bottomRight,
                    const glm::vec4 &color, int depth);
    void drawText(std::u32string_view text, const glm::vec2 &pos, const glm::vec4 &color, int depth);

    FontCache *fontCache() const { return m_fontCache.get(); }
    PixmapCache *pixmapCache() const { return m_pixmapCache.get(); }

private:
    void render();

    std::unique_ptr<FontCache> m_fontCache;
    std::unique_ptr<PixmapCache> m_pixmapCache;
    std::unique_ptr<TextureAtlas> m_fontTextureAtlas;
    std::unique_ptr<TextureAtlas> m_pixmapTextureAtlas;
    std::unique_ptr<SpriteBatcher> m_spriteBatcher;
    GlyphCache *m_font = nullptr;

    friend class GlyphCache;
    friend class PixmapCache;
};

}
