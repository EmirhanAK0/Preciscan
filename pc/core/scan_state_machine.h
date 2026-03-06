#pragma once

#include <chrono>
#include <functional>
#include <vector>

namespace preciscan::core
{

// ---------------------------------------------------------------------------
// State definitions
// ---------------------------------------------------------------------------

enum class ScanState
{
    Idle,
    HomingX,
    HomingZ,
    HomingDone,
    AdaptivePositioning,
    Scanning,
    LayerComplete,
    Processing,
    Complete,
    Error
};

// ---------------------------------------------------------------------------
// Event definitions
// ---------------------------------------------------------------------------

enum class ScanEvent
{
    // Normal flow
    Start,
    HomingXDone,
    HomingZDone,
    AdaptiveOk,
    AdaptiveFailed,
    ScanComplete,
    MoreLayers,
    LastLayer,
    ZStepDone,
    ProcessingDone,

    // Error triggers
    EStop,
    EncoderTimeout,
    LaserTimeout,
    UnexpectedLimit,

    // Recovery
    Reset
};

// ---------------------------------------------------------------------------
// History entry: one recorded transition
// ---------------------------------------------------------------------------

struct TransitionRecord
{
    std::chrono::steady_clock::time_point timestamp;
    ScanState                             from;
    ScanEvent                             event;
    ScanState                             to;
};

// ---------------------------------------------------------------------------
// ScanStateMachine
//
// Thread safety: NOT thread-safe.
// All calls to process() must come from the same thread (app thread).
// ---------------------------------------------------------------------------

class ScanStateMachine
{
public:
    using StateChangedCallback = std::function<void(ScanState /*from*/, ScanState /*to*/)>;

    ScanStateMachine() = default;

    // Process an incoming event. Ignored if no valid transition exists
    // for the current state + event combination.
    void process(ScanEvent event);

    // Returns the current state.
    ScanState state() const;

    // Returns the full transition history since construction.
    const std::vector<TransitionRecord>& history() const;

    // Register a callback invoked on every valid state transition.
    // Pass nullptr to clear. Only one callback is supported.
    void setOnStateChanged(StateChangedCallback callback);

private:
    ScanState                      state_    { ScanState::Idle };
    std::vector<TransitionRecord>  history_;
    StateChangedCallback           onStateChanged_;

    // Performs the transition: records history, fires callback.
    void transition(ScanState to, ScanEvent triggeredBy);

    // Returns true if event is a global error trigger (EStop etc.)
    static bool isErrorEvent(ScanEvent event);
};

} // namespace preciscan::core

// ---------------------------------------------------------------------------
// Convenience: bring types into global scope for tests and app code
// that prefer not to qualify with preciscan::core::
// ---------------------------------------------------------------------------

using preciscan::core::ScanEvent;
using preciscan::core::ScanState;
using preciscan::core::ScanStateMachine;
