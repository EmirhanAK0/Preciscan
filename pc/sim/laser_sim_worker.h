#pragma once
#include <QThread>
#include <QVector>
#include <QPointF>
#include <QVector3D>
#include <atomic>
#include "mesh_slicer.h"

namespace sim {

class LaserSimWorker : public QThread {
    Q_OBJECT
public:
    LaserSimWorker(const Mesh& mesh, const SliceParams& params, QObject* parent = nullptr);
    void stop() { m_running = false; }

signals:
    void profileReady(float theta_deg, const QVector<QPointF>& profile);
    void progressUpdated(int percent);
    void scanComplete(const QVector<QVector3D>& pointCloud);

protected:
    void run() override;

private:
    Mesh m_mesh;
    SliceParams m_params;
    std::atomic<bool> m_running{true};
};

} // namespace sim