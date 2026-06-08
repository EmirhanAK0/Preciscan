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

    /// Mevcut nokta bulutunu disa aktarim icin dondurur.
    const QVector<QVector3D>& getPoints() const { return m_points; }
    int pointCount() const { return m_points.size(); }

    void setManualSelectionEnabled(bool enabled);
    QVector<int> getSelectedPointIndices() const;
    void clearSelection();

public slots:
    void clearPoints();
    void addPoints(const QVector<QVector3D>& points);
    void addProfile(float theta_deg, const QVector<QPointF>& profile, float tableZ = 66.0f, float zOffset = 3.5f, float lateralOffset = 0.0f);
    void setMesh(const QVector<QVector3D>& triangles); // 3'erli gruplar halinde v0,v1,v2

    bool saveMeshToOBJ(const QString& filePath) const;

    void setTopView();
    void setFrontView();
    void setLeftView();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void drawGrid();
    void drawAxes();
    void drawPoints();
    void drawMesh();
    void drawMeasurementBox();
    void calculateAndDrawDimensions();

    QVector<QVector3D> m_points;
    QVector<QVector3D> m_meshTriangles;

    // Camera/View State
    float m_xRot = 30.0f;
    float m_yRot = -45.0f;
    float m_xPan = 0.0f;
    float m_yPan = 0.0f;
    float m_zoom = -250.0f;
    bool m_isOrtho = false;
    QPoint m_lastPos;

    // Measurement Tool State
    bool m_isMeasuring = false;
    QPoint m_measureStart;
    QPoint m_measureEnd;

    // Overlay Buttons
    class QPushButton* m_btnTop;
    class QPushButton* m_btnFront;
    class QPushButton* m_btnLeft;

    // Selection State
    bool m_selectionEnabled = false;
    bool m_isSelecting = false;
    QPoint m_selectionStart;
    QPoint m_selectionEnd;
    QVector<int> m_selectedIndices;
    
    void updateSelection();
    void drawSelectionBox();
};