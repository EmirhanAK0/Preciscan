#include "VisualizerWidget.hpp"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QMatrix4x4>
#include <cmath>

VisualizerWidget::VisualizerWidget(QWidget* parent) : QOpenGLWidget(parent)
{
    setStyleSheet("background: #050505; border: 1px solid #1a1a1a;");

    // Add Preset View Buttons
    m_btnTop = new QPushButton("Üst", this);
    m_btnFront = new QPushButton("Ön", this);
    m_btnLeft = new QPushButton("Yan", this);

    QString btnStyle = "QPushButton { background: rgba(30, 30, 30, 180); color: #ccc; border: 1px solid #444; border-radius: 3px; padding: 4px 8px; } QPushButton:hover { background: rgba(60, 60, 60, 200); color: white; }";
    m_btnTop->setStyleSheet(btnStyle);
    m_btnFront->setStyleSheet(btnStyle);
    m_btnLeft->setStyleSheet(btnStyle);

    m_btnTop->setCursor(Qt::PointingHandCursor);
    m_btnFront->setCursor(Qt::PointingHandCursor);
    m_btnLeft->setCursor(Qt::PointingHandCursor);

    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(m_btnTop);
    btnLayout->addWidget(m_btnFront);
    btnLayout->addWidget(m_btnLeft);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(btnLayout);
    mainLayout->addStretch();

    connect(m_btnTop, &QPushButton::clicked, this, &VisualizerWidget::setTopView);
    connect(m_btnFront, &QPushButton::clicked, this, &VisualizerWidget::setFrontView);
    connect(m_btnLeft, &QPushButton::clicked, this, &VisualizerWidget::setLeftView);
}

void VisualizerWidget::clearPoints()
{
    m_points.clear();
    m_selectedIndices.clear();
    update();
}

void VisualizerWidget::setManualSelectionEnabled(bool enabled)
{
    m_selectionEnabled = enabled;
    m_isSelecting = false;
    if (!enabled) clearSelection();
}

QVector<int> VisualizerWidget::getSelectedPointIndices() const
{
    return m_selectedIndices;
}

void VisualizerWidget::clearSelection()
{
    m_selectedIndices.clear();
    m_isSelecting = false;
    update();
}

void VisualizerWidget::setMesh(const QVector<QVector3D>& triangles)
{
    m_meshTriangles = triangles;
    update();
}

void VisualizerWidget::setTopView()
{
    m_xRot = 0.0f;
    m_yRot = 0.0f;
    m_xPan = 0.0f;
    m_yPan = 0.0f;
    m_isOrtho = true;
    update();
}

void VisualizerWidget::setFrontView()
{
    m_xRot = -90.0f;
    m_yRot = 0.0f;
    m_xPan = 0.0f;
    m_yPan = 0.0f;
    m_isOrtho = true;
    update();
}

void VisualizerWidget::setLeftView()
{
    m_xRot = -90.0f;
    m_yRot = 90.0f;
    m_xPan = 0.0f;
    m_yPan = 0.0f;
    m_isOrtho = true;
    update();
}

void VisualizerWidget::addPoints(const QVector<QVector3D>& points)
{
    m_points.append(points);
    update();
}

void VisualizerWidget::addProfile(float theta_deg, const QVector<QPointF>& profile, float tableZ, float zOffset, float lateralOffset)
{
    float theta_rad = theta_deg * (M_PI / 180.0f);
    float cosA      = std::cos(theta_rad);
    float sinA      = std::sin(theta_rad);

    for (const auto& p : profile)
    {
        // p.x() = Yukseklik (Z)
        // p.y() = Lazerin objeye uzakligi (raw_r)

        float z = p.x() - zOffset; 
        float r = tableZ - p.y();  

        // Eger yaricap cok kucukse gurultudur
        if (std::abs(r) < 0.05f) continue; 

        // R gercek cap, ancak lazer merkeze gore lateral(X) olarak kayik olabilir (lateralOffset)
        float X = r * cosA - lateralOffset * sinA;
        float Y = r * sinA + lateralOffset * cosA;

        m_points.push_back(QVector3D(X, Y, z));
    }
    update();
}

void VisualizerWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.02f, 0.02f, 0.02f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void VisualizerWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    // Projection will be dynamically handled in paintGL
}

void VisualizerWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Apply Dynamic Projection (Ortho vs Perspective)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)width() / (height() ? (float)height() : 1.0f);
    if (m_isOrtho) {
        float orthoScale = std::abs(m_zoom) * 0.1f; 
        glOrtho(-aspect * orthoScale, aspect * orthoScale, -orthoScale, orthoScale, 0.1f, 5000.0f);
    } else {
        glFrustum(-aspect * 0.1f, aspect * 0.1f, -0.1f, 0.1f, 0.1f, 5000.0f);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(m_xPan, m_yPan, m_zoom);
    glRotatef(m_xRot, 1, 0, 0);
    glRotatef(m_yRot, 0, 0, 1);

    drawGrid();
    drawAxes();
    drawMesh();
    drawPoints();
    
    // Draw 2D measurement overlay
    if (m_isMeasuring) {
        calculateAndDrawDimensions();
    }
    
    // Draw 2D selection overlay
    if (m_selectionEnabled && m_isSelecting) {
        drawSelectionBox();
    }
}

void VisualizerWidget::drawSelectionBox()
{
    QPainter painter(this);
    painter.setPen(QPen(Qt::yellow, 2, Qt::DashLine));
    painter.setBrush(QBrush(QColor(255, 255, 0, 40)));
    painter.drawRect(QRect(m_selectionStart, m_selectionEnd).normalized());
}

void VisualizerWidget::calculateAndDrawDimensions()
{
    QPainter painter(this);
    painter.setPen(QPen(Qt::green, 2, Qt::DashLine));
    painter.setBrush(QBrush(QColor(0, 255, 0, 30)));

    QRect rect(m_measureStart, m_measureEnd);
    painter.drawRect(rect);

    // To measure physical distance, unproject the screen points
    QMatrix4x4 modelView;
    modelView.translate(m_xPan, m_yPan, m_zoom);
    modelView.rotate(m_xRot, 1, 0, 0);
    modelView.rotate(m_yRot, 0, 0, 1);

    QMatrix4x4 projection;
    float aspect = (float)width() / (height() ? (float)height() : 1.0f);
    if (m_isOrtho) {
        float orthoScale = std::abs(m_zoom) * 0.1f; 
        projection.ortho(-aspect * orthoScale, aspect * orthoScale, -orthoScale, orthoScale, 0.1f, 5000.0f);
    } else {
        projection.frustum(-aspect * 0.1f, aspect * 0.1f, -0.1f, 0.1f, 0.1f, 5000.0f);
    }

    QRect viewport(0, 0, width(), height());

    // Function to unproject a point onto Z=0 plane (if looking down Z), or plane facing camera
    // Near and Far points:
    QVector3D p1Near = QVector3D(m_measureStart.x(), height() - m_measureStart.y(), 0.0f).unproject(modelView, projection, viewport);
    QVector3D p1Far  = QVector3D(m_measureStart.x(), height() - m_measureStart.y(), 1.0f).unproject(modelView, projection, viewport);
    
    QVector3D p2Near = QVector3D(m_measureEnd.x(), height() - m_measureEnd.y(), 0.0f).unproject(modelView, projection, viewport);
    QVector3D p2Far  = QVector3D(m_measureEnd.x(), height() - m_measureEnd.y(), 1.0f).unproject(modelView, projection, viewport);

    // Compute Ray vectors
    QVector3D dir1 = (p1Far - p1Near).normalized();
    QVector3D dir2 = (p2Far - p2Near).normalized();

    // Plane intersection. We choose the plane based on rotation.
    QVector3D planeNormal(0, 0, 1);
    if (std::abs(m_xRot + 90.0f) < 1.0f) {
        if (std::abs(m_yRot) < 1.0f) planeNormal = QVector3D(0, 1, 0); // Front view
        else if (std::abs(m_yRot - 90.0f) < 1.0f) planeNormal = QVector3D(1, 0, 0); // Left view
    }

    // Intersect Ray 1
    float t1 = -QVector3D::dotProduct(p1Near, planeNormal) / QVector3D::dotProduct(dir1, planeNormal);
    QVector3D w1 = p1Near + dir1 * t1;

    // Intersect Ray 2
    float t2 = -QVector3D::dotProduct(p2Near, planeNormal) / QVector3D::dotProduct(dir2, planeNormal);
    QVector3D w2 = p2Near + dir2 * t2;

    float distW = 0.0f;
    float distH = 0.0f;

    // Based on plane, calculate the primary axes distances
    if (planeNormal.z() > 0.5f) { // Top view
        distW = std::abs(w2.x() - w1.x());
        distH = std::abs(w2.y() - w1.y());
    } else if (planeNormal.y() > 0.5f) { // Front view
        distW = std::abs(w2.x() - w1.x());
        distH = std::abs(w2.z() - w1.z());
    } else { // Left view
        distW = std::abs(w2.y() - w1.y());
        distH = std::abs(w2.z() - w1.z());
    }

    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 11, QFont::Bold));
    QString text = QString("W: %1 mm, H: %2 mm").arg(distW, 0, 'f', 1).arg(distH, 0, 'f', 1);
    
    // Draw text near the rect, with a black outline or background for readability
    QRect textRect(rect.bottomRight() + QPoint(10, 10), QSize(150, 30));
    painter.fillRect(textRect, QColor(0, 0, 0, 180));
    painter.drawText(textRect, Qt::AlignCenter, text);
}

