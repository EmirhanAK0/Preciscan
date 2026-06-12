#include "AutoCalibrator.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtConcurrent/QtConcurrent>
#include <QDebug>
#include <QQuaternion>
#include <cmath>

// PCL Headers
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>
#include <pcl/ModelCoefficients.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/features/normal_3d.h>
#include <pcl/search/kdtree.h>
#include <pcl/filters/extract_indices.h>

#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QStandardPaths>

AutoCalibrator::AutoCalibrator(QObject* parent)
    : QObject(parent)
{
    // Artık otomatik olarak eski kalibrasyonu yüklemiyoruz (butterfly effect engelleme).
    // Kullanıcı UI üzerinden seçecek.
    
    connect(&m_watcher, &QFutureWatcher<CalibrationConfig>::finished, this, [this]() {
        CalibrationConfig result = m_watcher.result();
        if (result.isValid) {
            m_config = result;
            // Artik otomatik "calibration_data.json" kaydetmiyoruz.
            // Kullanici "Farkli Kaydet" diyecek.
            emit calibrationFinished(true, m_config.transform, result.reportMsg);
        } else {
            emit calibrationFinished(false, QMatrix4x4(), "PCL Kalibrasyonu basarisiz. Yeterli veya uygun data bulunamadi.");
        }
    });
}

AutoCalibrator::~AutoCalibrator() {
    if (m_watcher.isRunning()) {
        m_watcher.waitForFinished();
    }
}

QString AutoCalibrator::getCalibrationDir()
{
    // Programın çalıştığı dizin altına "calibrations" klasörü oluştur
    QDir dir(QCoreApplication::applicationDirPath());
    if (!dir.exists("calibrations")) {
        dir.mkdir("calibrations");
    }
    return dir.absoluteFilePath("calibrations");
}

bool AutoCalibrator::loadCalibration(const QString& filename)
{
    QString filePath = QDir(getCalibrationDir()).absoluteFilePath(filename);
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return false;
    QJsonObject obj = doc.object();

    if (!obj.contains("transform") || !obj["transform"].isArray()) return false;
    QJsonArray arr = obj["transform"].toArray();
    if (arr.size() != 16) return false;

    QMatrix4x4 mat;
    float* ptr = mat.data();
    for (int i = 0; i < 16; ++i) {
        ptr[i] = (float)arr[i].toDouble();
    }

    m_config.transform = mat;
    m_config.isValid = true;
    return true;
}

bool AutoCalibrator::saveCalibration(const QString& filename) const
{
    if (!m_config.isValid) return false;

    QString filePath = QDir(getCalibrationDir()).absoluteFilePath(filename);
    QJsonArray arr;
    const float* ptr = m_config.transform.constData();
    for (int i = 0; i < 16; ++i) {
        arr.append(ptr[i]);
    }

    QJsonObject obj;
    obj["transform"] = arr;

    QJsonDocument doc(obj);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(doc.toJson());
    file.close();
    return true;
}

void AutoCalibrator::clearCalibration()
{
    m_config.isValid = false;
    m_config.transform.setToIdentity();
    m_config.reportMsg.clear();
}

QStringList AutoCalibrator::getAvailableCalibrations()
{
    QDir dir(getCalibrationDir());
    QStringList filters;
    filters << "*.json";
    return dir.entryList(filters, QDir::Files | QDir::NoSymLinks);
}

QMatrix4x4 AutoCalibrator::getTransform() const {
    return m_config.transform;
}

void AutoCalibrator::setTransform(const QMatrix4x4& mat) {
    m_config.transform = mat;
    m_config.isValid = true;
}

bool AutoCalibrator::isCalibrated() const {
    return m_config.isValid;
}

void AutoCalibrator::startCalibration(const QVector<QVector3D>& pointCloud, CalibrationMethod method) {
    if (m_watcher.isRunning()) {
        qWarning() << "Kalibrasyon zaten calisiyor!";
        return;
    }
    QFuture<CalibrationConfig> future = QtConcurrent::run(processCalibration, pointCloud, method);
    m_watcher.setFuture(future);
}

QVector3D AutoCalibrator::applyTransform(const QVector3D& point) const {
    if (!m_config.isValid) return point;
    return m_config.transform.map(point);
}

