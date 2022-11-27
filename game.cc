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

    auto makeLabel = [](std::u32string_view text, int fontSize) {
        auto label = std::make_unique<miniui::Label>(text);
        label->fillBackground = true;
        label->bgColor = glm::vec4(0, 1, 0, 0.5);
        label->setMargins({10, 10, 10, 10});
        label->setFont(miniui::Font("OpenSans_Regular", fontSize));
        return label;
    };
    container->addItem(makeLabel(U"The quick brown fox"sv, 44));
    container->addItem(makeLabel(U"Lorem ipsum"sv, 44));

    {
        auto rect = std::make_unique<miniui::Rectangle>();
        rect->setSize(200, 20);
        rect->fillBackground = true;
        rect->bgColor = glm::vec4(1, 0, 0, 1);
        rect->alignment = miniui::Align::HCenter;
        container->addItem(std::move(rect));
    }

    {
        auto row = std::make_unique<miniui::Row>();
        row->fillBackground = true;
        row->bgColor = glm::vec4(1, 1, 0, 0.5);
        row->alignment = miniui::Align::Right;
        row->setMargins({20, 20, 20, 20});
        row->setSpacing(20);
        row->setMinimumHeight(200);
        row->addItem(makeLabel(U"Here"sv, 30));
        row->addItem(makeLabel(U"is"sv, 40));
        row->addItem(makeLabel(U"some"sv, 50));
        row->addItem(makeLabel(U"text"sv, 60));
        container->addItem(std::move(row));
    }

    {
        auto row = std::make_unique<miniui::Row>();
        row->setSpacing(30);

        row->addItem(std::make_unique<miniui::Image>("peppers.jpg"));

        auto l0 = makeLabel(U"Hello"sv, 60);
        l0->alignment = miniui::Align::Bottom;
        row->addItem(std::move(l0));

        auto l1 = makeLabel(U"Hello"sv, 60);
        l1->alignment = miniui::Align::VCenter;
        row->addItem(std::move(l1));

        auto l2 = makeLabel(U"Hello"sv, 60);
        l2->alignment = miniui::Align::Top;
        row->addItem(std::move(l2));

        auto l3 = makeLabel(U""sv, 120);
        l3->alignment = miniui::Align::Bottom;
        m_counterLabel = l3.get();
        row->addItem(std::move(l3));

        container->addItem(std::move(row));
    }

    {
        auto l = makeLabel(U"Sphinx of black quartz"sv, 44);
        l->alignment = miniui::Align::Right;
        container->addItem(std::move(l));
    }

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
    m_item->render(m_itemOffset);
    painter->end();
}

void Game::update(float elapsed)
{
    m_time += elapsed;
    auto text = std::to_string(static_cast<int>(m_time * 10.0f));
    m_counterLabel->setText(std::u32string(text.begin(), text.end()));
}

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

void Game::onMouseButtonPress(miniui::MouseButtons button)
{
    m_mouseButtons |= button;
    using namespace miniui;
    mouseEvent(MouseEvent{MouseEvent::Type::Press, button, m_mousePosition});
}

void Game::onMouseButtonRelease(miniui::MouseButtons button)
{
    m_mouseButtons &= ~button;
    using namespace miniui;
    mouseEvent(MouseEvent{MouseEvent::Type::Release, button, m_mousePosition});
}

void Game::onMouseMove(const glm::vec2 &pos)
{
    m_mousePosition = pos;
    using namespace miniui;
    mouseEvent(MouseEvent{MouseEvent::Type::Move, m_mouseButtons, m_mousePosition});
}

void Game::mouseEvent(const miniui::MouseEvent &event)
{
    const auto &p = event.position;
    if (p.x >= m_itemOffset.x && p.x < m_itemOffset.x + m_item->width() && p.y >= m_itemOffset.y &&
        p.y < m_itemOffset.y + m_item->height())
    {
        miniui::MouseEvent itemEvent = event;
        itemEvent.position -= m_itemOffset;
        m_item->mouseEvent(itemEvent);
    }
}
