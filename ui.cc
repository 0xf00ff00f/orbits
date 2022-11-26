#include "ui.h"

#include "textureatlas.h"
#include "fontcache.h"
#include "spritebatcher.h"
#include "log.h"

namespace
{
constexpr auto TextureAtlasPageSize = 512;

std::string fontPath(std::string_view basename)
{
    return std::string("assets/fonts/") + std::string(basename);
}

glm::vec4 asVec4(mu_Color color)
{
    return (1.0f / 255.0f) * glm::vec4(color.r, color.g, color.b, color.a);
}
}

UI::UI()
    : m_textureAtlas(std::make_unique<TextureAtlas>(TextureAtlasPageSize, TextureAtlasPageSize, PixelType::Grayscale))
    , m_fontCache(std::make_unique<FontCache>(m_textureAtlas.get()))
    , m_spriteBatcher(std::make_unique<SpriteBatcher>())
{
    mu_init(&m_ctx);
    m_ctx.extra = this;
    m_ctx.text_width = [](mu_Font font, const char *text, int len, void *extra) {
        return reinterpret_cast<UI *>(extra)->textWidth(font, text, len);
    };
    m_ctx.text_height = [](mu_Font font, void *extra) { return reinterpret_cast<UI *>(extra)->textHeight(font); };

    if (!m_fontCache->load("assets/fonts/OpenSans_Regular.ttf", 18))
        log("Failed to load font\n");
}

UI::~UI() = default;

void UI::setTransformMatrix(const glm::mat4 &matrix)
{
    m_spriteBatcher->setTransformMatrix(matrix);
}

glm::mat4 UI::transformMatrix() const
{
    return m_spriteBatcher->transformMatrix();
}

int UI::textWidth(mu_Font font, const char *text, int len)
{
    int width = 0;
    for (const char *p = text; *p != '\0'; ++p)
    {
        if (const auto *g = m_fontCache->getGlyph(*p))
            width += g->advanceWidth;
    }
    return width;
}

int UI::textHeight(mu_Font font)
{
    return m_fontCache->pixelHeight();
}

void UI::begin()
{
    mu_begin(&m_ctx);
}

void UI::end()
{
    mu_end(&m_ctx);
    render();
}

int UI::beginWindow(const char *title, mu_Rect rect, int opt)
{
    return mu_begin_window_ex(&m_ctx, title, rect, opt);
}

void UI::endWindow()
{
    mu_end_window(&m_ctx);
}

void UI::render()
{
    m_spriteBatcher->begin();

    mu_Command *cmd = nullptr;
    while (mu_next_command(&m_ctx, &cmd))
    {
        switch (cmd->type)
        {
        case MU_COMMAND_TEXT:
            drawText(cmd->text.str, cmd->text.pos, cmd->text.color);
            break;
        case MU_COMMAND_RECT:
            drawRect(cmd->rect.rect, cmd->rect.color);
            break;
        case MU_COMMAND_ICON:
            drawIcon(cmd->icon.id, cmd->icon.rect, cmd->icon.color);
            break;
        case MU_COMMAND_CLIP:
            setClipRect(cmd->clip.rect);
            break;
        default:
            break;
        }
    }

    m_spriteBatcher->flush();
}

void UI::drawRect(mu_Rect rect, mu_Color color)
{
    const auto topLeft = glm::vec2(rect.x, rect.y);
    const auto bottomRight = topLeft + glm::vec2(rect.w, rect.h);
    m_spriteBatcher->setBatchProgram(ShaderManager::Flat);
    m_spriteBatcher->addSprite(topLeft, bottomRight, asVec4(color), 0);
}

void UI::drawText(const char *text, mu_Vec2 pos, mu_Color color)
{
    m_spriteBatcher->setBatchProgram(ShaderManager::Text);

    auto basePos = glm::vec2(pos.x, pos.y + m_fontCache->ascent());
    for (const char *p = text; *p != '\0'; ++p)
    {
        const auto glyph = m_fontCache->getGlyph(*p);
        if (!glyph)
            continue;
        const auto topLeft = basePos + glm::vec2(glyph->boundingBox.min);
        const auto bottomRight = topLeft + glm::vec2(glyph->boundingBox.max - glyph->boundingBox.min);
        const auto &pixmap = glyph->pixmap;
        m_spriteBatcher->addSprite(pixmap, topLeft, bottomRight, asVec4(color), 0);

        basePos.x += glyph->advanceWidth;
    }
}

void UI::drawIcon(int id, mu_Rect rect, mu_Color color)
{
    // XXX
}

void UI::setClipRect(mu_Rect rect)
{
    // XXX
}
