#pragma once

#include <QObject>
#include "AppState.hpp"

class AppStateMachine : public QObject {
    Q_OBJECT

public:
    explicit AppStateMachine(QObject* parent = nullptr);

    AppState currentState() const;

public slots:
    void setState(AppState newState);

signals:
    void stateChanged(AppState newState);

private:
    AppState m_state = AppState::Idle;
};
