#include "miniui.h"

#include "system.h"
#include "painter.h"
#include "fontcache.h"
#include "pixmapcache.h"
#include "log.h"

#include <algorithm>
#include <memory>

namespace miniui
{
namespace
{
Font *defaultFont()
{
    static Font *font = System::instance()->fontCache()->font("OpenSans_Regular", 40);
    return font;
}
} // namespace

Item::~Item() = default;

void Item::renderBackground(Painter *painter, const glm::vec2 &pos, int depth)
{
    if (!fillBackground)
        return;
    const auto rect = RectF{pos, pos + glm::vec2(width(), height())};
    painter->drawRect(rect, bgColor, depth);
}

void Item::setSize(Size size)
{
    if (size == m_size)
        return;
    m_size = size;
    m_resizedEvent.notify(m_size);
}

Item *Item::findItem(const glm::vec2 &pos)
{
    const auto rect = RectF{{0, 0}, {m_size.width, m_size.height}};
    return rect.contains(pos) ? this : nullptr;
}

Rectangle::Rectangle()
    : Rectangle(Size{})
{
}

Rectangle::Rectangle(float width, float height)
    : Rectangle(Size{width, height})
{
}

Rectangle::Rectangle(Size size)
{
    setSize(size);
}

void Rectangle::setSize(float width, float height)
{
    setSize({width, height});
}

void Rectangle::setWidth(float width)
{
    setSize({width, m_size.height});
}

void Rectangle::setHeight(float height)
{
    setSize({m_size.width, height});
}

void Rectangle::render(Painter *painter, const glm::vec2 &pos, int depth)
{
    renderBackground(painter, pos, depth);
}

void Rectangle::mouseEvent(const MouseEvent &event) {}

Label::Label(std::u32string_view text)
    : Label(defaultFont(), text)
{
}

Label::Label(Font *font, std::u32string_view text)
    : m_font(font)
    , m_text(text)
{
    updateSize();
}

void Label::mouseEvent(const MouseEvent &event)
{
    if (event.type == MouseEvent::Type::Press)
        log("**** clicked label %s\n", std::string(m_text.begin(), m_text.end()).c_str());
}

void Label::setFont(Font *font)
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
    const float height = m_font->pixelHeight() + m_margins.top + m_margins.bottom;
    const float width = m_font->textWidth(m_text) + m_margins.left + m_margins.right;
    setSize({width, height});
}

void Label::render(Painter *painter, const glm::vec2 &pos, int depth)
{
    renderBackground(painter, pos, depth);
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
        auto *cache = System::instance()->pixmapCache();
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
    setSize({width, height});
}

void Image::render(Painter *painter, const glm::vec2 &pos, int depth)
{
    renderBackground(painter, pos, depth);
    if (m_pixmap)
    {
        const auto p = pos + glm::vec2(m_margins.left, m_margins.top);
        const auto rect = RectF{p, p + glm::vec2(m_pixmap->width, m_pixmap->height)};
        painter->drawPixmap(*m_pixmap, rect, color, depth);
    }
}

Item *Container::findItem(const glm::vec2 &pos)
{
    const auto rect = RectF{{0, 0}, {m_size.width, m_size.height}};
    if (!rect.contains(pos))
        return nullptr;
    for (auto &layoutItem : m_layoutItems)
    {
        const auto &item = layoutItem->item;
        const auto &offset = layoutItem->offset;
        const auto childRect = RectF{layoutItem->offset, layoutItem->offset + glm::vec2(item->width(), item->height())};
        if (childRect.contains(pos))
            return item->findItem(pos - offset);
    }
    return this;
}

void Container::mouseEvent(const MouseEvent &event)
{
    if (event.type == MouseEvent::Type::Press)
        log("**** clicked container\n");
}

