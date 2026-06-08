#include "scan_engine.h"
#include "bsp_pins.h"

void scan_init(ScanEngine* eng) {
    stepper_init(&eng->rot_motor, BSP_ROT_MOTOR.step, BSP_ROT_MOTOR.dir,
                 CFG_ROT_STEP_US);
    laser_init(&eng->laser, BSP_LASER_PIN, CFG_LASER_PULSE_US);

    eng->step_accum       = 0.0f;
    eng->steps_per_trigger = CFG_ROT_STEPS_PER_TRIG;
    eng->total_steps      = 0;
    eng->direction        = CFG_ROT_DEFAULT_DIR;
    eng->running          = false;
}

void scan_start(ScanEngine* eng, bool cw) {
    laser_off(&eng->laser);
    eng->total_steps = 0;
    eng->step_accum  = 0.0f;
    eng->direction   = cw;

    stepper_set_dir(&eng->rot_motor, eng->direction);
    stepper_enable(&eng->rot_motor);
    eng->running = true;
}

void scan_stop(ScanEngine* eng) {
    stepper_disable(&eng->rot_motor);
    laser_off(&eng->laser);
    eng->running = false;
}

static float steps_to_deg(long steps) {
    float deg = steps * CFG_ROT_DEG_PER_STEP;
    return deg;
}

bool scan_update(ScanEngine* eng, unsigned long now_us, float* angle_out) {
    if (!eng->running) return false;

    // Rotasyon adimi at
    if (!stepper_update(&eng->rot_motor, now_us)) return false;

    // Adim sayacini guncelle
    if (eng->direction) eng->total_steps++;
    else                eng->total_steps--;

    // Tetik biriktirici
    eng->step_accum += 1.0f;
    if (eng->step_accum >= eng->steps_per_trigger) {
        eng->step_accum -= eng->steps_per_trigger;

        // Lazeri tetikle
        laser_fire(&eng->laser);

        // Aciyi hesapla ve bildir
        *angle_out = steps_to_deg(eng->total_steps);
        return true;
    }

    return false;
}

void scan_laser_update(ScanEngine* eng, unsigned long now_us) {
    laser_update(&eng->laser, now_us);
}