// ARKA PLAN HESAPLAMA ISLEMI
CalibrationConfig AutoCalibrator::processCalibration(QVector<QVector3D> cloud, CalibrationMethod method) {
    if (method == CalibrationMethod::MATH_PCA) {
        return processCalibrationMath(cloud);
    }

    CalibrationConfig result;
    result.isValid = false;

    if (cloud.isEmpty()) {
        return result;
    }

    // 0. Arka Plan Gurultusunu (60 mm yari cap disi) Sil
    QVector<QVector3D> filtered;
    for (const auto& p : cloud) {
        if (p.x() * p.x() + p.y() * p.y() < 60.0f * 60.0f) {
            filtered.push_back(p);
        }
    }

    if (filtered.size() < 100) {
        qWarning() << "Filtreleme sonrasi yeterli nokta kalmadi!";
        return result;
    }

    // 1. Calculate centroid
    QVector3D centroid(0, 0, 0);
    for (const auto& p : filtered) centroid += p;
    centroid /= filtered.size();

    // 2. Extract planes iteratively
    pcl::PointCloud<pcl::PointXYZ>::Ptr remaining(new pcl::PointCloud<pcl::PointXYZ>());
    for (const auto& p : filtered) remaining->push_back(pcl::PointXYZ(p.x(), p.y(), p.z()));

    struct PlaneData {
        QVector3D normal;
        float d;
        int inlier_count;
    };
    std::vector<PlaneData> planes;

    pcl::SACSegmentation<pcl::PointXYZ> segPlane;
    segPlane.setOptimizeCoefficients(true);
    segPlane.setModelType(pcl::SACMODEL_PLANE);
    segPlane.setMethodType(pcl::SAC_RANSAC);
    segPlane.setMaxIterations(2000);
    segPlane.setDistanceThreshold(0.7); // 0.7 mm tolerance

    for (int i = 0; i < 6; ++i) { // En fazla 6 yuzey (Kupun 6 yuzeyi vardir)
        if (remaining->points.size() < 100) break;
        
        segPlane.setInputCloud(remaining);
        pcl::ModelCoefficients::Ptr coeffs(new pcl::ModelCoefficients);
        pcl::PointIndices::Ptr inliers(new pcl::PointIndices);
        segPlane.segment(*inliers, *coeffs);
        
        if (inliers->indices.size() < 200) break; // Bir yuzey icin en az 200 nokta
        
        QVector3D n(coeffs->values[0], coeffs->values[1], coeffs->values[2]);
        if (n.lengthSquared() > 0) n.normalize();
        float d = coeffs->values[3];
        
        // Normalin her zaman objenin MERKEZINDEN DIŞARIYA (outwards) bakmasini garantile
        if (QVector3D::dotProduct(n, centroid) + d > 0) {
            n = -n;
            d = -d;
        }
        
        planes.push_back({n, d, (int)inliers->indices.size()});
        
        // Bu duzlemdeki inlier'lari bulutun icinden cikar ki siradaki duzlemi bulalim
        pcl::ExtractIndices<pcl::PointXYZ> extract;
        extract.setInputCloud(remaining);
        extract.setIndices(inliers);
        extract.setNegative(true);
        extract.filter(*remaining);
    }

    if (planes.size() < 2) {
        qWarning() << "PCL: Yeterli duzlem bulunamadi! Bulunan yuzey:" << planes.size();
        return result;
    }

    // 3. Birbirine DIK iki yuzey (Kupun yan yuzleri) bul. 
    // Dot product 0'a yakinsa diktir.
    int idxA = -1, idxB = -1;
    for (size_t i = 0; i < planes.size(); ++i) {
        for (size_t j = i + 1; j < planes.size(); ++j) {
            if (std::abs(QVector3D::dotProduct(planes[i].normal, planes[j].normal)) < 0.25f) {
                idxA = static_cast<int>(i);
                idxB = static_cast<int>(j);
                break;
            }
        }
        if (idxA != -1) break;
    }

    if (idxA == -1 || idxB == -1) {
        qWarning() << "PCL: Birbirine dik (yaklasik 90 derece) en az 2 yan yuzey bulunamadi!";
        return result;
    }

    PlaneData pA = planes[idxA];
    PlaneData pB = planes[idxB];
    
    // Karsilikli (paralel) yuzeyleri bul. Dot product -1'e yakinsa karsiliklidir.
    int idxA_opp = -1, idxB_opp = -1;
    for (size_t i = 0; i < planes.size(); ++i) {
        if (i == (size_t)idxA || i == (size_t)idxB) continue;
        if (QVector3D::dotProduct(pA.normal, planes[i].normal) < -0.8f) idxA_opp = static_cast<int>(i);
        if (QVector3D::dotProduct(pB.normal, planes[i].normal) < -0.8f) idxB_opp = static_cast<int>(i);
    }
    
    // 4. Donus Ekseni (Z) ve Merkez Tespiti
    QVector3D rotDir = QVector3D::crossProduct(pA.normal, pB.normal).normalized();
    if (rotDir.z() < 0) rotDir = -rotDir; // Her zaman yukari (pozitif Z) baksin

    // Kupun 15x15 oldugu biliniyor. Yuzeylerden merkeze olan mesafe 7.5 mm dir.
    float center_dist_A = -pA.d - 7.5f; // Eger karsilikli yuzey yoksa 7.5 mm iceri git
    if (idxA_opp != -1) {
        // Eger karsilikli yuzey bulunduysa, tam merkez ikisinin ortasindadir.
        center_dist_A = -(pA.d - planes[idxA_opp].d) / 2.0f;
    }
    
    float center_dist_B = -pB.d - 7.5f; 
    if (idxB_opp != -1) {
        center_dist_B = -(pB.d - planes[idxB_opp].d) / 2.0f;
    }

    QVector3D x_axis = pA.normal;
    QVector3D y_axis = QVector3D::crossProduct(rotDir, x_axis).normalized();
    
    // pB.normal'in y_axis ile ayni yonde mi ters yonde mi olduguna gore isareti belirle
    float signB = (QVector3D::dotProduct(pB.normal, y_axis) > 0) ? 1.0f : -1.0f;

    // A ve B eksenlerindeki offset'i uygulayarak X, Y merkez noktasini bul. Z'yi centroid ile ayni seviyede tut.
    QVector3D rotPoint = center_dist_A * x_axis + (center_dist_B * signB) * y_axis + QVector3D::dotProduct(centroid, rotDir) * rotDir;

    // 5. Donusum Matrisini Olustur
    QVector3D Z_ideal(0, 0, 1);
    QQuaternion qAlignZ = QQuaternion::rotationTo(rotDir, Z_ideal);

    // Yaw hizalamasi: A yuzeyini (x_axis) X eksenine hizala
    QVector3D rotatedNormal = qAlignZ.rotatedVector(x_axis);
    rotatedNormal.setZ(0); 
    if (rotatedNormal.lengthSquared() > 1e-4f) {
        rotatedNormal.normalize();
    } else {
        rotatedNormal = QVector3D(1, 0, 0);
    }
    
    QVector3D X_ideal(1, 0, 0);
    QQuaternion qAlignYaw = QQuaternion::rotationTo(rotatedNormal, X_ideal);

    QQuaternion qTotal = qAlignYaw * qAlignZ;

    QVector3D p0 = rotPoint;
    p0.setZ(0); // Z yuksekligini bozma, 0 noktasindan dondurecegiz

    QMatrix4x4 transform;
    transform.setToIdentity();
    transform.rotate(qTotal);      // Z ve X hizalamalarini uygula
    transform.translate(-p0);      // Merkeze(origin'e) cek

    result.transform = transform;
    result.isValid = true;

    // --- 6. Raporlama ---
    float pitch_angle = std::acos(qBound(-1.0f, QVector3D::dotProduct(Z_ideal, rotDir), 1.0f)) * 180.0 / M_PI;
    float yaw_angle = std::acos(qBound(-1.0f, QVector3D::dotProduct(X_ideal, rotatedNormal), 1.0f)) * 180.0 / M_PI;
    
    int total_inliers = 0;
    for (const auto& p : planes) total_inliers += p.inlier_count;
    float inlier_ratio = (float)total_inliers / filtered.size() * 100.0f;

    result.reportMsg = QString("15x15x15 Kup Kalibrasyonu (PCL) Basarili!\n\n"
                               "[ Performans Metrikleri ]\n"
                               "• Tespit Edilen Yuzey Sayisi: %1 / 4\n"
                               "• Eksen Egikligi (Wobble): %2° derece\n"
                               "• Donme Hizasi Sapmasi (Yaw): %3° derece\n"
                               "• Orijin / Merkez: (X: %4, Y: %5) mm\n"
                               "• Filtrelenmis Nokta Eslesmesi: %6%")
                           .arg(planes.size())
                           .arg(pitch_angle, 0, 'f', 2)
                           .arg(yaw_angle, 0, 'f', 2)
                           .arg(p0.x(), 0, 'f', 2)
                           .arg(p0.y(), 0, 'f', 2)
                           .arg(inlier_ratio, 0, 'f', 1);

    return result;
}

