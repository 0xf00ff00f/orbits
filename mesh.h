#pragma once

#include "noncopyable.h"

#include <GL/glew.h>

#include <memory>
#include <span>

namespace gl
{

class Buffer;

template<typename VertexT>
class Mesh : private NonCopyable
{
public:
    Mesh();
    ~Mesh();

    void setData(std::span<const VertexT> data);
    void render(GLenum primitive) const;

private:
    std::size_t m_vertexCount = 0;
    std::unique_ptr<Buffer> m_buffer;
};

} // namespace gl

#include "mesh.inl"
