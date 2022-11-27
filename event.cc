#include "event.h"

EventBase::~EventBase() = default;

Connection::Connection(EventBase *e)
    : m_event(e)
{
}

Connection::~Connection()
{
    if (m_event)
        m_event->disconnect(this);
}

void Connection::setEvent(EventBase *e)
{
    m_event = e;
}
