// ============================================================
// Preciscan — Arduino Firmware (Step-Count Based Angle)
// AS5600 / I2C bağımlılığı kaldırıldı.
// Açı, step sayacından hesaplanır.
// ============================================================

// PİNLER
#define LIMIT_PIN 4
#define LASER_PIN 5
#define STEP_PIN  6
#define DIR_PIN   7

// MOTOR AYARLARI
const unsigned long stepInterval = 40000UL; // µs. Küçüldükçe hızlanır.

// STEP / AÇI HESABI
const float STEPS_PER_REV      = 3200.0f;
const float DEG_PER_STEP       = 360.0f / STEPS_PER_REV;          // 0.1125 deg/step
const float stepsPerHalfDegree = STEPS_PER_REV / 720.0f;          // ~4.444 step / 0.5 deg

// Yön bilgili adım sayacı (signed → CW pozitif, CCW negatif)
long totalSteps  = 0L;
float stepAccum  = 0.0f;   // Bir sonraki trigger'a kalan kesirli step

// LAZER DARBE AYARLARI
volatile bool         laserActive       = false;
volatile unsigned long laserTriggerTime = 0UL;
const unsigned long   laserPulseDuration = 2000UL; // µs

// Motor durumu
bool motorCalisiyor = true;
bool yonDurumu      = true;   // true = DIR_FORWARD, false = DIR_REVERSE

// Switch debounce
bool          lastSwitchState = HIGH;
unsigned long sonBasmaZamani  = 0UL;
const unsigned long debounceMs = 200UL;

// Step zamanlayıcı
unsigned long lastStepTime = 0UL;

const uint8_t DIR_FORWARD = LOW;
const uint8_t DIR_REVERSE = HIGH;

// ----------------------------------------------------------
// Geçerli açıyı step sayısından hesapla (0..359.99 deg)
// ----------------------------------------------------------
float stepsToDeg(long steps)
{
    float deg = steps * DEG_PER_STEP;
    // 0-360 aralığına normalize et
    deg = fmod(deg, 360.0f);
    if (deg < 0.0f) deg += 360.0f;
    return deg;
}

void motorYonunuAyarla()
{
    digitalWrite(DIR_PIN, yonDurumu ? DIR_FORWARD : DIR_REVERSE);
}

void stepMotor()
{
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(5);
    digitalWrite(STEP_PIN, LOW);
}

void setup()
{
    Serial.begin(115200);

    pinMode(STEP_PIN,  OUTPUT);
    pinMode(DIR_PIN,   OUTPUT);
    pinMode(LASER_PIN, OUTPUT);
    pinMode(LIMIT_PIN, INPUT_PULLUP);

    digitalWrite(LASER_PIN, LOW);
    motorYonunuAyarla();

    Serial.println("Sistem basladi.");
}

void loop()
{
    unsigned long simdiMs = millis();

    // ── SWITCH KONTROL ────────────────────────────────────────
    bool switchState = digitalRead(LIMIT_PIN);
    if (lastSwitchState == HIGH && switchState == LOW)
    {
        if (simdiMs - sonBasmaZamani > debounceMs)
        {
            sonBasmaZamani = simdiMs;
            if (motorCalisiyor)
            {
                motorCalisiyor = false;
                Serial.println("Limit switch basildi -> Motor DURDU");
            }
            else
            {
                yonDurumu = !yonDurumu;
                motorYonunuAyarla();
                motorCalisiyor = true;
                Serial.println("Limit switch basildi -> Yon degisti, motor CALISIYOR");
            }
        }
    }
    lastSwitchState = switchState;

    // ── MOTOR DÖNDÜRME + LAZER TETİKLEME ────────────────────
    if (motorCalisiyor)
    {
        unsigned long nowUs = micros(); // Her kontrol noktasında taze al

        if (nowUs - lastStepTime >= stepInterval)
        {
            lastStepTime = nowUs;
            stepMotor();

            // Yön bilgili adım sayacı
            if (yonDurumu) totalSteps++;
            else           totalSteps--;

            stepAccum += 1.0f;

            if (stepAccum >= stepsPerHalfDegree)
            {
                stepAccum -= stepsPerHalfDegree;

                // Gerçek trigger anını al (I2C gecikmesi yok artık)
                unsigned long trigUs = micros();

                // Lazeri tetikle
                digitalWrite(LASER_PIN, HIGH);
                laserActive      = true;
                laserTriggerTime = trigUs;

                // Açıyı step sayısından hesapla ve PC'ye gönder
                float angleDeg = stepsToDeg(totalSteps);
                Serial.println(angleDeg, 2);
            }
        }
    }

    // ── LAZER DARBE SONU KONTROLÜ ────────────────────────────
    if (laserActive)
    {
        // Taze micros() ile karşılaştır
        if (micros() - laserTriggerTime >= laserPulseDuration)
        {
            digitalWrite(LASER_PIN, LOW);
            laserActive = false;
        }
    }
}
