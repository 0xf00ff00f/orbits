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

GLenum toGLUsage(Buffer::Usage usage)
{
    switch (usage)
    {
    case Buffer::Usage::StaticDraw:
    default:
        return GL_STATIC_DRAW;
    case Buffer::Usage::DynamicDraw:
        return GL_DYNAMIC_DRAW;
    case Buffer::Usage::StreamDraw:
        return GL_STREAM_DRAW;
    }
}
}

Buffer::Buffer(Type type, Usage usage)
    : m_type(toGLTarget(type))
    , m_usage(toGLUsage(usage))
{
    glGenBuffers(1, &m_handle);
}

Buffer::~Buffer()
{
    glDeleteBuffers(1, &m_handle);
}

void Buffer::bind() const
{
    glBindBuffer(m_type, m_handle);
}

void Buffer::allocate(std::size_t size) const
{
    allocate(size, nullptr);
}

void Buffer::allocate(std::span<const std::byte> data) const
{
    allocate(data.size(), data.data());
}

void Buffer::allocate(std::size_t size, const std::byte *data) const
{
    glBufferData(m_type, size, data, m_usage);
}

void Buffer::write(std::size_t offset, std::span<const std::byte> data) const
{
    glBufferSubData(m_type, offset, data.size(), data.data());
}
