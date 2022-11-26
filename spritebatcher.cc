#include "spritebatcher.h"
#include "abstracttexture.h"
#include "textureatlas.h"
#include "log.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

SpriteBatcher::SpriteBatcher(ShaderManager *shaderManager)
    : m_shaderManager(shaderManager)
    , m_buffer(gl::Buffer::Type::Vertex, gl::Buffer::Usage::DynamicDraw)
{
}

SpriteBatcher::~SpriteBatcher() = default;

void SpriteBatcher::setTransformMatrix(const glm::mat4 &matrix)
{
    m_transformMatrix = matrix;
}

glm::mat4 SpriteBatcher::transformMatrix() const
{
    return m_transformMatrix;
}

void SpriteBatcher::setBatchProgram(ShaderManager::Program program)
{
    m_batchProgram = program;
}

ShaderManager::Program SpriteBatcher::batchProgram() const
{
    return m_batchProgram;
}

void SpriteBatcher::begin()
{
    m_quadCount = 0;
}

void SpriteBatcher::addSprite(const glm::vec2 &topLeft, const glm::vec2 &bottomRight, const glm::vec4 &color, int depth)
{
    const auto &p0 = topLeft;
    const auto &p1 = bottomRight;

    const auto verts = QuadVerts{
        {{{p0.x, p0.y}, {}, color}, {{p1.x, p0.y}, {}, color}, {{p1.x, p1.y}, {}, color}, {{p0.x, p1.y}, {}, color}}};

    addSprite(nullptr, verts, depth);
}

void SpriteBatcher::addSprite(const QuadVerts &verts, int depth)
{
    addSprite(nullptr, verts, depth);
}

void SpriteBatcher::addSprite(const PackedPixmap &pixmap, const glm::vec2 &topLeft, const glm::vec2 &bottomRight,
                              const glm::vec4 &color, int depth)
{
    const auto &p0 = topLeft;
    const auto &p1 = bottomRight;

    const auto &t0 = pixmap.texCoord.min;
    const auto &t1 = pixmap.texCoord.max;

    const auto verts = QuadVerts{{{{p0.x, p0.y}, {t0.x, t0.y}, color},
                                  {{p1.x, p0.y}, {t1.x, t0.y}, color},
                                  {{p1.x, p1.y}, {t1.x, t1.y}, color},
                                  {{p0.x, p1.y}, {t0.x, t1.y}, color}}};

    addSprite(pixmap.texture, verts, depth);
}

void SpriteBatcher::addSprite(const AbstractTexture *texture, const QuadVerts &verts, int depth)
{
    if (m_quadCount == MaxQuadsPerBatch)
        flush();

    auto &quad = m_quads[m_quadCount++];
    quad.texture = texture;
    quad.program = m_batchProgram;
    quad.verts = verts;
    quad.depth = depth;
}

