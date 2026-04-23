/**
 * firmware/src/encoder_trigger.c
 * 
 * Encoder-based trigger generation for laser profiling.
 * Fires on encoder edges, sends TriggerEvent to PC via UART.
 * 
 * Architecture:
 * - Encoder interrupt (ISR) increments counter
 * - Trigger logic: every N encoder counts → trigger edge
 * - On trigger: fill TriggerEvent, serialize, UART Tx
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// ============================================================================
// State & Configuration
// ============================================================================

typedef struct {
    uint32_t session_id;
    uint32_t seq;
    uint32_t encoder_offset;        // Encoder count at session start (for zeroing)
    uint32_t encoder_steps_per_revolution;
    uint32_t trigger_step;          // Encoder counts between triggers
} TriggerSession;

static TriggerSession g_session = {0};
static bool g_session_active = false;

// Current encoder count (updated by ISR)
static volatile uint32_t g_encoder_count = 0;
static volatile uint32_t g_encoder_count_at_last_trigger = 0;

// Tick counter (microsecond resolution)
// Assume a 1µs timer tick source available (e.g., SysTick or TIM)
static volatile uint64_t g_tick_us = 0;

// UART transmit callback (hardware-specific, must be implemented by HAL)
extern void uart_send_bytes(const uint8_t* data, size_t len);

// ============================================================================
// TriggerEvent Serialization (Protobuf manual)
// ============================================================================

/**
 * Encode a TriggerEvent to binary (Protobuf wire format, little-endian).
 * Returns number of bytes written.
 * 
 * Wire format (Protobuf varint + fixed-length fields):
 * [0:4]   session_id (field 1, type fixed32) → 4 bytes
 * [4:8]   seq (field 2, type fixed32) → 4 bytes
 * [8:12]  encoder_count (field 3, type fixed32) → 4 bytes
 * [12:16] angle_mdeg (field 4, type sfixed32) → 4 bytes
 * [16:24] tick_us (field 5, type fixed64) → 8 bytes
 * Total: 24 bytes
 */
static size_t encode_trigger_event(
    uint8_t* buf,
    uint32_t session_id,
    uint32_t seq,
    uint32_t encoder_count,
    int32_t angle_mdeg,
    uint64_t tick_us)
{
    // Manual varint/fixed encoding (little-endian)
    // Field 1: session_id (fixed32)
    buf[0] = (session_id >>  0) & 0xFF;
    buf[1] = (session_id >>  8) & 0xFF;
    buf[2] = (session_id >> 16) & 0xFF;
    buf[3] = (session_id >> 24) & 0xFF;
    
    // Field 2: seq (fixed32)
    buf[4] = (seq >>  0) & 0xFF;
    buf[5] = (seq >>  8) & 0xFF;
    buf[6] = (seq >> 16) & 0xFF;
    buf[7] = (seq >> 24) & 0xFF;
    
    // Field 3: encoder_count (fixed32)
    buf[8]  = (encoder_count >>  0) & 0xFF;
    buf[9]  = (encoder_count >>  8) & 0xFF;
    buf[10] = (encoder_count >> 16) & 0xFF;
    buf[11] = (encoder_count >> 24) & 0xFF;
    
    // Field 4: angle_mdeg (sfixed32, signed)
    buf[12] = (angle_mdeg >>  0) & 0xFF;
    buf[13] = (angle_mdeg >>  8) & 0xFF;
    buf[14] = (angle_mdeg >> 16) & 0xFF;
    buf[15] = (angle_mdeg >> 24) & 0xFF;
    
    // Field 5: tick_us (fixed64)
    buf[16] = (tick_us >>  0) & 0xFF;
    buf[17] = (tick_us >>  8) & 0xFF;
    buf[18] = (tick_us >> 16) & 0xFF;
    buf[19] = (tick_us >> 24) & 0xFF;
    buf[20] = (tick_us >> 32) & 0xFF;
    buf[21] = (tick_us >> 40) & 0xFF;
    buf[22] = (tick_us >> 48) & 0xFF;
    buf[23] = (tick_us >> 56) & 0xFF;
    
    return 24;
}

// ============================================================================
// Trigger Firing
// ============================================================================

/**
 * Called when a hardware trigger event occurs (encoder-based).
 * Fills TriggerEvent and sends via UART + Envelope.
 */
static void fire_trigger(void)
{
    if (!g_session_active) {
        return;
    }
    
    // Compute angle from encoder
    uint32_t encoder_rel = g_encoder_count - g_session.encoder_offset;
    // angle = (encoder_rel / steps_per_rev) * 360000 mdeg
    uint32_t angle_mdeg = (uint64_t)encoder_rel * 360000 / g_session.encoder_steps_per_revolution;
    angle_mdeg %= 360000;  // Wrap around
    
    // Encode payload
    uint8_t payload[24];
    size_t payload_len = encode_trigger_event(
        payload,
        g_session.session_id,
        g_session.seq,
        g_encoder_count,
        (int32_t)angle_mdeg,
        g_tick_us
    );
    
    // Wrap in Envelope (msg_id for TriggerEvent, e.g., 0x10)
    // Format: [msg_id (varint)] + [payload_len (varint)] + [payload bytes]
    // For simplicity, use fixed framing: [0x10] [payload...]
    uint8_t frame[32];
    frame[0] = 0x10;  // TriggerEvent msg_id
    memcpy(frame + 1, payload, payload_len);
    
    // Send
    uart_send_bytes(frame, 1 + payload_len);
    
    // Increment for next trigger
    g_session.seq++;
    g_encoder_count_at_last_trigger = g_encoder_count;
}

