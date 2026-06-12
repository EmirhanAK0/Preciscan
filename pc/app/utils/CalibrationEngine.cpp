#include "CalibrationEngine.hpp"
#include <cmath>
#include <vector>
#include <algorithm>
#include <random>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

AutoCalibResult CalibrationEngine::findOptimalLateralOffset(
    const QVector<QVector3D>& cloud,
    float currentLateralOffset,
    float minOffset,
    float maxOffset,
    float step)
{
    AutoCalibResult result;
    result.success = false;
    result.bestLateralOffset = currentLateralOffset;
    result.maxScore = -1.0;

    if (cloud.isEmpty()) {
        return result;
    }

    // 1. Subsample points to improve performance.
    // We only care about X and Y coordinates to find straight lines.
    // Also, we can filter out points that are too close to the center (noise) 
    // or perfectly circular cylinder points. But for simplicity and robustness, 
    // a random sample of the whole cloud is enough since lines create sharp peaks.
    const int MAX_POINTS = 30000;
    std::vector<QVector3D> sampled;
    if (cloud.size() > MAX_POINTS) {
        sampled.reserve(MAX_POINTS);
        int stepSize = cloud.size() / MAX_POINTS;
        for (int i = 0; i < cloud.size(); i += stepSize) {
            sampled.push_back(cloud[i]);
            if (sampled.size() >= MAX_POINTS) break;
        }
    } else {
        sampled.reserve(cloud.size());
        for (const auto& p : cloud) sampled.push_back(p);
    }

    // Precompute sine and cosine for Hough Transform to save time
    const int numAngles = 180;
    std::vector<float> cosTable(numAngles);
    std::vector<float> sinTable(numAngles);
    for (int a = 0; a < numAngles; ++a) {
        float phi = a * (M_PI / 180.0);
        cosTable[a] = std::cos(phi);
        sinTable[a] = std::sin(phi);
    }

    // Hough accumulator parameters
    const float rhoMin = -300.0f;
    const float rhoMax = 300.0f;
    const int numRhos = 600; // 1 mm resolution
    const float rhoStep = (rhoMax - rhoMin) / numRhos;

    float L_old = currentLateralOffset;
    
    // We will test all offsets
    for (float testOffset = minOffset; testOffset <= maxOffset; testOffset += step) {
        
        std::vector<int> accum(numAngles * numRhos, 0);

        for (const auto& pt : sampled) {
            float X = pt.x();
            float Y = pt.y();
            float R_sq = X * X + Y * Y;
            
            // Skip points where it's mathematically impossible
            if (R_sq < L_old * L_old) continue;
            
            // Back-calculate raw (r, theta) based on the old lateral offset
            float r = std::sqrt(R_sq - L_old * L_old);
            float A = std::atan2(Y, X) - std::atan2(L_old, r);
            
            // Forward-calculate new (X, Y) with the test lateral offset
            float X_new = r * std::cos(A) - testOffset * std::sin(A);
            float Y_new = r * std::sin(A) + testOffset * std::cos(A);

            // Calculate Hough parameters
            for (int a = 0; a < numAngles; ++a) {
                float rho = X_new * cosTable[a] + Y_new * sinTable[a];
                
                if (rho >= rhoMin && rho < rhoMax) {
                    int rBin = static_cast<int>((rho - rhoMin) / rhoStep);
                    if (rBin >= 0 && rBin < numRhos) {
                        accum[a * numRhos + rBin]++;
                    }
                }
            }
        }

        // Calculate score: Sum of squares of accumulator values.
        // A square sum heavily penalizes uniform distribution (curved lines)
        // and highly rewards concentrated peaks (perfectly straight lines).
        double currentScore = 0.0;
        for (int v : accum) {
            if (v > 10) { // Add a small noise floor threshold
                currentScore += static_cast<double>(v) * v;
            }
        }

        if (currentScore > result.maxScore) {
            result.maxScore = currentScore;
            result.bestLateralOffset = testOffset;
            result.success = true;
        }
    }

    return result;
}
