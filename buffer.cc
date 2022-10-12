#include "buffer.h"

namespace
{
GLenum toGLTarget(Buffer::Type type)
{
    switch (type)
    {
    case Buffer::Type::Vertex:
    default:
        return GL_ARRAY_BUFFER;
    case Buffer::Type::Index:
        return GL_ELEMENT_ARRAY_BUFFER;
    }
}
}

Buffer::Buffer(Type type)
    : m_type(type)
{
    glGenBuffers(1, &m_handle);
}

Buffer::~Buffer()
{
    glDeleteBuffers(1, &m_handle);
}

void Buffer::bind() const
{
    glBindBuffer(toGLTarget(m_type), m_handle);
}

void Buffer::setData(std::span<std::byte> data) const
{
    const auto target = toGLTarget(m_type);
    glBindBuffer(target, m_handle);
    glBufferData(target, data.size(), data.data(), GL_STATIC_DRAW);
}

void Buffer::setSubData(std::size_t offset, std::span<std::byte> data) const
{
    const auto target = toGLTarget(m_type);
    glBindBuffer(target, m_handle);
    glBufferSubData(target, offset, data.size(), data.data());
}
