#include "system.h"

#include "shadermanager.h"
#include "painter.h"

void System::initialize()
{
    m_shaderManager = new ShaderManager;
    m_uiPainter = new miniui::Painter;
}

void System::release()
{
    delete m_uiPainter;
    m_uiPainter = nullptr;

    delete m_shaderManager;
    m_shaderManager = nullptr;
}

System &System::instance()
{
    static System system;
    return system;
}
