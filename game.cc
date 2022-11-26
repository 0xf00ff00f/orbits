#include "game.h"

#include "shadermanager.h"
#include "ui.h"
#include "log.h"

#include <GL/glew.h>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

Game::Game()
    : m_mesh(std::make_unique<gl::Mesh<Vertex>>())
    , m_shaderManager(std::make_unique<ShaderManager>())
    , m_ui(std::make_unique<UI>(m_shaderManager.get()))
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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const auto mvp = glm::ortho(0.0f, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f);

    m_shaderManager->useProgram(ShaderManager::Flat);
    m_shaderManager->setUniform(ShaderManager::Uniform::ModelViewProjection, mvp);
    m_mesh->render(GL_LINE_LOOP);

    m_ui->setTransformMatrix(mvp);
    m_ui->begin();
    if (m_ui->beginWindow("The quick brown fox jumps over the lazy dog", mu_Rect{50, 50, 400, 400}))
    {
        m_ui->endWindow();
    }
    m_ui->end();
}

void Game::update(float /* elapsed */) {}

void Game::initialize()
{
    // initialize mesh
    {
        constexpr auto VertexCount = 60;
        std::array<Vertex, VertexCount> vertices;
        for (int i = 0; i < VertexCount; ++i)
        {
            const auto t = static_cast<float>(i) / VertexCount;
            const auto a = t * (2.0f * glm::pi<float>());
            auto &v = vertices[i];
            v.position = 150.0f * glm::vec2(std::cos(a), std::sin(a));
            v.color = glm::vec4(t);
        }
        m_mesh->setData(vertices);
    }
}
