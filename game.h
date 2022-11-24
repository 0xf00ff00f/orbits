#pragma once

#include "noncopyable.h"
#include "mesh.h"

#include <memory>

class ShaderManager;
class UI;

class Game : private NonCopyable
{
public:
    explicit Game();
    ~Game();

    void resize(int width, int height);
    void render();
    void update(float elapsed);

private:
    void initialize();

    int m_width = 0;
    int m_height = 0;

    struct Vertex
    {
        glm::vec2 position;
        glm::vec4 color;
    };
    std::unique_ptr<Mesh<Vertex>> m_mesh;
    std::unique_ptr<ShaderManager> m_shaderManager;
    std::unique_ptr<UI> m_ui;
};