void SpriteBatcher::flush()
{
    if (m_quadCount == 0)
        return;

    static std::array<const Quad *, MaxQuadsPerBatch> sortedQuads;
    const auto quadsEnd = m_quads.begin() + m_quadCount;
    std::transform(m_quads.begin(), quadsEnd, sortedQuads.begin(), [](const Quad &quad) { return &quad; });
    const auto sortedQuadsEnd = sortedQuads.begin() + m_quadCount;
    std::stable_sort(sortedQuads.begin(), sortedQuadsEnd, [](const Quad *a, const Quad *b) {
        return std::tie(a->depth, a->texture, a->program) < std::tie(b->depth, b->texture, b->program);
    });

    m_buffer.bind();

    const AbstractTexture *currentTexture = nullptr;
    std::optional<ShaderManager::Program> currentProgram = std::nullopt;
    int positionLocation = -1, texCoordLocation = -1, colorLocation = -1;

    auto batchStart = sortedQuads.begin();
    while (batchStart != sortedQuadsEnd)
    {
        const auto *batchTexture = (*batchStart)->texture;
        const auto batchProgram = (*batchStart)->program;
        const auto batchEnd =
            std::find_if(batchStart + 1, sortedQuadsEnd, [batchTexture, batchProgram](const Quad *quad) {
                return quad->texture != batchTexture || quad->program != batchProgram;
            });

        const auto quadCount = batchEnd - batchStart;
        const auto bufferRangeSize = quadCount * GLQuadSize;

        if (!m_bufferAllocated || (m_bufferOffset + bufferRangeSize > BufferCapacity))
        {
            // orphan the old buffer and grab a new memory block
            m_buffer.allocate(BufferCapacity * sizeof(GLfloat));
            m_bufferOffset = 0;
            m_bufferAllocated = true;
        }

        static std::array<GLfloat, BufferCapacity> bufferData;
        auto *data = bufferData.data();
        for (auto it = batchStart; it != batchEnd; ++it)
        {
            auto *quadPtr = *it;
            const auto &verts = quadPtr->verts;
            const auto emitVertex = [&data, &verts](int index) {
                const auto &v = verts[index];

                *data++ = v.position.x;
                *data++ = v.position.y;

                *data++ = v.texCoord.x;
                *data++ = v.texCoord.y;

                *data++ = v.color.x;
                *data++ = v.color.y;
                *data++ = v.color.z;
                *data++ = v.color.w;
            };

            emitVertex(0);
            emitVertex(1);
            emitVertex(2);

            emitVertex(2);
            emitVertex(3);
            emitVertex(0);
        }
        assert(data == bufferData.data() + bufferRangeSize);
        m_buffer.write(m_bufferOffset * sizeof(GLfloat), std::as_bytes(std::span(bufferData.data(), bufferRangeSize)));

        if (currentTexture != batchTexture)
        {
            currentTexture = batchTexture;
            if (currentTexture)
                currentTexture->bind();
        }

        if (currentProgram != batchProgram)
        {
            if (positionLocation != -1)
                glDisableVertexAttribArray(positionLocation);
            if (texCoordLocation != -1)
                glDisableVertexAttribArray(texCoordLocation);
            if (colorLocation != -1)
                glDisableVertexAttribArray(colorLocation);

            currentProgram = batchProgram;
            m_shaderManager->useProgram(batchProgram);
            m_shaderManager->setUniform(ShaderManager::Uniform::ModelViewProjection, m_transformMatrix);
            if (currentTexture)
                m_shaderManager->setUniform(ShaderManager::Uniform::BaseColorTexture, 0);

            positionLocation = m_shaderManager->attributeLocation(ShaderManager::Attribute::Position);
            if (positionLocation != -1)
            {
                glEnableVertexAttribArray(positionLocation);
                glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                      reinterpret_cast<GLvoid *>(0));
            }

            if (currentTexture)
            {
                texCoordLocation = m_shaderManager->attributeLocation(ShaderManager::Attribute::TexCoord);
                if (texCoordLocation != -1)
                {
                    glVertexAttribPointer(texCoordLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                          reinterpret_cast<GLvoid *>(2 * sizeof(GLfloat)));
                    glEnableVertexAttribArray(texCoordLocation);
                }
            }
            else
            {
                texCoordLocation = -1;
            }

            colorLocation = m_shaderManager->attributeLocation(ShaderManager::Attribute::Color);
            if (colorLocation != -1)
            {
                glVertexAttribPointer(colorLocation, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                      reinterpret_cast<GLvoid *>(4 * sizeof(GLfloat)));
                glEnableVertexAttribArray(colorLocation);
            }
        }

        glDrawArrays(GL_TRIANGLES, m_bufferOffset / GLVertexSize, quadCount * 6);

        m_bufferOffset += bufferRangeSize;
        batchStart = batchEnd;
    }

    if (positionLocation != -1)
        glDisableVertexAttribArray(positionLocation);
    if (texCoordLocation != -1)
        glDisableVertexAttribArray(texCoordLocation);
    if (colorLocation != -1)
        glDisableVertexAttribArray(colorLocation);

    m_quadCount = 0;
}
