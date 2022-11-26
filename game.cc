#include "game.h"

#include "shadermanager.h"
#include "miniui.h"
#include "painter.h"
#include "log.h"
#include "system.h"

#include <GL/glew.h>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

Game::Game()
    : m_mesh(std::make_unique<gl::Mesh<Vertex>>())
    , m_item(std::make_unique<miniui::Column>())
{
    using namespace std::literals;

    auto *container = static_cast<miniui::Container *>(m_item.get());
    container->fillBackground = true;
    container->bgColor = glm::vec4(1, 0, 0, 0.5);
    container->setMargins({10, 10, 10, 10});
    container->setSpacing(5);

    auto addLabel = [](miniui::Container *container, std::u32string_view text, int fontSize) {
        auto label = std::make_unique<miniui::Label>(text);
        label->fillBackground = true;
        label->bgColor = glm::vec4(0, 1, 0, 0.5);
        label->setMargins({10, 10, 10, 10});
        label->setFont(miniui::Font("OpenSans_Regular", fontSize));
        container->addItem(std::move(label));
    };
    addLabel(container, U"The quick brown fox"sv, 44);
    addLabel(container, U"Lorem ipsum"sv, 44);

    auto row = std::make_unique<miniui::Row>();
    row->fillBackground = true;
    row->bgColor = glm::vec4(1, 1, 0, 0.5);
    row->setMargins({20, 20, 20, 20});
    row->setSpacing(20);
    addLabel(row.get(), U"Here"sv, 30);
    addLabel(row.get(), U"is"sv, 40);
    addLabel(row.get(), U"some"sv, 50);
    addLabel(row.get(), U"text"sv, 60);
    container->addItem(std::move(row));

    addLabel(container, U"Sphinx of black quartz"sv, 44);

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

    auto &system = System::instance();

    auto *shaderManager = system.shaderManager();
    shaderManager->useProgram(ShaderManager::Flat);
    shaderManager->setUniform(ShaderManager::Uniform::ModelViewProjection, mvp);
    m_mesh->render(GL_LINE_LOOP);

    auto *painter = system.uiPainter();
    painter->setTransformMatrix(mvp);
    painter->begin();
    m_item->render(glm::vec2(20, 20));
    painter->end();
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
