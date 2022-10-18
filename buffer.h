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

    enum class Usage
    {
        StaticDraw,
        DynamicDraw,
        StreamDraw,
    };

    explicit Buffer(Type type = Type::Vertex, Usage usage = Usage::StaticDraw);
    ~Buffer();

    void bind() const;
    void allocate(std::size_t size) const { allocate({static_cast<const std::byte *>(nullptr), size}); }
    void allocate(std::span<const std::byte> data) const;
    void write(std::size_t offset, std::span<const std::byte> data) const;

    GLuint handle() const { return m_handle; }

private:
    void initialize();

    GLenum m_type;
    GLenum m_usage;
    GLuint m_handle = 0;
};
