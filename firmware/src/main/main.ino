// ============================================================
// Preciscan -- Tam Entegre Firmware v3.5 
// Guncelleme: A0 pini üzerinden Lazer Tetikleme (gecikmesiz) 
// sisteme tekrar eklendi.
// ============================================================

#include <math.h>

// ---------------------------------------------------------------
// GUVENLI PINLER
// ---------------------------------------------------------------
#define Z_STEP_PIN      3
#define Z_DIR_PIN       4
#define ROT_STEP_PIN    9
#define ROT_DIR_PIN     10
#define LIN_STEP_PIN    11  
#define LIN_DIR_PIN     12  

#define Z_LIMIT_1       7   // Z home (ust) - Duzeltilmis Pin
#define Z_LIMIT_2       8   // Z alt guvenlik - Duzeltilmis Pin
#define LIN_LIMIT_1     5   // Lineer home
#define LIN_LIMIT_2     6   // Lineer guvenlik

#define LASER_PIN       A0  // Lazer tetik pini eklendi

// ---------------------------------------------------------------
// YON SABITLERI VE MOTOR HIZLARI
// ---------------------------------------------------------------
const uint8_t DIR_FORWARD = LOW;
const uint8_t DIR_REVERSE = HIGH;

const unsigned long LIN_STEP_US     = 800UL;     
const unsigned long ROT_STEP_US     = 40000UL;   
const unsigned long Z_STEP_US       = 800UL;     
const unsigned long Z_HOME_STEP_US  = 1200UL;    

// ---------------------------------------------------------------
// HESAPLAMALAR
// ---------------------------------------------------------------
const float LIN_STEPS_PER_REV  = 3200.0f; 
const float ROT_STEPS_PER_REV  = 3200.0f;
const float Z_STEPS_PER_REV    = 3200.0f;

const float ROT_DEG_PER_STEP       = 360.0f / ROT_STEPS_PER_REV;
const float ROT_STEPS_PER_HALF_DEG = ROT_STEPS_PER_REV / 720.0f;

const float LIN_LEAD_MM     = 2.0f; 
const float LIN_MM_PER_STEP = LIN_LEAD_MM / LIN_STEPS_PER_REV;
const float Z_LEAD_MM     = 2.0f;
const float Z_MM_PER_STEP = Z_LEAD_MM / Z_STEPS_PER_REV;

const float LIN_LIMIT1_MASA_UZAKLIGI = 119.525f; 
const float LIN_HEDEF_MESAFE         = 66.0f;    
const float LIN_GIDILECEK            = LIN_LIMIT1_MASA_UZAKLIGI - LIN_HEDEF_MESAFE;

bool LIN_LIMIT1_YONU  = false;  
bool ROT_ILK_YON      = true;   
bool Z_HOME_YONU      = true;   

// ---------------------------------------------------------------
// SISTEM VE LAZER DEGISKENLERI
// ---------------------------------------------------------------
enum SistemDurumu { LIN_HOMING, LIN_POSITIONING, Z_HOMING, Z_MOVING, TARAMA, BEKLIYOR, HATA };
SistemDurumu durum = BEKLIYOR; 

bool linMotorOn = false; bool linYon = false; long linHedefSteps = 0L;
bool rotMotorOn = false; bool rotYon = true;  long rotTotalSteps = 0L; float rotStepAccum = 0.0f;
bool zMotorOn   = false; bool zYon   = true;  long zCurrentSteps = 0L; long zTargetSteps = 0L; bool zHomed = false;

unsigned long linLastUs = 0UL, rotLastUs = 0UL, zLastUs = 0UL;

bool lastLin1State = HIGH, lastLin2State = HIGH, lastZ1State = HIGH, lastZ2State = HIGH;
unsigned long lin1DebMs = 0UL, lin2DebMs = 0UL, z1DebMs = 0UL, z2DebMs = 0UL;
const unsigned long DEB_MS = 200UL;

String gelenKomut = "";

// Lazer Zamanlayici Degiskenleri (Titremeyi Onlemek Icin)
volatile bool laserActive = false;
volatile unsigned long laserTrigUs = 0UL;
const unsigned long LASER_PULSE_US = 2000UL; // Lazerin 2 milisaniye acik kalma suresi

// ===============================================================
// YARDIMCI FONKSIYONLAR
// ===============================================================
void linYonAyarla() { digitalWrite(LIN_DIR_PIN, linYon ? DIR_FORWARD : DIR_REVERSE); }
void rotYonAyarla() { digitalWrite(ROT_DIR_PIN, rotYon ? DIR_FORWARD : DIR_REVERSE); }
void zYonAyarla()   { digitalWrite(Z_DIR_PIN,   zYon   ? DIR_FORWARD : DIR_REVERSE); }