// ... existing draw functions ...


void VisualizerWidget::drawGrid()
{
    glBegin(GL_LINES);
    glColor3f(0.1f, 0.1f, 0.15f);
    for (int i = -10; i <= 10; ++i)
    {
        glVertex3f(i * 10.0f, -100.0f, 0);
        glVertex3f(i * 10.0f, 100.0f, 0);
        glVertex3f(-100.0f, i * 10.0f, 0);
        glVertex3f(100.0f, i * 10.0f, 0);
    }
    glEnd();
}

void VisualizerWidget::drawAxes()
{
    glBegin(GL_LINES);
    glColor3f(1.0f, 0, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(50, 0, 0); // X (Kirmizi)
    glColor3f(0, 1.0f, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 50, 0); // Y (Yesil)
    glColor3f(0.3f, 0.3f, 1.0f);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 50); // Z (Mavi)
    glEnd();
}

void VisualizerWidget::drawMesh()
{
    if (m_meshTriangles.isEmpty())
        return;

    glDisable(GL_LIGHTING);
    // Wireframe Mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_TRIANGLES);
    glColor4f(0.8f, 0.8f, 0.8f, 0.4f); // Daha parlak gri-gumus
    for (const auto& v : m_meshTriangles)
    {
        glVertex3f(v.x(), v.y(), v.z());
    }
    glEnd();

    // Transparent Surface
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_TRIANGLES);
    glColor4f(0.5f, 0.5f, 0.5f, 0.15f); // Saydam dolgu
    for (const auto& v : m_meshTriangles)
    {
        glVertex3f(v.x(), v.y(), v.z());
    }
    glEnd();
}

void VisualizerWidget::drawPoints()
{
    if (m_points.isEmpty())
        return;

    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (const auto& p : m_points)
    {
        // Select color based on selection state
        bool isSelected = m_selectedIndices.contains(&p - m_points.constData());
        if (isSelected) {
            glColor3f(1.0f, 1.0f, 0.0f); // Yellow for selected
        } else {
            // Yukseklige gore Heatmap renk gradyani (Z: 0 - 30 mm arasi)
            // Mavi (Alcak) -> Yesil (Orta) -> Kirmizi (Yuksek)
            float factor = qBound(0.0f, p.z() / 30.0f, 1.0f);
            
            float r = 0.0f, g = 0.0f, b = 0.0f;
            if (factor < 0.5f) {
                b = 1.0f - (factor * 2.0f); // Maviden
                g = factor * 2.0f;          // Yesile
            } else {
                g = 1.0f - ((factor - 0.5f) * 2.0f); // Yesilden
                r = ((factor - 0.5f) * 2.0f);        // Kirmiziya
            }
            // Noktalari biraz daha parlak tutalim
            r = qBound(0.2f, r, 1.0f);
            g = qBound(0.2f, g, 1.0f);
            b = qBound(0.2f, b, 1.0f);

            glColor3f(r, g, b);
        }
        glVertex3f(p.x(), p.y(), p.z());
    }
    glEnd();
}

