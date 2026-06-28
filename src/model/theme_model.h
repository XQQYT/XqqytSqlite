#pragma once

#include <QObject>
#include <QColor>

namespace remote_sqlite_qt {

// 主题模型单例——将拟态化设计规范的颜色/阴影暴露为 QML 属性
class ThemeModel : public QObject {
    Q_OBJECT

    // 颜色属性
    Q_PROPERTY(QColor bg READ bg NOTIFY themeChanged)
    Q_PROPERTY(QColor sidebar READ sidebar NOTIFY themeChanged)
    Q_PROPERTY(QColor panel READ panel NOTIFY themeChanged)
    Q_PROPERTY(QColor border READ border NOTIFY themeChanged)
    Q_PROPERTY(QColor accent READ accent NOTIFY themeChanged)
    Q_PROPERTY(QColor accent2 READ accent2 NOTIFY themeChanged)
    Q_PROPERTY(QColor text READ text NOTIFY themeChanged)
    Q_PROPERTY(QColor textMuted READ textMuted NOTIFY themeChanged)
    Q_PROPERTY(QColor textDim READ textDim NOTIFY themeChanged)
    Q_PROPERTY(QColor success READ success NOTIFY themeChanged)
    Q_PROPERTY(QColor error READ error NOTIFY themeChanged)
    Q_PROPERTY(QColor warning READ warning NOTIFY themeChanged)
    Q_PROPERTY(QColor info READ info NOTIFY themeChanged)
    Q_PROPERTY(QColor hover READ hover NOTIFY themeChanged)
    Q_PROPERTY(QColor selected READ selected NOTIFY themeChanged)

    // 阴影
    Q_PROPERTY(QString raisedShadow READ raisedShadow NOTIFY themeChanged)
    Q_PROPERTY(QString insetShadow READ insetShadow NOTIFY themeChanged)

    // 圆角
    Q_PROPERTY(int cardRadius READ cardRadius CONSTANT)
    Q_PROPERTY(int buttonRadius READ buttonRadius CONSTANT)

    // 字号
    Q_PROPERTY(int fontSize READ fontSize CONSTANT)

public:
    static ThemeModel& instance();

    explicit ThemeModel(QObject* parent = nullptr);

    bool isDark() const { return dark_; }

public slots:
    void toggleTheme();
    void setDarkMode(bool dark);

signals:
    void themeChanged();

private:
    // Dark theme colors (Catppuccin Mocha inspired)
    QColor bg_{"#1e1e2e"};
    QColor sidebar_{"#181825"};
    QColor panel_{"#24243e"};
    QColor border_{"#313145"};
    QColor accent_{"#7c6af7"};
    QColor accent2_{"#56b6c2"};
    QColor text_{"#cdd6f4"};
    QColor textMuted_{"#6c7086"};
    QColor textDim_{"#a6adc8"};
    QColor success_{"#a6e3a1"};
    QColor error_{"#f38ba8"};
    QColor warning_{"#f9e2af"};
    QColor info_{"#89b4fa"};
    QColor hover_{"#313150"};
    QColor selected_{"#3d3d62"};

    // Light theme colors
    QColor lightBg_{"#eff1f5"};
    QColor lightSidebar_{"#e6e9ef"};
    QColor lightPanel_{"#ffffff"};
    QColor lightBorder_{"#ccd0da"};
    QColor lightAccent_{"#8839ef"};
    QColor lightAccent2_{"#179299"};
    QColor lightText_{"#4c4f69"};
    QColor lightTextMuted_{"#9ca0b0"};
    QColor lightTextDim_{"#5c5f77"};
    QColor lightSuccess_{"#40a02b"};
    QColor lightError_{"#d20f39"};
    QColor lightWarning_{"#df8e1d"};
    QColor lightInfo_{"#1e66f5"};
    QColor lightHover_{"#e6e9ef"};
    QColor lightSelected_{"#dce0e8"};

    bool dark_{true};

    // Getters
    QColor bg() const { return dark_ ? bg_ : lightBg_; }
    QColor sidebar() const { return dark_ ? sidebar_ : lightSidebar_; }
    QColor panel() const { return dark_ ? panel_ : lightPanel_; }
    QColor border() const { return dark_ ? border_ : lightBorder_; }
    QColor accent() const { return dark_ ? accent_ : lightAccent_; }
    QColor accent2() const { return dark_ ? accent2_ : lightAccent2_; }
    QColor text() const { return dark_ ? text_ : lightText_; }
    QColor textMuted() const { return dark_ ? textMuted_ : lightTextMuted_; }
    QColor textDim() const { return dark_ ? textDim_ : lightTextDim_; }
    QColor success() const { return dark_ ? success_ : lightSuccess_; }
    QColor error() const { return dark_ ? error_ : lightError_; }
    QColor warning() const { return dark_ ? warning_ : lightWarning_; }
    QColor info() const { return dark_ ? info_ : lightInfo_; }
    QColor hover() const { return dark_ ? hover_ : lightHover_; }
    QColor selected() const { return dark_ ? selected_ : lightSelected_; }

    QString raisedShadow() const {
        return dark_ ? QStringLiteral("4px 4px 8px rgba(0,0,0,0.25), -4px -4px 8px rgba(255,255,255,0.03)")
                     : QStringLiteral("4px 4px 8px rgba(0,0,0,0.08), -4px -4px 8px rgba(255,255,255,0.8)");
    }
    QString insetShadow() const {
        return dark_ ? QStringLiteral("inset 3px 3px 6px rgba(0,0,0,0.25), inset -3px -3px 6px rgba(255,255,255,0.03)")
                     : QStringLiteral("inset 3px 3px 6px rgba(0,0,0,0.08), inset -3px -3px 6px rgba(255,255,255,0.8)");
    }
    int cardRadius() const { return 16; }
    int buttonRadius() const { return 12; }
    int fontSize() const { return 13; }
};

}  // namespace remote_sqlite_qt
