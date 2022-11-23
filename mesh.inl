#include <concepts>

#include <glm/glm.hpp>

#include "reflect.h"
#include "buffer.h"
#include "log.h"

namespace detail
{
template<typename T>
concept Scalar = std::integral<T> || std::floating_point<T>;

template<typename T>
struct GLSize;

template<Scalar T>
struct GLSize<T>
{
    static constexpr GLint value = 1;
};

template<glm::length_t L, typename T, glm::qualifier Q>
struct GLSize<glm::vec<L, T, Q>>
{
    static constexpr GLint value = L;
};

template<typename T>
struct GLType;

template<>
struct GLType<char>
{
    static constexpr GLenum value = GL_BYTE;
};

template<>
struct GLType<unsigned char>
{
    static constexpr GLenum value = GL_UNSIGNED_BYTE;
};

template<>
struct GLType<short>
{
    static constexpr GLenum value = GL_SHORT;
};

template<>
struct GLType<unsigned short>
{
    static constexpr GLenum value = GL_UNSIGNED_SHORT;
};

template<>
struct GLType<float>
{
    static constexpr GLenum value = GL_FLOAT;
};

template<glm::length_t L, typename T, glm::qualifier Q>
struct GLType<glm::vec<L, T, Q>>
{
    static constexpr auto value = GLType<T>::value;
};

}

template<typename VertexT>
Mesh<VertexT>::Mesh()
    : m_buffer(new Buffer(Buffer::Type::Vertex, Buffer::Usage::StaticDraw))
{
}

template<typename VertexT>
Mesh<VertexT>::~Mesh() = default;

template<typename VertexT>
void Mesh<VertexT>::setData(std::span<const VertexT> data)
{
    m_vertexCount = data.size();
    m_buffer->bind();
    m_buffer->allocate(std::as_bytes(data));
}

template<typename VertexT>
void Mesh<VertexT>::render(GLenum primitive) const
{
    m_buffer->bind();

    reflect::forEachMember(VertexT{}, [index = 0, offset = static_cast<const std::byte *>(nullptr)](const auto &m) mutable {
        using Type = std::decay_t<decltype(m)>;
        glEnableVertexAttribArray(index);
        const auto size = detail::GLSize<Type>::value;
        const auto type = detail::GLType<Type>::value;
        glVertexAttribPointer(index, size, type, GL_FALSE, sizeof(VertexT), offset);
        ++index;
        offset += sizeof(m);
    });

    glDrawArrays(primitive, 0, m_vertexCount);

    reflect::forEachMember(VertexT{}, [index = 0](const auto &) mutable {
        glDisableVertexAttribArray(index);
        ++index;
    });
}
