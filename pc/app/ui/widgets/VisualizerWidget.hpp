#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QVector3D>
#include <QPoint>
#include <QVector>

class VisualizerWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit VisualizerWidget(QWidget* parent = nullptr);

public slots:
    void clearPoints();
    void addPoints(const QVector<QVector3D>& points);
    void addProfile(float theta_deg, const QVector<QPointF>& profile);
    void setMesh(const QVector<QVector3D>& triangles); // 3'erli gruplar halinde v0,v1,v2

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void drawGrid();
    void drawAxes();
    void drawPoints();
    void drawMesh();

    QVector<QVector3D> m_points;
    QVector<QVector3D> m_meshTriangles;
    float m_xRot = 30.0f;
    float m_yRot = -45.0f;
    float m_zoom = -250.0f;
    QPoint m_lastPos;
};