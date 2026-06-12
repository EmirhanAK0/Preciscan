#pragma once

#include <QVector>
#include <QVector3D>
#include <QMatrix4x4>
#include <QQuaternion>
#include "../../third_party/nanoflann.hpp"

namespace core {

// Nanoflann Adapter for QVector<QVector3D>
struct PointCloudAdapter {
    const QVector<QVector3D>& points;
    PointCloudAdapter(const QVector<QVector3D>& pts) : points(pts) {}

    inline size_t kdtree_get_point_count() const { return points.size(); }

    inline float kdtree_get_pt(const size_t idx, const size_t dim) const {
        if (dim == 0) return points[idx].x();
        else if (dim == 1) return points[idx].y();
        else return points[idx].z();
    }

    template <class BBOX>
    bool kdtree_get_bbox(BBOX& /*bb*/) const { return false; }
};

typedef nanoflann::KDTreeSingleIndexAdaptor<
    nanoflann::L2_Simple_Adaptor<float, PointCloudAdapter>,
    PointCloudAdapter, 3> PointCloudKDTree;

/// Hassas hizalama (GICP) sonucu
struct AlignResult {
    QMatrix4x4 transform;       ///< source -> target donusumu (kaba + ince)
    float rmsMm = 0.0f;         ///< Ortusen bolgedeki RMS mesafe (inlier)
    float overlapRatio = 0.0f;  ///< Esik altinda eslesen kaynak nokta orani
    bool success = false;       ///< GICP yakinsadi mi
};

/// Birlestirme pipeline'i sonucu (hizalama + fuzyon)
struct MergeResult {
    QVector<QVector3D> cloud;   ///< Fuzyonlanmis birlesik bulut
    float rmsMm = 0.0f;
    float overlapRatio = 0.0f;
    bool icpSuccess = false;
};

class PointCloudProcessor {
public:
    // Remove points outside the given cylindrical radius (relative to X=0, Y=0)
    static QVector<QVector3D> filterCylindrical(const QVector<QVector3D>& input, float radiusMm, float minZ = -1000.0f, float maxZ = 1000.0f);

    // Remove points based on a list of indices
    static QVector<QVector3D> removePoints(const QVector<QVector3D>& input, const QVector<int>& indicesToRemove);

    // Yeni Filtreler
    static QVector<QVector3D> filterStatisticalOutlier(const QVector<QVector3D>& input, int meanK = 50, float stdDevMulThresh = 1.0f);
    static QVector<QVector3D> filterRadiusOutlier(const QVector<QVector3D>& input, float radius = 2.0f, int minNeighbors = 10);
    static QVector<QVector3D> filterVoxelGrid(const QVector<QVector3D>& input, float leafSize = 1.0f);

    // Hızlı Silindirik Mesh (Triangulation)
    static QVector<QVector3D> generateCylindricalMesh(const QVector<QVector3D>& input, int angleSteps = 720, int zSteps = 1000);

    // Kaba (Coarse) Hizalama
    static QVector<QVector3D> transformCloud(const QVector<QVector3D>& input, const QMatrix4x4& transform);

    // Hassas Hizalama (ICP - Iterative Closest Point)
    static QMatrix4x4 calculateICP(const QVector<QVector3D>& source, const QVector<QVector3D>& target, int icpMode = 0, int maxIterations = 50, float tolerance = 1e-5f);

    // ===== Cok-gorunumlu birlestirme pipeline'i =====

    /// Tabla kisitli kaba hizalama. Turntable taramasinda her iki bulutta da
    /// tabla duzlemi z=0 ve donme ekseni Z oldugu icin hizalama 6 DOF degil,
    /// pratikte 1 DOF'tur (Z etrafinda donus):
    ///  1. Preset rotasyon (0: yok, 1: Ters/X180, 2: X+90, 3: X-90)
    ///  2. Taban esitleme (robust min-Z -> hedefin min-Z'si)
    ///  3. XY agirlik merkezi esitleme
    ///  4. Z etrafinda kaba+ince yaw taramasi (en dusuk eslesme hatasi)
    static QMatrix4x4 coarseAlignWithTableConstraint(
        const QVector<QVector3D>& source,
        const QVector<QVector3D>& target,
        int presetMode,
        bool yawSearch = true);

    /// GICP (Generalized ICP / plane-to-plane) ile iki asamali ince hizalama.
    /// Kismi ortusmeye dayaniklidir; kaba hizalamadan gelen initialGuess sart.
    static AlignResult refineAlignGICP(
        const QVector<QVector3D>& source,
        const QVector<QVector3D>& target,
        const QMatrix4x4& initialGuess);

    /// Hizalanmis birlesik buluttaki cift cidari tek yuzeye eritir:
    /// voxel-ortalama (centroid) + MLS yuzey projeksiyonu + hafif SOR.
    /// Mesh oncesi "puturlu yuzey" sorununun ilaci.
    static QVector<QVector3D> fuseClouds(
        const QVector<QVector3D>& merged,
        float voxelMm = 0.25f,
        float mlsRadiusMm = 1.2f);

    /// k-NN PCA ile normal tahmini; yonelim "eksen noktasindan disari"
    /// kuraliyla tutarlilastirilir (turntable verisi icin guvenilir).
    /// Donus degeri input ile ayni boyuttadir; hata durumunda bos doner.
    static QVector<QVector3D> computeNormals(
        const QVector<QVector3D>& cloud,
        int k = 16);
};

} // namespace core
