#include "VisualizerWidget.hpp"
#include <cmath>
#include <QMouseEvent>
#include <QWheelEvent>

VisualizerWidget::VisualizerWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    setStyleSheet("background: #050505; border: 1px solid #1a1a1a;");
}

void VisualizerWidget::clearPoints() {
    m_points.clear();
    update();
}

void VisualizerWidget::setMesh(const QVector<QVector3D>& triangles) {
    m_meshTriangles = triangles;
    update();
}

void VisualizerWidget::addPoints(const QVector<QVector3D>& points) {
    m_points.append(points);
    update();
}

void VisualizerWidget::addProfile(float theta_deg, const QVector<QPointF>& profile) {
    float theta_rad = theta_deg * (M_PI / 180.0f);
    float cosA = std::cos(theta_rad);
    float sinA = std::sin(theta_rad);

    for (const auto& p : profile) {
        // p.x() = r, p.y() = z
        m_points.push_back(QVector3D(p.x() * cosA, p.x() * sinA, p.y()));
    }
    update();
}

void VisualizerWidget::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(0.02f, 0.02f, 0.02f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void VisualizerWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)w / (h ? (float)h : 1.0f);
    glFrustum(-aspect * 0.1f, aspect * 0.1f, -0.1f, 0.1f, 0.1f, 5000.0f);
    glMatrixMode(GL_MODELVIEW);
}

void VisualizerWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glLoadIdentity();
    glTranslatef(0, 0, m_zoom);
    glRotatef(m_xRot, 1, 0, 0);
    glRotatef(m_yRot, 0, 0, 1);

    drawGrid();
    drawAxes();
    drawMesh();
    drawPoints();
}

void VisualizerWidget::drawGrid() {
    glBegin(GL_LINES);
    glColor3f(0.1f, 0.1f, 0.15f);
    for(int i = -10; i <= 10; ++i) {
        glVertex3f(i*10.0f, -100.0f, 0); glVertex3f(i*10.0f, 100.0f, 0);
        glVertex3f(-100.0f, i*10.0f, 0); glVertex3f(100.0f, i*10.0f, 0);
    }
    glEnd();
}

void VisualizerWidget::drawAxes() {
    glBegin(GL_LINES);
    glColor3f(1.0f, 0, 0); glVertex3f(0,0,0); glVertex3f(50,0,0); // X (Kirmizi)
    glColor3f(0, 1.0f, 0); glVertex3f(0,0,0); glVertex3f(0,50,0); // Y (Yesil)
    glColor3f(0.3f, 0.3f, 1.0f); glVertex3f(0,0,0); glVertex3f(0,0,50); // Z (Mavi)
    glEnd();
}

void VisualizerWidget::drawMesh() {
    if (m_meshTriangles.isEmpty()) return;

    glDisable(GL_LIGHTING);
    // Wireframe Mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_TRIANGLES);
    glColor4f(0.8f, 0.8f, 0.8f, 0.4f); // Daha parlak gri-gumus
    for (const auto& v : m_meshTriangles) {
        glVertex3f(v.x(), v.y(), v.z());
    }
    glEnd();
    
    // Transparent Surface
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_TRIANGLES);
    glColor4f(0.5f, 0.5f, 0.5f, 0.15f); // Saydam dolgu
    for (const auto& v : m_meshTriangles) {
        glVertex3f(v.x(), v.y(), v.z());
    }
    glEnd();
}

void VisualizerWidget::drawPoints() {
    if (m_points.isEmpty()) return;

    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (const auto& p : m_points) {
        // Yukseklige gore renk gradyani (Z: 0 - 100)
        float factor = qBound(0.0f, p.z() / 150.0f, 1.0f);
        glColor3f(factor * 0.5f, 1.0f - factor * 0.5f, 1.0f); // Mavi-Turkuaz-Beyazimsi tonlar
        glVertex3f(p.x(), p.y(), p.z());
    }
    glEnd();
}

void VisualizerWidget::mousePressEvent(QMouseEvent* event) {
    m_lastPos = event->pos();
}

void VisualizerWidget::mouseMoveEvent(QMouseEvent* event) {
    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();
    if (event->buttons() & Qt::LeftButton) {
        m_xRot += dy * 0.5f;
        m_yRot += dx * 0.5f;
        update();
    }
    m_lastPos = event->pos();
}

void VisualizerWidget::wheelEvent(QWheelEvent* event) {
    m_zoom += event->angleDelta().y() / 4.0f;
    update();
}