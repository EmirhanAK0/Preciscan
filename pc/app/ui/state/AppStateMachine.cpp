#include "AppStateMachine.hpp"

AppStateMachine::AppStateMachine(QObject* parent)
    : QObject(parent)
{
}

AppState AppStateMachine::currentState() const
{
    return m_state;
}

void AppStateMachine::setState(AppState newState)
{
    if (m_state == newState)
        return;

    m_state = newState;
    emit stateChanged(m_state);
}
