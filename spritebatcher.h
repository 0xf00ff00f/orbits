#pragma once

#include "noncopyable.h"
#include "shadermanager.h"
#include "util.h"
#include "buffer.h"

#include <glm/vec2.hpp>

#include <array>

class AbstractTexture;
struct PackedPixmap;

class SpriteBatcher : private NonCopyable
{
public:
    explicit SpriteBatcher(ShaderManager *shaderManager);
    ~SpriteBatcher();

    void setTransformMatrix(const glm::mat4 &matrix);
    glm::mat4 transformMatrix() const;

    void setBatchProgram(ShaderManager::Program program);
    ShaderManager::Program batchProgram() const;

    struct Vertex
    {
        glm::vec2 position;
        glm::vec2 texCoord;
        glm::vec4 color;
    };
    using QuadVerts = std::array<Vertex, 4>;

    void begin();
    void flush();

    void addSprite(const glm::vec2 &topLeft, const glm::vec2 &bottomRight, const glm::vec4 &color, int depth);
    void addSprite(const QuadVerts &verts, int depth);
    void addSprite(const PackedPixmap &pixmap, const glm::vec2 &topLeft, const glm::vec2 &bottomRight,
                   const glm::vec4 &color, int depth);
    void addSprite(const AbstractTexture *texture, const QuadVerts &verts, int depth);

private:
    struct Quad
    {
        const AbstractTexture *texture;
        ShaderManager::Program program;
        QuadVerts verts;
        int depth;
    };

    static constexpr int BufferCapacity = 0x100000;                       // in floats
    static constexpr int GLVertexSize = sizeof(Vertex) / sizeof(GLfloat); // in floats
    static constexpr int GLQuadSize = 6 * GLVertexSize;                   // 6 verts per quad
    static constexpr int MaxQuadsPerBatch = BufferCapacity / GLQuadSize;

    ShaderManager *m_shaderManager;
    std::array<Quad, MaxQuadsPerBatch> m_quads;
    int m_quadCount = 0;
    Buffer m_buffer;
    glm::mat4 m_transformMatrix;
    ShaderManager::Program m_batchProgram = ShaderManager::Program::Flat;
    bool m_bufferAllocated = false;
    int m_bufferOffset = 0;
};
