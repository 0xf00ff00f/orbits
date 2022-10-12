#pragma once

#include "noncopyable.h"
#include "mesh.h"

#include <memory>

class ShaderProgram;

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

    std::unique_ptr<ShaderProgram> m_program;
    struct Vertex
    {
        glm::vec4 position;
        glm::vec4 color;
    };
    std::unique_ptr<Mesh<Vertex>> m_mesh;
};
