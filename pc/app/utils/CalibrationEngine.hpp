#ifndef CALIBRATION_ENGINE_HPP
#define CALIBRATION_ENGINE_HPP

#include <QVector>
#include <QVector3D>

#include "../controller/ScanProfileFrame.hpp"

struct AutoCalibResult {
    float bestLateralOffset;
    double maxScore;
    bool success;
};

class CalibrationEngine {
public:
    // Calculates the optimal lateral offset by testing values between minOffset and maxOffset
    // Uses Hough Transform to maximize the straightness of the scanned object's edges
    static AutoCalibResult findOptimalLateralOffset(
        const QVector<QVector3D>& cloud,
        float currentLateralOffset,
        float minOffset = -2.0f,
        float maxOffset = 2.0f,
        float step = 0.05f);

    // Ham (aci, profil) verisi uzerinden offset optimizasyonu.
    // findOptimalLateralOffset'in aksine geri-hesaplama (inverse mapping)
    // gerektirmez: her test offseti dogrudan projeksiyonla denenir.
    // Bu nedenle kalibre edilmis/filtrelenmis bulutlardaki belirsizliklerden
    // etkilenmez ve tam dogrudur.
    static AutoCalibResult findOptimalLateralOffsetFromRaw(
        const QVector<RawProfileSample>& rawProfiles,
        float dOffsetMm,
        float minOffset = -2.0f,
        float maxOffset = 2.0f,
        float step = 0.05f);
};

#endif // CALIBRATION_ENGINE_HPP
