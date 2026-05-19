// ============================================================
// Preciscan — Kullanici Yapilandirma Sabitleri
// Algoritma & kinematik parametreleri buradan ayarlanir.
// ============================================================
#pragma once

// ---- Motor Adim Zamanlama (mikrosaniye / adim) ----
static const unsigned long CFG_LIN_STEP_US     = 200UL;
static const unsigned long CFG_ROT_STEP_US     = 50000UL;   // 50ms/adim
static const unsigned long CFG_Z_STEP_US       = 800UL;
static const unsigned long CFG_Z_HOME_STEP_US  = 1200UL;

// ---- Adim / Devir ----
static const float CFG_LIN_STEPS_PER_REV = 3200.0f;
static const float CFG_ROT_STEPS_PER_REV = 3200.0f;
static const float CFG_Z_STEPS_PER_REV   = 3200.0f;

// ---- Vida Hatveleri (mm/devir) ----
static const float CFG_LIN_LEAD_MM = 2.0f;
static const float CFG_Z_LEAD_MM   = 2.0f;

// ---- Turetilmis Kinematik ----
static const float CFG_LIN_MM_PER_STEP  = CFG_LIN_LEAD_MM / CFG_LIN_STEPS_PER_REV;
static const float CFG_Z_MM_PER_STEP    = CFG_Z_LEAD_MM   / CFG_Z_STEPS_PER_REV;
static const float CFG_ROT_DEG_PER_STEP = 360.0f / CFG_ROT_STEPS_PER_REV;

// ---- Tarama Tetik Cozunurlugu ----
static const float CFG_TRIGGER_DEG        = 2.0f;
static const float CFG_ROT_STEPS_PER_TRIG = CFG_ROT_STEPS_PER_REV / (360.0f / CFG_TRIGGER_DEG);

// ---- Lineer Konumlandirma ----
static const float CFG_LIN_HOME_DISTANCE_MM  = 119.525f;   // Limit1 → masa mesafesi
static const float CFG_LIN_TARGET_DISTANCE_MM = 78.0f;     // Hedef mesafe
static const float CFG_LIN_TRAVEL_MM = CFG_LIN_HOME_DISTANCE_MM - CFG_LIN_TARGET_DISTANCE_MM;

// ---- Varsayilan Yonler ----
static const bool CFG_LIN_HOME_DIR    = false;
static const bool CFG_ROT_DEFAULT_DIR = true;
static const bool CFG_Z_HOME_DIR      = true;

// ---- Debounce & Lazer ----
static const unsigned long CFG_DEBOUNCE_MS    = 200UL;
static const unsigned long CFG_LASER_PULSE_US = 2000UL;
