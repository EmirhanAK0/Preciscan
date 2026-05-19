#include "motion_axis.h"
#include <math.h>

void axis_init(Axis* ax,
               const MotorPins& mp, unsigned long step_us,
               const LimitPins& lp, bool has_safety,
               float mm_per_step, bool home_dir) {
    // Step motor
    stepper_init(&ax->motor, mp.step, mp.dir, step_us);

    // Home limit — her zaman var
    limit_init(&ax->home_limit, lp.home, CFG_DEBOUNCE_MS);

    // Guvenlik limiti — opsiyonel
    ax->has_safety = has_safety;
    if (has_safety) {
        limit_init(&ax->safety_limit, lp.safety, CFG_DEBOUNCE_MS);
    }

    ax->mm_per_step     = mm_per_step;
    ax->home_direction  = home_dir;
    ax->mode            = AX_IDLE;
    ax->homed           = false;
    ax->target_steps    = 0;
    ax->position_steps  = 0;
}

void axis_start_homing(Axis* ax) {
    ax->mode = AX_HOMING;
    ax->target_steps = 0;
    stepper_set_dir(&ax->motor, ax->home_direction);
    stepper_enable(&ax->motor);
}

void axis_start_move_steps(Axis* ax, long steps, bool direction) {
    if (steps <= 0) return;
    ax->target_steps = steps;
    ax->mode = AX_TARGET_MOVE;
    stepper_set_dir(&ax->motor, direction);
    stepper_enable(&ax->motor);
}

void axis_start_move_mm(Axis* ax, float mm) {
    long steps = (long)(fabs(mm) / ax->mm_per_step + 0.5f);
    if (steps == 0) return;
    bool dir = (mm > 0) ? !ax->home_direction : ax->home_direction;
    axis_start_move_steps(ax, steps, dir);
}

void axis_stop(Axis* ax) {
    stepper_disable(&ax->motor);
    ax->mode = AX_IDLE;
}

AxisEvent axis_update(Axis* ax, unsigned long now_ms, unsigned long now_us) {
    // --- Limit switch'leri guncelle ---
    limit_update(&ax->home_limit, now_ms);
    if (ax->has_safety) {
        limit_update(&ax->safety_limit, now_ms);
    }

    // --- Guvenlik limiti kontrolu (her modda) ---
    if (ax->has_safety && ax->motor.enabled && limit_is_triggered(&ax->safety_limit)) {
        stepper_disable(&ax->motor);
        ax->mode = AX_IDLE;
        return AX_SAFETY_FAULT;
    }

    // --- Moda gore davranis ---
    switch (ax->mode) {
        case AX_HOMING:
            // Home limiti tetiklendiyse dur
            if (limit_is_triggered(&ax->home_limit)) {
                stepper_disable(&ax->motor);
                ax->position_steps = 0;
                ax->homed = true;
                ax->mode = AX_IDLE;
                return AX_HOME_FOUND;
            }
            stepper_update(&ax->motor, now_us);
            break;

        case AX_TARGET_MOVE:
            // Hareket ederken home limitine carparsa dur (guvenlik)
            if (ax->motor.direction == ax->home_direction &&
                limit_is_triggered(&ax->home_limit)) {
                stepper_disable(&ax->motor);
                ax->position_steps = 0;
                ax->mode = AX_IDLE;
                return AX_HOME_FOUND;  // Home'a vurdu, konum sifirlandi
            }
            // Step at ve geri sayimi azalt
            if (stepper_update(&ax->motor, now_us)) {
                // Konum takibi
                if (ax->motor.direction == ax->home_direction)
                    ax->position_steps--;
                else
                    ax->position_steps++;

                ax->target_steps--;
                if (ax->target_steps <= 0) {
                    stepper_disable(&ax->motor);
                    ax->mode = AX_IDLE;
                    return AX_TARGET_REACHED;
                }
            }
            break;

        case AX_IDLE:
        default:
            break;
    }

    return AX_NONE;
}
