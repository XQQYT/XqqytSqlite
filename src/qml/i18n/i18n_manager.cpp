#include "i18n_manager.h"

#include <QCoreApplication>
#include <QLocale>

namespace remote_sqlite_qt {

I18nManager& I18nManager::instance() {
    static I18nManager mgr;
    return mgr;
}

void I18nManager::switchToLanguage(const QString& langCode) {
    if (currentLang_ == langCode) return;

    // 移除旧翻译
    if (translator_) {
        QCoreApplication::removeTranslator(translator_.get());
    }
    if (qtTranslator_) {
        QCoreApplication::removeTranslator(qtTranslator_.get());
    }

    translator_ = std::make_unique<QTranslator>();
    qtTranslator_ = std::make_unique<QTranslator>();

    // 加载应用翻译
    QString appQm = QString(":/i18n/remote_sqlite_qt_%1.qm").arg(langCode);
    if (translator_->load(appQm)) {
        QCoreApplication::installTranslator(translator_.get());
    }

    // 加载 Qt 内置翻译
    QString qtQm = QString("qt_%1").arg(langCode);
    if (qtTranslator_->load(qtQm, QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        QCoreApplication::installTranslator(qtTranslator_.get());
    }

    currentLang_ = langCode;
    emit languageChanged();
}

QStringList I18nManager::availableLanguages() const {
    return {"zh", "en"};
}

}  // namespace remote_sqlite_qt
