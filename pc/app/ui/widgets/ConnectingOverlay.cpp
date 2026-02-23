#include "ConnectingOverlay.hpp"
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>

ConnectingOverlay::ConnectingOverlay(QWidget* parent)
    : QWidget(parent, Qt::Widget)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setFixedSize(280, 120);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(10);
    layout->setAlignment(Qt::AlignCenter);

    m_iconLabel = new QLabel("", this);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setStyleSheet("font-size: 28px;");
    layout->addWidget(m_iconLabel);

    m_textLabel = new QLabel("", this);
    m_textLabel->setAlignment(Qt::AlignCenter);
    m_textLabel->setStyleSheet("color: #eee; font-size: 13px; font-weight: bold;");
    m_textLabel->setWordWrap(true);
    layout->addWidget(m_textLabel);

    // Spinner timer (60fps)
    m_spinTimer = new QTimer(this);
    m_spinTimer->setInterval(16);
    connect(m_spinTimer, &QTimer::timeout, this, [this]{
        m_spinAngle = (m_spinAngle + 8) % 360;
        m_iconLabel->repaint();
        repaint();
    });

    // Auto-close timer
    m_closeTimer = new QTimer(this);
    m_closeTimer->setSingleShot(true);
    connect(m_closeTimer, &QTimer::timeout, this, [this]{
        hide();
        stopSpinner();
    });

    hide();
    reposition();
}

void ConnectingOverlay::showConnecting() {
    m_phase = Phase::Connecting;
    m_spinning = true;
    m_spinAngle = 0;
    m_textLabel->setText("Lazere baglaniyor...");
    m_iconLabel->setText(""); // boş — spinner paintEvent'te cizilecek
    reposition();
    show();
    raise();
    startSpinner();
}

void ConnectingOverlay::showSuccess() {
    stopSpinner();
    m_phase = Phase::Success;
    m_iconLabel->setText("✓");
    m_iconLabel->setStyleSheet("font-size: 28px; color: #2ecc71;");
    m_textLabel->setText("Baglanti basarili!");
    m_textLabel->setStyleSheet("color: #2ecc71; font-size: 13px; font-weight: bold;");
    repaint();
    m_closeTimer->start(1200);
}

void ConnectingOverlay::showError(const QString& msg) {
    stopSpinner();
    m_phase = Phase::Error;
    m_iconLabel->setText("✗");
    m_iconLabel->setStyleSheet("font-size: 28px; color: #e74c3c;");
    m_textLabel->setText(msg.isEmpty() ? "Baglanti basarisiz!" : msg);
    m_textLabel->setStyleSheet("color: #e74c3c; font-size: 12px; font-weight: bold;");
    repaint();
    m_closeTimer->start(2500);
}

void ConnectingOverlay::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Arka plan: koyu yuvarlatilmis kart
    QPainterPath path;
    path.addRoundedRect(rect(), 14, 14);
    p.fillPath(path, QColor(20, 20, 22, 235));

    // Karti cercevele
    p.setPen(QPen(QColor(60, 60, 70), 1));
    p.drawPath(path);

    // Spinner ciz (sadece Connecting fazinda)
    if (m_phase == Phase::Connecting) {
        const int cx = width() / 2;
        const int cy = 34;
        const int r  = 16;
        QRectF arcRect(cx - r, cy - r, r * 2, r * 2);

        // Arka arc (koyu)
        p.setPen(QPen(QColor(50, 50, 55), 3, Qt::SolidLine, Qt::RoundCap));
        p.drawArc(arcRect, 0, 360 * 16);

        // Donen arc (cyan)
        p.setPen(QPen(QColor("#00d4ff"), 3, Qt::SolidLine, Qt::RoundCap));
        p.drawArc(arcRect, m_spinAngle * 16, 100 * 16);
    }
}

void ConnectingOverlay::resizeEvent(QResizeEvent*) {
    reposition();
}

void ConnectingOverlay::reposition() {
    if (parentWidget()) {
        QRect pr = parentWidget()->rect();
        int x = (pr.width()  - width())  / 2;
        int y = (pr.height() - height()) / 2;
        move(x, y);
    }
}

void ConnectingOverlay::startSpinner() {
    m_spinning = true;
    m_spinTimer->start();
}

void ConnectingOverlay::stopSpinner() {
    m_spinning = false;
    m_spinTimer->stop();
}
