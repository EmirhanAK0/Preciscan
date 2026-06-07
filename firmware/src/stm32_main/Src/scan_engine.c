#include "scan_engine.h"
#include <math.h>

void scan_init(ScanEngine* eng) {
    stepper_init(&eng->rot_motor, BSP_ROT_MOTOR, CFG_ROT_STEP_US);
    laser_init(&eng->laser, CFG_LASER_PULSE_US);

    eng->step_accum        = 0.0f;
    eng->steps_per_trigger = CFG_ROT_STEPS_PER_TRIG;
    eng->total_steps       = 0;
    eng->direction         = CFG_ROT_DEFAULT_DIR;
    eng->running           = false;
}

void scan_start(ScanEngine* eng) {
    eng->running     = true;
    eng->step_accum  = 0.0f;
    eng->total_steps = 0;
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

    if (stepper_update(&eng->rot_motor, now_us)) {
        if (eng->direction) eng->total_steps++;
        else                eng->total_steps--;

        eng->step_accum += 1.0f;

        if (eng->step_accum >= eng->steps_per_trigger) {
            eng->step_accum -= eng->steps_per_trigger;
            
            laser_fire(&eng->laser);

            float a = (float)eng->total_steps * CFG_ROT_DEG_PER_STEP;
            // Pozitif aciya normalize et (0-360)
            a = fmodf(a, 360.0f);
            if (a < 0) a += 360.0f;
            
            if (angle_out) *angle_out = a;
            return true;
        }
    }
    return false;
}

void scan_laser_update(ScanEngine* eng, uint32_t now_us) {
    laser_update(&eng->laser, now_us);
}
