#include "CalibrationEngine.hpp"
#include <cmath>
#include <vector>
#include <algorithm>
#include <random>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {

// Hough parametreleri — iki arama fonksiyonu da ayni skoru kullanir
constexpr int   kNumAngles = 180;
constexpr float kRhoMin    = -300.0f;
constexpr float kRhoMax    = 300.0f;
constexpr int   kNumRhos   = 600; // 1 mm cozunurluk

// XY noktalarinin "duz cizgililik" skoru: Hough akumulatorundeki
// degerlerin kareler toplami. Keskin pikler (duz kenarlar) yuksek,
// dagilmis (egri/dalgali) dagilimlar dusuk skor uretir.
double houghStraightnessScore(const std::vector<std::pair<float, float>>& ptsXY,
                              const std::vector<float>& cosTable,
                              const std::vector<float>& sinTable)
{
    const float rhoStep = (kRhoMax - kRhoMin) / kNumRhos;
    std::vector<int> accum(kNumAngles * kNumRhos, 0);

    for (const auto& pt : ptsXY) {
        for (int a = 0; a < kNumAngles; ++a) {
            float rho = pt.first * cosTable[a] + pt.second * sinTable[a];
            if (rho >= kRhoMin && rho < kRhoMax) {
                int rBin = static_cast<int>((rho - kRhoMin) / rhoStep);
                if (rBin >= 0 && rBin < kNumRhos) {
                    accum[a * kNumRhos + rBin]++;
                }
            }
        }
    }

    double score = 0.0;
    for (int v : accum) {
        if (v > 10) { // kucuk gurultu tabani esigi
            score += static_cast<double>(v) * v;
        }
    }
    return score;
}

} // namespace

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

AutoCalibResult CalibrationEngine::findOptimalLateralOffsetFromRaw(
    const QVector<RawProfileSample>& rawProfiles,
    float dOffsetMm,
    float minOffset,
    float maxOffset,
    float step)
{
    AutoCalibResult result;
    result.success = false;
    result.bestLateralOffset = 0.0f;
    result.maxScore = -1.0;

    if (rawProfiles.isEmpty()) {
        return result;
    }

    // 1. (r, cosA, sinA) uclularini cikar — projeksiyonun offsetten
    //    bagimsiz kismi. Her test offseti icin sadece X,Y yeniden kurulur.
    struct RT { float r; float cosA; float sinA; };
    std::vector<RT> rts;
    {
        size_t total = 0;
        for (const auto& s : rawProfiles) total += s.profile.size();
        rts.reserve(total);

        for (const auto& s : rawProfiles) {
            const float theta_rad = s.thetaDegree * (float)(M_PI / 180.0);
            const float cosA = std::cos(theta_rad);
            const float sinA = std::sin(theta_rad);
            for (const auto& p : s.profile) {
                const float r = dOffsetMm - (float)p.y();
                if (std::abs(r) < 0.05f) continue;
                // Tabladan cok uzak arka plan noktalarini ele (gurultu)
                if (std::abs(r) > 60.0f) continue;
                rts.push_back({r, cosA, sinA});
            }
        }
    }

    if (rts.size() < 100) {
        return result;
    }

    // 2. Performans icin altornekle
    const size_t MAX_POINTS = 30000;
    if (rts.size() > MAX_POINTS) {
        std::vector<RT> sampled;
        sampled.reserve(MAX_POINTS);
        const size_t stride = rts.size() / MAX_POINTS;
        for (size_t i = 0; i < rts.size(); i += stride) {
            sampled.push_back(rts[i]);
            if (sampled.size() >= MAX_POINTS) break;
        }
        rts.swap(sampled);
    }

    // 3. Hough tablolari
    std::vector<float> cosTable(kNumAngles), sinTable(kNumAngles);
    for (int a = 0; a < kNumAngles; ++a) {
        float phi = a * (float)(M_PI / 180.0);
        cosTable[a] = std::cos(phi);
        sinTable[a] = std::sin(phi);
    }

    // 4. Her test offsetini DOGRUDAN projeksiyonla dene
    std::vector<std::pair<float, float>> ptsXY;
    ptsXY.resize(rts.size());

    for (float testOffset = minOffset; testOffset <= maxOffset; testOffset += step) {
        for (size_t i = 0; i < rts.size(); ++i) {
            const RT& rt = rts[i];
            ptsXY[i].first  = rt.r * rt.cosA - testOffset * rt.sinA;
            ptsXY[i].second = rt.r * rt.sinA + testOffset * rt.cosA;
        }

        const double score = houghStraightnessScore(ptsXY, cosTable, sinTable);
        if (score > result.maxScore) {
            result.maxScore = score;
            result.bestLateralOffset = testOffset;
            result.success = true;
        }
    }

    return result;
}
