#pragma once

#include "textureatlas.h"
#include "font.h"
#include "event.h"
#include "mouseevent.h"

#include <glm/glm.hpp>

#include <string_view>
#include <string>
#include <vector>
#include <memory>
#include <functional>

class Pixmap;

namespace miniui
{
class Painter;

enum class Alignment : unsigned
{
    None = 0,
    Left = 1 << 0,
    HCenter = 1 << 1,
    Right = 1 << 2,
    Top = 1 << 3,
    VCenter = 1 << 4,
    Bottom = 1 << 5
};

constexpr Alignment operator&(Alignment x, Alignment y)
{
    using UT = typename std::underlying_type_t<Alignment>;
    return static_cast<Alignment>(static_cast<UT>(x) & static_cast<UT>(y));
}

constexpr Alignment &operator&=(Alignment &x, Alignment y)
{
    return x = x & y;
}

constexpr Alignment operator|(Alignment x, Alignment y)
{
    using UT = typename std::underlying_type_t<Alignment>;
    return static_cast<Alignment>(static_cast<UT>(x) | static_cast<UT>(y));
}

constexpr Alignment &operator|=(Alignment &x, Alignment y)
{
    return x = x | y;
}

struct Margins
{
    float top = 0;
    float bottom = 0;
    float left = 0;
    float right = 0;
    bool operator==(const Margins &other) const
    {
        return top == other.top && bottom == other.bottom && left == other.left && right == other.right;
    }
};

struct Size
{
    float width = 0;
    float height = 0;
    bool operator==(const Size &other) const { return width == other.width && height == other.height; }
};

class Item
{
public:
    using ResizedEvent = Event<std::function<void(Size)>>;

    virtual ~Item();

    Size size() const { return m_size; }
    float width() const { return m_size.width; }
    float height() const { return m_size.height; }

    void render(Painter *painter, const glm::vec2 &pos, int depth = 0);

    virtual void mouseEvent(const MouseEvent &event) = 0;
    virtual Item *findItem(const glm::vec2 &pos);

    ResizedEvent &resizedEvent() { return m_resizedEvent; }

    enum class Shape
    {
        Rectangle,
        RoundedRectangle
    };
    Shape shape = Shape::Rectangle;
    bool fillBackground = false;
    glm::vec4 backgroundColor;
    float cornerRadius = 0.0f;
    Alignment containerAlignment = Alignment::VCenter | Alignment::Left;

protected:
    void setSize(Size size);
    void renderBackground(Painter *painter, const glm::vec2 &pos, int depth);
    virtual void renderContents(Painter *painter, const glm::vec2 &pos, int depth = 0) = 0;

    Size m_size;
    ResizedEvent m_resizedEvent;
};

class Rectangle : public Item
{
public:
    Rectangle();
    Rectangle(Size size);
    Rectangle(float width, float height);

    void mouseEvent(const MouseEvent &event) override;

    using Item::setSize;
    void setSize(float width, float height);
    void setWidth(float width);
    void setHeight(float height);

protected:
    void renderContents(Painter *painter, const glm::vec2 &pos, int depth = 0) override;
};

class Label : public Item
{
public:
    explicit Label(std::u32string_view text = {});
    Label(Font *font, std::u32string_view text);

    void mouseEvent(const MouseEvent &event) override;

    void setFont(Font *font);
    Font *font() const;

    void setText(std::u32string_view text);
    const std::u32string &text() const { return m_text; }

    void setMargins(Margins margins);
    Margins margins() const { return m_margins; }

    void setFixedWidth(float width);
    float fixedWidth() const { return m_fixedWidth; }

    void setFixedHeight(float height);
    float fixedHeight() const { return m_fixedHeight; }

    glm::vec4 color = glm::vec4(0, 0, 0, 1);
    Alignment alignment = Alignment::VCenter | Alignment::Left;

protected:
    void renderContents(Painter *painter, const glm::vec2 &pos, int depth = 0) override;

private:
    void updateSize();

    Font *m_font;
    std::u32string m_text;
    Margins m_margins;
    float m_contentWidth = 0;
    float m_contentHeight = 0;
    float m_fixedWidth = -1;  // ignored if < 0
    float m_fixedHeight = -1; // ignored if < 0
};

class Image : public Item
{
public:
    Image();
    explicit Image(std::string_view source);

    void mouseEvent(const MouseEvent &event) override;

    void setSource(std::string_view source);
    const std::string &source() const { return m_source; }

    void setMargins(Margins margins);
    Margins margins() const { return m_margins; }

    glm::vec4 color = glm::vec4(1, 1, 1, 1);

protected:
    void renderContents(Painter *painter, const glm::vec2 &pos, int depth = 0) override;

private:
    void updateSize();

    std::string m_source;
    std::optional<PackedPixmap> m_pixmap;
    Margins m_margins;
};

class Container : public Item
{
public:
    void mouseEvent(const MouseEvent &event) override;
    Item *findItem(const glm::vec2 &pos) override;

    void addItem(std::unique_ptr<Item> item);

    void setMargins(Margins margins);
    Margins margins() const { return m_margins; }

    void setSpacing(float spacing);
    float spacing() const { return m_spacing; }

protected:
    virtual void updateLayout() = 0;

    void renderContents(Painter *painter, const glm::vec2 &pos, int depth = 0) override;

    struct LayoutItem
    {
        glm::vec2 offset;
        std::unique_ptr<Item> item;
    };
    std::vector<std::unique_ptr<LayoutItem>> m_layoutItems;
    Margins m_margins;
    float m_spacing = 0.0f;

private:
    std::vector<std::unique_ptr<Connection>> m_childResizedConnections;
};

class Column : public Container
{
public:
    void setMinimumWidth(float width);
    float minimumWidth() const { return m_minimumWidth; }

private:
    void updateLayout() override;

    float m_minimumWidth = 0;
};

class Row : public Container
{
public:
    void setMinimumHeight(float height);
    float minimumHeight() const { return m_minimumHeight; }

private:
    void updateLayout() override;

    float m_minimumHeight = 0;
};

class ScrollArea : public Item
{
public:
    explicit ScrollArea(std::unique_ptr<Item> viewportClient);
    ScrollArea(float viewportWidth, float viewportHeight, std::unique_ptr<Item> viewportClient);

    void mouseEvent(const MouseEvent &event) override;

    void setMargins(Margins margins);
    Margins margins() const { return m_margins; }

    void setViewportSize(Size size);
    Size viewportSize() const;

protected:
    void renderContents(Painter *painter, const glm::vec2 &pos, int depth = 0) override;

private:
    void updateSize();

    std::unique_ptr<Item> m_viewportClient;
    Margins m_margins;
    Size m_viewportSize;
    glm::vec2 m_viewportOffset = glm::vec2(0, 0);
    bool m_dragging = false;
    glm::vec2 m_mousePressPos;
};
} // namespace miniui
