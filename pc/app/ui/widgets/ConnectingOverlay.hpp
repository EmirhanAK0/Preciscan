#pragma once
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QPainter>
#include <QPainterPath>

// Ekranin ortasinda cikan semi-transparent baglanti overlay'i.
// Kullanim:
//   ConnectingOverlay* ov = new ConnectingOverlay(mainWindow);
//   ov->showConnecting();           // "Lazere baglaniyor..."
//   ov->showSuccess();              // "Baglandi!" -> 1.2s sonra kapanir
//   ov->showError("Hata mesaji");   // "Hata!" -> 2s sonra kapanir
class ConnectingOverlay : public QWidget {
    Q_OBJECT
public:
    explicit ConnectingOverlay(QWidget* parent);

    void showConnecting();
    void showSuccess();
    void showError(const QString& msg);

protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;

private:
    void reposition();
    void startSpinner();
    void stopSpinner();

    QLabel*  m_iconLabel;
    QLabel*  m_textLabel;
    QTimer*  m_spinTimer;
    QTimer*  m_closeTimer;
    int      m_spinAngle   = 0;
    bool     m_spinning    = false;

    enum class Phase { Connecting, Success, Error };
    Phase    m_phase = Phase::Connecting;
};
