#pragma once

#include <algorithm>
#include <memory>
#include <vector>

class Connection;

class SignalBase
{
public:
    virtual ~SignalBase();
    virtual void disconnect(Connection *connection) = 0;
};

class Connection
{
public:
    explicit Connection(SignalBase *e);
    ~Connection();

    void setEvent(SignalBase *e);

private:
    SignalBase *m_event;
};

using ConnectionPtr = std::unique_ptr<Connection>;

template<typename HandlerType>
class Signal : public SignalBase
{
public:
    ~Signal() override
    {
        for (auto &c : m_connections)
            c.connection->setEvent(nullptr);
        m_connections.clear();
    }

    void disconnect(Connection *connection) override
    {
        m_connections.erase(std::remove_if(m_connections.begin(), m_connections.end(),
                                           [connection](auto &c) { return c.connection == connection; }),
                            m_connections.end());
    }

    template<typename... Args>
    void notify(Args &&...args)
    {
        for (auto &c : m_connections)
            c.handler(std::forward<Args>(args)...);
    }

    std::unique_ptr<Connection> connect(const HandlerType &handler)
    {
        auto connection = std::make_unique<Connection>(this);
        m_connections.push_back({handler, connection.get()});
        return connection;
    }

private:
    struct HandlerConnection
    {
        HandlerType handler;
        Connection *connection;
    };
    std::vector<HandlerConnection> m_connections;
};
