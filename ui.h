#pragma once

#include "noncopyable.h"

#include <microui.h>

#include <glm/glm.hpp>

#include <memory>

class FontCache;
class ShaderManager;
class TextureAtlas;
class SpriteBatcher;

class UI : private NonCopyable
{
public:
    explicit UI(ShaderManager *shaderManager);
    ~UI();

    void setTransformMatrix(const glm::mat4 &matrix);
    glm::mat4 transformMatrix() const;

    bool initialize();

    int textWidth(mu_Font font, const char *text, int len);
    int textHeight(mu_Font font);

    void begin();
    void end();

    int beginWindow(const char *title, mu_Rect rect, int opt = 0);
    void endWindow();

private:
    void render();
    void drawRect(mu_Rect rect, mu_Color color);
    void drawText(const char *text, mu_Vec2 pos, mu_Color color);
    void drawIcon(int id, mu_Rect rect, mu_Color color);
    void setClipRect(mu_Rect rect);

    mu_Context m_ctx;
    std::unique_ptr<TextureAtlas> m_textureAtlas;
    std::unique_ptr<FontCache> m_fontCache;
    std::unique_ptr<SpriteBatcher> m_spriteBatcher;
};
