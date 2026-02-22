#include "laser_sim_worker.h"
#include <QDebug>
#include <QElapsedTimer>
#include <cmath>

namespace sim {

LaserSimWorker::LaserSimWorker(const Mesh& mesh, const SliceParams& params, QObject* parent)
    : QThread(parent), m_mesh(mesh), m_params(params) {}

void LaserSimWorker::run() {
    float angle_step = m_params.deg_per_step;
    float current_angle = 0.0f;
    QVector<QVector3D> cloud;
    cloud.reserve(200000);

    while (current_angle < 360.0f && m_running) {
        QElapsedTimer timer;
        timer.start();

        auto profile = sliceAtAngle(m_mesh, current_angle * (M_PI / 180.0f), m_params);
        QVector<QPointF> sensorData;
        sensorData.reserve(profile.size());
        
        float cosA = std::cos(current_angle * (M_PI / 180.0f));
        float sinA = std::sin(current_angle * (M_PI / 180.0f));

        for (const auto& p : profile) {
            // p.r = distance, p.z = height
            // Sensor mapping: X = Height, Y = Distance
            sensorData.push_back(QPointF(p.z, p.r));
            
            float r = m_params.D_offset_mm - p.r;
            if (r > 0.05f) {
                cloud.push_back(QVector3D(r * cosA, r * sinA, p.z));
            }
        }

        emit profileReady(current_angle, sensorData);
        emit progressUpdated(static_cast<int>((current_angle / 360.0f) * 100));
        current_angle += angle_step;

        float total_steps = 360.0f / angle_step;
        int delay_ms = static_cast<int>((1000.0f / m_params.rps) / total_steps);
        int sleep_time = delay_ms - (int)timer.elapsed();
        if (sleep_time > 0) msleep(sleep_time);
    }
    if (m_running) emit scanComplete(cloud);
}

} // namespace sim