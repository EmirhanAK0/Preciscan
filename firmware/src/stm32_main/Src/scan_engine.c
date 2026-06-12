#include "scan_engine.h"
#include "hal_encoder.h"
#include "hal_time.h"
#include "uart_comm.h"
#include <math.h>

void scan_init(ScanEngine* eng) {
    stepper_init(&eng->rot_motor, BSP_ROT_MOTOR, CFG_ROT_STEP_US);
    laser_init(&eng->laser, CFG_LASER_PULSE_US);

    eng->angle_per_trigger = CFG_TRIGGER_DEG;
    eng->direction         = CFG_ROT_DEFAULT_DIR;
    eng->running           = false;
    eng->seq               = 0;
    eng->session_id        = 0;
}

void scan_start(ScanEngine* eng, bool cw) {
    eng->running          = true;
    eng->direction        = cw;
    eng->scan_start_angle = encoder_get_angle();
    eng->seq              = 0;
    eng->session_id++;
    stepper_reset_count(&eng->rot_motor);

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

    float current_angle;
    if (eng->trigger_src == 1) {
        current_angle = eng->scan_start_angle + (eng->rot_motor.step_count * CFG_ROT_DEG_PER_STEP);
    } else {
        current_angle = encoder_get_angle();
    }

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

    bool due = eng->direction ? (current_angle >= eng->next_trigger_angle)
                              : (current_angle <= eng->next_trigger_angle);
    if (!due) return false;

    /* AS5600 kaynaginda aci bayatligini gider: enkoder ana donguden 5ms'de bir
       okunuyor; tetik karari bayat aciyla verilmis olabilir. Lazer profili
       SIMDI cekecegi icin, raporlanan acinin da ayni ana ait olmasi gerekir.
       Tetiklemeden hemen once tek bir taze okuma yapiyoruz (tetik basina
       1 ekstra I2C okumasi; <=60 Hz tetikte ihmal edilebilir yuk). */
    if (eng->trigger_src != 1) {
        encoder_update();
        current_angle = encoder_get_angle();
    }

    /* Coklu esik korumasi: aci bir okumada birden fazla esik atladiysa,
       eski "+= adim" mantigi ayni anda pesi sira ikinci bir tetik uretir ve
       pulse hala HIGH oldugundan lazerde kenar olusmazdi (1 profil, 2 aci →
       kalici eslesme kaymasi). Esigi mevcut aciya gore yeniden senkronluyoruz:
       her tetikte tam olarak 1 profil + 1 aci paketi garanti. */
    if (eng->direction) {
        eng->next_trigger_angle = current_angle + eng->angle_per_trigger;
    } else {
        eng->next_trigger_angle = current_angle - eng->angle_per_trigger;
    }

    /* Yukselen kenar garantisi: onceki pulse henuz bitmediyse pini LOW'a
       cekip kisa bekle; lazer ancak LOW->HIGH gecisinde profil alir. */
    if (laser_is_active(&eng->laser)) {
        laser_off(&eng->laser);
        hal_delay_us(10);
    }

    laser_fire(&eng->laser);
    eng->seq++;
    if (angle_out) *angle_out = current_angle;
    return true;
}

void scan_laser_update(ScanEngine* eng, uint32_t now_us) {
    laser_update(&eng->laser, now_us);
}
