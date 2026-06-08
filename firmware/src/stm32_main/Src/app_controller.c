#include "app_controller.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// ---- Dahili ileri bildirimler ----
static void process_command(AppController* app, const char* cmd);
static void handle_lin_event(AppController* app, AxisEvent evt);
static void handle_z_event(AppController* app, AxisEvent evt);
static void enter_fault(AppController* app, const char* msg);

// ==================================================================
// Baslatma
// ==================================================================
void app_init(AppController* app) {
    // Lineer eksen
    axis_init(&app->lin_axis,
              BSP_LIN_MOTOR, CFG_LIN_STEP_US,
              BSP_LIN_LIMITS, true,              // guvenlik limiti var
              CFG_LIN_MM_PER_STEP, CFG_LIN_HOME_DIR);

    // Z ekseni
    axis_init(&app->z_axis,
              BSP_Z_MOTOR, CFG_Z_STEP_US,
              BSP_Z_LIMITS, true,                // guvenlik limiti var
              CFG_Z_MM_PER_STEP, CFG_Z_HOME_DIR);

    // Tarama motoru
    scan_init(&app->scanner);

    app->state      = SYS_READY;
    memset(app->serial_buf, 0, sizeof(app->serial_buf));
    app->z_move_mm  = 0.0f;
}

// ==================================================================
// Ana guncelleme dongusu
// ==================================================================
void app_update(AppController* app, uint32_t now_ms, uint32_t now_us) {
    // ---- 1. Seri komut okuma ----
    if (uart_comm_read_line(app->serial_buf, sizeof(app->serial_buf))) {
        if (strlen(app->serial_buf) > 0) {
            process_command(app, app->serial_buf);
        }
    }

    if (app->state == SYS_FAULT) {
        // Hata durumundaysak hareket isleme
        return;
    }

    // ---- 2. Eksen guncellemeleri ----
    AxisEvent lin_evt = axis_update(&app->lin_axis, now_ms, now_us);
    AxisEvent z_evt   = axis_update(&app->z_axis,   now_ms, now_us);

    // ---- 3. Olay isleme ----
    if (lin_evt != AX_NONE) handle_lin_event(app, lin_evt);
    if (z_evt   != AX_NONE) handle_z_event(app, z_evt);

    // ---- 4. Tarama guncelleme ----
    if (app->state == SYS_SCANNING) {
        float angle;
        if (scan_update(&app->scanner, now_us, &angle)) {
            uart_comm_print_float("", angle, 2); // Aciyi PC'ye gonder
        }
    }

    // ---- 5. Lazer suresi kontrolu ----
    scan_laser_update(&app->scanner, now_us);
}

// ==================================================================
// Lineer eksen olay isleme
// ==================================================================
static void handle_lin_event(AppController* app, AxisEvent evt) {
    switch (evt) {
        case AX_HOME_FOUND:
            if (app->state == SYS_LIN_HOMING) {
                app->state = SYS_LIN_POSITIONING;
                uart_comm_println("STATUS:LIN_POSITIONING");

                long steps = (long)(CFG_LIN_TRAVEL_MM / CFG_LIN_MM_PER_STEP + 0.5f);
                axis_start_move_steps(&app->lin_axis, steps, !CFG_LIN_HOME_DIR);
            }
            break;

        case AX_TARGET_REACHED:
            if (app->state == SYS_LIN_POSITIONING) {
                float mm = app->lin_axis.position_steps * CFG_LIN_MM_PER_STEP;
                uart_comm_print_float("STATUS:LIN_POSITIONED:", mm, 1);

                app->state = SYS_Z_HOMING;
                uart_comm_println("STATUS:Z_HOMING");
                stepper_set_interval(&app->z_axis.motor, CFG_Z_HOME_STEP_US);
                axis_start_homing(&app->z_axis);
            }
            break;

        case AX_SAFETY_FAULT:
            enter_fault(app, "LIN_LIMIT2_HIT");
            break;

        default: break;
    }
}

// ==================================================================
// Z ekseni olay isleme
// ==================================================================
static void handle_z_event(AppController* app, AxisEvent evt) {
    switch (evt) {
        case AX_HOME_FOUND:
            if (app->state == SYS_Z_HOMING) {
                app->state = SYS_READY;
                uart_comm_println("STATUS:Z_HOMED");
                uart_comm_println("STATUS:READY");
            } else if (app->state == SYS_Z_MOVING) {
                app->state = SYS_READY;
                uart_comm_println("STATUS:Z_HIT_HOME_LIMIT");
                uart_comm_println("STATUS:READY");
            }
            break;

        case AX_TARGET_REACHED:
            if (app->state == SYS_Z_MOVING) {
                uart_comm_print_float("STATUS:Z_MOVED:", app->z_move_mm, 1);
                uart_comm_println("STATUS:READY");
                app->state = SYS_READY;
            }
            break;

        case AX_SAFETY_FAULT:
            enter_fault(app, "Z_LIMIT2_HIT");
            break;

        default: break;
    }
}

