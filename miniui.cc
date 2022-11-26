#include "miniui.h"

#include "system.h"
#include "painter.h"

#include <algorithm>

namespace miniui
{
Item::~Item() = default;

void Item::renderBackground(const glm::vec2 &pos, int depth)
{
    if (!fillBackground)
        return;
    auto *painter = System::instance().uiPainter();
    painter->drawRect(pos, pos + glm::vec2(width(), height()), bgColor, depth);
}

Label::Label(std::u32string_view text)
    : m_text(text)
{
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
    auto *fontCache = System::instance().uiPainter()->fontCache();
    m_height = fontCache->pixelHeight() + m_margins.top + m_margins.bottom;
    m_width = fontCache->textWidth(m_text) + m_margins.left + m_margins.right;
}

void Label::render(const glm::vec2 &pos, int depth)
{
    renderBackground(pos, depth);
    auto *painter = System::instance().uiPainter();
    painter->drawText(m_text, pos + glm::vec2(m_margins.left, m_margins.top), color, depth + 1);
}

void Container::addItem(std::unique_ptr<Item> item)
{
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

void Column::updateLayout()
{
    m_width = 0;
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
}

void Column::render(const glm::vec2 &pos, int depth)
{
    renderBackground(pos, depth);
    auto p = pos + glm::vec2(m_margins.left, m_margins.top);
    for (auto &item : m_items)
    {
        item->render(p, depth + 1);
        p.y += item->height() + m_spacing;
    }
}

void Row::updateLayout()
{
    m_width = 0;
    m_height = 0;
    for (auto &item : m_items)
    {
        m_width += item->width();
        m_height = std::max(m_height, item->height());
    }
    if (!m_items.empty())
        m_width += (m_items.size() - 1) * m_spacing;
    m_width += m_margins.left + m_margins.right;
    m_height += m_margins.top + m_margins.bottom;
}

void Row::render(const glm::vec2 &pos, int depth)
{
    renderBackground(pos, depth);
    auto p = pos + glm::vec2(m_margins.left, m_margins.top);
    for (auto &item : m_items)
    {
        item->render(p, depth + 1);
        p.x += item->width() + m_spacing;
    }
}

}
