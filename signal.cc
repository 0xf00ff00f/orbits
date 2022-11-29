#include "signal.h"

SignalBase::~SignalBase() = default;

Connection::Connection(SignalBase *e)
    : m_event(e)
{
}

Connection::~Connection()
{
    if (m_event)
        m_event->disconnect(this);
}

void Connection::setEvent(SignalBase *e)
{
    m_event = e;
}
