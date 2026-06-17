#pragma once
//
// Preciscan UI tema — TEK kaynak (single source of truth).
//
// Onceden her widget kendi setStyleSheet'ini tasiyordu (71 cagri, 76 farkli
// renk). Burada renk/olcu token'lari bir kez tanimlanir ve uygulama genelinde
// gecerli tek bir QSS uretilir. Widget'lar artik cogunlukla "ciplak" olusturulur
// ve bu temayi miras alir. Ozel durumlar (buton katmanlari, baglanti durumu)
// dinamik property + QSS selektoru ile yonetilir:
//
//   button->setProperty("tier", "primary");   // primary|secondary|destructive|ghost
//   theme::repolish(button);                   // property degisince yeniden boya
//
#include <QString>
#include <QWidget>
#include <QStyle>

namespace theme {

// ─── Renk token'lari ─────────────────────────────────────────────
inline constexpr const char* BG_DEEP       = "#0d0d0d"; // log, liste
inline constexpr const char* BG            = "#141414"; // panel / sekme yuzeyi
inline constexpr const char* SURFACE       = "#1a1a1a"; // giris alanlari
inline constexpr const char* SURFACE_RAISED= "#1e1e1e"; // ikincil buton
inline constexpr const char* SURFACE_HOVER = "#262626";
inline constexpr const char* BORDER        = "#2a2a2a";
inline constexpr const char* BORDER_STRONG = "#3a3a3a";
inline constexpr const char* ACCENT        = "#2ecc71"; // birincil / olumlu
inline constexpr const char* ACCENT_HOVER  = "#3ee084";
inline constexpr const char* ACCENT_TEXT   = "#06210f"; // accent zemin uzerine
inline constexpr const char* ACCENT_BG     = "#0d2a0d"; // bagli durum zemini
inline constexpr const char* DANGER        = "#e74c3c";
inline constexpr const char* DANGER_DIM    = "#6e2a26";
inline constexpr const char* DANGER_BG     = "#2a1514";
inline constexpr const char* WARNING       = "#f1c40f";
inline constexpr const char* INFO          = "#4aa3ff";
inline constexpr const char* TEXT          = "#e6e6e6";
inline constexpr const char* TEXT_MUTED    = "#aaaaaa";
inline constexpr const char* TEXT_DIM      = "#777777";
inline constexpr const char* TEXT_FAINT    = "#555555";
inline constexpr const char* TEXT_DISABLED = "#4a4a4a";
inline constexpr const char* LOG_TEXT      = "#00cc66";

// Property/objectName degistikten sonra widget'i yeniden boyar.
inline void repolish(QWidget* w)
{
    if (!w) return;
    w->style()->unpolish(w);
    w->style()->polish(w);
    w->update();
}

// ─── Uygulama geneli QSS ─────────────────────────────────────────
inline QString globalStyleSheet()
{
    QString s = QStringLiteral(R"QSS(
QMainWindow, QDialog { background: @bg; color: @text; }

/* Sekmeler */
QTabWidget::pane { border: none; background: @bg; }
QTabBar::tab { background: @surface; color: @textDim; padding: 7px 16px;
               font-size: 12px; font-weight: 500; border: none; }
QTabBar::tab:selected { background: @bg; color: @text; border-bottom: 2px solid @accent; }
QTabBar::tab:hover { color: @textMuted; }

/* Etiketler */
QLabel { color: @text; font-size: 12px; background: transparent; }
QLabel#sectionLabel { color: @textDim; font-size: 11px; font-weight: 500; }

/* Grup kutulari */
QGroupBox { color: @textMuted; font-size: 13px; font-weight: 500;
            border: 1px solid @border; border-radius: 6px;
            margin-top: 10px; padding-top: 14px; }
QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; }

/* Butonlar — varsayilan = ikincil (secondary) */
QPushButton { background: @surfaceRaised; color: @text; border: 1px solid @borderStrong;
              border-radius: 5px; padding: 7px 12px; font-size: 12px; min-height: 18px; }
QPushButton:hover { background: @surfaceHover; border-color: #4a4a4a; }
QPushButton:pressed { background: #2c2c2c; }
QPushButton:disabled { background: @surface; color: @textDisabled; border-color: @border; }

QPushButton[tier="primary"] { background: @accent; color: @accentText; border: none; font-weight: 500; }
QPushButton[tier="primary"]:hover { background: @accentHover; }
QPushButton[tier="primary"]:disabled { background: @surface; color: @textDisabled; }

QPushButton[tier="destructive"] { background: transparent; color: @danger; border: 1px solid @dangerDim; }
QPushButton[tier="destructive"]:hover { background: @dangerBg; }
QPushButton[tier="destructive"]:disabled { color: @textDisabled; border-color: @border; }

QPushButton[tier="ghost"] { background: transparent; color: @textDim; border: none; }
QPushButton[tier="ghost"]:hover { color: @text; background: @surface; }

/* Baglanti butonlari (toolbar) — durum property'sine gore */
QPushButton#connectBtn { background: @surfaceRaised; color: @textMuted;
                         border: 1px solid @borderStrong; font-weight: 500; min-width: 96px; }
QPushButton#connectBtn:hover { background: @surfaceHover; color: @text; }
QPushButton#connectBtn[state="on"] { background: @accentBg; color: @accent; border: 1px solid @accent; }

/* Giris alanlari */
QComboBox, QSpinBox, QDoubleSpinBox, QLineEdit {
    background: @surface; color: @text; border: 1px solid @border;
    border-radius: 4px; padding: 4px 6px; font-size: 12px; min-height: 20px; }
QComboBox:hover, QSpinBox:hover, QDoubleSpinBox:hover, QLineEdit:hover { border-color: @borderStrong; }
QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus, QLineEdit:focus { border: 1px solid @accent; }
QComboBox::drop-down { border: none; width: 18px; }
QComboBox QAbstractItemView { background: @surface; color: @text; border: 1px solid @borderStrong;
    selection-background-color: @accent; selection-color: @accentText; outline: none; }

/* Liste + log */
QListWidget { background: @bgDeep; color: @text; border: 1px solid @border;
              border-radius: 4px; font-size: 11px; outline: none; }
QListWidget::item { border: none; padding: 0; }
QTextEdit { background: @bgDeep; color: @logText; border: 1px solid @border;
            border-radius: 4px; font-family: Consolas, monospace; font-size: 11px; }

/* Onay kutusu */
QCheckBox { color: @textMuted; font-size: 12px; spacing: 6px; background: transparent; }

/* Kaydirici */
QSlider::groove:horizontal { height: 4px; background: @border; border-radius: 2px; }
QSlider::handle:horizontal { width: 14px; margin: -6px 0; border-radius: 7px; background: @accent; }
QSlider::sub-page:horizontal { background: @accent; border-radius: 2px; }

/* Arac cubugu + durum cubugu */
QToolBar { background: #111111; border: none; border-bottom: 1px solid @border;
           spacing: 6px; padding: 5px 8px; }
QStatusBar { background: @bgDeep; color: @textFaint; font-size: 11px; border-top: 1px solid @border; }
QStatusBar::item { border: none; }

/* Ilerleme + ipucu + kaydirma cubugu */
QProgressBar { background: @surface; border: 1px solid @border; border-radius: 4px;
               text-align: center; color: @text; }
QProgressBar::chunk { background: @accent; border-radius: 3px; }
QToolTip { background: #222222; color: @text; border: 1px solid @borderStrong; padding: 4px; }
QScrollBar:vertical { background: @bg; width: 10px; margin: 0; }
QScrollBar::handle:vertical { background: @borderStrong; border-radius: 5px; min-height: 24px; }
QScrollBar::handle:vertical:hover { background: #4a4a4a; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }
)QSS");

    s.replace("@bgDeep",        BG_DEEP);
    s.replace("@bg",            BG);
    s.replace("@surfaceRaised", SURFACE_RAISED);
    s.replace("@surfaceHover",  SURFACE_HOVER);
    s.replace("@surface",       SURFACE);
    s.replace("@borderStrong",  BORDER_STRONG);
    s.replace("@border",        BORDER);
    s.replace("@accentHover",   ACCENT_HOVER);
    s.replace("@accentText",    ACCENT_TEXT);
    s.replace("@accentBg",      ACCENT_BG);
    s.replace("@accent",        ACCENT);
    s.replace("@dangerDim",     DANGER_DIM);
    s.replace("@dangerBg",      DANGER_BG);
    s.replace("@danger",        DANGER);
    s.replace("@warning",       WARNING);
    s.replace("@info",          INFO);
    s.replace("@textMuted",     TEXT_MUTED);
    s.replace("@textDim",       TEXT_DIM);
    s.replace("@textFaint",     TEXT_FAINT);
    s.replace("@textDisabled",  TEXT_DISABLED);
    s.replace("@text",          TEXT);
    s.replace("@logText",       LOG_TEXT);
    return s;
}

} // namespace theme
