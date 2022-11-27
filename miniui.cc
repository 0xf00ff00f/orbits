#include "miniui.h"

#include "system.h"
#include "painter.h"
#include "glyphcache.h"
#include "pixmapcache.h"

#include <algorithm>

namespace miniui
{
namespace
{
Font defaultFont()
{
    return Font("OpenSans_Regular", 40);
}
}

Item::~Item() = default;

void Item::renderBackground(const glm::vec2 &pos, int depth)
{
    if (!fillBackground)
        return;
    auto *painter = System::instance().uiPainter();
    painter->drawRect(pos, pos + glm::vec2(width(), height()), bgColor, depth);
}

Label::Label(std::u32string_view text)
    : Label(defaultFont(), text)
{
}

Label::Label(const Font &font, std::u32string_view text)
    : m_font(font)
    , m_text(text)
{
    updateSize();
}

void Label::setFont(const Font &font)
{
    m_font = font;
    updateSize();
}

void Label::setText(std::u32string_view text)
{
    m_text = text;
    updateSize();
}

void Label::setMargins(Margins margins)
{
    m_margins = margins;
    updateSize();
}

void Label::updateSize()
{
    m_height = m_font.pixelHeight() + m_margins.top + m_margins.bottom;
    m_width = m_font.textWidth(m_text) + m_margins.left + m_margins.right;
    if (m_parentContainer)
        m_parentContainer->updateLayout();
}

void Label::render(const glm::vec2 &pos, int depth)
{
    renderBackground(pos, depth);
    auto *painter = System::instance().uiPainter();
    painter->setFont(m_font);
    painter->drawText(m_text, pos + glm::vec2(m_margins.left, m_margins.top), color, depth + 1);
}

Image::Image() = default;

Image::Image(std::string_view source)
{
    setSource(source);
}

void Image::setSource(std::string_view source)
{
    m_source = source;
    m_pixmap = [this] {
        auto *cache = System::instance().uiPainter()->pixmapCache();
        return cache->pixmap(m_source);
    }();
    updateSize();
}

void Image::setMargins(Margins margins)
{
    m_margins = margins;
    updateSize();
}

void Image::updateSize()
{
    m_height = m_margins.top + m_margins.bottom;
    m_width = m_margins.left + m_margins.right;
    if (m_pixmap)
    {
        m_width += m_pixmap->width;
        m_height += m_pixmap->height;
    }
    if (m_parentContainer)
        m_parentContainer->updateLayout();
}

void Image::render(const glm::vec2 &pos, int depth)
{
    renderBackground(pos, depth);
    if (m_pixmap)
    {
        auto *painter = System::instance().uiPainter();
        painter->drawPixmap(*m_pixmap, pos, pos + glm::vec2(m_pixmap->width, m_pixmap->height), color, depth);
    }
}

void Container::addItem(std::unique_ptr<Item> item)
{
    assert(!item->m_parentContainer);
    item->m_parentContainer = this;
    m_items.push_back(std::move(item));
    updateLayout();
}

void Container::setMargins(Margins margins)
{
    m_margins = margins;
    updateLayout();
}

void Container::setSpacing(float spacing)
{
    m_spacing = spacing;
    updateLayout();
}

void Container::updateLayout()
{
    if (m_parentContainer)
        m_parentContainer->updateLayout();
}

void Column::setMinimumWidth(float width)
{
    m_minimumWidth = width;
    updateLayout();
}

void Column::updateLayout()
{
    m_width = m_minimumWidth;
    m_height = 0;
    for (auto &item : m_items)
    {
        m_width = std::max(m_width, item->width());
        m_height += item->height();
    }
    if (!m_items.empty())
        m_height += (m_items.size() - 1) * m_spacing;
    m_width += m_margins.left + m_margins.right;
    m_height += m_margins.top + m_margins.bottom;
    Container::updateLayout();
}

void Column::render(const glm::vec2 &pos, int depth)
{
    renderBackground(pos, depth);
    auto p = pos + glm::vec2(m_margins.left, m_margins.top);
    for (auto &item : m_items)
    {
        const float offset = [this, &item] {
            const auto alignment = item->alignment & (Align::Left | Align::HCenter | Align::Right);
            switch (alignment)
            {
            case Align::Left:
            default:
                return 0.0f;
            case Align::HCenter:
                return 0.5f * (m_width - (m_margins.left + m_margins.right) - item->width());
            case Align::Right:
                return m_width - (m_margins.left + m_margins.right) - item->width();
            }
        }();
        item->render(p + glm::vec2(offset, 0.0f), depth + 1);
        p.y += item->height() + m_spacing;
    }
}

void Row::setMinimumHeight(float height)
{
    m_minimumHeight = height;
}

void Row::updateLayout()
{
    m_width = 0;
    m_height = m_minimumHeight;
    for (auto &item : m_items)
    {
        m_width += item->width();
        m_height = std::max(m_height, item->height());
    }
    if (!m_items.empty())
        m_width += (m_items.size() - 1) * m_spacing;
    m_width += m_margins.left + m_margins.right;
    m_height += m_margins.top + m_margins.bottom;
    Container::updateLayout();
}

void Row::render(const glm::vec2 &pos, int depth)
{
    renderBackground(pos, depth);
    auto p = pos + glm::vec2(m_margins.left, m_margins.top);
    for (auto &item : m_items)
    {
        const float offset = [this, &item] {
            const auto alignment = item->alignment & (Align::Top | Align::VCenter | Align::Bottom);
            switch (alignment)
            {
            case Align::Top:
                return 0.0f;
            case Align::VCenter:
            default:
                return 0.5f * (m_height - (m_margins.top + m_margins.bottom) - item->height());
            case Align::Bottom:
                return m_height - (m_margins.top + m_margins.bottom) - item->height();
            }
        }();
        item->render(p + glm::vec2(0.0f, offset), depth + 1);
        p.x += item->width() + m_spacing;
    }
}

}