void stepPulse(uint8_t pin) { digitalWrite(pin, HIGH); delayMicroseconds(10); digitalWrite(pin, LOW); }
void laserOff() { digitalWrite(LASER_PIN, LOW); laserActive = false; }

float rotStepsToDeg(long steps) {
    float deg = steps * ROT_DEG_PER_STEP;
    deg = fmod(deg, 360.0f);
    if (deg < 0.0f) deg += 360.0f;
    return deg;
}

// ===============================================================
// HAREKET KONTROLLERI
// ===============================================================
void startLinHoming() {
    rotMotorOn = false; zMotorOn = false; linMotorOn = true;
    linHedefSteps = 0L; linYon = LIN_LIMIT1_YONU; linYonAyarla();
    durum = LIN_HOMING; Serial.println("STATUS:LIN_HOMING");
}
void linRefBulundu() {
    linMotorOn = false; linHedefSteps = 0L;
    linYon = !LIN_LIMIT1_YONU; linYonAyarla(); linMotorOn = true;
    durum = LIN_POSITIONING; Serial.println("STATUS:LIN_POSITIONING");
}
void linHedefeVardi() {
    linMotorOn = false; float mm = linHedefSteps * LIN_MM_PER_STEP;
    Serial.print("STATUS:LIN_POSITIONED:"); Serial.println(mm, 1);
    startZHoming();
}
void startZHoming() {
    rotMotorOn = false; linMotorOn = false; zMotorOn = true;
    zYon = Z_HOME_YONU; zYonAyarla(); durum = Z_HOMING; Serial.println("STATUS:Z_HOMING");
}
void zHomeBulundu() {
    zMotorOn = false; zCurrentSteps = 0L; zHomed = true;
    durum = BEKLIYOR; Serial.println("STATUS:Z_HOMED\nSTATUS:READY");
}
void startZMove(float mm) {
    if (!zHomed) { Serial.println("STATUS:ERR:Z_NOT_HOMED"); return; }
    long steps = (long)(fabs(mm) / Z_MM_PER_STEP + 0.5f);
    if (steps == 0) return;
    zTargetSteps = steps; zYon = (mm > 0) ? !Z_HOME_YONU : Z_HOME_YONU;
    zYonAyarla(); zMotorOn = true; durum = Z_MOVING;
    Serial.print("STATUS:Z_MOVING:"); Serial.println(mm, 1);
}
void zHareketBitti() {
    zMotorOn = false; float mm = zTargetSteps * Z_MM_PER_STEP; durum = BEKLIYOR;
    Serial.print("STATUS:Z_MOVED:"); Serial.println(mm, 1); Serial.println("STATUS:READY");
}
void startTarama() {
    laserOff();
    rotTotalSteps = 0L; rotStepAccum = 0.0f; rotYon = ROT_ILK_YON;
    rotYonAyarla(); rotMotorOn = true; linMotorOn = false; zMotorOn = false;
    rotLastUs = micros(); durum = TARAMA; Serial.println("STATUS:SCANNING");
}
void stopTarama() { 
    rotMotorOn = false; laserOff(); durum = BEKLIYOR; Serial.println("STATUS:STOPPED\nSTATUS:READY"); 
}
void sistemHata(const char* msg) {
    linMotorOn = false; rotMotorOn = false; zMotorOn = false; laserOff(); durum = HATA;
    Serial.print("STATUS:FAULT:"); Serial.println(msg);
}

// ===============================================================
// SERI OKUMA
// ===============================================================
void komutIsle(String cmd) {
    if (cmd == "HOME") startLinHoming();
    else if (cmd == "ZHOME") startZHoming();
    else if (cmd.startsWith("ZMOVE:")) {
        float mm = cmd.substring(6).toFloat();
        if (durum == BEKLIYOR || durum == TARAMA) { if (durum == TARAMA) stopTarama(); startZMove(mm); } 
        else Serial.println("STATUS:ERR:BUSY");
    }
    else if (cmd == "START") { if (durum == BEKLIYOR) startTarama(); else Serial.println("STATUS:ERR:NOT_READY"); }
    else if (cmd == "STOP") { if (durum == TARAMA) stopTarama(); }
    else if (cmd == "RESET") startLinHoming();
    else if (cmd == "STATUS") { /* Status basimi */ }
}

void serialEngelsizOku() {
    while (Serial.available()) {
        char c = (char)Serial.read();
        if (c == '\n') {
            gelenKomut.trim();
            if (gelenKomut.length() > 0) komutIsle(gelenKomut);
            gelenKomut = "";
        } else {
            gelenKomut += c;
        }
    }
}

