#include "game.h"

#include "shadermanager.h"
#include "miniui.h"
#include "painter.h"
#include "log.h"
#include "system.h"
#include "fontcache.h"

#include <GL/glew.h>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <string>

Game::Game()
    : m_mesh(std::make_unique<gl::Mesh<Vertex>>())
    , m_item(std::make_unique<miniui::Column>())
{
    using namespace std::literals;
    using namespace miniui;

    auto *fontCache = System::instance()->fontCache();

    auto *titleFont = fontCache->font("OpenSans_Regular", 40);
    auto *smallFont = fontCache->font("OpenSans_Regular", 32);
    auto *tinyFont = fontCache->font("OpenSans_Regular", 20);

    auto *container = static_cast<Container *>(m_item.get());
    container->fillBackground = true;
    container->backgroundColor = glm::vec4(1, 0, 0, 0.5);
    container->shape = Item::Shape::RoundedRectangle;
    container->cornerRadius = 12;
    container->setMargins({10, 10, 10, 10});
    container->setSpacing(5);

    auto makeLabel = [](std::u32string_view text, Font *font) {
        auto l = std::make_unique<Label>(text);
        l->color = glm::vec4(1, 1, 1, 1);
        l->setFont(font);
        return l;
    };

    {
        auto p = std::make_unique<Image>("peppers.jpg");
        p->setFixedHeight(100);
        p->setFixedWidth(400);
        p->alignment = Alignment::Right | Alignment::Bottom;
        container->addItem(std::move(p));
    }

    {
        auto l = makeLabel(U"EASY MODE"sv, titleFont);
        l->containerAlignment = Alignment::HCenter;
        container->addItem(std::move(l));
    }

    // title row
    {
        auto row = std::make_unique<Row>();

        {
            auto r = std::make_unique<Rectangle>();
            r->setSize(61, 1);
            row->addItem(std::move(r));
        }

        {
            auto l = makeLabel(U"NAME"sv, smallFont);
            l->alignment = Alignment::Left;
            l->setFixedWidth(200);
            row->addItem(std::move(l));
        }

        {
            auto r = std::make_unique<Rectangle>();
            r->fillBackground = true;
            r->backgroundColor = glm::vec4(1, 1, 1, 0.5);
            r->setSize(1, 20);
            row->addItem(std::move(r));
        }

        {
            auto l = makeLabel(U"SCORE"sv, smallFont);
            l->alignment = Alignment::HCenter;
            l->setFixedWidth(200);
            row->addItem(std::move(l));
        }

        {
            auto r = std::make_unique<Rectangle>();
            r->fillBackground = true;
            r->backgroundColor = glm::vec4(1, 1, 1, 0.5);
            r->setSize(1, 20);
            row->addItem(std::move(r));
        }

        {
            auto l = makeLabel(U"ACCURACY"sv, smallFont);
            l->alignment = Alignment::Right;
            l->setFixedWidth(200);
            l->color = glm::vec4(1, 1, 1, 1);
            row->addItem(std::move(l));
        }

        {
            auto r = std::make_unique<Rectangle>();
            r->setSize(10, 1);
            row->addItem(std::move(r));
        }

        container->addItem(std::move(row));
    }

    {
        auto entryColumn = std::make_unique<Column>();
        entryColumn->setSpacing(5);

        struct Entry
        {
            std::u32string name;
            std::u32string score;
            std::u32string accuracy;
        };
        std::vector<Entry> entries = {
            {U"Erwin A.", U"123,123,123", U"99.51%"},    {U"Mary B.", U"122,737,212", U"97.53%"},
            {U"John C.", U"23,123,423", U"96.27%"},      {U"Alice D.", U"123,123,123", U"99.51%"},
            {U"Bob E.", U"122,737,212", U"97.53%"},      {U"Cristina F.", U"23,123,423", U"96.27%"},
            {U"Sophie G.", U"23,123,423", U"96.27%"},    {U"Jim H.", U"23,123,423", U"96.27%"},
            {U"Francisco I.", U"23,123,423", U"96.27%"},
        };
        for (size_t i = 0; i < entries.size(); ++i)
        {
            auto row = std::make_unique<Row>();
            row->fillBackground = true;
            row->shape = Item::Shape::RoundedRectangle;
            row->cornerRadius = 8;
            row->backgroundColor = glm::vec4(1, 1, 1, 0.25);
            row->setMargins({10, 10, 10, 10});
            row->setSpacing(1);

            auto indexText = std::to_string(i + 1) + std::string(".");
            auto index = makeLabel(std::u32string(indexText.begin(), indexText.end()), smallFont);
            index->setFixedWidth(50);
            row->addItem(std::move(index));

            {
                auto c = std::make_unique<Column>();

                auto name = makeLabel(entries[i].name, smallFont);
                name->setFixedWidth(200);
                c->addItem(std::move(name));

                auto type = makeLabel(U"Lorem ipsum"sv, tinyFont);
                c->addItem(std::move(type));

                row->addItem(std::move(c));
            }

            auto score = makeLabel(entries[i].score, smallFont);
            score->setFixedWidth(200);
            score->alignment = Alignment::HCenter;
            row->addItem(std::move(score));

            auto accuracy = makeLabel(entries[i].accuracy, smallFont);
            accuracy->setFixedWidth(200);
            accuracy->alignment = Alignment::Right;
            row->addItem(std::move(accuracy));

            entryColumn->addItem(std::move(row));
        }

        const auto columnWidth = entryColumn->width();

        auto scrollArea = std::make_unique<ScrollArea>(std::move(entryColumn));
        scrollArea->setViewportSize({columnWidth, 400});

        container->addItem(std::move(scrollArea));
    }

    initialize();
}

