#pragma once
#include <QWidget>
#include <QVector>
#include <QPointF>
#include <QPainter>
#include <QMutex>
#include <deque>

// 2D Profil Görselleştirici
// Her tarama açısında gelen (height, distance) profilini canlı olarak gösterir.
// Sol eksen = Distance (mm), Alt eksen = Height (mm)
class ProfileWidget : public QWidget {
    Q_OBJECT
public:
    explicit ProfileWidget(QWidget* parent = nullptr);

public slots:
    void updateProfile(float theta_deg, const QVector<QPointF>& profile);
    void clear();

protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;

private:
    struct ProfileEntry {
        float theta;
        QVector<QPointF> data;
    };

    static constexpr int MAX_HISTORY = 10;
    std::deque<ProfileEntry> m_history;
    QVector<QPointF> m_current;
    float m_currentTheta = 0.0f;

    // Eksen sınırları — gerçek veriye göre otomatik güncellenir
    float m_minHeight =  1e9f;
    float m_maxHeight = -1e9f;
    float m_minDist   =  1e9f;
    float m_maxDist   = -1e9f;

    QMutex m_mutex;

    // Yardımcı coord dönüşümleri
    QRect m_plotArea;
    QPointF toScreen(float height, float dist) const;
    void computePlotArea(int w, int h);
};
