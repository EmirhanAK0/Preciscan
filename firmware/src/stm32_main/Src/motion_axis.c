#include "motion_axis.h"

void axis_init(Axis* ax,
               MotorPins mp, uint32_t step_us,
               LimitPins lp, bool has_safety,
               float mm_per_step, bool home_dir) {

    stepper_init(&ax->motor, mp, step_us);
    limit_init(&ax->home_limit, lp.home, CFG_DEBOUNCE_MS);
    
    ax->has_safety = has_safety;
    if (has_safety) {
        limit_init(&ax->safety_limit, lp.safety, CFG_DEBOUNCE_MS);
    }

    ax->mm_per_step    = mm_per_step;
    ax->home_direction = home_dir;
    ax->mode           = AX_IDLE;
    ax->homed          = false;
    ax->target_steps   = 0;
    ax->position_steps = 0;
}

void axis_start_homing(Axis* ax) {
    ax->mode  = AX_HOMING;
    ax->homed = false;
    stepper_set_dir(&ax->motor, ax->home_direction);
    stepper_enable(&ax->motor);
}

void axis_start_move_steps(Axis* ax, long steps, bool direction) {
    ax->mode         = AX_TARGET_MOVE;
    ax->target_steps = steps;
    stepper_set_dir(&ax->motor, direction);
    stepper_enable(&ax->motor);
}

void axis_start_move_mm(Axis* ax, float mm) {
    long steps = (long)(mm / ax->mm_per_step + 0.5f);
    
    // mm pozitifse home yonunun tersine, negatifse home yonune (veya projeye gore ayarlanir)
    // Su anki mantikta: mm pozitif = ileri, negatif = geri.
    bool dir = (mm > 0) ? !ax->home_direction : ax->home_direction;
    if (mm < 0) steps = -steps;

    axis_start_move_steps(ax, steps, dir);
}

void axis_stop(Axis* ax) {
    stepper_disable(&ax->motor);
    ax->mode = AX_IDLE;
}

AxisEvent axis_update(Axis* ax, uint32_t now_ms, uint32_t now_us) {
    limit_update(&ax->home_limit, now_ms);
    if (ax->has_safety) {
        limit_update(&ax->safety_limit, now_ms);
    }

    // Guvenlik siniri - Yalnizca eksen hareket halindeyken (IDLE degilken) kontrol et
    if (ax->has_safety && ax->mode != AX_IDLE && limit_is_triggered(&ax->safety_limit)) {
        axis_stop(ax);
        return AX_SAFETY_FAULT;
    }

    // Homing Modu
    if (ax->mode == AX_HOMING) {
        if (limit_is_triggered(&ax->home_limit)) {
            axis_stop(ax);
            ax->homed          = true;
            ax->position_steps = 0;
            stepper_reset_count(&ax->motor);
            return AX_HOME_FOUND;
        } else {
            stepper_update(&ax->motor, now_us);
        }
    }
    // Hedefe Gitme Modu
    else if (ax->mode == AX_TARGET_MOVE) {
        // Hedefe giderken home limitine carpma korumasi (Sadece home yonune gidiyorsak gecerli)
        if (ax->motor.direction == ax->home_direction && limit_is_triggered(&ax->home_limit)) {
            axis_stop(ax);
            ax->homed          = true;
            ax->position_steps = 0;
            stepper_reset_count(&ax->motor);
            return AX_HOME_FOUND; 
        }

        if (ax->target_steps > 0) {
            if (stepper_update(&ax->motor, now_us)) {
                ax->target_steps--;
                
                // Pozisyon guncelle
                if (ax->motor.direction == ax->home_direction) ax->position_steps--;
                else ax->position_steps++;
            }
        } else {
            axis_stop(ax);
            return AX_TARGET_REACHED;
        }
    }

    return AX_NONE;
}