Game::~Game() = default;

void Game::resize(int width, int height)
{
    m_width = width;
    m_height = height;
    System::instance()->uiPainter()->setWindowSize(width, height);
}

void Game::render()
{
    glClearColor(0, 0.5, 1, 1);
    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const auto mvp = glm::ortho(0.0f, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f);

    auto *system = System::instance();

    auto *shaderManager = system->shaderManager();
    shaderManager->useProgram(ShaderManager::Flat);
    shaderManager->setUniform(ShaderManager::Uniform::ModelViewProjection, mvp);
    m_mesh->render(GL_LINE_LOOP);

    glEnable(GL_SCISSOR_TEST);
    auto *painter = system->uiPainter();
    painter->begin();
    m_item->render(painter, m_itemOffset);
#if 0
    painter->drawCircle({400, 200}, 160, {1, 1, 1, 0.5}, 1000);
    painter->drawCapsule({{40, 40}, {80, 150}}, {1, 1, 1, 0.5}, 1000);
    painter->drawCapsule({{200, 400}, {400, 450}}, {1, 1, 1, 0.5}, 1000);
    painter->drawRoundedRect({{200, 600}, {400, 720}}, 12.0f, {1, 1, 1, 0.5}, 1000);
#endif
    painter->end();
    glDisable(GL_SCISSOR_TEST);
}

void Game::update(float elapsed)
{
    m_time += elapsed;
#if 0
    auto text = std::to_string(static_cast<int>(m_time * 10.0f));
    m_counterLabel->setText(std::u32string(text.begin(), text.end()));
#endif
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
    using namespace miniui;
    const auto pos = m_mousePosition - m_itemOffset;
    m_mouseButtons |= button;
    m_item->mouseEvent({MouseEvent::Type::Press, button, pos});
    if (button == miniui::MouseButtons::Left)
    {
        assert(m_mouseGrabber == nullptr);
        m_mouseGrabber = m_item->findGrabbableItem(pos);
        if (m_mouseGrabber)
        {
            m_mouseGrabber->mouseEvent({MouseEvent::Type::DragBegin, button, pos});
        }
    }
    m_aboutToClick = true;
}

void Game::onMouseButtonRelease(miniui::MouseButtons button)
{
    using namespace miniui;
    const auto pos = m_mousePosition - m_itemOffset;
    m_mouseButtons &= ~button;
    m_item->mouseEvent({MouseEvent::Type::Release, button, pos});
    if (m_aboutToClick)
    {
        m_item->mouseEvent({MouseEvent::Type::Click, button, pos});
    }
    if (button == miniui::MouseButtons::Left && m_mouseGrabber != nullptr)
    {
        const auto event = MouseEvent{MouseEvent::Type::DragEnd, button, pos};
        m_mouseGrabber->mouseEvent(event);
        m_mouseGrabber = nullptr;
    }
    m_aboutToClick = false;
}

void Game::onMouseMove(const glm::vec2 &pos)
{
    using namespace miniui;
    m_mousePosition = pos;
    if (m_mouseGrabber)
    {
        m_mouseGrabber->mouseEvent({MouseEvent::Type::DragMove, m_mouseButtons, m_mousePosition - m_itemOffset});
    }
    m_aboutToClick = false;
}
