// ============================================================
// Preciscan — Kullanici Yapilandirma Sabitleri (STM32)
// Algoritma & kinematik parametreleri buradan ayarlanir.
// ============================================================
#pragma once
#include <stdbool.h>
#include <stdint.h>

// ---- Motor Adim Zamanlama (mikrosaniye / adim) ----
static const uint32_t CFG_LIN_STEP_US     = 200UL;
static const uint32_t CFG_ROT_STEP_US     = 50000UL;   // 50ms/adim
static const uint32_t CFG_Z_STEP_US       = 800UL;
static const uint32_t CFG_Z_HOME_STEP_US  = 1200UL;

// ---- Adim / Devir ----
#define CFG_LIN_STEPS_PER_REV 3200.0f
#define CFG_ROT_STEPS_PER_REV 3200.0f
#define CFG_Z_STEPS_PER_REV   3200.0f

// ---- Vida Hatveleri (mm/devir) ----
#define CFG_LIN_LEAD_MM 2.0f
#define CFG_Z_LEAD_MM   2.0f

// ---- Turetilmis Kinematik ----
#define CFG_LIN_MM_PER_STEP  (CFG_LIN_LEAD_MM / CFG_LIN_STEPS_PER_REV)
#define CFG_Z_MM_PER_STEP    (CFG_Z_LEAD_MM   / CFG_Z_STEPS_PER_REV)
#define CFG_ROT_DEG_PER_STEP (360.0f / CFG_ROT_STEPS_PER_REV)

// ---- Tarama Tetik Cozunurlugu ----
#define CFG_TRIGGER_DEG        0.1f
#define CFG_ROT_STEPS_PER_TRIG (CFG_ROT_STEPS_PER_REV / (360.0f / CFG_TRIGGER_DEG))

// ---- Lineer Konumlandirma ----
#define CFG_LIN_HOME_DISTANCE_MM  119.525f   // Limit1 -> masa mesafesi
#define CFG_LIN_TARGET_DISTANCE_MM 78.0f     // Hedef mesafe
#define CFG_LIN_TRAVEL_MM (CFG_LIN_HOME_DISTANCE_MM - CFG_LIN_TARGET_DISTANCE_MM)

// ---- Varsayilan Yonler ----
static const bool CFG_LIN_HOME_DIR    = false;
static const bool CFG_ROT_DEFAULT_DIR = true;
static const bool CFG_Z_HOME_DIR      = true;

// ---- Debounce & Lazer ----
static const uint32_t CFG_DEBOUNCE_MS    = 200UL;
static const uint32_t CFG_LASER_PULSE_US = 2000UL;
