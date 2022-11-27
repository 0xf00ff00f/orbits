#pragma once

#include "noncopyable.h"
#include "mesh.h"
#include "mouseevent.h"

#include <memory>

namespace miniui
{
class Item;
class Label;
}

class Game : private NonCopyable
{
public:
    explicit Game();
    ~Game();

    void resize(int width, int height);
    void render();
    void update(float elapsed);

    void onMouseButtonPress(miniui::MouseButtons button);
    void onMouseButtonRelease(miniui::MouseButtons button);
    void onMouseMove(const glm::vec2 &pos);

private:
    void initialize();
    void mouseEvent(const miniui::MouseEvent &mouseEvent);

    int m_width = 0;
    int m_height = 0;
    miniui::MouseButtons m_mouseButtons = miniui::MouseButtons::None;
    glm::vec2 m_mousePosition;

    struct Vertex
    {
        glm::vec2 position;
        glm::vec4 color;
    };
    std::unique_ptr<gl::Mesh<Vertex>> m_mesh;
    std::unique_ptr<miniui::Item> m_item;
    glm::vec2 m_itemOffset = glm::vec2(20, 20);
    miniui::Label *m_counterLabel;
    float m_time = 0.0f;
};
