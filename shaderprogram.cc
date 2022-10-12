#include "shaderprogram.h"

#include "ioutil.h"
#include "log.h"

#include <fstream>
#include <sstream>
#include <optional>
#include <memory>

#include <glm/gtc/type_ptr.hpp>

ShaderProgram::ShaderProgram()
{
    initialize();
}

ShaderProgram::~ShaderProgram() = default;

void ShaderProgram::initialize()
{
    m_id = glCreateProgram();
}

bool ShaderProgram::addShader(GLenum type, std::string_view filename)
{
    auto source = Util::readFile(std::string(filename));
    if (!source)
    {
        std::stringstream ss;
        ss << "failed to load " << filename;
        m_log = ss.str();
        return false;
    }
    return addShaderSource(type, std::string(source->begin(), source->end()));
}

bool ShaderProgram::addShaderSource(GLenum type, const std::string &source)
{
    m_shaderSources[type] = source;
    return compileAndAttachShader(type, source);
}

bool ShaderProgram::compileAndAttachShader(GLenum type, const std::string &source)
{
    const auto shader = glCreateShader(type);

    const auto *s = source.c_str();
    glShaderSource(shader, 1, &s, nullptr);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        m_log.clear();
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 1)
        {
            auto buffer = std::make_unique<char[]>(logLength);
            GLsizei dummy;
            glGetShaderInfoLog(shader, logLength, &dummy, buffer.get());
            m_log = buffer.get();
        }
        return false;
    }

    glAttachShader(m_id, shader);

    return true;
}

bool ShaderProgram::link()
{
    glLinkProgram(m_id);

    GLint status;
    glGetProgramiv(m_id, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        m_log.clear();
        GLint logLength;
        glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 1)
        {
            auto buffer = std::make_unique<char[]>(logLength);
            GLsizei dummy;
            glGetProgramInfoLog(m_id, logLength, &dummy, buffer.get());
            m_log = buffer.get();
        }
        return false;
    }

    return true;
}

void ShaderProgram::bind() const
{
    glUseProgram(m_id);
}

int ShaderProgram::uniformLocation(const char *name) const
{
    return glGetUniformLocation(m_id, name);
}

int ShaderProgram::attributeLocation(const char *name) const
{
    return glGetAttribLocation(m_id, name);
}

void ShaderProgram::bindAttributeLocation(std::size_t index, const char *name) const
{
    glBindAttribLocation(m_id, index, name);
}

void ShaderProgram::setUniform(int location, int value) const
{
    glUniform1i(location, value);
}

void ShaderProgram::setUniform(int location, float value) const
{
    glUniform1f(location, value);
}

void ShaderProgram::setUniform(int location, const glm::vec2 &value) const
{
    glUniform2fv(location, 1, glm::value_ptr(value));
}

void ShaderProgram::setUniform(int location, const glm::vec3 &value) const
{
    glUniform3fv(location, 1, glm::value_ptr(value));
}

void ShaderProgram::setUniform(int location, const glm::vec4 &value) const
{
    glUniform4fv(location, 1, glm::value_ptr(value));
}

void ShaderProgram::setUniform(int location, const std::vector<float> &value) const
{
    glUniform1fv(location, value.size(), value.data());
}

void ShaderProgram::setUniform(int location, const std::vector<glm::vec2> &value) const
{
    glUniform2fv(location, value.size(), reinterpret_cast<const float *>(value.data()));
}

void ShaderProgram::setUniform(int location, const std::vector<glm::vec3> &value) const
{
    glUniform3fv(location, value.size(), reinterpret_cast<const float *>(value.data()));
}

void ShaderProgram::setUniform(int location, const std::vector<glm::vec4> &value) const
{
    glUniform4fv(location, value.size(), reinterpret_cast<const float *>(value.data()));
}

void ShaderProgram::setUniform(int location, const glm::mat3 &value) const
{
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void ShaderProgram::setUniform(int location, const glm::mat4 &value) const
{
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}
