#include "miniui.h"

#include "glm/detail/qualifier.hpp"
#include "glm/fwd.hpp"
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

void Item::update(float) {}

void Item::renderBackground(Painter *painter, const glm::vec2 &pos, int depth)
{
    if (!fillBackground)
        return;
    const auto rect = RectF{pos, pos + glm::vec2(width(), height())};
    switch (shape)
    {
    case Shape::Rectangle:
        painter->drawRect(rect, backgroundColor, depth);
        break;
    case Shape::Capsule:
        painter->drawCapsule(rect, backgroundColor, depth);
        break;
    case Shape::RoundedRectangle:
        painter->drawRoundedRect(rect, cornerRadius, backgroundColor, depth);
        break;
    default:
        break;
    }
}

void Item::setSize(Size size)
{
    if (size == m_size)
        return;
    m_size = size;
    resizedSignal.notify(m_size);
}

Item *Item::findGrabbableItem(const glm::vec2 &pos)
{
    return nullptr;
}

void Item::render(Painter *painter, const glm::vec2 &pos, int depth)
{
    const auto rect = RectF{pos, pos + glm::vec2(width(), height())};
    if (!painter->clipRect().intersects(rect))
        return;
    renderBackground(painter, pos, depth);
    renderContents(painter, pos, depth);
}

