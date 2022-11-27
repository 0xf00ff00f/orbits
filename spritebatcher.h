#pragma once

#include "noncopyable.h"
#include "shadermanager.h"
#include "util.h"
#include "buffer.h"

#include <glm/vec2.hpp>

#include <array>

class AbstractTexture;
struct PackedPixmap;

namespace gl
{

class SpriteBatcher : private NonCopyable
{
public:
    SpriteBatcher();
    ~SpriteBatcher();

    void setTransformMatrix(const glm::mat4 &matrix);
    glm::mat4 transformMatrix() const { return m_transformMatrix; }

    void setBatchProgram(ShaderManager::Program program);
    ShaderManager::Program batchProgram() const { return m_batchProgram; }

    void setClipRect(const RectF &rect);

    void begin();
    void flush();

    void addSprite(const RectF &rect, const glm::vec4 &color, int depth);
    void addSprite(const PackedPixmap &pixmap, const RectF &rect, const glm::vec4 &color, int depth);
    void addSprite(const AbstractTexture *texture, const RectF &rect, const RectF &texRect, const glm::vec4 &color,
                   int depth);

private:
    struct Quad
    {
        ShaderManager::Program program;
        const AbstractTexture *texture;
        RectF rect;
        RectF texRect;
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
    Buffer m_buffer;
    glm::mat4 m_transformMatrix;
    ShaderManager::Program m_batchProgram = ShaderManager::Program::Flat;
    bool m_bufferAllocated = false;
    int m_bufferOffset = 0;
    RectF m_clipRect;
};

} // namespace gl
