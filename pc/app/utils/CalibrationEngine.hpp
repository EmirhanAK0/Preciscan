#ifndef CALIBRATION_ENGINE_HPP
#define CALIBRATION_ENGINE_HPP

#include <QVector>
#include <QVector3D>

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
};

#endif // CALIBRATION_ENGINE_HPP
