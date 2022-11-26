#pragma once

class ShaderManager;

namespace miniui
{
class Painter;
}

class System
{
public:
    void initialize();
    void release();

    ShaderManager *shaderManager() const { return m_shaderManager; }
    miniui::Painter *uiPainter() const { return m_uiPainter; }

    static System &instance();

private:
    ShaderManager *m_shaderManager = nullptr;
    miniui::Painter *m_uiPainter = nullptr;
};
