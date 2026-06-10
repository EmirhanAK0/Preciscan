#include "scan_engine.h"
#include "hal_encoder.h"
#include <math.h>

void scan_init(ScanEngine* eng) {
    stepper_init(&eng->rot_motor, BSP_ROT_MOTOR, CFG_ROT_STEP_US);
    laser_init(&eng->laser, CFG_LASER_PULSE_US);

    eng->angle_per_trigger = CFG_TRIGGER_DEG;
    eng->direction         = CFG_ROT_DEFAULT_DIR;
    eng->running           = false;
}

void scan_start(ScanEngine* eng, bool cw) {
    eng->running          = true;
    eng->direction        = cw;
    eng->scan_start_angle = encoder_get_angle();
    
    if (cw) {
        eng->next_trigger_angle = eng->scan_start_angle + eng->angle_per_trigger;
    } else {
        eng->next_trigger_angle = eng->scan_start_angle - eng->angle_per_trigger;
    }

    stepper_set_dir(&eng->rot_motor, eng->direction);
    stepper_enable(&eng->rot_motor);
}

void scan_stop(ScanEngine* eng) {
    eng->running = false;
    stepper_disable(&eng->rot_motor);
    laser_off(&eng->laser);
}

bool scan_update(ScanEngine* eng, uint32_t now_us, float* angle_out) {
    if (!eng->running) return false;

    stepper_update(&eng->rot_motor, now_us);

    float current_angle = encoder_get_angle();

    static uint32_t last_dbg_us = 0;
    if (now_us - last_dbg_us >= 500000) { // 500ms'de bir logla
        last_dbg_us = now_us;
        uart_comm_print_float("DEBUG_ANGLE:", current_angle, 2);
    }

    // Otomatik olarak 360 dereceyi doldurdugunda taramayi durdur
    if (fabs(current_angle - eng->scan_start_angle) >= 360.0f) {
        // Aslinda app_controller'in state'ini de degistirmek gerekir, 
        // ama scan_stop(eng) rotasyonu durduracaktir.
        scan_stop(eng);
        return false;
    }

    bool triggered = false;

    if (eng->direction) {
        if (current_angle >= eng->next_trigger_angle) {
            eng->next_trigger_angle += eng->angle_per_trigger;
            triggered = true;
        }
    } else {
        if (current_angle <= eng->next_trigger_angle) {
            eng->next_trigger_angle -= eng->angle_per_trigger;
            triggered = true;
        }
    }

    if (triggered) {
        laser_fire(&eng->laser);
        if (angle_out) *angle_out = current_angle;
        return true;
    }

    return false;
}

void scan_laser_update(ScanEngine* eng, uint32_t now_us) {
    laser_update(&eng->laser, now_us);
}
