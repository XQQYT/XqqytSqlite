#include "theme_model.h"

namespace remote_sqlite_qt {

ThemeModel& ThemeModel::instance() {
    static ThemeModel model;
    return model;
}

ThemeModel::ThemeModel(QObject* parent) : QObject(parent) {}

void ThemeModel::toggleTheme() {
    dark_ = !dark_;
    emit themeChanged();
}

void ThemeModel::setDarkMode(bool dark) {
    if (dark_ != dark) {
        dark_ = dark;
        emit themeChanged();
    }
}

}  // namespace remote_sqlite_qt
