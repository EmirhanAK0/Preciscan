#include "scan_state_machine.h"

#include <catch2/catch_test_macros.hpp>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static ScanStateMachine makeFsm()
{
    return ScanStateMachine{};
}

static void sendEvent(ScanStateMachine& fsm, ScanEvent e)
{
    fsm.process(e);
}

// ---------------------------------------------------------------------------
// Initial state
// ---------------------------------------------------------------------------

TEST_CASE("Initial state is IDLE", "[fsm]")
{
    auto fsm = makeFsm();
    REQUIRE(fsm.state() == ScanState::Idle);
}

// ---------------------------------------------------------------------------
// Happy path: full scan sequence
// ---------------------------------------------------------------------------

TEST_CASE("START transitions IDLE to HOMING_X", "[fsm][homing]")
{
    auto fsm = makeFsm();
    sendEvent(fsm, ScanEvent::Start);
    REQUIRE(fsm.state() == ScanState::HomingX);
}

TEST_CASE("HOMING_X_DONE transitions to HOMING_Z", "[fsm][homing]")
{
    auto fsm = makeFsm();
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::HomingXDone);
    REQUIRE(fsm.state() == ScanState::HomingZ);
}

TEST_CASE("HOMING_Z_DONE transitions to HOMING_DONE", "[fsm][homing]")
{
    auto fsm = makeFsm();
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::HomingXDone);
    sendEvent(fsm, ScanEvent::HomingZDone);
    REQUIRE(fsm.state() == ScanState::HomingDone);
}

TEST_CASE("START from HOMING_DONE transitions to ADAPTIVE_POSITIONING", "[fsm][adaptive]")
{
    auto fsm = makeFsm();
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::HomingXDone);
    sendEvent(fsm, ScanEvent::HomingZDone);
    sendEvent(fsm, ScanEvent::Start);
    REQUIRE(fsm.state() == ScanState::AdaptivePositioning);
}

TEST_CASE("ADAPTIVE_OK transitions to SCANNING", "[fsm][adaptive]")
{
    auto fsm = makeFsm();
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::HomingXDone);
    sendEvent(fsm, ScanEvent::HomingZDone);
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::AdaptiveOk);
    REQUIRE(fsm.state() == ScanState::Scanning);
}

TEST_CASE("SCAN_COMPLETE + MORE_LAYERS transitions to LAYER_COMPLETE", "[fsm][scanning]")
{
    auto fsm = makeFsm();
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::HomingXDone);
    sendEvent(fsm, ScanEvent::HomingZDone);
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::AdaptiveOk);
    sendEvent(fsm, ScanEvent::ScanComplete);
    sendEvent(fsm, ScanEvent::MoreLayers);
    REQUIRE(fsm.state() == ScanState::LayerComplete);
}

TEST_CASE("LAYER_COMPLETE transitions back to ADAPTIVE_POSITIONING", "[fsm][scanning]")
{
    auto fsm = makeFsm();
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::HomingXDone);
    sendEvent(fsm, ScanEvent::HomingZDone);
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::AdaptiveOk);
    sendEvent(fsm, ScanEvent::ScanComplete);
    sendEvent(fsm, ScanEvent::MoreLayers);
    sendEvent(fsm, ScanEvent::ZStepDone);
    REQUIRE(fsm.state() == ScanState::AdaptivePositioning);
}

TEST_CASE("SCAN_COMPLETE + LAST_LAYER transitions to PROCESSING", "[fsm][scanning]")
{
    auto fsm = makeFsm();
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::HomingXDone);
    sendEvent(fsm, ScanEvent::HomingZDone);
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::AdaptiveOk);
    sendEvent(fsm, ScanEvent::ScanComplete);
    sendEvent(fsm, ScanEvent::LastLayer);
    REQUIRE(fsm.state() == ScanState::Processing);
}

TEST_CASE("PROCESSING_DONE transitions to COMPLETE", "[fsm][processing]")
{
    auto fsm = makeFsm();
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::HomingXDone);
    sendEvent(fsm, ScanEvent::HomingZDone);
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::AdaptiveOk);
    sendEvent(fsm, ScanEvent::ScanComplete);
    sendEvent(fsm, ScanEvent::LastLayer);
    sendEvent(fsm, ScanEvent::ProcessingDone);
    REQUIRE(fsm.state() == ScanState::Complete);
}

// ---------------------------------------------------------------------------
// Multi-layer: adaptive loop tekrar çalışıyor mu?
// ---------------------------------------------------------------------------

TEST_CASE("Multi-layer: adaptive runs again after each layer", "[fsm][multilayer]")
{
    auto fsm = makeFsm();
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::HomingXDone);
    sendEvent(fsm, ScanEvent::HomingZDone);
    sendEvent(fsm, ScanEvent::Start);

    // Layer 1
    sendEvent(fsm, ScanEvent::AdaptiveOk);
    REQUIRE(fsm.state() == ScanState::Scanning);
    sendEvent(fsm, ScanEvent::ScanComplete);
    sendEvent(fsm, ScanEvent::MoreLayers);
    sendEvent(fsm, ScanEvent::ZStepDone);

    // Layer 2: adaptive tekrar çalışmalı
    REQUIRE(fsm.state() == ScanState::AdaptivePositioning);
    sendEvent(fsm, ScanEvent::AdaptiveOk);
    REQUIRE(fsm.state() == ScanState::Scanning);

    // Layer 2 son katman
    sendEvent(fsm, ScanEvent::ScanComplete);
    sendEvent(fsm, ScanEvent::LastLayer);
    REQUIRE(fsm.state() == ScanState::Processing);
}

