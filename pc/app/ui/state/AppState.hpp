#pragma once

#include <QString>

enum class AppState {
    Idle,
    Ready,
    Scanning,
    Paused,
    Completed,
    Fault
};

inline QString appStateToString(AppState state) {
    switch (state) {
        case AppState::Idle: return "Idle";
        case AppState::Ready: return "Ready";
        case AppState::Scanning: return "Scanning";
        case AppState::Paused: return "Paused";
        case AppState::Completed: return "Completed";
        case AppState::Fault: return "Fault";
        default: return "Unknown";
    }
}
