#pragma once

class ShaderManager;

class System
{
public:
    void initialize();
    void release();

    ShaderManager *shaderManager() const { return m_shaderManager; }

    static System &instance();

private:
    ShaderManager *m_shaderManager = nullptr;
};