// ==================================================================
// Hata durumu
// ==================================================================
static void enter_fault(AppController* app, const char* msg) {
    axis_stop(&app->lin_axis);
    axis_stop(&app->z_axis);
    scan_stop(&app->scanner);
    app->state = SYS_FAULT;
    uart_comm_print("STATUS:FAULT:");
    uart_comm_println(msg);
}

// ==================================================================
// Seri komut isleme
// ==================================================================
static void start_lin_homing(AppController* app) {
    axis_stop(&app->z_axis);
    scan_stop(&app->scanner);
    app->state = SYS_LIN_HOMING;
    uart_comm_println("STATUS:LIN_HOMING");
    axis_start_homing(&app->lin_axis);
}

static void start_z_homing(AppController* app) {
    scan_stop(&app->scanner);
    axis_stop(&app->lin_axis);
    app->state = SYS_Z_HOMING;
    uart_comm_println("STATUS:Z_HOMING");
    stepper_set_interval(&app->z_axis.motor, CFG_Z_HOME_STEP_US);
    axis_start_homing(&app->z_axis);
}

static void start_z_move(AppController* app, float mm) {
    if (!app->z_axis.homed) {
        uart_comm_println("STATUS:ERR:Z_NOT_HOMED");
        return;
    }
    app->state      = SYS_Z_MOVING;
    app->z_move_mm  = mm;
    stepper_set_interval(&app->z_axis.motor, CFG_Z_STEP_US);
    uart_comm_print_float("STATUS:Z_MOVING:", mm, 1);
    axis_start_move_mm(&app->z_axis, mm);
}

static void start_scan(AppController* app, bool cw) {
    axis_stop(&app->lin_axis);
    axis_stop(&app->z_axis);
    app->state = SYS_SCANNING;
    uart_comm_println("STATUS:SCANNING");
    scan_start(&app->scanner, cw);
}

static void stop_scan(AppController* app) {
    scan_stop(&app->scanner);
    app->state = SYS_READY;
    uart_comm_println("STATUS:STOPPED");
    uart_comm_println("STATUS:READY");
}

static void process_command(AppController* app, const char* cmd) {
    if (strcmp(cmd, "HOME") == 0) {
        start_lin_homing(app);
    }
    else if (strcmp(cmd, "ZHOME") == 0) {
        start_z_homing(app);
    }
    else if (strncmp(cmd, "ZMOVE:", 6) == 0) {
        float mm = (float)atof(cmd + 6);
        if (app->state == SYS_READY || app->state == SYS_SCANNING) {
            if (app->state == SYS_SCANNING) stop_scan(app);
            start_z_move(app, mm);
        } else {
            uart_comm_println("STATUS:ERR:BUSY");
        }
    }
    else if (strcmp(cmd, "START_CW") == 0) {
        if (app->state == SYS_READY) start_scan(app, true);
        else uart_comm_println("STATUS:ERR:NOT_READY");
    }
    else if (strcmp(cmd, "START_CCW") == 0) {
        if (app->state == SYS_READY) start_scan(app, false);
        else uart_comm_println("STATUS:ERR:NOT_READY");
    }
    else if (strcmp(cmd, "STOP") == 0) {
        if (app->state == SYS_SCANNING) stop_scan(app);
    }
    else if (strcmp(cmd, "RESET") == 0) {
        start_lin_homing(app);
    }
    else if (strcmp(cmd, "STATUS") == 0) {
        char buf[64];
        // STATE numarasini ve kalan adimlari STATUS: onekiyle bildir
        // boylece PC parser tutarli formatta okur
        const char* state_str = "UNKNOWN";
        switch (app->state) {
            case SYS_READY:         state_str = "READY";         break;
            case SYS_LIN_HOMING:    state_str = "LIN_HOMING";    break;
            case SYS_LIN_POSITIONING: state_str = "LIN_POSITIONING"; break;
            case SYS_Z_HOMING:      state_str = "Z_HOMING";      break;
            case SYS_SCANNING:      state_str = "SCANNING";      break;
            case SYS_FAULT:         state_str = "FAULT";         break;
            default: break;
        }
        snprintf(buf, sizeof(buf), "STATUS:%s", state_str);
        uart_comm_println(buf);
    }
}
