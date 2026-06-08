#include "app_controller.h"
#include "bsp_pins.h"
#include <math.h>

// ---- Dahili ileri bildirimler ----
static void process_command(AppController* app, const String& cmd);
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
    app->serial_buf = "";
    app->z_move_mm  = 0.0f;
}

// ==================================================================
// Ana guncelleme dongusu
// ==================================================================
void app_update(AppController* app, unsigned long now_ms, unsigned long now_us) {
    // ---- 1. Seri komut okuma ----
    while (Serial.available()) {
        char c = (char)Serial.read();
        if (c == '\n') {
            app->serial_buf.trim();
            if (app->serial_buf.length() > 0) {
                process_command(app, app->serial_buf);
            }
            app->serial_buf = "";
        } else {
            app->serial_buf += c;
        }
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
            Serial.println(angle, 2);    // Aciyi PC'ye gonder
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
                // Home bulundu → hedef konuma git
                app->state = SYS_LIN_POSITIONING;
                Serial.println("STATUS:LIN_POSITIONING");

                // Hedef mesafeye kadar adim sayisi hesapla
                long steps = (long)(CFG_LIN_TRAVEL_MM / CFG_LIN_MM_PER_STEP + 0.5f);
                axis_start_move_steps(&app->lin_axis, steps, !CFG_LIN_HOME_DIR);
            }
            break;

        case AX_TARGET_REACHED:
            if (app->state == SYS_LIN_POSITIONING) {
                float mm = app->lin_axis.position_steps * CFG_LIN_MM_PER_STEP;
                Serial.print("STATUS:LIN_POSITIONED:");
                Serial.println(mm, 1);

                // Otomatik Z homing baslatili
                app->state = SYS_Z_HOMING;
                Serial.println("STATUS:Z_HOMING");
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
                Serial.println("STATUS:Z_HOMED\nSTATUS:READY");
            } else if (app->state == SYS_Z_MOVING) {
                // Hareket sirasinda home limitine vurdu
                app->state = SYS_READY;
                Serial.println("STATUS:Z_HIT_HOME_LIMIT\nSTATUS:READY");
            }
            break;

        case AX_TARGET_REACHED:
            if (app->state == SYS_Z_MOVING) {
                Serial.print("STATUS:Z_MOVED:");
                Serial.println(app->z_move_mm, 1);
                Serial.println("STATUS:READY");
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
    Serial.print("STATUS:FAULT:");
    Serial.println(msg);
}

// ==================================================================
// Seri komut isleme
// ==================================================================
static void start_lin_homing(AppController* app) {
    axis_stop(&app->z_axis);
    scan_stop(&app->scanner);
    app->state = SYS_LIN_HOMING;
    Serial.println("STATUS:LIN_HOMING");
    axis_start_homing(&app->lin_axis);
}

static void start_z_homing(AppController* app) {
    scan_stop(&app->scanner);
    axis_stop(&app->lin_axis);
    app->state = SYS_Z_HOMING;
    Serial.println("STATUS:Z_HOMING");
    stepper_set_interval(&app->z_axis.motor, CFG_Z_HOME_STEP_US);
    axis_start_homing(&app->z_axis);
}

static void start_z_move(AppController* app, float mm) {
    if (!app->z_axis.homed) {
        Serial.println("STATUS:ERR:Z_NOT_HOMED");
        return;
    }
    app->state      = SYS_Z_MOVING;
    app->z_move_mm  = mm;
    stepper_set_interval(&app->z_axis.motor, CFG_Z_STEP_US);
    Serial.print("STATUS:Z_MOVING:");
    Serial.println(mm, 1);
    axis_start_move_mm(&app->z_axis, mm);
}

static void start_scan(AppController* app, bool cw) {
    axis_stop(&app->lin_axis);
    axis_stop(&app->z_axis);
    app->state = SYS_SCANNING;
    Serial.println("STATUS:SCANNING");
    scan_start(&app->scanner, cw);
}

static void stop_scan(AppController* app) {
    scan_stop(&app->scanner);
    app->state = SYS_READY;
    Serial.println("STATUS:STOPPED\nSTATUS:READY");
}

static void process_command(AppController* app, const String& cmd) {
    if (cmd == "HOME") {
        start_lin_homing(app);
    }
    else if (cmd == "ZHOME") {
        start_z_homing(app);
    }
    else if (cmd.startsWith("ZMOVE:")) {
        float mm = cmd.substring(6).toFloat();
        if (app->state == SYS_READY || app->state == SYS_SCANNING) {
            if (app->state == SYS_SCANNING) stop_scan(app);
            start_z_move(app, mm);
        } else {
            Serial.println("STATUS:ERR:BUSY");
        }
    }
    else if (cmd == "START_CW") {
        if (app->state == SYS_READY) start_scan(app, true);
        else Serial.println("STATUS:ERR:NOT_READY");
    }
    else if (cmd == "START_CCW") {
        if (app->state == SYS_READY) start_scan(app, false);
        else Serial.println("STATUS:ERR:NOT_READY");
    }
    else if (cmd == "STOP") {
        if (app->state == SYS_SCANNING) stop_scan(app);
    }
    else if (cmd == "RESET") {
        start_lin_homing(app);
    }
    else if (cmd == "STATUS") {
        // Durum sorgusu — ileride genisletilebilir
    }
}