// ---------------------------------------------------------------------------
// Error transitions: herhangi bir state'den error'a düşmeli
// ---------------------------------------------------------------------------

TEST_CASE("ESTOP from IDLE transitions to ERROR", "[fsm][error]")
{
    auto fsm = makeFsm();
    sendEvent(fsm, ScanEvent::EStop);
    REQUIRE(fsm.state() == ScanState::Error);
}

TEST_CASE("ESTOP from SCANNING transitions to ERROR", "[fsm][error]")
{
    auto fsm = makeFsm();
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::HomingXDone);
    sendEvent(fsm, ScanEvent::HomingZDone);
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::AdaptiveOk);
    sendEvent(fsm, ScanEvent::EStop);
    REQUIRE(fsm.state() == ScanState::Error);
}

TEST_CASE("ENCODER_TIMEOUT from HOMING_X transitions to ERROR", "[fsm][error]")
{
    auto fsm = makeFsm();
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::EncoderTimeout);
    REQUIRE(fsm.state() == ScanState::Error);
}

TEST_CASE("LASER_TIMEOUT from SCANNING transitions to ERROR", "[fsm][error]")
{
    auto fsm = makeFsm();
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::HomingXDone);
    sendEvent(fsm, ScanEvent::HomingZDone);
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::AdaptiveOk);
    sendEvent(fsm, ScanEvent::LaserTimeout);
    REQUIRE(fsm.state() == ScanState::Error);
}

TEST_CASE("UNEXPECTED_LIMIT from SCANNING transitions to ERROR", "[fsm][error]")
{
    auto fsm = makeFsm();
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::HomingXDone);
    sendEvent(fsm, ScanEvent::HomingZDone);
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::AdaptiveOk);
    sendEvent(fsm, ScanEvent::UnexpectedLimit);
    REQUIRE(fsm.state() == ScanState::Error);
}

TEST_CASE("ADAPTIVE_FAILED transitions to ERROR", "[fsm][error][adaptive]")
{
    auto fsm = makeFsm();
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::HomingXDone);
    sendEvent(fsm, ScanEvent::HomingZDone);
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::AdaptiveFailed);
    REQUIRE(fsm.state() == ScanState::Error);
}

// ---------------------------------------------------------------------------
// Error recovery: sadece RESET ile IDLE'a dönülür
// ---------------------------------------------------------------------------

TEST_CASE("RESET from ERROR transitions to IDLE", "[fsm][error][recovery]")
{
    auto fsm = makeFsm();
    sendEvent(fsm, ScanEvent::EStop);
    REQUIRE(fsm.state() == ScanState::Error);
    sendEvent(fsm, ScanEvent::Reset);
    REQUIRE(fsm.state() == ScanState::Idle);
}

TEST_CASE("After RESET, full scan sequence works again", "[fsm][error][recovery]")
{
    auto fsm = makeFsm();
    sendEvent(fsm, ScanEvent::EStop);
    sendEvent(fsm, ScanEvent::Reset);
    sendEvent(fsm, ScanEvent::Start);
    REQUIRE(fsm.state() == ScanState::HomingX);
}

// ---------------------------------------------------------------------------
// Guard conditions: yanlış event'ler state değiştirmemeli
// ---------------------------------------------------------------------------

TEST_CASE("HOMING_X_DONE from IDLE is ignored", "[fsm][guard]")
{
    auto fsm = makeFsm();
    sendEvent(fsm, ScanEvent::HomingXDone);
    REQUIRE(fsm.state() == ScanState::Idle);
}

TEST_CASE("SCAN_COMPLETE from IDLE is ignored", "[fsm][guard]")
{
    auto fsm = makeFsm();
    sendEvent(fsm, ScanEvent::ScanComplete);
    REQUIRE(fsm.state() == ScanState::Idle);
}

TEST_CASE("START from SCANNING is ignored", "[fsm][guard]")
{
    auto fsm = makeFsm();
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::HomingXDone);
    sendEvent(fsm, ScanEvent::HomingZDone);
    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::AdaptiveOk);
    sendEvent(fsm, ScanEvent::Start);
    REQUIRE(fsm.state() == ScanState::Scanning);
}

TEST_CASE("RESET from non-ERROR state is ignored", "[fsm][guard]")
{
    auto fsm = makeFsm();
    sendEvent(fsm, ScanEvent::Reset);
    REQUIRE(fsm.state() == ScanState::Idle);

    sendEvent(fsm, ScanEvent::Start);
    sendEvent(fsm, ScanEvent::Reset);
    REQUIRE(fsm.state() == ScanState::HomingX);
}
