#include "game.h"

#include "shaderprogram.h"
#include "log.h"

#include <GL/glew.h>

#include <glm/gtc/constants.hpp>

Game::Game()
    : m_program(std::make_unique<ShaderProgram>())
    , m_mesh(std::make_unique<Mesh<Vertex>>())
{
    initialize();
}

Game::~Game() = default;

void Game::resize(int width, int height)
{
    m_width = width;
    m_height = height;
}

void Game::render()
{
    glClearColor(0, 0.5, 1, 1);
    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_program->bind();
    m_mesh->render(GL_LINE_LOOP);
}

void Game::update(float /* elapsed */) {}

void Game::initialize()
{
    // initialize shader program
    {
        static const std::string vs = R"(#version 100

attribute vec4 position;
attribute vec4 color;

uniform mat4 mvp;

varying vec4 vs_color;

void main(void)
{
    vs_color = color;
    gl_Position = position;
})";
        static const std::string fs = R"(#version 100

precision highp float;

varying vec4 vs_color;

void main(void)
{
    gl_FragColor = vs_color;
})";

        if (!m_program->addShaderSource(GL_VERTEX_SHADER, vs))
        {
            log("Failed to add vertex shader: %s\n", m_program->log().c_str());
        }
        if (!m_program->addShaderSource(GL_FRAGMENT_SHADER, fs))
        {
            log("Failed to add fragment shader: %s\n", m_program->log().c_str());
        }
        m_program->bindAttributeLocation(0, "position");
        m_program->bindAttributeLocation(1, "color");
        if (!m_program->link())
        {
            log("Failed to link program: %s\n", m_program->log().c_str());
        }
        assert(m_program->attributeLocation("position") == 0);
        assert(m_program->attributeLocation("color") == 1);
    }

    // initialize mesh
    {
        constexpr auto VertexCount = 60;
        std::array<Vertex, VertexCount> vertices;
        for (int i = 0; i < VertexCount; ++i)
        {
            const auto t = static_cast<float>(i) / VertexCount;
            const auto a = t * (2.0f * glm::pi<float>());
            const auto p = 0.5f * glm::vec2(std::cos(a), std::sin(a));
            auto &v = vertices[i];
            v.position = glm::vec4(p, 0.0f, 1.0f);
            v.color = glm::vec4(t);
        }
        m_mesh->setData(vertices);
    }
}
