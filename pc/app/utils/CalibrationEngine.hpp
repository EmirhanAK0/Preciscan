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

/// Silindir bandina daire fit sonucu (dOffset cap kalibrasyonu icin)
struct CircleFitResult {
    bool  success = false;
    float centerXMm = 0.0f;     ///< Fit merkezi X (donme eksenine gore)
    float centerYMm = 0.0f;     ///< Fit merkezi Y
    float radiusMm = 0.0f;      ///< Olculen yaricap
    float rmsResidualMm = 0.0f; ///< Fit kalitesi (RMS radyal artik)
    int   usedPoints = 0;       ///< Robust eleme sonrasi kullanilan nokta
    int   totalPoints = 0;      ///< Banttaki toplam nokta
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

    // [zMin, zMax] bandindaki noktalarin XY izdusumune robust daire fit'i.
    // Bilinen capli bir silindir taramasindan dOffset hatasini olcmek icin
    // kullanilir: dOffset hatasi (eps), olculen yaricapi yaklasik eps kadar
    // kaydirir (sekli bozmaz). Kasa algebraik fit + 2 tur 2.5-sigma elemesi.
    static CircleFitResult fitCircleXY(
        const QVector<QVector3D>& cloud,
        float zMin,
        float zMax);
};

#endif // CALIBRATION_ENGINE_HPP