// ===============================================================
// SENSOR VE SURUCU DONGULERI
// ===============================================================
void limitKontrol(unsigned long ms) {
    bool l1 = digitalRead(LIN_LIMIT_1); bool l2 = digitalRead(LIN_LIMIT_2);
    bool z1 = digitalRead(Z_LIMIT_1);   bool z2 = digitalRead(Z_LIMIT_2);

    if (durum == LIN_HOMING && l1 == LOW) { if (lastLin1State == HIGH) lin1DebMs = ms; if (ms - lin1DebMs > DEB_MS) linRefBulundu(); }
    if (linMotorOn && l2 == LOW) { if (lastLin2State == HIGH) lin2DebMs = ms; if (ms - lin2DebMs > DEB_MS) sistemHata("LIN_LIMIT2_HIT"); }
    if (durum == Z_HOMING && z1 == LOW) { if (lastZ1State == HIGH) z1DebMs = ms; if (ms - z1DebMs > DEB_MS) zHomeBulundu(); }
    if (durum == Z_MOVING && zYon == Z_HOME_YONU && z1 == LOW) {
        if (lastZ1State == HIGH) z1DebMs = ms;
        if (ms - z1DebMs > DEB_MS) { zMotorOn = false; zCurrentSteps = 0L; durum = BEKLIYOR; Serial.println("STATUS:Z_HIT_HOME_LIMIT\nSTATUS:READY"); }
    }
    if (zMotorOn && z2 == LOW) { if (lastZ2State == HIGH) z2DebMs = ms; if (ms - z2DebMs > DEB_MS) sistemHata("Z_LIMIT2_HIT"); }

    lastLin1State = l1; lastLin2State = l2; lastZ1State = z1; lastZ2State = z2;
}

void motorlariSur() {
    unsigned long us = micros();
    if (linMotorOn && us - linLastUs >= LIN_STEP_US) {
        linLastUs = us; stepPulse(LIN_STEP_PIN);
        if (durum == LIN_POSITIONING) {
            linHedefSteps++;
            if (linHedefSteps * LIN_MM_PER_STEP >= LIN_GIDILECEK) linHedefeVardi();
        }
    }
    if (durum == TARAMA && rotMotorOn && us - rotLastUs >= ROT_STEP_US) {
        rotLastUs = us; stepPulse(ROT_STEP_PIN);
        if (rotYon) rotTotalSteps++; else rotTotalSteps--;
        
        rotStepAccum += 1.0f;
        if (rotStepAccum >= ROT_STEPS_PER_HALF_DEG) {
            rotStepAccum -= ROT_STEPS_PER_HALF_DEG;
            
            // --- LAZER TETIKLEME NOKTASI ---
            digitalWrite(LASER_PIN, HIGH);
            laserActive = true;
            laserTrigUs = micros();
            
            Serial.println(rotStepsToDeg(rotTotalSteps), 2);
        }
    }
    if (zMotorOn) {
        unsigned long interval = (durum == Z_HOMING) ? Z_HOME_STEP_US : Z_STEP_US;
        if (us - zLastUs >= interval) {
            zLastUs = us; stepPulse(Z_STEP_PIN);
            if (zYon == Z_HOME_YONU) zCurrentSteps--; else zCurrentSteps++;
            if (durum == Z_MOVING) { zTargetSteps--; if (zTargetSteps <= 0) zHareketBitti(); }
        }
    }
}

// ===============================================================
// KURULUM
// ===============================================================
void setup() {
    Serial.begin(115200);

    pinMode(LIN_LIMIT_1, INPUT_PULLUP);
    pinMode(LIN_LIMIT_2, INPUT_PULLUP);
    pinMode(Z_LIMIT_1,   INPUT_PULLUP);
    pinMode(Z_LIMIT_2,   INPUT_PULLUP);

    pinMode(LIN_STEP_PIN, OUTPUT); pinMode(LIN_DIR_PIN,  OUTPUT);
    pinMode(ROT_STEP_PIN, OUTPUT); pinMode(ROT_DIR_PIN,  OUTPUT);
    pinMode(Z_STEP_PIN,   OUTPUT); pinMode(Z_DIR_PIN,    OUTPUT);
    
    // Lazer Pin Ayari
    pinMode(LASER_PIN, OUTPUT);
    digitalWrite(LASER_PIN, LOW);

    linYonAyarla(); rotYonAyarla(); zYonAyarla();
    linLastUs = micros(); rotLastUs = micros(); zLastUs = micros();

    Serial.println("STATUS:BOOT_OK_WAITING_COMMAND");
}

// ===============================================================
// DONGU
// ===============================================================
void loop() {
    unsigned long ms = millis();

    serialEngelsizOku(); 
    limitKontrol(ms);
    motorlariSur();
    
    // --- Lazerin Suresi Dolunca Kapatilmasi (Gecikmesiz) ---
    if (laserActive && (micros() - laserTrigUs >= LASER_PULSE_US)) {
        digitalWrite(LASER_PIN, LOW); 
        laserActive = false;
    }
}