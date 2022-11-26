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
    SpriteBatcher();
    ~SpriteBatcher();

    void setTransformMatrix(const glm::mat4 &matrix);
    glm::mat4 transformMatrix() const;

    void setBatchProgram(ShaderManager::Program program);
    ShaderManager::Program batchProgram() const;

    void begin();
    void flush();

    void addSprite(const glm::vec2 &topLeft, const glm::vec2 &bottomRight, const glm::vec4 &color, int depth);
    void addSprite(const PackedPixmap &pixmap, const glm::vec2 &topLeft, const glm::vec2 &bottomRight,
                   const glm::vec4 &color, int depth);

private:
    struct PositionUV
    {
        glm::vec2 position;
        glm::vec2 texCoord;
    };
    void addSprite(const AbstractTexture *texture, const PositionUV &topLeft, const PositionUV &bottomRight,
                   const glm::vec4 &color, int depth);

    struct Quad
    {
        ShaderManager::Program program;
        const AbstractTexture *texture;
        PositionUV topLeft;
        PositionUV bottomRight;
        glm::vec4 color;
        int depth;
    };

    struct Vertex
    {
        glm::vec2 position;
        glm::vec2 texCoord;
        glm::vec4 color;
    };

    static constexpr int BufferCapacity = 0x100000;                       // in floats
    static constexpr int GLVertexSize = sizeof(Vertex) / sizeof(GLfloat); // in floats
    static constexpr int GLQuadSize = 6 * GLVertexSize;                   // 6 verts per quad
    static constexpr int MaxQuadsPerBatch = BufferCapacity / GLQuadSize;

    std::array<Quad, MaxQuadsPerBatch> m_quads;
    int m_quadCount = 0;
    gl::Buffer m_buffer;
    glm::mat4 m_transformMatrix;
    ShaderManager::Program m_batchProgram = ShaderManager::Program::Flat;
    bool m_bufferAllocated = false;
    int m_bufferOffset = 0;
};
