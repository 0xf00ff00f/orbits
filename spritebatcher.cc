#include "spritebatcher.h"
#include "abstracttexture.h"
#include "textureatlas.h"
#include "log.h"
#include "system.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

SpriteBatcher::SpriteBatcher()
    : m_buffer(gl::Buffer::Type::Vertex, gl::Buffer::Usage::DynamicDraw)
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
    addSprite(nullptr, {topLeft, {}}, {bottomRight, {}}, color, depth);
}

void SpriteBatcher::addSprite(const PackedPixmap &pixmap, const glm::vec2 &topLeft, const glm::vec2 &bottomRight,
                              const glm::vec4 &color, int depth)
{
    const auto &topLeftUV = pixmap.texCoord.min;
    const auto &bottomRightUV = pixmap.texCoord.max;
    addSprite(pixmap.texture, {topLeft, topLeftUV}, {bottomRight, bottomRightUV}, color, depth);
}

void SpriteBatcher::addSprite(const AbstractTexture *texture, const PositionUV &topLeft, const PositionUV &bottomRight,
                              const glm::vec4 &color, int depth)
{
    if (m_quadCount == MaxQuadsPerBatch)
        flush();

    auto &quad = m_quads[m_quadCount++];
    quad.texture = texture;
    quad.program = m_batchProgram;
    quad.topLeft = topLeft;
    quad.bottomRight = bottomRight;
    quad.color = color;
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

            const auto emitVertex = [&data, color = quadPtr->color](const glm::vec2 &position,
                                                                    const glm::vec2 &texCoord) {
                *data++ = position.x;
                *data++ = position.y;

                *data++ = texCoord.x;
                *data++ = texCoord.y;

                *data++ = color.x;
                *data++ = color.y;
                *data++ = color.z;
                *data++ = color.w;
            };

            const auto &p0 = quadPtr->topLeft.position;
            const auto &t0 = quadPtr->topLeft.texCoord;

            const auto &p1 = quadPtr->bottomRight.position;
            const auto &t1 = quadPtr->bottomRight.texCoord;

            emitVertex({p0.x, p0.y}, {t0.x, t0.y});
            emitVertex({p1.x, p0.y}, {t1.x, t0.y});
            emitVertex({p1.x, p1.y}, {t1.x, t1.y});

            emitVertex({p1.x, p1.y}, {t1.x, t1.y});
            emitVertex({p0.x, p1.y}, {t0.x, t1.y});
            emitVertex({p0.x, p0.y}, {t0.x, t0.y});
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
            auto *shaderManager = System::instance().shaderManager();
            shaderManager->useProgram(batchProgram);
            shaderManager->setUniform(ShaderManager::Uniform::ModelViewProjection, m_transformMatrix);
            if (currentTexture)
                shaderManager->setUniform(ShaderManager::Uniform::BaseColorTexture, 0);

            positionLocation = shaderManager->attributeLocation(ShaderManager::Attribute::Position);
            if (positionLocation != -1)
            {
                glEnableVertexAttribArray(positionLocation);
                glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                      reinterpret_cast<GLvoid *>(0));
            }

            if (currentTexture)
            {
                texCoordLocation = shaderManager->attributeLocation(ShaderManager::Attribute::TexCoord);
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

            colorLocation = shaderManager->attributeLocation(ShaderManager::Attribute::Color);
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