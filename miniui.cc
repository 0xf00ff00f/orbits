#include "miniui.h"

#include "system.h"
#include "painter.h"
#include "glyphcache.h"
#include "pixmapcache.h"
#include "log.h"

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

void Item::setSize(float width, float height)
{
    if (width == m_width && height == m_height)
        return;
    m_width = width;
    m_height = height;
    m_resizedEvent.notify(m_width, m_height);
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

void Label::mouseEvent(const MouseEvent &event)
{
    if (event.type == MouseEvent::Type::Press)
        log("**** clicked label %s\n", std::string(m_text.begin(), m_text.end()).c_str());
    ;
}

void Label::setFont(const Font &font)
{
    if (font == m_font)
        return;
    m_font = font;
    updateSize();
}

void Label::setText(std::u32string_view text)
{
    if (text == m_text)
        return;
    m_text = text;
    updateSize();
}

void Label::setMargins(Margins margins)
{
    if (margins == m_margins)
        return;
    m_margins = margins;
    updateSize();
}

void Label::updateSize()
{
    const float height = m_font.pixelHeight() + m_margins.top + m_margins.bottom;
    const float width = m_font.textWidth(m_text) + m_margins.left + m_margins.right;
    setSize(width, height);
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

void Image::mouseEvent(const MouseEvent &event)
{
    if (event.type == MouseEvent::Type::Press)
        log("**** clicked image\n");
}

void Image::setSource(std::string_view source)
{
    if (source == m_source)
        return;
    m_source = source;
    m_pixmap = [this] {
        auto *cache = System::instance().uiPainter()->pixmapCache();
        return cache->pixmap(m_source);
    }();
    updateSize();
}

void Image::setMargins(Margins margins)
{
    if (margins == m_margins)
        return;
    m_margins = margins;
    updateSize();
}

void Image::updateSize()
{
    float height = m_margins.top + m_margins.bottom;
    float width = m_margins.left + m_margins.right;
    if (m_pixmap)
    {
        width += m_pixmap->width;
        height += m_pixmap->height;
    }
    setSize(width, height);
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

void Container::mouseEvent(const MouseEvent &event)
{
    const auto &p = event.position;
    for (auto &layoutItem : m_layoutItems)
    {
        const auto &item = layoutItem->item;
        const auto &offset = layoutItem->offset;
        if (p.x >= offset.x && p.x < offset.x + item->width() && p.y >= offset.y && p.y < offset.y + item->height())
        {
            MouseEvent itemEvent = event;
            itemEvent.position -= offset;
            item->mouseEvent(itemEvent);
        }
    }
}

void Container::addItem(std::unique_ptr<Item> item)
{
    auto resizedConnection = item->resizedEvent().connect([this](float width, float height) {
        log("child resized: %f %f\n", width, height);
        updateLayout();
    });
    m_layoutItems.emplace_back(new LayoutItem{{}, std::move(item)});
    m_childResizedConnections.push_back(std::move(resizedConnection));
    updateLayout();
}

void Container::setMargins(Margins margins)
{
    if (margins == m_margins)
        return;
    m_margins = margins;
    updateLayout();
}

void Container::setSpacing(float spacing)
{
    if (spacing == m_spacing)
        return;
    m_spacing = spacing;
    updateLayout();
}

void Container::render(const glm::vec2 &pos, int depth)
{
    renderBackground(pos, depth);
    for (auto &layoutItem : m_layoutItems)
        layoutItem->item->render(pos + layoutItem->offset, depth + 1);
}

void Column::setMinimumWidth(float width)
{
    if (width == m_minimumWidth)
        return;
    m_minimumWidth = width;
    updateLayout();
}

void Column::updateLayout()
{
    // update size
    float width = m_minimumWidth;
    float height = 0;
    for (auto &layoutItem : m_layoutItems)
    {
        auto &item = layoutItem->item;
        width = std::max(width, item->width());
        height += item->height();
    }
    if (!m_layoutItems.empty())
        height += (m_layoutItems.size() - 1) * m_spacing;
    width += m_margins.left + m_margins.right;
    height += m_margins.top + m_margins.bottom;
    setSize(width, height);

    // update item offsets
    auto p = glm::vec2(m_margins.left, m_margins.top);
    for (auto &layoutItem : m_layoutItems)
    {
        const float offset = [this, &item = layoutItem->item] {
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
        layoutItem->offset = p + glm::vec2(offset, 0.0f);
        p.y += layoutItem->item->height() + m_spacing;
    }
}

void Row::setMinimumHeight(float height)
{
    if (height == m_minimumHeight)
        return;
    m_minimumHeight = height;
    updateLayout();
}

void Row::updateLayout()
{
    // update size
    float width = 0;
    float height = m_minimumHeight;
    for (auto &layoutItem : m_layoutItems)
    {
        auto &item = layoutItem->item;
        width += item->width();
        height = std::max(height, item->height());
    }
    if (!m_layoutItems.empty())
        width += (m_layoutItems.size() - 1) * m_spacing;
    width += m_margins.left + m_margins.right;
    height += m_margins.top + m_margins.bottom;
    setSize(width, height);

    // update item offsets
    auto p = glm::vec2(m_margins.left, m_margins.top);
    for (auto &layoutItem : m_layoutItems)
    {
        const float offset = [this, &item = layoutItem->item] {
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
        layoutItem->offset = p + glm::vec2(0.0f, offset);
        p.x += layoutItem->item->width() + m_spacing;
    }
}

}
