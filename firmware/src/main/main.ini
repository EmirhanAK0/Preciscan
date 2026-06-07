// ============================================================
// Preciscan — Firmware v4.0 (HAL Mimarisi)
// 
// Bu dosya sadece setup() ve loop() icerir.
// Tum is mantigi app_controller modulu tarafindan yonetilir.
// Pin tanimlari       → bsp_pins.h
// Yapilandirma        → preciscan_config.h
// Donanim suruculeri  → hal_stepper / hal_limit_switch / hal_laser
// Hareket kontrolu    → motion_axis / scan_engine
// Durum & Komutlar    → app_controller
// ============================================================

#include "app_controller.h"
#include "bsp_pins.h"

AppController app;

void setup() {
    Serial.begin(BSP_SERIAL_BAUD);
    app_init(&app);
    Serial.println("STATUS:BOOT_OK_WAITING_COMMAND");
}

void loop() {
    app_update(&app, millis(), micros());
}