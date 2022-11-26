#include "system.h"

#include "shadermanager.h"

void System::initialize()
{
    m_shaderManager = new ShaderManager;
}

void System::release()
{
    delete m_shaderManager;
    m_shaderManager = nullptr;
}

System &System::instance()
{
    static System system;
    return system;
}
