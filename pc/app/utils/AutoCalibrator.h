#ifndef AUTO_CALIBRATOR_H
#define AUTO_CALIBRATOR_H

#include <QVector3D>
#include <QVector>
#include <QMatrix4x4>
#include <QString>
#include <QObject>
#include <QFuture>
#include <QFutureWatcher>

enum class CalibrationMethod {
    PCL_RANSAC,
    MATH_PCA
};

struct CalibrationConfig {
    QMatrix4x4 transform;
    bool isValid = false;
    QString reportMsg;
    CalibrationMethod method = CalibrationMethod::PCL_RANSAC;
};

class AutoCalibrator : public QObject {
    Q_OBJECT
public:
    explicit AutoCalibrator(QObject* parent = nullptr);
    ~AutoCalibrator();

    // Kalıcılık fonksiyonları
    static QString getCalibrationDir();
    bool loadCalibration(const QString& filename);
    bool saveCalibration(const QString& filename) const;
    void clearCalibration();
    static QStringList getAvailableCalibrations();

    // Matris okuma/yazma
    QMatrix4x4 getTransform() const;
    void setTransform(const QMatrix4x4& mat);
    bool isCalibrated() const;

    // Asenkron kalibrasyon başlatıcı
    void startCalibration(const QVector<QVector3D>& pointCloud, CalibrationMethod method = CalibrationMethod::PCL_RANSAC);

    // Uygulama
    QVector3D applyTransform(const QVector3D& point) const;

signals:
    void calibrationFinished(bool success, QMatrix4x4 transform, QString message);
    void progressUpdated(int percent, QString stage);

private:
    CalibrationConfig m_config;
    QFutureWatcher<CalibrationConfig> m_watcher;

    // Asıl matematiksel işlemi yapan statik (thread-safe) fonksiyon
    static CalibrationConfig processCalibration(QVector<QVector3D> cloud, CalibrationMethod method);
    
    // Matematiksel alternatif (PCA tabanlı) kalibrasyon
    static CalibrationConfig processCalibrationMath(QVector<QVector3D> cloud);
};

#endif // AUTO_CALIBRATOR_H