void VisualizerWidget::updateSelection()
{
    if (m_points.isEmpty() || !m_selectionEnabled) return;

    m_selectedIndices.clear();
    QRect selectionRect = QRect(m_selectionStart, m_selectionEnd).normalized();
    if (selectionRect.width() < 5 && selectionRect.height() < 5) {
        return; // Too small, just a click
    }

    QMatrix4x4 modelView;
    modelView.translate(m_xPan, m_yPan, m_zoom);
    modelView.rotate(m_xRot, 1, 0, 0);
    modelView.rotate(m_yRot, 0, 0, 1);

    QMatrix4x4 projection;
    float aspect = (float)width() / (height() ? (float)height() : 1.0f);
    if (m_isOrtho) {
        float orthoScale = std::abs(m_zoom) * 0.1f; 
        projection.ortho(-aspect * orthoScale, aspect * orthoScale, -orthoScale, orthoScale, 0.1f, 5000.0f);
    } else {
        projection.frustum(-aspect * 0.1f, aspect * 0.1f, -0.1f, 0.1f, 0.1f, 5000.0f);
    }

    QMatrix4x4 MVP = projection * modelView;

    // Convert screen selection rect to Normalized Device Coordinates (NDC)
    // Screen X: 0 (left) to width (right) -> NDC X: -1 to 1
    // Screen Y: 0 (top) to height (bottom) -> NDC Y: 1 to -1 (OpenGL Y is up)
    float w = (float)width();
    float h = (float)height();
    
    float ndc_xmin = (selectionRect.left() / w) * 2.0f - 1.0f;
    float ndc_xmax = (selectionRect.right() / w) * 2.0f - 1.0f;
    
    // Y is inverted in screen coordinates
    float ndc_ymin = ((h - selectionRect.bottom()) / h) * 2.0f - 1.0f;
    float ndc_ymax = ((h - selectionRect.top()) / h) * 2.0f - 1.0f;

    // Pre-allocate memory to avoid reallocations
    m_selectedIndices.reserve(m_points.size() / 10); 

    const QVector3D* data = m_points.constData();
    int count = m_points.size();

    for (int i = 0; i < count; ++i) {
        // Direct matrix multiplication
        QVector4D p_clip = MVP * QVector4D(data[i], 1.0f);
        
        if (p_clip.w() == 0.0f) continue;
        
        float nx = p_clip.x() / p_clip.w();
        float ny = p_clip.y() / p_clip.w();
        float nz = p_clip.z() / p_clip.w();

        // Check if point is behind camera or outside near/far planes
        if (nz < -1.0f || nz > 1.0f) continue;

        if (nx >= ndc_xmin && nx <= ndc_xmax && ny >= ndc_ymin && ny <= ndc_ymax) {
            m_selectedIndices.push_back(i);
        }
    }
}

void VisualizerWidget::mousePressEvent(QMouseEvent* event)
{
    m_lastPos = event->pos();
    if (m_selectionEnabled && event->button() == Qt::LeftButton) {
        m_isSelecting = true;
        m_selectionStart = event->pos();
        m_selectionEnd = event->pos();
        m_selectedIndices.clear();
        update();
    } else if (event->button() == Qt::RightButton && !m_selectionEnabled) {
        m_isMeasuring = true;
        m_measureStart = event->pos();
        m_measureEnd = event->pos();
        update();
    }
}

void VisualizerWidget::mouseMoveEvent(QMouseEvent* event)
{
    int dx = event->position().x() - m_lastPos.x();
    int dy = event->position().y() - m_lastPos.y();
    
    if (m_isSelecting && (event->buttons() & Qt::LeftButton)) {
        m_selectionEnd = event->pos();
        update();
    }
    else if (m_isMeasuring && (event->buttons() & Qt::RightButton)) {
        m_measureEnd = event->pos();
        update();
    }
    else if (event->buttons() & Qt::LeftButton && !m_selectionEnabled)
    {
        m_xRot += dy * 0.5f;
        m_yRot += dx * 0.5f;
        m_isOrtho = false; // Switch to perspective when rotating
        update();
    }
    else if (event->buttons() & Qt::MiddleButton)
    {
        float panFactor = std::max(0.1f, std::abs(m_zoom) / 500.0f);
        m_xPan += dx * panFactor;
        m_yPan -= dy * panFactor;
        update();
    }
    m_lastPos = event->pos();
}

void VisualizerWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_isSelecting && event->button() == Qt::LeftButton) {
        updateSelection();
        m_isSelecting = false;
        update();
    }
    else if (event->button() == Qt::RightButton) {
        // Keep measuring box visible until next click
    }
}

void VisualizerWidget::wheelEvent(QWheelEvent* event)
{
    float delta = event->angleDelta().y() / 120.0f; // typically +1 or -1
    float step = std::max(5.0f, std::abs(m_zoom) * 0.15f);
    m_zoom += delta * step;
    update();
}