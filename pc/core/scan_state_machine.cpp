#include "scan_state_machine.h"

namespace preciscan::core
{

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

void ScanStateMachine::process(ScanEvent event)
{
    // Global error events — valid from any state except ERROR itself
    if (isErrorEvent(event) && state_ != ScanState::Error)
    {
        transition(ScanState::Error, event);
        return;
    }

    // State-specific transitions
    switch (state_)
    {
        case ScanState::Idle:
            if (event == ScanEvent::Start)
                transition(ScanState::HomingX, event);
            break;

        case ScanState::HomingX:
            if (event == ScanEvent::HomingXDone)
                transition(ScanState::HomingZ, event);
            break;

        case ScanState::HomingZ:
            if (event == ScanEvent::HomingZDone)
                transition(ScanState::HomingDone, event);
            break;

        case ScanState::HomingDone:
            if (event == ScanEvent::Start)
                transition(ScanState::AdaptivePositioning, event);
            break;

        case ScanState::AdaptivePositioning:
            if (event == ScanEvent::AdaptiveOk)
                transition(ScanState::Scanning, event);
            else if (event == ScanEvent::AdaptiveFailed)
                transition(ScanState::Error, event);
            break;

        case ScanState::Scanning:
            if (event == ScanEvent::ScanComplete)
                transition(ScanState::LayerComplete, event);
            break;

        case ScanState::LayerComplete:
            if (event == ScanEvent::MoreLayers)
                transition(ScanState::LayerComplete, event);
            else if (event == ScanEvent::LastLayer)
                transition(ScanState::Processing, event);
            else if (event == ScanEvent::ZStepDone)
                transition(ScanState::AdaptivePositioning, event);
            break;

        case ScanState::Processing:
            if (event == ScanEvent::ProcessingDone)
                transition(ScanState::Complete, event);
            break;

        case ScanState::Complete:
            // Terminal state — only reset via ERROR path or new start
            break;

        case ScanState::Error:
            if (event == ScanEvent::Reset)
                transition(ScanState::Idle, event);
            break;
    }
}

ScanState ScanStateMachine::state() const
{
    return state_;
}

const std::vector<TransitionRecord>& ScanStateMachine::history() const
{
    return history_;
}

void ScanStateMachine::setOnStateChanged(StateChangedCallback callback)
{
    onStateChanged_ = std::move(callback);
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void ScanStateMachine::transition(ScanState to, ScanEvent triggeredBy)
{
    ScanState from = state_;

    history_.push_back(TransitionRecord{
        std::chrono::steady_clock::now(),
        from,
        triggeredBy,
        to
    });

    state_ = to;

    if (onStateChanged_)
        onStateChanged_(from, to);
}

bool ScanStateMachine::isErrorEvent(ScanEvent event)
{
    switch (event)
    {
        case ScanEvent::EStop:
        case ScanEvent::EncoderTimeout:
        case ScanEvent::LaserTimeout:
        case ScanEvent::UnexpectedLimit:
            return true;
        default:
            return false;
    }
}

} // namespace preciscan::core
