#include "shadermanager.h"

#include "log.h"

#include <type_traits>
#include <span>

namespace
{
std::string shaderPath(std::string_view basename)
{
    return std::string("assets/shaders/") + std::string(basename);
}

const char *attributeName(ShaderManager::Attribute id)
{
    static constexpr const char *attributeNames[] = {
        // clang-format off
            "position",
            "texCoord",
            "color",
        // clang-format on
    };
    static_assert(std::extent_v<decltype(attributeNames)> == ShaderManager::NumAttributes,
                  "expected number of attributes to match");
    return attributeNames[id];
}

std::unique_ptr<gl::ShaderProgram> loadProgram(const char *vertexShader, const char *fragmentShader,
                                               std::span<const ShaderManager::Attribute> attributes)
{
    auto program = std::make_unique<gl::ShaderProgram>();
    if (!program->addShader(GL_VERTEX_SHADER, shaderPath(vertexShader)))
    {
        log("Failed to add vertex shader for program %s: %s\n", vertexShader, program->log().c_str());
        return {};
    }
    if (!program->addShader(GL_FRAGMENT_SHADER, shaderPath(fragmentShader)))
    {
        log("Failed to add fragment shader for program %s: %s\n", fragmentShader, program->log().c_str());
        return {};
    }
    for (std::size_t i = 0; i < attributes.size(); ++i)
    {
        program->bindAttributeLocation(i, attributeName(attributes[i]));
    }
    if (!program->link())
    {
        log("Failed to link program: %s\n", program->log().c_str());
        return {};
    }
    return program;
}

std::unique_ptr<gl::ShaderProgram> loadProgram(ShaderManager::Program id)
{
    struct ProgramSource
    {
        const char *vertexShader;
        const char *fragmentShader;
        std::vector<ShaderManager::Attribute> attributes;
    };
    static const ProgramSource programSources[] = {
        // flat
        {"flat.vert", "flat.frag", {ShaderManager::Attribute::Position, ShaderManager::Attribute::Color}},
        // decal
        {"text.vert",
         "text.frag",
         {ShaderManager::Attribute::Position, ShaderManager::Attribute::TexCoord, ShaderManager::Attribute::Color}},
    };
    static_assert(std::extent_v<decltype(programSources)> == ShaderManager::NumPrograms,
                  "expected number of programs to match");

    const auto &sources = programSources[id];
    return loadProgram(sources.vertexShader, sources.fragmentShader, sources.attributes);
}

} // namespace

ShaderManager::ShaderManager() = default;
ShaderManager::~ShaderManager() = default;

void ShaderManager::useProgram(Program id)
{
    auto &cachedProgram = m_cachedPrograms[id];
    if (!cachedProgram)
    {
        cachedProgram.reset(new CachedProgram);
        cachedProgram->program = loadProgram(id);
        auto &uniforms = cachedProgram->uniformLocations;
        std::fill(uniforms.begin(), uniforms.end(), -1);
        auto &attributes = cachedProgram->attributeLocations;
        std::fill(attributes.begin(), attributes.end(), -1);
    }
    if (cachedProgram.get() == m_currentProgram)
        return;
    if (cachedProgram->program)
        cachedProgram->program->bind();
    m_currentProgram = cachedProgram.get();
}

int ShaderManager::uniformLocation(Uniform id)
{
    if (!m_currentProgram || !m_currentProgram->program)
    {
        return -1;
    }
    auto location = m_currentProgram->uniformLocations[id];
    if (location == -1)
    {
        static constexpr const char *uniformNames[] = {
            // clang-format off
            "mvp",
            "baseColorTexture",
            // clang-format on
        };
        static_assert(std::extent_v<decltype(uniformNames)> == NumUniforms, "expected number of uniforms to match");

        location = m_currentProgram->program->uniformLocation(uniformNames[id]);
        m_currentProgram->uniformLocations[id] = location;
    }
    return location;
}

int ShaderManager::attributeLocation(Attribute id)
{
    if (!m_currentProgram || !m_currentProgram->program)
    {
        return -1;
    }
    auto location = m_currentProgram->attributeLocations[id];
    if (location == -1)
    {
        location = m_currentProgram->program->attributeLocation(attributeName(id));
        m_currentProgram->attributeLocations[id] = location;
    }
    return location;
}
