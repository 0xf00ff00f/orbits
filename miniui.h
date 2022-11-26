#pragma once

#include "textureatlas.h"
#include "font.h"

#include <glm/glm.hpp>

#include <string_view>
#include <string>
#include <vector>
#include <memory>

class Pixmap;

namespace miniui
{

struct Margins
{
    float top = 0;
    float bottom = 0;
    float left = 0;
    float right = 0;
};

class Item
{
public:
    virtual ~Item();

    virtual float width() = 0;
    virtual float height() = 0;
    virtual void render(const glm::vec2 &pos, int depth = 0) = 0;

    bool fillBackground = false;
    glm::vec4 bgColor;

protected:
    void renderBackground(const glm::vec2 &pos, int depth);
};

class Label : public Item
{
public:
    explicit Label(std::u32string_view text = {});
    Label(const Font &font, std::u32string_view text);

    float width() override { return m_width; }
    float height() override { return m_height; }

    void setFont(const Font &font);
    Font font() const;

    void setText(std::u32string_view text);
    const std::u32string &text() const { return m_text; }

    void setMargins(Margins margins);
    Margins margins() const { return m_margins; }

    void render(const glm::vec2 &pos, int depth = 0) override;

    glm::vec4 color = glm::vec4(0, 0, 0, 1);

private:
    void updateSize();

    Margins m_margins;
    Font m_font;
    std::u32string m_text;
    float m_width = 0;
    float m_height = 0;
};

class Image : public Item
{
public:
    explicit Image(const Pixmap &pixmap);

    float width() override;
    float height() override;
    void render(const glm::vec2 &pos, int depth = 0) override;

    Margins margins;

private:
    void updateSize();

    PackedPixmap m_pixmap;
    float m_width = 0;
    float m_height = 0;
};

class Container : public Item
{
public:
    void addItem(std::unique_ptr<Item> item);

    void setMargins(Margins margins);
    Margins margins() const { return m_margins; }

    void setSpacing(float spacing);
    float spacing() const { return m_spacing; }

protected:
    virtual void updateLayout() = 0;

    std::vector<std::unique_ptr<Item>> m_items;
    Margins m_margins;
    float m_spacing = 0.0f;
};

class Column : public Container
{
public:
    float width() override { return m_width; }
    float height() override { return m_height; }
    void render(const glm::vec2 &pos, int depth = 0) override;

private:
    void updateLayout() override;

    float m_width = 0;
    float m_height = 0;
};

class Row : public Container
{
public:
    float width() override { return m_width; }
    float height() override { return m_height; }
    void render(const glm::vec2 &pos, int depth = 0) override;

private:
    void updateLayout() override;

    float m_width = 0;
    float m_height = 0;
};

}