bool Item::mouseEvent(const MouseEvent &)
{
    return false;
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

void Rectangle::renderContents(Painter *, const glm::vec2 &, int) {}

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

bool Label::mouseEvent(const MouseEvent &event)
{
    if (event.type == MouseEvent::Type::Click)
        log("**** clicked label %s\n", std::string(m_text.begin(), m_text.end()).c_str());
    return Item::mouseEvent(event);
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

void Label::setFixedWidth(float width)
{
    if (width == m_fixedWidth)
        return;
    m_fixedWidth = width;
    updateSize();
}

void Label::setFixedHeight(float height)
{
    if (height == m_fixedHeight)
        return;
    m_fixedHeight = height;
    updateSize();
}

void Label::updateSize()
{
    m_contentHeight = m_font->pixelHeight();
    m_contentWidth = m_font->textWidth(m_text);
    const float height = [this] {
        if (m_fixedHeight > 0)
            return m_fixedHeight;
        return m_contentHeight + m_margins.top + m_margins.bottom;
    }();
    const float width = [this] {
        if (m_fixedWidth > 0)
            return m_fixedWidth;
        return m_contentWidth + m_margins.left + m_margins.right;
    }();
    setSize({width, height});
}

void Label::renderContents(Painter *painter, const glm::vec2 &pos, int depth)
{
    const auto availableWidth = m_size.width - (m_margins.left + m_margins.right);
    if (availableWidth < 0.0f)
        return;

    const auto availableHeight = m_size.height - (m_margins.top + m_margins.bottom);
    if (availableHeight < 0.0f)
        return;

    const bool clipped = availableWidth < m_contentWidth - 0.5f || availableHeight < m_contentHeight - 0.5f;
    RectF prevClipRect;
    if (clipped)
    {
        prevClipRect = painter->clipRect();
        const auto p = pos + glm::vec2(m_margins.left, m_margins.top);
        const auto rect = RectF{p, p + glm::vec2(availableWidth, availableHeight)};
        painter->setClipRect(prevClipRect.intersected(rect));
    }

    const auto xOffset = [this, availableWidth] {
        const auto horizAlignment = alignment & (Alignment::Left | Alignment::HCenter | Alignment::Right);
        switch (horizAlignment)
        {
        case Alignment::Left:
        default:
            return 0.0f;
        case Alignment::HCenter:
            return 0.5f * (availableWidth - m_contentWidth);
        case Alignment::Right:
            return availableWidth - m_contentWidth;
        }
    }();
    const auto yOffset = [this, availableHeight] {
        const auto vertAlignment = alignment & (Alignment::Top | Alignment::VCenter | Alignment::Bottom);
        switch (vertAlignment)
        {
        case Alignment::Top:
            return 0.0f;
        case Alignment::VCenter:
        default:
            return 0.5f * (availableHeight - m_contentHeight);
        case Alignment::Bottom:
            return availableHeight - m_contentHeight;
        }
    }();
    const auto textPos = pos + glm::vec2(m_margins.left, m_margins.top) + glm::vec2(xOffset, yOffset);
    painter->setFont(m_font);
    painter->drawText(m_text, textPos, color, depth + 1);

    if (clipped)
        painter->setClipRect(prevClipRect);
}

Image::Image() = default;

Image::Image(std::string_view source)
{
    setSource(source);
}

bool Image::mouseEvent(const MouseEvent &event)
{
    if (event.type == MouseEvent::Type::Click)
        log("**** clicked image\n");
    return Item::mouseEvent(event);
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

void Image::setFixedWidth(float width)
{
    if (width == m_fixedWidth)
        return;
    m_fixedWidth = width;
    updateSize();
}

void Image::setFixedHeight(float height)
{
    if (height == m_fixedHeight)
        return;
    m_fixedHeight = height;
    updateSize();
}

void Image::updateSize()
{
    const float height = [this] {
        if (m_fixedHeight > 0)
            return m_fixedHeight;
        float height = m_margins.top + m_margins.bottom;
        if (m_pixmap)
            height += m_pixmap->height;
        return height;
    }();
    const float width = [this] {
        if (m_fixedWidth > 0)
            return m_fixedWidth;
        float width = m_margins.left + m_margins.right;
        if (m_pixmap)
            width += m_pixmap->width;
        return width;
    }();
    setSize({width, height});
}

void Image::renderContents(Painter *painter, const glm::vec2 &pos, int depth)
{
    if (!m_pixmap)
        return;
    const auto availableWidth = m_size.width - (m_margins.left + m_margins.right);
    const auto availableHeight = m_size.height - (m_margins.top + m_margins.bottom);
    const bool clipped = availableWidth < m_pixmap->width - 0.5f || availableHeight < m_pixmap->height - 0.5f;

    const auto xOffset = [this, availableWidth] {
        const auto horizAlignment = alignment & (Alignment::Left | Alignment::HCenter | Alignment::Right);
        switch (horizAlignment)
        {
        case Alignment::Left:
        default:
            return 0.0f;
        case Alignment::HCenter:
            return 0.5f * (availableWidth - m_pixmap->width);
        case Alignment::Right:
            return availableWidth - m_pixmap->width;
        }
    }();
    const auto yOffset = [this, availableHeight] {
        const auto vertAlignment = alignment & (Alignment::Top | Alignment::VCenter | Alignment::Bottom);
        switch (vertAlignment)
        {
        case Alignment::Top:
            return 0.0f;
        case Alignment::VCenter:
        default:
            return 0.5f * (availableHeight - m_pixmap->height);
        case Alignment::Bottom:
            return availableHeight - m_pixmap->height;
        }
    }();
    const auto topLeft = pos + glm::vec2(m_margins.left, m_margins.top);
    const auto imagePos = topLeft + glm::vec2(xOffset, yOffset);
    const auto rect = RectF{imagePos, imagePos + glm::vec2(m_pixmap->width, m_pixmap->height)};
    if (!clipped)
    {
        painter->drawPixmap(*m_pixmap, rect, color, depth);
    }
    else
    {
        const auto clipRect = RectF{topLeft, topLeft + glm::vec2(availableWidth, availableHeight)};
        painter->drawPixmap(*m_pixmap, rect, clipRect, color, depth);
    }
}

void Container::update(float elapsed)
{
    for (auto &layoutItem : m_layoutItems)
        layoutItem->item->update(elapsed);
}

Item *Container::findGrabbableItem(const glm::vec2 &pos)
{
    if (!rect().contains(pos))
        return nullptr;
    for (auto &layoutItem : m_layoutItems)
    {
        const auto &item = layoutItem->item;
        const auto &offset = layoutItem->offset;
        const auto childRect = RectF{layoutItem->offset, layoutItem->offset + glm::vec2(item->width(), item->height())};
        if (childRect.contains(pos))
            return item->findGrabbableItem(pos - offset);
    }
    return nullptr;
}

bool Container::mouseEvent(const MouseEvent &event)
{
    const auto &pos = event.position;
    if (!rect().contains(pos))
        return false;
    for (auto &layoutItem : m_layoutItems)
    {
        const auto &item = layoutItem->item;
        const auto &offset = layoutItem->offset;
        const auto childRect = RectF{layoutItem->offset, layoutItem->offset + glm::vec2(item->width(), item->height())};
        if (childRect.contains(pos))
        {
            MouseEvent childEvent = event;
            childEvent.position -= offset;
            if (item->mouseEvent(childEvent))
                return true;
        }
    }
    return false;
}

void Container::addItem(std::unique_ptr<Item> item)
{
    auto resizedConnection = item->resizedSignal.connect([this](Size size) {
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

void Container::renderContents(Painter *painter, const glm::vec2 &pos, int depth)
{
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
            const auto availableWidth = m_size.width - (m_margins.left + m_margins.right);
            switch (alignment)
            {
            case Alignment::Left:
            default:
                return 0.0f;
            case Alignment::HCenter:
                return 0.5f * (availableWidth - item->width());
            case Alignment::Right:
                return availableWidth - item->width();
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
            const auto availableHeight = m_size.height - (m_margins.top + m_margins.bottom);
            const auto alignment = item->containerAlignment & (Alignment::Top | Alignment::VCenter | Alignment::Bottom);
            switch (alignment)
            {
            case Alignment::Top:
                return 0.0f;
            case Alignment::VCenter:
            default:
                return 0.5f * (availableHeight - item->height());
            case Alignment::Bottom:
                return availableHeight - item->height();
            }
        }();
        layoutItem->offset = p + glm::vec2(0.0f, offset);
        p.x += layoutItem->item->width() + m_spacing;
    }
}

ScrollArea::ScrollArea(float viewportWidth, float viewportHeight, std::unique_ptr<Item> contentItem)
    : m_contentItem(std::move(contentItem))
{
    updateSize();
}

ScrollArea::ScrollArea(std::unique_ptr<Item> contentItem)
    : ScrollArea(0, 0, std::move(contentItem))
{
}

void ScrollArea::update(float elapsed)
{
    m_contentItem->update(elapsed);
}

void ScrollArea::renderContents(Painter *painter, const glm::vec2 &pos, int depth)
{
    const auto viewportPos = pos + glm::vec2(m_margins.left, m_margins.top);
    const auto prevClipRect = painter->clipRect();
    const auto viewportRect = RectF{viewportPos, viewportPos + glm::vec2(m_viewportSize.width, m_viewportSize.height)};
    painter->setClipRect(prevClipRect.intersected(viewportRect));
    m_contentItem->render(painter, viewportPos + m_viewportOffset, depth + 1);
    painter->setClipRect(prevClipRect);
}

bool ScrollArea::mouseEvent(const MouseEvent &event)
{
    switch (event.type)
    {
    case MouseEvent::Type::DragBegin:
        m_dragging = true;
        m_mousePressPos = event.position;
        return true;
    case MouseEvent::Type::DragEnd:
        m_dragging = false;
        return true;
    case MouseEvent::Type::DragMove:
        if (m_dragging)
        {
            const auto offset = event.position - m_mousePressPos;
            m_viewportOffset += offset;
            m_viewportOffset = glm::max(m_viewportOffset, glm::vec2(m_viewportSize.width - m_contentItem->width(),
                                                                    m_viewportSize.height - m_contentItem->height()));
            m_viewportOffset = glm::min(m_viewportOffset, glm::vec2(0, 0));
            m_mousePressPos = event.position;
        }
        return true;
    default: {
        MouseEvent childEvent = event;
        childEvent.position -= m_viewportOffset;
        return m_contentItem->mouseEvent(childEvent);
    }
    }
}

Item *ScrollArea::findGrabbableItem(const glm::vec2 &pos)
{
    if (!rect().contains(pos))
        return nullptr;
    if (auto *childItem = m_contentItem->findGrabbableItem(pos - m_viewportOffset))
        return childItem;
    return this;
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

Switch::Switch()
{
    m_size = Size{80, 32};

    fillBackground = true;
    shape = Shape::Capsule;
    backgroundColor = glm::vec4(0, 0, 0, 1);

    m_animationConn = m_animation.valueChangedSignal.connect([this](float value) { m_indicatorPosition = value; });
    m_animation.duration = 0.2f;
}

bool Switch::mouseEvent(const MouseEvent &event)
{
    if (event.type == MouseEvent::Type::Click)
    {
        toggle();
        return true;
    }
    return Item::mouseEvent(event);
}

void Switch::toggle()
{
    setChecked(!m_checked);
}

void Switch::setChecked(bool checked)
{
    if (checked == m_checked)
        return;
    m_checked = checked;

    if (m_checked)
    {
        m_animation.startValue = 0.0f;
        m_animation.endValue = 1.0f;
    }
    else
    {
        m_animation.startValue = 1.0f;
        m_animation.endValue = 0.0f;
    }
    m_animation.start();
    toggledSignal.notify(checked);
}

void Switch::update(float elapsed)
{
    m_animation.update(elapsed);
}

void Switch::renderContents(Painter *painter, const glm::vec2 &pos, int depth)
{
    const float radius = 0.5f * m_size.height;
    const float indicatorRadius = 0.75f * radius;
    const float centerX = radius + m_indicatorPosition * (m_size.width - 2 * radius);
    const auto center = pos + glm::vec2(centerX, 0.5f * m_size.height);
    painter->drawCircle(center, indicatorRadius, indicatorColor, depth + 1);
}

MultiLineText::MultiLineText(std::u32string_view text)
    : MultiLineText(defaultFont(), text)
{
}

MultiLineText::MultiLineText(Font *font, std::u32string_view text)
    : m_font(font)
    , m_text(text)
{
    updateSize();
}

void MultiLineText::setFont(Font *font)
{
    if (font == m_font)
        return;
    m_font = font;
    updateSize();
}

void MultiLineText::setText(std::u32string_view text)
{
    if (text == m_text)
        return;
    m_text = text;
    updateSize();
}

void MultiLineText::setMargins(Margins margins)
{
    if (margins == m_margins)
        return;
    m_margins = margins;
    updateSize();
}

void MultiLineText::setFixedWidth(float width)
{
    if (width == m_fixedWidth)
        return;
    m_fixedWidth = width;
    updateSize();
}

void MultiLineText::setFixedHeight(float height)
{
    if (height == m_fixedHeight)
        return;
    m_fixedHeight = height;
    updateSize();
}

void MultiLineText::updateSize()
{
    breakTextLines();
    m_contentWidth = 0.0f;
    for (const auto &line : m_lines)
        m_contentWidth = std::max(line.width, m_contentWidth);
    m_contentHeight = m_lines.size() * m_font->pixelHeight();
    const float height = [this] {
        if (m_fixedHeight > 0)
            return m_fixedHeight;
        return m_contentHeight + m_margins.top + m_margins.bottom;
    }();
    setSize({m_fixedWidth, height});
}

void MultiLineText::renderContents(Painter *painter, const glm::vec2 &pos, int depth)
{
    const auto availableWidth = m_size.width - (m_margins.left + m_margins.right);
    if (availableWidth < 0.0f)
        return;

    const auto availableHeight = m_size.height - (m_margins.top + m_margins.bottom);
    if (availableHeight < 0.0f)
        return;

    const bool clipped = availableWidth < m_contentWidth - 0.5f || availableHeight < m_contentHeight - 0.5f;
    RectF prevClipRect;
    if (clipped)
    {
        prevClipRect = painter->clipRect();
        const auto p = pos + glm::vec2(m_margins.left, m_margins.top);
        const auto rect = RectF{p, p + glm::vec2(availableWidth, availableHeight)};
        painter->setClipRect(prevClipRect.intersected(rect));
    }

    const auto yOffset = [this, availableHeight] {
        const auto vertAlignment = alignment & (Alignment::Top | Alignment::VCenter | Alignment::Bottom);
        switch (vertAlignment)
        {
        case Alignment::Top:
            return 0.0f;
        case Alignment::VCenter:
        default:
            return 0.5f * (availableHeight - m_contentHeight);
        case Alignment::Bottom:
            return availableHeight - m_contentHeight;
        }
    }();
    auto textPos = pos + glm::vec2(m_margins.left, m_margins.top) + glm::vec2(0.0f, yOffset);
    painter->setFont(m_font);
    for (const auto &line : m_lines)
    {
        const auto offset = [this, &line, availableWidth] {
            const auto horizAlignment = alignment & (Alignment::Left | Alignment::HCenter | Alignment::Right);
            switch (horizAlignment)
            {
            case Alignment::Left:
            default:
                return 0.0f;
            case Alignment::HCenter:
                return 0.5f * (availableWidth - line.width);
            case Alignment::Right:
                return availableWidth - line.width;
            }
        }();
        painter->drawText(line.text, textPos + glm::vec2(offset, 0), color, depth + 1);
        textPos.y += m_font->pixelHeight();
    }

    if (clipped)
        painter->setClipRect(prevClipRect);
}

void MultiLineText::breakTextLines()
{
    assert(m_font);

    m_lines.clear();

    const float availableWidth = m_fixedWidth - (m_margins.left + m_margins.right);
    if (availableWidth < 0.0f)
        return;

    struct Position
    {
        std::u32string::const_iterator it;
        float x;
    };
    Position rowStart{m_text.begin(), 0.0f};
    std::optional<Position> lastBreak;

    const auto spaceWidth = m_font->glyph(' ')->advanceWidth;

    const auto makeLine = [](Position start, Position end) {
        return TextLine{std::u32string_view(start.it, end.it), end.x - start.x};
    };

    float lineWidth = 0.0f;
    for (auto it = m_text.begin(); it != m_text.end(); ++it)
    {
        const auto ch = *it;
        if (ch == ' ')
        {
            if (lineWidth - rowStart.x > availableWidth)
            {
                if (lastBreak)
                {
                    m_lines.push_back(makeLine(rowStart, *lastBreak));
                    rowStart = {lastBreak->it + 1, lastBreak->x + spaceWidth};
                    lastBreak = {it, lineWidth};
                }
                else
                {
                    m_lines.push_back(makeLine(rowStart, Position{it, lineWidth}));
                    rowStart = {it + 1, lineWidth + spaceWidth};
                }
            }
            else
            {
                lastBreak = {it, lineWidth};
            }
        }
        lineWidth += m_font->glyph(ch)->advanceWidth;
    }
    if (rowStart.it != m_text.end())
    {
        if (lineWidth - rowStart.x > availableWidth && lastBreak)
        {
            m_lines.push_back(makeLine(rowStart, *lastBreak));
            m_lines.push_back(
                makeLine(Position{lastBreak->it + 1, lastBreak->x + spaceWidth}, Position{m_text.end(), lineWidth}));
        }
        else
        {
            m_lines.push_back(makeLine(rowStart, Position{m_text.end(), lineWidth}));
        }
    }
}

} // namespace miniui
