#include "noncopyable.h"

#include <span>

#include <GL/glew.h>

class Buffer : private NonCopyable
{
public:
    enum class Type
    {
        Vertex,
        Index
    };

    explicit Buffer(Type type = Type::Vertex);
    ~Buffer();

    void bind() const;
    void setData(std::span<std::byte> data) const;
    void setSubData(std::size_t offset, std::span<std::byte> data) const;

    GLuint handle() const { return m_handle; }

private:
    void initialize();

    Type m_type = Type::Vertex;
    GLuint m_handle = 0;
};
