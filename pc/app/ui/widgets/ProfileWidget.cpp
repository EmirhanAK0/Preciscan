#include "ProfileWidget.hpp"
#include <QMutexLocker>
#include <QPainterPath>
#include <cmath>

ProfileWidget::ProfileWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(200, 150);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet("background-color: #0a0a0a;");
}

void ProfileWidget::updateProfile(float theta_deg, const QVector<QPointF>& profile) {
    QMutexLocker lk(&m_mutex);

    // Max dist hesapla
    for (const auto& p : profile)
        if (p.y() > m_maxDist) m_maxDist = p.y();

    m_current = profile;
    m_currentTheta = theta_deg;

    m_history.push_back({ theta_deg, profile });
    if ((int)m_history.size() > MAX_HISTORY)
        m_history.pop_front();

    update();
}

void ProfileWidget::clear() {
    QMutexLocker lk(&m_mutex);
    m_current.clear();
    m_history.clear();
    m_maxDist = 100.0f;
    update();
}

void ProfileWidget::resizeEvent(QResizeEvent*) {
    computePlotArea(width(), height());
}

void ProfileWidget::computePlotArea(int w, int h) {
    const int padL = 44, padR = 10, padT = 10, padB = 28;
    m_plotArea = QRect(padL, padT, w - padL - padR, h - padT - padB);
}

QPointF ProfileWidget::toScreen(float height, float dist) const {
    float nx = height / m_maxHeight;
    float ny = dist / m_maxDist;
    float sx = m_plotArea.left() + nx * m_plotArea.width();
    float sy = m_plotArea.bottom() - ny * m_plotArea.height();
    return { sx, sy };
}

void ProfileWidget::paintEvent(QPaintEvent*) {
    QMutexLocker lk(&m_mutex);
    computePlotArea(width(), height());

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // --- Arka plan ---
    p.fillRect(rect(), QColor("#0a0a0a"));
    p.fillRect(m_plotArea, QColor("#0f0f0f"));

    // --- Grid ---
    p.setPen(QPen(QColor("#1c1c1c"), 1));
    const int gridLines = 5;
    for (int i = 0; i <= gridLines; ++i) {
        float fx = m_plotArea.left() + i * m_plotArea.width() / gridLines;
        float fy = m_plotArea.top() + i * m_plotArea.height() / gridLines;
        p.drawLine(QPointF(fx, m_plotArea.top()), QPointF(fx, m_plotArea.bottom()));
        p.drawLine(QPointF(m_plotArea.left(), fy), QPointF(m_plotArea.right(), fy));
    }

    // --- Eksen çerçevesi ---
    p.setPen(QPen(QColor("#2a2a2a"), 1));
    p.drawRect(m_plotArea);

    // --- Eksen etiketler ---
    p.setPen(QColor("#555"));
    QFont font = p.font(); font.setPointSize(7); p.setFont(font);

    // X ekseni: Height 0..22mm
    for (int i = 0; i <= 5; ++i) {
        float h = i * m_maxHeight / 5.0f;
        QPointF sp = toScreen(h, 0.0f);
        p.drawText(QRectF(sp.x() - 12, m_plotArea.bottom() + 4, 24, 14),
                   Qt::AlignCenter, QString::number((int)h));
    }

    // Y ekseni: Distance 0..maxDist
    for (int i = 0; i <= 5; ++i) {
        float d = i * m_maxDist / 5.0f;
        QPointF sp = toScreen(0.0f, d);
        p.drawText(QRectF(2, sp.y() - 7, m_plotArea.left() - 4, 14),
                   Qt::AlignRight | Qt::AlignVCenter, QString::number((int)d));
    }

    // Eksen başlıkları
    p.setPen(QColor("#444"));
    QFont titleFont = p.font(); titleFont.setPointSize(7); p.setFont(titleFont);
    p.drawText(QRectF(m_plotArea.left(), height() - 14, m_plotArea.width(), 14),
               Qt::AlignCenter, "Height (mm)");
    // Y label (rotasyonlu)
    p.save();
    p.translate(0, m_plotArea.center().y());
    p.rotate(-90);
    p.drawText(QRectF(-30, -12, 60, 12), Qt::AlignCenter, "Dist (mm)");
    p.restore();

    // --- Eski profiller (ghost) ---
    if (!m_history.empty()) {
        for (int hi = 0; hi < (int)m_history.size() - 1; ++hi) {
            float alpha = 15.0f + 20.0f * (hi / (float)m_history.size());
            QPen ghostPen(QColor(0, 200, 255, (int)alpha), 1);
            p.setPen(ghostPen);
            const auto& entry = m_history[hi];
            for (const auto& pt : entry.data) {
                QPointF sp = toScreen((float)pt.x(), (float)pt.y());
                p.drawPoint(sp);
            }
        }
    }

    // --- Güncel profil ---
    if (!m_current.isEmpty()) {
        QPen linePen(QColor("#00d4ff"), 1.5f);
        p.setPen(linePen);
        for (int i = 0; i < m_current.size(); ++i) {
            QPointF sp = toScreen((float)m_current[i].x(), (float)m_current[i].y());
            if (i == 0) p.drawPoint(sp);
            else {
                QPointF prev = toScreen((float)m_current[i-1].x(), (float)m_current[i-1].y());
                p.drawLine(prev, sp);
            }
        }

        // Profil noktaları
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#00ffcc"));
        for (const auto& pt : m_current) {
            QPointF sp = toScreen((float)pt.x(), (float)pt.y());
            p.drawEllipse(sp, 1.5, 1.5);
        }
    }

    // --- Başlık ---
    p.setPen(QColor("#444"));
    QFont hFont; hFont.setPointSize(7); hFont.setBold(true); p.setFont(hFont);
    p.drawText(QRectF(m_plotArea.left(), 2, m_plotArea.width(), 10),
               Qt::AlignCenter,
               QString("2D Profil   θ=%.1f°   %2 nokta")
                   .arg((double)m_currentTheta, 0, 'f', 1)
                   .arg(m_current.size()));
}
