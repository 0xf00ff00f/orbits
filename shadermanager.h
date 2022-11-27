#pragma once

#include "shaderprogram.h"
#include "noncopyable.h"

#include <array>
#include <memory>

class Connection;

namespace gl
{
class ShaderProgram;
}

class ShaderManager : private NonCopyable
{
public:
    ShaderManager();
    ~ShaderManager();

    enum Program
    {
        Flat,
        Text,
        Decal,
        Circle,
        NumPrograms
    };
    void useProgram(Program program);

    enum Uniform
    {
        ModelViewProjection,
        BaseColorTexture,
        NumUniforms
    };

    enum Attribute
    {
        Position,
        TexCoord,
        Color,
        NumAttributes
    };

    template<typename T>
    void setUniform(Uniform uniform, T &&value)
    {
        if (!m_currentProgram)
            return;
        const auto location = uniformLocation(uniform);
        if (location == -1)
            return;
        m_currentProgram->program->setUniform(location, std::forward<T>(value));
    }

    int attributeLocation(Attribute attribute);

    const gl::ShaderProgram *currentProgram() const
    {
        return m_currentProgram ? m_currentProgram->program.get() : nullptr;
    }

private:
    int uniformLocation(Uniform uniform);

    struct CachedProgram
    {
        std::unique_ptr<gl::ShaderProgram> program;
        std::array<int, Uniform::NumUniforms> uniformLocations;
        std::array<int, Attribute::NumAttributes> attributeLocations;
    };
    std::array<std::unique_ptr<CachedProgram>, Program::NumPrograms> m_cachedPrograms;
    CachedProgram *m_currentProgram = nullptr;
};