void Container::addItem(std::unique_ptr<Item> item)
{
    auto resizedConnection = item->resizedEvent().connect([this](Size size) {
        log("child resized: %f %f\n", size.width, size.height);
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

void Container::render(Painter *painter, const glm::vec2 &pos, int depth)
{
    renderBackground(painter, pos, depth);
    for (auto &layoutItem : m_layoutItems)
        layoutItem->item->render(painter, pos + layoutItem->offset, depth + 1);
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
    float width = std::max(m_minimumWidth - (m_margins.left + m_margins.right), 0.0f);
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
    setSize({width, height});

    // update item offsets
    auto p = glm::vec2(m_margins.left, m_margins.top);
    for (auto &layoutItem : m_layoutItems)
    {
        const float offset = [this, &item = layoutItem->item] {
            const auto alignment = item->containerAlignment & (Alignment::Left | Alignment::HCenter | Alignment::Right);
            const auto contentWidth = m_size.width - (m_margins.left + m_margins.right);
            switch (alignment)
            {
            case Alignment::Left:
            default:
                return 0.0f;
            case Alignment::HCenter:
                return 0.5f * (contentWidth - item->width());
            case Alignment::Right:
                return contentWidth - item->width();
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
    float height = std::max(m_minimumHeight - (m_margins.top + m_margins.bottom), 0.0f);
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
    setSize({width, height});

    // update item offsets
    auto p = glm::vec2(m_margins.left, m_margins.top);
    for (auto &layoutItem : m_layoutItems)
    {
        const float offset = [this, &item = layoutItem->item] {
            const auto contentHeight = m_size.height - (m_margins.top + m_margins.bottom);
            const auto alignment = item->containerAlignment & (Alignment::Top | Alignment::VCenter | Alignment::Bottom);
            switch (alignment)
            {
            case Alignment::Top:
                return 0.0f;
            case Alignment::VCenter:
            default:
                return 0.5f * (contentHeight - item->height());
            case Alignment::Bottom:
                return contentHeight - item->height();
            }
        }();
        layoutItem->offset = p + glm::vec2(0.0f, offset);
        p.x += layoutItem->item->width() + m_spacing;
    }
}

ScrollArea::ScrollArea(float viewportWidth, float viewportHeight, std::unique_ptr<Item> viewportClient)
    : m_viewportClient(std::move(viewportClient))
{
    updateSize();
}

ScrollArea::ScrollArea(std::unique_ptr<Item> viewportClient)
    : ScrollArea(0, 0, std::move(viewportClient))
{
}

void ScrollArea::render(Painter *painter, const glm::vec2 &pos, int depth)
{
    renderBackground(painter, pos, depth);
    const auto viewportPos = pos + glm::vec2(m_margins.left, m_margins.top);
    const auto prevClipRect = painter->clipRect();
    const auto viewportRect = RectF{viewportPos, viewportPos + glm::vec2(m_viewportSize.width, m_viewportSize.height)};
    painter->setClipRect(viewportRect);
    m_viewportClient->render(painter, viewportPos + m_viewportOffset, depth + 1);
    painter->setClipRect(prevClipRect);
}

void ScrollArea::mouseEvent(const MouseEvent &event)
{
    switch (event.type)
    {
    case MouseEvent::Type::Press:
        m_dragging = true;
        m_mousePressPos = event.position;
        break;
    case MouseEvent::Type::Release:
        m_dragging = false;
        break;
    case MouseEvent::Type::Move:
        if (m_dragging)
        {
            const auto offset = event.position - m_mousePressPos;
            m_viewportOffset += offset;
            m_viewportOffset =
                glm::max(m_viewportOffset, glm::vec2(m_viewportSize.width - m_viewportClient->width(),
                                                     m_viewportSize.height - m_viewportClient->height()));
            m_viewportOffset = glm::min(m_viewportOffset, glm::vec2(0, 0));
            m_mousePressPos = event.position;
        }
        break;
    }
}

void ScrollArea::setMargins(Margins margins)
{
    if (margins == m_margins)
        return;
    m_margins = margins;
    updateSize();
}

void ScrollArea::setViewportSize(Size size)
{
    if (size == m_viewportSize)
        return;
    m_viewportSize = size;
    updateSize();
}

void ScrollArea::updateSize()
{
    float height = m_viewportSize.height + m_margins.top + m_margins.bottom;
    float width = m_viewportSize.width + m_margins.left + m_margins.right;
    setSize({width, height});
}
} // namespace miniui