// ============================================================================
// Encoder ISR
// ============================================================================

/**
 * ISR handler for encoder edge (rising or falling, based on config).
 * Increments counter, checks trigger condition, fires if needed.
 */
void encoder_isr_handler(void)
{
    g_encoder_count++;
    
    if (!g_session_active) {
        return;
    }
    
    // Check if trigger threshold reached
    uint32_t counts_since_last = g_encoder_count - g_encoder_count_at_last_trigger;
    if (counts_since_last >= g_session.trigger_step) {
        fire_trigger();
    }
}

/**
 * Timer ISR for tick counter (e.g., 1µs per tick).
 * Call from SysTick or dedicated 1µs timer.
 */
void tick_isr_handler(void)
{
    g_tick_us++;
}

// ============================================================================
// Public API
// ============================================================================

/**
 * Start a scan session. Called when ScanStart command arrives.
 * 
 * @param scan_id Session ID (from ScanStart.scan_id)
 * @param step_angle_mdeg Angle between triggers (e.g., 1000 = 1 degree)
 * @param encoder_steps_per_rev Encoder counts per full rotation (e.g., 3600 for 0.1° resolution)
 */
void trigger_session_start(
    uint32_t scan_id,
    uint32_t step_angle_mdeg,
    uint32_t encoder_steps_per_rev)
{
    g_session.session_id = scan_id;
    g_session.seq = 0;
    g_session.encoder_offset = g_encoder_count;
    g_session.encoder_steps_per_revolution = encoder_steps_per_rev;
    
    // Compute trigger step in encoder counts
    // trigger_step = (step_angle_mdeg / 360000) * steps_per_rev
    g_session.trigger_step = (uint64_t)step_angle_mdeg * encoder_steps_per_rev / 360000;
    if (g_session.trigger_step == 0) {
        g_session.trigger_step = 1;  // At least 1 count
    }
    
    g_encoder_count_at_last_trigger = g_encoder_count;
    g_session_active = true;
}

/**
 * Stop the current scan session. Called when ScanStop command arrives.
 */
void trigger_session_stop(void)
{
    g_session_active = false;
    memset(&g_session, 0, sizeof(g_session));
}

/**
 * Get current encoder count (for debugging).
 */
uint32_t trigger_get_encoder_count(void)
{
    return g_encoder_count;
}

/**
 * Get current tick counter (for debugging).
 */
uint64_t trigger_get_tick(void)
{
    return g_tick_us;
}

/**
 * Reset tick counter (call at session start for clean timestamps).
 */
void trigger_reset_tick(void)
{
    g_tick_us = 0;
}

// ============================================================================
// MCU Command Handlers (integrate into main command dispatcher)
// ============================================================================

/**
 * Called when ScanStart command received (from PC via UART).
 * Extract parameters and start trigger session.
 */
void handle_scan_start(const ScanStart* cmd)
{
    // cmd->scan_id, cmd->step_angle_mdeg, encoder_steps_per_rev from config
    // Assume encoder_steps_per_rev is configured globally or stored in EEPROM
    uint32_t encoder_steps_per_rev = 3600;  // TODO: read from config
    
    trigger_session_start(cmd->scan_id, cmd->step_angle_mdeg, encoder_steps_per_rev);
    trigger_reset_tick();
}

/**
 * Called when ScanStop command received.
 */
void handle_scan_stop(const ScanStop* cmd)
{
    (void)cmd;
    trigger_session_stop();
}

// ============================================================================
// Notes for Integration
// ============================================================================

/*
 * 1. Encoder ISR:
 *    - Connect to encoder pulse output (GPIO pin, configured for rising/falling edge)
 *    - ISR handler calls encoder_isr_handler()
 *
 * 2. Tick Timer:
 *    - Use SysTick (STM32) or dedicated timer with 1µs period
 *    - ISR calls tick_isr_handler()
 *    - Or use HAL timer callback if available
 *
 * 3. UART Send:
 *    - uart_send_bytes() must be implemented by HAL
 *    - Should be non-blocking or buffered
 *    - If using DMA, ensure payload is stable (copy to frame buffer)
 *
 * 4. Configuration:
 *    - encoder_steps_per_rev: depends on hardware (read from EEPROM or #define)
 *    - step_angle_mdeg: from ScanStart command
 *    - baudrate: 115200 typical (match PC side)
 *
 * 5. Testing:
 *    - Use trigger_test.c to inject fake encoder counts
 *    - Verify TriggerEvent serialization with golden frames
 *    - Check timing accuracy with oscilloscope
 */