CalibrationConfig AutoCalibrator::processCalibrationMath(QVector<QVector3D> cloud) {
    CalibrationConfig result;
    result.isValid = false;

    if (cloud.isEmpty()) return result;

    // 1. Orijine (0,0) çok uzak olan noktaları (arka plan gürültüsünü) sil (R < 60mm)
    QVector<QVector3D> filtered;
    for (const auto& p : cloud) {
        if (p.x()*p.x() + p.y()*p.y() < 60.0f * 60.0f) {
            filtered.push_back(p);
        }
    }

    if (filtered.size() < 100) return result;

    // 2. Ağırlık merkezi (X, Y offseti)
    QVector3D centroid(0,0,0);
    for (const auto& p : filtered) centroid += p;
    centroid /= filtered.size();
    
    // Z eksenini matematiksel olarak 0,0,1 (dik) kabul ediyoruz
    QVector3D rotDir(0, 0, 1);

    // 3. Dönüklük açısını (Yaw) bulmak için Radial Mesafe Minimizasyonu yöntemi
    int bins = 360;
    QVector<float> min_dists(bins, 9999.0f);
    
    for (const auto& p : filtered) {
        float dx = p.x() - centroid.x();
        float dy = p.y() - centroid.y();
        float dist = std::sqrt(dx*dx + dy*dy);
        
        float angle = std::atan2(dy, dx) * 180.0f / M_PI;
        if (angle < 0) angle += 360.0f;
        
        int bin = qBound(0, (int)angle, bins - 1);
        if (dist < min_dists[bin]) {
            min_dists[bin] = dist;
        }
    }
    
    // En küçük mesafenin olduğu açıyı bul (Bu açı küpün bir yüzeyinin normalidir)
    float min_overall = 9999.0f;
    int best_angle_deg = 0;
    for (int i=0; i<bins; ++i) {
        // Yumuşatma (Smoothing)
        float avg_dist = (min_dists[(i - 1 + bins) % bins] + min_dists[i] + min_dists[(i + 1) % bins]) / 3.0f;
        if (avg_dist < min_overall) {
            min_overall = avg_dist;
            best_angle_deg = i;
        }
    }
    
    float yaw_rad = best_angle_deg * M_PI / 180.0f;
    QVector3D rotatedNormal(std::cos(yaw_rad), std::sin(yaw_rad), 0);
    
    // 4. Dönüşüm Matrisi
    QVector3D Z_ideal(0, 0, 1);
    QQuaternion qAlignZ = QQuaternion::rotationTo(rotDir, Z_ideal);

    QVector3D X_ideal(1, 0, 0);
    QQuaternion qAlignYaw = QQuaternion::rotationTo(rotatedNormal, X_ideal);

    QQuaternion qTotal = qAlignYaw * qAlignZ;

    QVector3D p0 = centroid;
    p0.setZ(0);

    QMatrix4x4 transform;
    transform.setToIdentity();
    transform.rotate(qTotal);      
    transform.translate(-p0);      

    result.transform = transform;
    result.isValid = true;

    // Raporlama
    float yaw_angle = std::acos(qBound(-1.0f, QVector3D::dotProduct(X_ideal, rotatedNormal), 1.0f)) * 180.0 / M_PI;

    result.reportMsg = QString("Matematiksel (Geometrik) Kalibrasyon Basarili!\n\n"
                               "[ Performans Metrikleri ]\n"
                               "• Filtrelenen Gurultu: %1 nokta atildi\n"
                               "• Donme Hizasi Sapmasi (Yaw): %2° derece\n"
                               "• Orijin / Merkez: (X: %3, Y: %4) mm")
                           .arg(cloud.size() - filtered.size())
                           .arg(yaw_angle, 0, 'f', 2)
                           .arg(p0.x(), 0, 'f', 2)
                           .arg(p0.y(), 0, 'f', 2);

    return result;
}
